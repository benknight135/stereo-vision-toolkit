﻿/*
* Copyright I3D Robotics Ltd, 2017
* Author: Josh Veitch-Michaelis
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  ui->imageViewTab->raise();
  ui->tabWidget->lower();

  ui->tabWidget->setCurrentIndex(0);

  /* Calibration */
  connect(ui->actionCalibration_wizard, SIGNAL(triggered(bool)), this,
          SLOT(startCalibration()));
  connect(ui->actionCalibrate_from_images, SIGNAL(triggered(bool)), this,
          SLOT(startCalibrationFromImages()));
  connect(ui->actionAutoload_Camera, SIGNAL(triggered(bool)), this,
          SLOT(autoloadCameraTriggered()));
  connect(ui->actionExit, SIGNAL(triggered(bool)), QApplication::instance(),
          SLOT(quit()));

  awesome = new QtAwesome(qApp);
  awesome->initFontAwesome();

  parameters = new ParamFile();

  save_directory = parameters->get_string("saveDir");
  calibration_directory = parameters->get_string("calDir");

  left_view = new CameraDisplayWidget(this);
  right_view = new CameraDisplayWidget(this);

  ui->stereoViewLayout->addWidget(left_view);
  ui->stereoViewLayout->addWidget(right_view);

  controlsInit();
  statusBarInit();
  int stereo_camera_exit_code = stereoCameraLoad();
  if (stereo_camera_exit_code < 0){
      //Display quit messagebox as program does not function correctly if no camera is connected
      QMessageBox::warning(this,"Stereo Vision Toolkit","Couldn't find any camera connected. Program will now exit. Check a Deimos or Phobos camera is connected and restart the program.");
      exit(EXIT_FAILURE);
  }
  pointCloudInit();
}

void MainWindow::pointCloudSaveStatus(QString msg){
    qDebug() << msg;
    QMessageBox::warning(this,"Stereo Vision Toolkit",msg);
}

void MainWindow::statusBarInit(void) {
  QWidget* status_widget = new QWidget;
  ui->statusBar->addPermanentWidget(status_widget);

  fps_counter = new QLabel(this);
  temp_label = new QLabel(this);
  frame_counter = new QLabel(this);
  status_bar_spacer = new QSpacerItem(20, 1);

  QHBoxLayout* _hlayout = new QHBoxLayout();
  _hlayout->addWidget(frame_counter);
  _hlayout->addSpacerItem(status_bar_spacer);
  _hlayout->addWidget(fps_counter);
  _hlayout->addWidget(temp_label);

  status_widget->setLayout(_hlayout);

  status_bar_timer = new QTimer;

  connect(status_bar_timer, SIGNAL(timeout()), this,
          SLOT(statusMessageTimeout()));

  ui->saveDirLabel->setText(save_directory);
}

