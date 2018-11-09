/*
* Copyright I3D Robotics Ltd, 2017
* Author: Josh Veitch-Michaelis
*/

#include "stereocameradeimos.h"

bool StereoCameraDeimos::find_systems(int devid) {
  bool res = false;

  if (devid < 0) {
    devid = findCamera();
  }

  if (devid >= 0) {
    openHID();

    camera = cv::VideoCapture(cv::CAP_DSHOW + devid);

    if (camera.isOpened()) {
      res = setFrameSize(752, 480);
      getFrameRate();
      getTemperature();
      setExposure(5);

      temperature_timer = new QTimer(parent());
      temperature_timer->setInterval(1000);
      //connect(temperature_timer, SIGNAL(timeout()), this, SLOT(getTemperature()));
      temperature_timer->start();

      if (!res) {
        camera.release();
        connected = false;
      } else {
        connected = true;
      }
    }
  }

  return connected;
}

void StereoCameraDeimos::openHID(void) {
  hid_init();

  deimos_device = hid_open(0x2560, 0xC114, NULL);

  if (deimos_device == NULL) {
    qDebug() << "Couldn't open device";
  } else {
    hid_set_nonblocking(deimos_device, 1);
    exposure = getExposure();
    qDebug() << "Serial: " << (qint64) getSerial();
  }

  hid_exit();
}

qint64 StereoCameraDeimos::getSerial() {
  if (deimos_device == NULL) return -1;

  qint64 serial = 0;

  std::vector<unsigned char> buffer;
  buffer.resize(16);

  buffer.at(1) = GETCAMERA_UNIQUEID;

  if (!send_hid(buffer, 1)) {
    serial = 0;
  } else {
    for (int i = 1, k = 3; i < 5; i++, k--) serial |= buffer[i] << (k * 8);
  }

  return serial;
}

int StereoCameraDeimos::getExposure() {
  /*
   *  Returns the current exposure time in uSeconds, or -1 if an error occurred.
   */

  if (deimos_device == NULL) return -1;

  std::vector<unsigned char> buffer;
  buffer.resize(16);

  buffer.at(1) = CAMERA_CONTROL_STEREO;
  buffer.at(2) = GET_EXPOSURE_VALUE;

  if (!send_hid(buffer, 2)) return -1;

  int exposure;

  if (!buffer.at(10)) {
    return -1;
  } else {
    exposure = ((int)buffer.at(2) << 24) + ((int)buffer.at(3) << 16) +
               ((int)buffer.at(4) << 8) + (int)buffer.at(5);
    return exposure;
  }
}

double StereoCameraDeimos::getTemperature(void){
    std::vector<unsigned char> buffer(16);

    buffer.at(1) = CAMERA_CONTROL_STEREO;
    buffer.at(2) = GET_IMU_TEMP_DATA;

    if (!send_hid(buffer, 2)) return -1;

    if (!buffer.at(6)) {
      return -1;
    } else {
      uint8_t msb = buffer.at(2);
      uint8_t lsb = buffer.at(3);
      double temp = (msb << 8) | lsb;

      temp = ((temp - 20.0) / 16.0) + 21.0;
      emit temperature_C(temp);

      return temp;
    }

}

bool StereoCameraDeimos::enableAutoExpose(bool enable) {
  if (enable)
    return setExposure(0.001);
  else
    return setExposure(exposure / 1000);
}

bool StereoCameraDeimos::setExposure(double exposure_milliseconds) {
  if (deimos_device == NULL) return false;
  if (exposure_milliseconds <= 0 || exposure_milliseconds > 1000) return false;

  std::vector<unsigned char> buffer;
  buffer.resize(16);

  buffer[1] = CAMERA_CONTROL_STEREO;
  buffer[2] = SET_EXPOSURE_VALUE;

  int exposure_val = exposure_milliseconds * 1000;

  qDebug() << "Setting exposure to" << exposure_val;

  buffer[3] = (char)((exposure_val >> 24) & 0xFF);
  buffer[4] = (char)((exposure_val >> 16) & 0xFF);
  buffer[5] = (char)((exposure_val >> 8) & 0xFF);
  buffer[6] = (char)(exposure_val & 0xFF);

  if (!send_hid(buffer, 2)){
      qDebug() << "Failed to send exposure command";
      return false;
  }

  if (getExposure() == exposure_val) {
    if (exposure_val != 1) exposure = exposure_val;
    return true;
  }else{
      qDebug() << "Exposure read back incorrect";
  }

  return false;
}

