#ifndef CAMERADISPLAYWIDGET_H
#define CAMERADISPLAYWIDGET_H

#include <QWidget>
#include "ui_cameradisplaywidget.h"

namespace Ui {
class CameraDisplayWidget;
}

class CameraDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CameraDisplayWidget(QWidget *parent = nullptr);
    ~CameraDisplayWidget();

    template <typename T>
    void updateView(T *new_image) {


      if(sizeof(T) != 1){
          int scale = 256 << 8*(sizeof(T) - 1);

          memcpy(displayBuffer.data(), new_image, sizeof(T) * displayBuffer.size());

          for (int i = 0; i < camwidth * camheight; i++) {
            display_image->bits()[i] = static_cast<T>(displayBuffer[i] / scale);
          }

      }else{
          memcpy(display_image->bits(), new_image, sizeof(T) * displayBuffer.size());
      }

      pixmap = QPixmap::fromImage(*display_image);

      if(pixmap.isNull()) return;

      ui->imageDisplay->setPixmap(pixmap.scaled(ui->imageDisplay->size(), Qt::KeepAspectRatio));

    }

    void setSettingsCallback(QObject* recevier, const char* slot);

    QLabel* getImageDisplay(void){return ui->imageDisplay;}

public slots:
    void setExposure(double value);
    void setSize(int height, int width, int bit_depth);

private:
    Ui::CameraDisplayWidget *ui;

    QVector<QRgb> colourMap;
    int camwidth = 0;
    int camheight = 0;

    QPixmap pixmap;
    QImage *display_image = nullptr;

    bool capturing = true;
    std::vector<unsigned char> displayBuffer;
};

#endif // CAMERADISPLAYWIDGET_H