void MainWindow::controlsInit(void) {

    ui->pauseButton->setDisabled(true);
    ui->singleShotButton->setDisabled(true);
    ui->saveButton->setDisabled(true);
    ui->toggleVideoButton->setDisabled(true);

  connect(ui->pauseButton, SIGNAL(clicked()), this, SLOT(toggleAcquire()));
  connect(ui->singleShotButton, SIGNAL(clicked()), this,
          SLOT(singleShotClicked()));
  connect(ui->setSaveDirButton, SIGNAL(clicked()), this,
          SLOT(setSaveDirectory()));
  connect(ui->toggleVideoButton, SIGNAL(clicked()), this,
          SLOT(startVideoCapture(void)));
  connect(ui->actionLoad_calibration, SIGNAL(triggered(bool)),this, SLOT(setCalibrationFolder()));
  connect(ui->autoExposeCheck, SIGNAL(clicked(bool)), ui->exposureSpinBox, SLOT(setDisabled(bool)));

  QVariantMap icon_options;
  icon_options.insert("color", QColor(255, 255, 255));
  ui->pauseButton->setIcon(awesome->icon(fa::pause, icon_options));
  ui->saveButton->setIcon(awesome->icon(fa::save, icon_options));
  ui->singleShotButton->setIcon(awesome->icon(fa::camera, icon_options));
  ui->enableStereo->setIcon(awesome->icon(fa::cubes, icon_options));
  ui->toggleVideoButton->setIcon(awesome->icon(fa::videocamera, icon_options));

  connect(ui->actionLoad_Video, SIGNAL(triggered(bool)), this,
          SLOT(videoStreamLoad()));

  connect(ui->tabWidget, SIGNAL(currentChanged(int)), this,
          SLOT(enable3DViz(int)));

  disparity_view = new DisparityViewer();
  disparity_view->setViewer(ui->disparityViewLabel);
  connect(disparity_view, SIGNAL(newDisparity(QPixmap)), ui->disparityViewLabel,
          SLOT(setPixmap(QPixmap)));
  ui->disparityViewSettingsLayout->addWidget(disparity_view);

  connect(ui->reset3DViewButton, SIGNAL(clicked(bool)), this, SLOT(resetPointCloudView()));
}

void MainWindow::enable3DViz(int tab) {
  if (!stereo_cam) return;

  if (tab == 2) {
    stereo_cam->enableReproject(true);
  } else {
    stereo_cam->enableReproject(false);
  }
}

void MainWindow::pointCloudInit() {
  viewer.reset(new pcl::visualization::PCLVisualizer("viewer", false));
  vtk_widget = new QVTKWidget();

  ui->visualiserTab->layout()->addWidget(vtk_widget);
  vtk_widget->setSizePolicy(QSizePolicy::MinimumExpanding,
                            QSizePolicy::MinimumExpanding);

  vtk_widget->SetRenderWindow(viewer->getRenderWindow());
  viewer->setupInteractor(vtk_widget->GetInteractor(),
                          vtk_widget->GetRenderWindow());
  vtk_widget->update();

  resetPointCloudView();

  vtk_widget->update();
}

void MainWindow::resetPointCloudView(){
    viewer->resetCamera();
    viewer->setCameraPosition(0, 0, 0.02, 0, 1, 0);

    ui->minZSpinBox->setValue(0.2);
    ui->maxZSpinBox->setValue(2);

    vtk_widget->update();
}

