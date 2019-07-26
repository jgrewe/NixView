#include "tagcontainer.h"
#include "common/Common.hpp"
#include "plotter/plotter.h"
#include "plotter/lineplotter.h"
#include "plotter/plotwidget.h"

TagContainer::TagContainer(const EntityInfo &info) {
    if (info.nix_type != NixType::NIX_MTAG && info.nix_type != NixType::NIX_TAG) {
        std::cerr << "TagContainer() - Exception: None tag entity in TagContainer." << std::endl;
    }
    this->data_src = info;
}

TagContainer::TagContainer(){}



std::vector<nix::Feature> TagContainer::features(){
    std::vector<nix::Feature> vec;
    if (entity.canConvert<nix::Tag>()) {
        vec = entity.value<nix::Tag>().features();
    } else if (entity.canConvert<nix::MultiTag>()) {
        vec = entity.value<nix::MultiTag>().features();
    }
    return vec;
}



/*
std::vector<nix::DataArray> TagContainer::references() {
    std::vector<nix::DataArray> vec;
    if (entity.canConvert<nix::Tag>()) {
        vec = entity.value<nix::Tag>().references();
    } else if (entity.canConvert<nix::MultiTag>()) {
        vec = entity.value<nix::MultiTag>().references();
    }
    return vec;
}
*/


QVector<double> TagContainer::positions(unsigned int index) {
    QVector<double> positions;

    if (index >= tagCount()) {
        std::cerr << "TagContainer::positions() - Index is bigger than the tagCount()." << std::endl;
        return positions;
    }

    if (entity.canConvert<nix::Tag>()) { // index has to be 0.
        std::vector<double> pos = entity.value<nix::Tag>().position();
        positions = QVector<double>::fromStdVector(pos);
        return positions;
    } else if (entity.canConvert<nix::MultiTag>()) {
        nix::DataArray array = entity.value<nix::MultiTag>().positions();
        if (array.dataExtent().size() == 1) { // index has to be 0.
            std::vector<double> data = std::vector<double>(array.dataExtent()[0]);
            array.getData(nix::DataType::Double, data.data(), {array.dataExtent()[0]}, {0});
            positions = QVector<double>::fromStdVector(data);
            return positions;
        } else if (array.dimensionCount() == 2) {     
               positions = Plotter::get_data_line(array, index, 1);
               return positions;
        } else {
            std::cerr << "Tagcontainer::positions(): can't handle arrays with more than 2 dimensions." << std::endl;
        }
    }
    return positions;
}


QVector<double> TagContainer::extents(unsigned int index) {
    QVector<double> extents;

    if (index >= tagCount()) {
        std::cerr << "TagContainer::extents() - Index is bigger than the tagCount()." << std::endl;
        return extents;
    }

    if(this->hasExtents()) {
        if (entity.canConvert<nix::Tag>()) {
            std::vector<double> ext = entity.value<nix::Tag>().extent();
            extents = QVector<double>::fromStdVector(ext);
            return extents;

        } else if (entity.canConvert<nix::MultiTag>()) {
            nix::DataArray array = entity.value<nix::MultiTag>().extents();

            if(array.dataExtent().size() == 1) {
                std::vector<double> data = std::vector<double>(array.dataExtent()[0]);
                array.getData(nix::DataType::Double, data.data(), {array.dataExtent()[0]}, {0});
                extents = QVector<double>::fromStdVector(data);
                return extents;

            } else if(array.dataExtent().size() == 2 ) {
                extents = Plotter::get_data_line(array, index, 1);
                return extents;

            } else {
                std::cerr << "TagContainer::extents(): cannot handle more than 2 dimensions." << std::endl;
            }
        }
    }
    return extents;
}


bool TagContainer::hasExtents() {
    if (entity.canConvert<nix::Tag>()) {
        return (entity.value<nix::Tag>().extent().size() > 0);
    } else if (entity.canConvert<nix::MultiTag>()) {
        return (entity.value<nix::MultiTag>().extents() != nix::none);
    }
    return false;
}


std::string TagContainer::name() {
    std::string name;
    if (entity.canConvert<nix::Tag>()) {
        name = entity.value<nix::Tag>().name();
    } else if (entity.canConvert<nix::MultiTag>()) {
        name = entity.value<nix::MultiTag>().name();
    }
    return name;
}


std::string TagContainer::type() {
    std::string type;
    if (entity.canConvert<nix::Tag>()) {
        type = entity.value<nix::Tag>().type();
    } else if (entity.canConvert<nix::MultiTag>()) {
        type = entity.value<nix::MultiTag>().type();
    }
    return type;
}


std::string TagContainer::description() {
    std::string description;
    if (entity.canConvert<nix::Tag>()) {
        description = EntityDescriptor::describe(entity.value<nix::Tag>());
    } else if (entity.canConvert<nix::MultiTag>()) {
        description = EntityDescriptor::describe(entity.value<nix::MultiTag>());
    }
    return description;
}


void TagContainer::refLabels(QString &ylabel, QVector<QString> &xlabels, unsigned int index) {
    if(index >= refCount()) {
        std::cerr << "TagContainer::refLabels() - Index bigger than refCount." << std::endl;
        return;
    }
    if (entity.canConvert<nix::Tag>()) {
        Plotter::data_array_ax_labels(entity.value<nix::Tag>().references()[index],ylabel,xlabels);
        return;
    } else if (entity.canConvert<nix::MultiTag>()) {
        Plotter::data_array_ax_labels(entity.value<nix::MultiTag>().references()[index],ylabel,xlabels);
        return;
    }
}


void TagContainer::tagLabels(QString &ylabel, QVector<QString> &xlabels, unsigned int index) {
    if(index >= tagCount()) {
        std::cerr << "TagContainer::tagLabels() - Index bigger than tagCount." << std::endl;
        return;
    }
    refLabels(ylabel, xlabels, 0); // get xlabels from the first ref.
    ylabel = QString::fromStdString("Tag: " + name() + " " + std::to_string(index));
}


void TagContainer::featureLabels(QString &ylabel, QVector<QString> &xlabels, unsigned int index) {
    if(index >= featureCount()) {
        std::cerr << "TagContainer::featureLabels() - Index bigger than featureCount." << std::endl;
        return;
    }
    nix::Feature f = features()[index];
    Plotter::data_array_ax_labels(f.data(),ylabel,xlabels);
}


unsigned int TagContainer::refCount() {
    if (entity.canConvert<nix::Tag>()) {
        return entity.value<nix::Tag>().referenceCount();
    } else if (entity.canConvert<nix::MultiTag>()) {
        return entity.value<nix::MultiTag>().referenceCount();
    }
    return -1;
}


unsigned int TagContainer::tagCount() {
    if (data_src.nix_type == NixType::NIX_TAG) {
        return 1;
    } else if (data_src.nix_type == NixType::NIX_MTAG) {
        nix::NDSize size = entity.value<nix::MultiTag>().positions().dataExtent();
        if (size.size() == 1) {
            return 1;
        } else if (size.size() == 2) {
            return size[1];
        }
    }
    return -1;
}


unsigned int TagContainer::featureCount() {
    if (entity.canConvert<nix::Tag>()) {
        return entity.value<nix::Tag>().featureCount();
    } else if (entity.canConvert<nix::MultiTag>()) {
        return entity.value<nix::MultiTag>().featureCount();
    }
    return -1;
}


EntityInfo TagContainer::dataSource() {
    return this->data_src;
}
