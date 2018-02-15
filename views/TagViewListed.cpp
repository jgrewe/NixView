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
    //clear all plotters/features/labels and lines from the plotwidget!
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
        }
    }

    //plot Tags
    EventPlotter* ep = new EventPlotter(this);
    ui->plotWidget->layout()->addWidget(ep);

    if(tag.hasExtents()) {
        ep->draw(tag.positions(), tag.extents(), tag.completeDescription());
    } else {
        ep->draw(tag.positions(), tag.completeDescription());
    }


    //plot Features


}