void MainWindow::stereoCameraInitConnections(void) {

  ui->exposureSpinBox->setEnabled(true);
  ui->autoExposeCheck->setEnabled(true);
  ui->enableHDRCheckbox->setEnabled(true);
  ui->matcherSelectBox->setEnabled(true);

  ui->pauseButton->setEnabled(true);
  ui->singleShotButton->setEnabled(true);
  ui->saveButton->setEnabled(true);
  ui->toggleVideoButton->setEnabled(true);
  ui->actionCalibration_wizard->setEnabled(false);

  connect(ui->exposureSpinBox, SIGNAL(valueChanged(double)), stereo_cam,
          SLOT(setExposure(double)));
  connect(stereo_cam, SIGNAL(stereopair_processed()), this, SLOT(updateDisplay()));
  connect(stereo_cam, SIGNAL(update_size(int, int, int)), left_view, SLOT(setSize(int, int, int)));
  connect(stereo_cam, SIGNAL(update_size(int, int, int)), right_view, SLOT(setSize(int, int, int)));

  connect(stereo_cam, SIGNAL(fps(qint64)), this, SLOT(updateFPS(qint64)));
  connect(stereo_cam, SIGNAL(framecount(qint64)), this,
          SLOT(updateFrameCount(qint64)));
  connect(stereo_cam, SIGNAL(temperature_C(double)), this, SLOT(updateTemperature(double)));
  connect(ui->saveButton, SIGNAL(clicked()), stereo_cam, SLOT(saveImageTimestamped()));
  connect(stereo_cam, SIGNAL(savedImage(QString)), this,
          SLOT(displaySaved(QString)));
  connect(ui->enableStereo, SIGNAL(clicked(bool)), stereo_cam,
          SLOT(enableMatching(bool)));
  connect(ui->toggleRectifyCheckBox, SIGNAL(clicked(bool)), stereo_cam,
          SLOT(enableRectify(bool)));
  connect(stereo_cam, SIGNAL(matched()), disparity_view,
          SLOT(updateDisparityAsync(void)));
  connect(ui->autoExposeCheck, SIGNAL(clicked(bool)), stereo_cam, SLOT(enableAutoExpose(bool)));
  connect(ui->enableHDRCheckbox, SIGNAL(clicked(bool)), stereo_cam, SLOT(toggleHDR(bool)));

  /* Point cloud */
  connect(stereo_cam, SIGNAL(reprojected()), this, SLOT(updateCloud()));
  connect(ui->minZSpinBox, SIGNAL(valueChanged(double)), stereo_cam,
          SLOT(setVisualZmin(double)));
  connect(ui->maxZSpinBox, SIGNAL(valueChanged(double)), stereo_cam,
          SLOT(setVisualZmax(double)));
  connect(ui->savePointCloudButton, SIGNAL(clicked()), stereo_cam, SLOT(savePointCloud()));
  connect(ui->dateInFilenameCheckbox, SIGNAL(stateChanged(int)), stereo_cam, SLOT(toggleDateInFilename(int)));
  connect(stereo_cam, SIGNAL(pointCloudSaveStatus(QString)),this,SLOT(pointCloudSaveStatus(QString)));
}

void MainWindow::stereoCameraRelease(void) {

  ui->exposureSpinBox->setDisabled(true);
  ui->autoExposeCheck->setDisabled(true);
  ui->enableHDRCheckbox->setDisabled(true);
  ui->matcherSelectBox->setDisabled(true);

  ui->pauseButton->setDisabled(true);
  ui->singleShotButton->setDisabled(true);
  ui->saveButton->setDisabled(true);
  ui->toggleVideoButton->setDisabled(true);
  ui->actionCalibration_wizard->setDisabled(true);

  if (cameras_connected) {
    disconnect(ui->exposureSpinBox, SIGNAL(valueChanged(double)), stereo_cam,
               SLOT(setExposure(double)));
    disconnect(stereo_cam, SIGNAL(acquired()), this, SLOT(updateDisplay()));
    disconnect(stereo_cam, SIGNAL(matched()), disparity_view,
               SLOT(updateDisparityAsync(void)));
    disconnect(stereo_cam, SIGNAL(temperature_C(double)), this, SLOT(updateTemperature(double)));
    disconnect(stereo_cam, SIGNAL(fps(qint64)), this, SLOT(updateFPS(qint64)));
    disconnect(stereo_cam, SIGNAL(framecount(qint64)), this,
               SLOT(updateFrameCount(qint64)));
    disconnect(ui->saveButton, SIGNAL(clicked()), stereo_cam,
               SLOT(saveImageTimestamped()));
    disconnect(stereo_cam, SIGNAL(savedImage(QString)), this,
               SLOT(displaySaved(QString)));
    disconnect(ui->enableStereo, SIGNAL(clicked(bool)), stereo_cam,
               SLOT(enableMatching(bool)));
    disconnect(ui->toggleRectifyCheckBox, SIGNAL(clicked(bool)), stereo_cam,
               SLOT(enableRectify(bool)));
    disconnect(ui->autoExposeCheck, SIGNAL(clicked(bool)), stereo_cam, SLOT(enableAutoExpose(bool)));
    disconnect(ui->enableHDRCheckbox, SIGNAL(clicked(bool)), stereo_cam, SLOT(toggleHDR(bool)));

    /* Point cloud */
    disconnect(stereo_cam, SIGNAL(reprojected()), this, SLOT(updateCloud()));
    disconnect(ui->minZSpinBox, SIGNAL(valueChanged(double)), stereo_cam,
               SLOT(setVisualZmin(double)));
    disconnect(ui->maxZSpinBox, SIGNAL(valueChanged(double)), stereo_cam,
               SLOT(setVisualZmax(double)));
    disconnect(ui->savePointCloudButton, SIGNAL(clicked()), stereo_cam, SLOT(savePointCloud()));
    disconnect(ui->dateInFilenameCheckbox, SIGNAL(stateChanged(int)), stereo_cam, SLOT(toggleDateInFilename(int)));
    disconnect(stereo_cam, SIGNAL(pointCloudSaveStatus(QString)),this,SLOT(pointCloudSaveStatus(QString)));

    stereo_cam->pause();
    stereo_cam->enableRectify(false);
    stereo_cam->enableMatching(false);

    qDebug() << "Waiting for acquisition to finish";
    while (stereo_cam->isAcquiring() || stereo_cam->isCapturing());
    qDebug() << "Acquisition finished";

    delete stereo_cam;
  }
}

