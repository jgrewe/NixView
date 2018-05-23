#ifndef TAGVIEWLISTED_H
#define TAGVIEWLISTED_H

#include <QWidget>
#include "model/NixModelItem.hpp"
#include "utils/entitydescriptor.h"
#include <nix.hpp>
#include "utils/tagcontainer.h"
#include "../plotter/plotter.h"

namespace Ui {
class TagViewListed;
}

class TagViewListed : public QWidget
{
    Q_OBJECT

public:
    explicit TagViewListed(QWidget *parent = 0);
    ~TagViewListed();

    void setEntity(QVariant var, QVariant block);
    void clear();

public slots:
    //void show_tag_info();
    void setAllPlotRanges(QCPRange range);

    void hScrollBarPosChanged(int value);
    void changeHScrollBarValue(QCPRange newRange, QCPRange completeRange);

signals:
    void hScrollBarToPlot(double); // signals for the plotter.

private:
    Ui::TagViewListed *ui;
    TagContainer tag;
    nix::Block block;
    QVector<Plotter*> plots;
    QCPRange tagRange;
    QCPRange refRange;
    double scrollFactor;

    void processTag();
    void plotData();
    void plotTags();
    void plotFeatures();
};

#endif // TAGVIEWLISTED_H
