#ifndef STEREOCAMERAPHOBOS_H
#define STEREOCAMERAPHOBOS_H

#include <QObject>
#include <abstractstereocamera.h>
#include <cameraimagingsource.h>

class StereoCameraPhobos : public AbstractStereoCamera
{

Q_OBJECT

signals:
    void start_capture();
    void stereopair_captured();
    void update_size(int width, int height, int bitdepth);

public slots:
    void loadLeftSettings();
    void loadRightSettings();

public:
    explicit StereoCameraPhobos(QObject *parent = 0) :
                AbstractStereoCamera(parent)
                {}
    bool capture();
    void disconnectCamera();
    bool setCameras(CameraImagingSource *left, CameraImagingSource *right);
    bool setExposure(double exposure);
    bool find_systems();

    ~StereoCameraPhobos(void);

private:
    CameraImagingSource *left_camera;
    CameraImagingSource *right_camera;

    std::vector<std::unique_ptr<CameraImagingSource>> stereo_cameras;
    std::vector<std::unique_ptr<Listener>> stereo_listeners;

    QList<qint64> load_serials(QString filename);
    QList<qint64> serials;
    QList<int> widths;
    QList<int> heights;

    void setup_cameras();
};

#endif // STEREOCAMERAPHOBOS_H