int MainWindow::stereoCameraLoad(void) {
     cameras_connected = false;
     int exit_code = -1;

     StereoCameraPhobos* stereo_cam_phobos = new StereoCameraPhobos;
     QThread* phobos_thread = new QThread;
     stereo_cam_phobos->assignThread(phobos_thread);

     StereoCameraDeimos* stereo_cam_deimos = new StereoCameraDeimos;
     QThread* deimos_thread = new QThread;
     stereo_cam_deimos->assignThread(deimos_thread);

     if(stereo_cam_phobos->find_systems()){
         stereo_cam = (AbstractStereoCamera*) stereo_cam_phobos;
         left_view->setSettingsCallback(stereo_cam, SLOT(loadLeftSettings()));
         right_view->setSettingsCallback(stereo_cam, SLOT(loadRightSettings()));
         qDebug() << "Connecting to Phobos system";
         cameras_connected = true;
         exit_code = 0;
     }else if (stereo_cam_deimos->find_systems()) {
         stereo_cam = (AbstractStereoCamera*) stereo_cam_deimos;
         qDebug() << "Connecting to Deimos system";
         cameras_connected = true;
         exit_code = 0;
     }else {
        ui->statusBar->showMessage("Couldn't find any cameras.");
        qDebug() << "Couldn't find any cameras";

        delete stereo_cam_deimos;
        delete stereo_cam_phobos;
        exit_code = -1;
     }

     if (cameras_connected) {
       stereoCameraInit();
     }
     return (exit_code);
}

void MainWindow::stereoCameraInit() {
  if (cameras_connected) {
    save_directory = parameters->get_string("saveDir");
    calibration_directory = parameters->get_string("calDir");

    stereoCameraInitConnections();
    setupMatchers();
    setMatcher(0);

    if (save_directory != "") {
      stereo_cam->setSavelocation(save_directory);
    }

    if (calibration_directory != ""){
        setCalibrationFolder(calibration_directory);
    }

    // TODO: Get this from calibration file
    disparity_view->setCalibration(stereo_cam->Q, 60e-3, 4.3e-3);
    disparity_view->updatePixmapRange();

    left_view->setSize(stereo_cam->getWidth(), stereo_cam->getHeight(), 1);
    right_view->setSize(stereo_cam->getWidth(), stereo_cam->getHeight(), 1);

    QTimer::singleShot(1, stereo_cam, SLOT(freerun()));
    ui->statusBar->showMessage("Freerunning.");
  }
}

void MainWindow::autoloadCameraTriggered() {
  stereoCameraRelease();
  stereoCameraLoad();
}

