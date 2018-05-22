#include "TagViewListed.h"
#include "ui_TagViewListed.h"
#include "common/Common.hpp"
#include "plotter/plotter.h"
#include "plotter/lineplotter.h"
#include "plotter/eventplotter.h"


TagViewListed::TagViewListed(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TagViewListed)
{
    ui->setupUi(this);
    scrollFactor = 100;
    connect(ui->hScrollBar, SIGNAL(valueChanged(int)), this, SLOT(hScrollBarPosChanged(int)));
}


TagViewListed::~TagViewListed()
{
    delete ui;
}


void TagViewListed::setEntity(QVariant var) {
    if(var.canConvert<nix::Tag>() | var.canConvert<nix::MultiTag>()) {
        this->tag = TagContainer(var);
    } else {
        return;
    }
    processTag();
}


void TagViewListed::clear() {
    ui->tagLabel->setText("");
    ui->tagLabel->setToolTip("");

    //TODO: clear all plotters/features/labels and lines from the plotwidget!
}


void TagViewListed::processTag() {
    clear();
    ui->tagLabel->setText(QString::fromStdString(tag.name() + " - " +tag.type()));
    ui->tagLabel->setToolTip(tag.description().c_str());

    for (nix::DataArray array: tag.references()) {
        PlotterType plotType = Plotter::suggested_plotter(array);

        if(plotType == PlotterType::Line) {
            LinePlotter* lp = new LinePlotter(this);
            ui->plotWidget->layout()->addWidget(lp);
            lp->draw(array);

            plots.append(lp);
        }
    }

    //plot Tags
    for (unsigned int i = 0; i < tag.tagCount(); i++) {
        EventPlotter* ep = new EventPlotter(this);
        ui->plotWidget->layout()->addWidget(ep);

        plots.append(ep);
        QString yLabel;
        QVector<QString> xLabels;
        tag.tagLabels(yLabel, xLabels, i);

        if(tag.hasExtents()) {
            ep->draw(tag.positions(i), tag.extents(i), yLabel, xLabels);
        } else {
            ep->draw(tag.positions(i), yLabel, xLabels);
        }

        tagRange = ep->getTotalxAxisRange();
    }

    //plot Features
    // TODO



    // connect all plots
    for (Plotter* p : plots) {
        if(p->plotter_type() == PlotterType::Line) {
            LinePlotter *lp = static_cast<LinePlotter*>(p);

            connect(lp,   SIGNAL(xAxisChanged(QCPRange, QCPRange)), this, SLOT(changeHScrollBarValue(QCPRange, QCPRange)) );
            connect(this, SIGNAL(hScrollBarToPlot(double)), lp, SLOT(changeXAxisPosition(double)));

        } else if( p->plotter_type() == PlotterType::Event) {
            EventPlotter *ep = static_cast<EventPlotter*>(p);
            connect(ep,   SIGNAL(xAxisChanged(QCPRange, QCPRange)), this, SLOT(changeHScrollBarValue(QCPRange, QCPRange)) );
            connect(this, SIGNAL(hScrollBarToPlot(double)), ep, SLOT(changeXAxisPosition(double)));
        }
    }
    int currentMin = ui->hScrollBar->minimum();
    int currentMax = ui->hScrollBar->maximum();

    if( (currentMax != std::round(tagRange.upper*scrollFactor)) | (currentMin != std::round(tagRange.lower*scrollFactor)) ) {
        ui->hScrollBar->setRange(std::round(tagRange.lower*scrollFactor), std::round(tagRange.upper*scrollFactor));
    }
    setAllPlotRanges(QCPRange(tagRange.lower - tagRange.size()*0.025, tagRange.upper + tagRange.size()*0.025));
}


void TagViewListed::setAllPlotRanges(QCPRange range) {
    for(Plotter* p : plots) {
        if(p->plotter_type() == PlotterType::Line) {
            LinePlotter *lp = static_cast<LinePlotter*>(p);

            if(lp->getCurrentxRange() != range) {
                lp->setxAxisrange(range);
            }
        } else if( p->plotter_type() == PlotterType::Event) {
            EventPlotter *ep = static_cast<EventPlotter*>(p);

            if(ep->getCurrentxAxisRange() != range) {
                ep->setxAxisrange(range);
            }
        }
    }
}


void TagViewListed::hScrollBarPosChanged(int value) {
    double pageStep = ui->hScrollBar->pageStep();
    double size = pageStep / scrollFactor;
    double center = static_cast<double>(value) / scrollFactor;

    QCPRange newRange = QCPRange(center - (size/2), center + (size/2));
    setAllPlotRanges(newRange);
}


void TagViewListed::changeHScrollBarValue(QCPRange newRange, QCPRange completeRange) {
    // ignore completeRange use member tagRange

    //Calculation from QCPRange to int and moving the scrollBar to the new position if needed!
    int currentMin = ui->hScrollBar->minimum();
    int currentMax = ui->hScrollBar->maximum();
    if( (currentMax != std::round(tagRange.upper*scrollFactor)) | (currentMin != std::round(tagRange.lower*scrollFactor)) ) {
        ui->hScrollBar->setRange(std::round(tagRange.lower*scrollFactor), std::round(tagRange.upper*scrollFactor));
    }
    //change pagestep of scrollbar if needed: after zoom change
    if((currentMax-currentMin)*scrollFactor != (newRange.size() * scrollFactor)) {
        ui->hScrollBar->setPageStep(std::round(newRange.size() * scrollFactor));
        setAllPlotRanges(newRange);
    }
    //change position of scrollbar
    if(ui->hScrollBar->value() != std::round(newRange.center()*scrollFactor)) {
        ui->hScrollBar->setValue(std::round(newRange.center()*scrollFactor));
    }
}