bool StereoCameraDeimos::toggleHDR(bool enable){
    if (deimos_device == NULL) return false;

    std::vector<unsigned char> buffer;
    buffer.resize(4);

    buffer.at(1) = CAMERA_CONTROL_STEREO;
    buffer.at(2) = SET_HDR_MODE_STEREO;

    if(enable){
        buffer.at(3) = 1;
    }else{
        buffer.at(3) = 0;
    }

    if (!send_hid(buffer, 2)){
        qDebug() << "Failed to send HDR command";
        return false;
    }else{
        return true;
    }
}


bool StereoCameraDeimos::send_hid(std::vector<unsigned char> &buffer,
                                  size_t command_len) {
  if (deimos_device == NULL) return false;

  std::vector<unsigned char> commands;
  for (int i = 0; i < command_len; i++) {
    commands.push_back(buffer[i + 1]);
  }

  int result = hid_write(deimos_device, buffer.data(), buffer.size());

  if (result < 0) {
    qDebug() << "Failed to write HID data";
    return false;
  }

  result = hid_read_timeout(deimos_device, buffer.data(), buffer.size(), 500);

  if (result < 0) {
    qDebug() << "Failed to read HID data";
    return false;
  } else {
    for (int i = 0; i < command_len; i++) {
      if (commands[i] != buffer[i]) return false;
    }
  }

  return true;
}

int StereoCameraDeimos::findCamera(void) {
  // Create the System Device Enumerator.
  ICreateDevEnum *pDevEnum;
  IEnumMoniker *pEnum;

  HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
                                CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));
  if (SUCCEEDED(hr)) {
    // Create an enumerator for the category.
    hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum,
                                         0);
    if (hr != S_OK) {
      hr = VFW_E_NOT_FOUND;  // The category is empty - no camera connected
      return -1;
    }

    pDevEnum->Release();

    IMoniker *pMoniker = NULL;
    int i = 0;
    while (pEnum->Next(1, &pMoniker, NULL) == S_OK) {
      IPropertyBag *pPropBag;
      HRESULT hr = pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));
      if (FAILED(hr)) {
        pMoniker->Release();
        continue;
      }

      VARIANT var;
      VariantInit(&var);

      // Get description or friendly name.
      hr = pPropBag->Read(L"Description", &var, 0);
      if (FAILED(hr)) {
        hr = pPropBag->Read(L"FriendlyName", &var, 0);
      }

      if (SUCCEEDED(hr)) {
        std::wstring str(var.bstrVal);

        if (std::string(str.begin(), str.end()) == "See3CAM_Stereo") {
          qDebug() << "Found Deimos at device " << i;

          hr = pPropBag->Read(L"DevicePath", &var, 0);

          device_path = var.bstrVal;
          return i;
        }

        VariantClear(&var);
      }

      hr = pPropBag->Write(L"FriendlyName", &var);

      pPropBag->Release();
      pMoniker->Release();
      i++;
    }
  }

  return -1;
}

bool StereoCameraDeimos::setFrameSize(int width, int height) {
  bool res = false;

  res = camera.set(CV_CAP_PROP_FRAME_WIDTH, width);
  res &= camera.set(CV_CAP_PROP_FRAME_HEIGHT, height);
  image_width = width;
  image_height = height;
  image_size = cv::Size(width, height);

  return res;
}

bool StereoCameraDeimos::setFrame16(void) {
  bool res = false;

  res = camera.set(CV_CAP_PROP_FOURCC, CV_FOURCC('Y', '1', '6', ' '));

  return res;
}

void StereoCameraDeimos::getFrameRate() {
  frame_rate = (int)camera.get(CV_CAP_PROP_FPS);
  qDebug() << "Frame rate: " << frame_rate;
}

void StereoCameraDeimos::disconnectCamera() {

    if (camera.isOpened()) {
        camera.release();
        hid_close(deimos_device);
        hid_exit();
    }

    connected = false;
    emit disconnected();

}

  bool StereoCameraDeimos::capture() {

    if(capturing) return false;

    frametimer.restart();

    captured_stereo = false;
    capturing = true;

    bool res = false;

    if(connected && camera.grab()){
        if(camera.retrieve(image_buffer)){

            flip(image_buffer, image_buffer, 0);
            split(image_buffer, channels);

            left_raw = channels[1].clone();
            right_raw = channels[2].clone();

            emit left_captured();
            emit right_captured();

            res = true;

        }else{
            qDebug() << "Retrieve fail";
            res = false;
        }
    }else{
        qDebug() << "Grab fail";
        res = false;
    }

    if(!res){
        disconnectCamera();
    }

    capturing = false;

    return res;
  }

  StereoCameraDeimos::~StereoCameraDeimos(void) {
    disconnectCamera();
    emit finished();
  }

  std::string wchar_to_string(WCHAR * buffer) {
    std::wstring ws(buffer);
    std::string buffer_s(ws.begin(), ws.end());
    return buffer_s;
  }