void MainWindow::videoStreamLoad(void) {
  QMessageBox msg;

  QString fname = QFileDialog::getOpenFileName(
      this, tr("Open Stereo Video"), "/home", tr("Videos (*.avi *.mp4)"));
  if (fname != "") {
    stereoCameraRelease();

    StereoCameraFromVideo* stereo_cam_video = new StereoCameraFromVideo;
    QThread* cam_thread = new QThread;
    stereo_cam_video->assignThread(cam_thread);

    if (stereo_cam_video->initCamera(fname)) {
      stereo_cam = (AbstractStereoCamera*) stereo_cam_video;
      cameras_connected = true;
      ui->frameCountSlider->setEnabled(true);
      connect(stereo_cam, SIGNAL(videoPosition(int)),ui->frameCountSlider, SLOT(setValue(int)));
      connect(ui->frameCountSlider, SIGNAL(sliderMoved(int)),stereo_cam, SLOT(setPosition(int)));
    } else {
      msg.setText("Failed to open video stream.");
      msg.exec();
      ui->statusBar->showMessage("Disconnected.");
    }

    stereoCameraInit();
    ui->exposureSpinBox->setDisabled(true);
    ui->autoExposeCheck->setDisabled(true);
  }
}

void MainWindow::updateCloud() {

  auto cloud = stereo_cam->getPointCloud();

  if(!cloud.get()) return;

  if (!cloud->empty()) {
    // Initial point cloud load

    if (!viewer->updatePointCloud(cloud, "cloud")) {
      viewer->addPointCloud(cloud, "cloud");
    }

    vtk_widget->update();
  } else {
    qDebug() << "Empty point cloud";
  }
}

void MainWindow::startVideoCapture(void) {
  stereo_cam->pause();
  stereo_cam->enableMatching(false);

  connect(ui->toggleVideoButton, SIGNAL(clicked()), this,
          SLOT(stopVideoCapture(void)));
  disconnect(ui->toggleVideoButton, SIGNAL(clicked()), this,
             SLOT(startVideoCapture(void)));

  ui->toggleVideoButton->setIcon(awesome->icon(fa::stop, icon_options));
  ui->statusBar->showMessage("Video capture started.");

  ui->pauseButton->setEnabled(false);
  ui->enableStereo->setEnabled(false);
  ui->enableStereo->setChecked(false);
  ui->saveButton->setEnabled(false);
  ui->singleShotButton->setEnabled(false);

  QTimer::singleShot(1, stereo_cam, SLOT(videoStreamStart()));
}

void MainWindow::stopVideoCapture(void) {
  stereo_cam->videoStreamStop();
  ui->statusBar->showMessage("Stopped video capture.");

  connect(ui->toggleVideoButton, SIGNAL(clicked()), this,
          SLOT(startVideoCapture(void)));
  disconnect(ui->toggleVideoButton, SIGNAL(clicked()), this,
             SLOT(stopVideoCapture(void)));

  ui->enableStereo->setEnabled(true);
  ui->pauseButton->setEnabled(true);
  ui->pauseButton->setIcon(awesome->icon(fa::pause, icon_options));
  ui->saveButton->setEnabled(true);
  ui->singleShotButton->setEnabled(true);

  ui->toggleVideoButton->setIcon(awesome->icon(fa::videocamera, icon_options));

  QTimer::singleShot(1, stereo_cam, SLOT(freerun()));
}

void MainWindow::startCalibrationFromImages(void) {
  calibration_images_dialog = new CalibrateFromImagesDialog(this);
  connect(calibration_images_dialog, SIGNAL(run_calibration()), this, SLOT(runCalibrationFromImages()));
  calibration_images_dialog->move(100, 100);
  calibration_images_dialog->show();
}

