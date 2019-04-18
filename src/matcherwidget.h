/*
* Copyright I3D Robotics Ltd, 2017
* Author: Josh Veitch-Michaelis
*/

#ifndef MATCHERWIDGET_H
#define MATCHERWIDGET_H

#include <QWidget>
#include <abstractstereomatcher.h>
#include <QSettings>

class MatcherWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MatcherWidget(QWidget *parent = nullptr) : QWidget(parent) {}
    virtual AbstractStereoMatcher* getMatcher(void) = 0;

signals:

public slots:
};

#endif // MATCHERWIDGET_H
