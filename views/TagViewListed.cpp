#include "TagViewListed.h"
#include "ui_TagViewListed.h"
#include "common/Common.hpp"
#include "plotter/plotter.h"
#include "plotter/lineplotter.h"
#include "plotter/eventplotter.h"

/****** TagContainerTemp ******/
// Copy from TagView: added positions(), extents() hasExtents()
TagContainerTemp::TagContainerTemp(QVariant entity):entity(entity) {}

TagContainerTemp::TagContainerTemp(){}

std::vector<nix::Feature> TagContainerTemp::features(){
    std::vector<nix::Feature> vec;
    if (entity.canConvert<nix::Tag>()) {
        vec = entity.value<nix::Tag>().features();
    } else if (entity.canConvert<nix::MultiTag>()) {
        vec = entity.value<nix::MultiTag>().features();
    }
    return vec;
}


std::vector<nix::DataArray> TagContainerTemp::references() {
    std::vector<nix::DataArray> vec;
    if (entity.canConvert<nix::Tag>()) {
        vec = entity.value<nix::Tag>().references();
    } else if (entity.canConvert<nix::MultiTag>()) {
        vec = entity.value<nix::MultiTag>().references();
    }
    return vec;
}

QVector<QVector<double>> TagContainerTemp::positions() {
    QVector<QVector<double>> positions;
    if (entity.canConvert<nix::Tag>()) {
        std::vector<double> pos = entity.value<nix::Tag>().position();
        positions.append(QVector<double>::fromStdVector(pos));

    } else if (entity.canConvert<nix::MultiTag>()) {
        nix::DataArray array = entity.value<nix::MultiTag>().positions();
        if(array.dataExtent().size() == 1) {
            std::vector<double> pos = std::vector<double>(array.dataExtent()[0]);
            array.getDataDirect(array.dataType(), pos.data(), {array.dataExtent()[0]}, {0});
            positions.append(QVector<double>::fromStdVector(pos));
        } else if(array.dataExtent().size() == 2 ) {
            for (unsigned int i = 0; i < array.dataExtent()[0]; i++) {
                std::vector<double> pos = std::vector<double>(array.dataExtent()[1]);
                array.getDataDirect(array.dataType(), pos.data(), {array.dataExtent()[1]}, {(int) i, 0});
                positions.append(QVector<double>::fromStdVector(pos));
            }
        } else {
            std::cerr << "TagContainerTemp::positions cannot handle more than 2 dimensions." << std::endl;
        }

    }
    return positions;
}

QVector<QVector<double>> TagContainerTemp::extents() {
    QVector<QVector<double>> extents;
    if(this->hasExtents()) {
        if (entity.canConvert<nix::Tag>()) {
            std::vector<double> ext = entity.value<nix::Tag>().extent();
            extents.append(QVector<double>::fromStdVector(ext));

        } else if (entity.canConvert<nix::MultiTag>()) {
            nix::DataArray array = entity.value<nix::MultiTag>().extents();
            if(array.dataExtent().size() == 1) {
                std::vector<double> ext = std::vector<double>(array.dataExtent()[0]);
                array.getDataDirect(array.dataType(), ext.data(), {array.dataExtent()[0]}, {0});
                extents.append(QVector<double>::fromStdVector(ext));
            } else if(array.dataExtent().size() == 2 ) {
                for (unsigned int i = 0; i < array.dataExtent()[0]; i++) {
                    std::vector<double> ext = std::vector<double>(array.dataExtent()[1]);
                    array.getDataDirect(array.dataType(), ext.data(), {array.dataExtent()[1]}, {(int) i, 0});
                    extents.append(QVector<double>::fromStdVector(ext));
                }
            } else {
                std::cerr << "TagContainerTemp::extents cannot handle more than 2 dimensions." << std::endl;
            }
        }
    }
    return extents;
}

bool TagContainerTemp::hasExtents() {
    if (entity.canConvert<nix::Tag>()) {
        return (entity.value<nix::Tag>().extent().size() > 0);
    } else if (entity.canConvert<nix::MultiTag>()) {
        return (entity.value<nix::MultiTag>().extents() != nix::none);
    }
    return false;
}


std::string TagContainerTemp::name() {
    std::string name;
    if (entity.canConvert<nix::Tag>()) {
        name = entity.value<nix::Tag>().name();
    } else if (entity.canConvert<nix::MultiTag>()) {
        name = entity.value<nix::MultiTag>().name();
    }
    return name;
}

std::string TagContainerTemp::type() {
    std::string type;
    if (entity.canConvert<nix::Tag>()) {
        type = entity.value<nix::Tag>().type();
    } else if (entity.canConvert<nix::MultiTag>()) {
        type = entity.value<nix::MultiTag>().type();
    }
    return type;
}

std::string TagContainerTemp::description() {
    std::string description;
    if (entity.canConvert<nix::Tag>()) {
        description = EntityDescriptor::describe(entity.value<nix::Tag>());
    } else if (entity.canConvert<nix::MultiTag>()) {
        description = EntityDescriptor::describe(entity.value<nix::MultiTag>());
    }
    return description;
}


QVariant TagContainerTemp::getEntity() {
    return this->entity;
}

/****** TagViewListed ******/

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
        this->tag = TagContainerTemp(var);
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
    std::cerr << "processTag():" << std::endl;
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
        ep->draw(tag.positions(), tag.extents());
    } else {
        ep->draw(tag.positions());
    }


    //plot Features


}