void MainWindow::runCalibrationFromImages(void){

    qDebug() << "Beginning calibration from images";

    if(calibration_images_dialog == NULL) return;
    qDebug() << "Getting parameters";

    int cols = calibration_images_dialog->getPatternCols();
    int rows = calibration_images_dialog->getPatternRows();
    double square_size_mm = calibration_images_dialog->getSquareSizeMm();
    auto left_images = calibration_images_dialog->getLeftImages();
    auto right_images = calibration_images_dialog->getRightImages();
    bool save_ros = calibration_images_dialog->getSaveROS();

    calibration_images_dialog->close();

    calibrator = new StereoCalibrate(this, NULL);
    cv::Size pattern(cols, rows);

    calibrator->setOutputPath(calibration_images_dialog->getOutputPath());
    calibrator->setPattern(pattern, square_size_mm);
    calibrator->setImages(left_images, right_images);
    calibrator->setSaveROS(save_ros);
    calibrator->jointCalibration();

}

void MainWindow::startCalibration(void) {
  stereo_cam->enableMatching(false);
  stereo_cam->enableRectify(false);

  /* Connect calibration */
  calibrator = new StereoCalibrate(this, stereo_cam);
  calibrator->setPattern(cv::Size(9, 6), 25e-3);
  calibrator->setDisplays(left_view->getImageDisplay(), right_view->getImageDisplay());
  calibrator->loadBoardPoses("./calibration_template_a4.ini");

  calibration_dialog = new CalibrationDialog(this, calibrator);

  ui->tabWidget->setCurrentIndex(0);

  /* Route image capture through via the calibrator */

  disconnect(stereo_cam, SIGNAL(acquired()), this, SLOT(updateDisplay()));

  connect(calibration_dialog, SIGNAL(stopCalibration()), calibrator,
          SLOT(abortCalibration()));
  connect(calibrator, SIGNAL(doneCalibration(bool)), this,
          SLOT(doneCalibration(bool)));

  calibrator->startCalibration();

  calibration_dialog->move(100, 100);
  calibration_dialog->show();
}

void MainWindow::doneCalibration(bool) {
  connect(stereo_cam, SIGNAL(acquired()), this, SLOT(updateDisplay()));

  disconnect(calibration_dialog, SIGNAL(stopCalibration()), calibrator,
             SLOT(abortCalibration()));
  disconnect(calibrator, SIGNAL(doneCalibration(bool)), this,
             SLOT(doneCalibration(bool)));
  stereo_cam->freerun();
}

void MainWindow::setMatcher(int index) {
  if (index < 0 || index > matcher_list.size()) return;

  auto matcher_widget = matcher_list.at(index);
  qDebug() << "Changing matcher to" << ui->matcherSelectBox->itemText(index);
  stereo_cam->setMatcher(matcher_widget->getMatcher());

  for (int i = 0; i < ui->matcherSettingsLayout->count(); ++i) {
    QWidget* widget = ui->matcherSettingsLayout->itemAt(i)->widget();
    if (widget != matcher_widget) {
      widget->setVisible(false);
    } else {
      widget->setVisible(true);
      disparity_view->setMatcher(matcher_widget->getMatcher());
    }
  }
}

void MainWindow::setupMatchers(void) {
  disconnect(ui->matcherSelectBox, SIGNAL(currentIndexChanged(int)), this,
             SLOT(setMatcher(int)));

  while (ui->matcherSelectBox->count() != 0) {
    ui->matcherSelectBox->removeItem(0);
  }

  QLayoutItem* child;
  while ((child = ui->matcherSettingsLayout->takeAt(0)) != 0) {
    child->widget()->hide();
    delete child;
  }

  matcher_list.clear();

  MatcherWidgetOpenCVBlock* block_matcher =
      new MatcherWidgetOpenCVBlock(this, stereo_cam->getSize());
  matcher_list.append(block_matcher);
  ui->matcherSelectBox->insertItem(0, "OpenCV Block");
  ui->matcherSettingsLayout->addWidget(block_matcher);

  MatcherWidgetOpenCVSGBM* opencv_sgm =
      new MatcherWidgetOpenCVSGBM(this, stereo_cam->getSize());
  matcher_list.append(opencv_sgm);
  ui->matcherSelectBox->insertItem(1, "OpenCV SGM");
  ui->matcherSettingsLayout->addWidget(opencv_sgm);

  ui->matcherSelectBox->setCurrentIndex(0);

  connect(ui->matcherSelectBox, SIGNAL(currentIndexChanged(int)), this,
          SLOT(setMatcher(int)));
}

