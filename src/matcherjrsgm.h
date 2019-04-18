#ifndef MATCHERJRSGM_H
#define MATCHERJRSGM_H

#include <abstractstereomatcher.h>
#include <PhobosIntegration/PhobosIntegration.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <QSettings>

class MatcherJrSGM : public AbstractStereoMatcher {
    Q_OBJECT
public:
    explicit MatcherJrSGM(QObject *parent = nullptr,
                          cv::Size image_size = cv::Size(0, 0))
                          : AbstractStereoMatcher(parent, image_size) {
                        init();
    }

    ~MatcherJrSGM(void){
        if(matcher_handle != nullptr){
            JR::Phobos::DestroyMatchStereoHandle(matcher_handle);
        }
    }

    void forwardMatch(void);
    void backwardMatch(void);

    void parseConfig(std::string input_file);
    int getErrorDisparity();

    void writeConfig(const std::string &sConfigFile);
    void readConfig(QString &sConfigFile);
    void setConfig();

public slots:
    void setDisparityShift(int shift);
    void setDisparityRange(int n);
    void enableSubpixel(bool enable);
    void setMatchCosts(float P1, float P2);
    void setWindowSize(int census_size);
    void enableInterpolation(bool enable);
    void enableOcclusionDetection(bool enable);

    float getP1(void){ return params.oPyramidParams[0].oSGMParams.fP1_E_W; }
    float getP2(void){ return params.oPyramidParams[0].oSGMParams.fP2_E_W; }
    int getDisparityRange(void){ return params.oPyramidParams[0].nMaximumNumberOfDisparities; }
    int getCensusSize(void){ return params.oPyramidParams[0].oMetricParams.nWindowSizeX; }
    bool getInterpolate(void){return params.oFinalSubPixelParameters.bInterpol; }
    bool getOcclusionDetect(void){ return params.oFinalSubPixelParameters.bOcclusionDetection; }
    bool getSubpixel(void){return params.oFinalSubPixelParameters.bCompute; }
    int getDisparityShift(void){return params.fTopPredictionShift * pow(2, params.nNumberOfPyramids-1) ; }

private:
    JR::Phobos::TSTEREOHANDLE matcher_handle = nullptr;
    JR::Phobos::SMatchingParametersInput params;
    QString config_path = "jr_sgm_config.cfg";

    void init(void);
    void initParameters(int nPyramids, JR::Phobos::SSGMParameters sgm_params);
    JR::Phobos::SMatchingParameters parsePyramid(QSettings *settings, QString group);
    JR::Phobos::SSGMParameters initSGMParameters();

};

#endif // MATCHERJRSGM_H
