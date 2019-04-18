#ifndef MATCHERWIDGETJRSGM_H
#define MATCHERWIDGETJRSGM_H

#include <matcherjrsgm.h>
#include <matcherwidget.h>
#include <QFile>

namespace Ui {
class MatcherWidgetJrSgm;
}

class MatcherWidgetJrSgm : public MatcherWidget
{
    Q_OBJECT

public:
    explicit MatcherWidgetJrSgm(QWidget *parent = 0, cv::Size image_size =cv::Size(0,0));
     ~MatcherWidgetJrSgm();

public slots:
    void onCostUpdated(void);
    void updateDisparityRange(int range);
    void updateCensus(int width);
    void initParameters();
    void reloadParameters();
    AbstractStereoMatcher* getMatcher(void);


private:
    Ui::MatcherWidgetJrSgm *ui;
    MatcherJrSGM* matcher;
    void init(void);

signals:
    void setMatchCosts(double p1, double p2);
    void setOcclusion(bool enabled);
    void setInterpolation(bool enabled);
    void setSubpixel(bool enabled);
    void setDisparityRange(int range);
    void setCensusWindow(int width);

};

#endif // MATCHERWIDGETJRSGM_H