void MainWindow::setCalibrationFolder(QString dir) {
    if(dir == ""){
      dir = QFileDialog::getExistingDirectory(
      this, tr("Open Calibration Folder"), "/home",
      QFileDialog::DontResolveSymlinks);
    }

    if(dir == "") return;

    bool res = stereo_cam->loadCalibration(dir);

    if(!res) {
      QMessageBox msg;
      msg.setText("Unable to load calibration files");
      msg.exec();
    }else{
        calibration_directory = dir;
        parameters->update_string("calDir", calibration_directory);
    }

    ui->toggleRectifyCheckBox->setEnabled(res);
    ui->toggleRectifyCheckBox->setChecked(res);

}

void MainWindow::statusMessageTimeout(void) {
  if (stereo_cam->isAcquiring()) {
    ui->statusBar->showMessage("Freerunning.");
  } else {
    ui->statusBar->showMessage("Paused.");
  }
}

void MainWindow::singleShotClicked(void) {
  stereo_cam->singleShot();
  ui->statusBar->showMessage("Paused.");
  ui->pauseButton->setIcon(awesome->icon(fa::play, icon_options));
}

void MainWindow::saveSingle(void) { stereo_cam->saveImageTimestamped(); }

void MainWindow::toggleAcquire(void) {
  if (stereo_cam->isAcquiring()) {
    stereo_cam->pause();
    ui->statusBar->showMessage("Paused.");
    ui->pauseButton->setIcon(awesome->icon(fa::play, icon_options));
  } else {
    QTimer::singleShot(1, stereo_cam, SLOT(freerun()));
    ui->statusBar->showMessage("Freerunning.");
    ui->pauseButton->setIcon(awesome->icon(fa::pause, icon_options));
  }
}

void MainWindow::displaySaved(QString fname) {
  ui->statusBar->showMessage(QString("Saved to: %1").arg(fname));
  status_bar_timer->setSingleShot(true);
  status_bar_timer->start(1500);
}

void MainWindow::updateDisplay(void) {
    cv::Mat left, right;

      stereo_cam->getLeftImage(left);
      stereo_cam->getRightImage(right);

      left_view->updateView(left.data);
      right_view->updateView(right.data);

      QImage im_left(left.data, left.cols, left.rows, QImage::Format_Indexed8);
      pmap_left = QPixmap::fromImage(im_left);

      ui->left_image_view_stereo->setPixmap(pmap_left.scaled(
      ui->left_image_view_stereo->size(), Qt::KeepAspectRatio));
}

void MainWindow::setSaveDirectory(QString dir) {

  if(dir == ""){
      dir = QFileDialog::getExistingDirectory(
      this, tr("Open Directory"), "/home",
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  }

  if (dir != "") {
    ui->saveDirLabel->setText(dir);

    parameters->updatePreviousDirectory(dir);
    save_directory = parameters->get_string("saveDir");
    if(stereo_cam)
        stereo_cam->setSavelocation(save_directory);

  }
}

void MainWindow::updateFrameCount(qint64 count) {
  frame_counter->setText(QString("Frame count: %1").arg(count));
}

void MainWindow::updateFPS(qint64 time) {
  int fps = 1000.0 / time;
  fps_counter->setText(QString("FPS: %1").arg(fps));
}

void MainWindow::updateTemperature(double temperature) {
  temp_label->setText(QString("Temp: %1 C").arg(QString::number(temperature, 'f', 2)));
}

MainWindow::~MainWindow() {
  stereoCameraRelease();

  delete ui;
}
