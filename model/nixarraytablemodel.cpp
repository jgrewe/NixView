#include "nixarraytablemodel.h"
#include "views/datatable.h"
#include <stdint.h>
#include "utils/datacontroller.h"

NixArrayTableModel::NixArrayTableModel(QObject *parent)
    : QAbstractTableModel(parent), h_labels(), v_labels(), src_info("") {
}

void NixArrayTableModel::set_source(const EntityInfo &info, int page) {
    this->src_info = info;
    this->page = page;
    shape = this->src_info.shape;
    cols = shape.size() > 1 ?  shape[1] : 1;
    rows = shape[0];

    this->insertColumns(0, cols);
    this->insertRows(0, rows);

    /*
    if (array.getDimension(1).dimensionType() == nix::DimensionType::Set) {
        v_labels = array.getDimension(1).asSetDimension().labels();
    }
    if (shape.size() > 1) {
        if (array.getDimension(2).dimensionType() == nix::DimensionType::Set) {
            h_labels = array.getDimension(2).asSetDimension().labels();
        }
    }
    */
}


QVariant NixArrayTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole && role != Qt::ToolTipRole) {
        return QVariant();
    }
    /*
    if (orientation == Qt::Orientation::Horizontal) {
        if (shape.size() == 1) {
            return QString::fromStdString((array.label() ? *array.label() : "") +
                                          (array.unit() ? " [" + *array.unit() + "]" : ""));
        } else {
            nix::Dimension dim = array.getDimension(2);
            return get_dimension_label(section, role, orientation, dim);
        }
    } else if (orientation == Qt::Orientation::Vertical) {
        nix::Dimension dim = array.getDimension(1);
        return get_dimension_label(section, role, orientation, dim);
        return "1";
    }
    */
    return "1";
}


QVariant NixArrayTableModel::get_dimension_label(int section, int role, Qt::Orientation orientation, const nix::Dimension &dim) const {
    if (dim.dimensionType() == nix::DimensionType::Set) {
        if (orientation == Qt::Orientation::Vertical)
            return (size_t)section < v_labels.size() ? QString::fromStdString(v_labels[section]) :
                                                       QString::fromStdString(nix::util::numToStr(section));
        else
            return (size_t)section < h_labels.size() ? QString::fromStdString(h_labels[section]) :
                                                       QString::fromStdString(nix::util::numToStr(section));
    } else if (dim.dimensionType() == nix::DimensionType::Sample) {
        if (role == Qt::DisplayRole)
            return dim.asSampledDimension()[section];
        nix::SampledDimension sd = dim.asSampledDimension();
        std::string label = (sd.label() ? *sd.label() : "" ) + (sd.unit() ? " [" + *sd.unit() +"]" : "");
        return QString::fromStdString(label);
    } else if (dim.dimensionType() == nix::DimensionType::Range) {
        if (role == Qt::DisplayRole)
            return dim.asRangeDimension()[section];
        nix::RangeDimension rd = dim.asRangeDimension();
        std::string label = (rd.label() ? *rd.label() : "" ) + (rd.unit() ? " [" + *rd.unit() +"]" : "");
        return QString::fromStdString(label);
    }
    return QString();
}


int NixArrayTableModel::rowCount(const QModelIndex &parent) const {
    return rows;
}


int NixArrayTableModel::columnCount(const QModelIndex &parent) const {
    return cols;
}


QVariant NixArrayTableModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || (role != Qt::DisplayRole && role != Qt::ToolTipRole)) {
        return QVariant();
    }
    DataController &dc = DataController::instance();
    if (role == Qt::DisplayRole) {
        double d = 0.0;
        nix::NDSize count(shape.size(), 1);
        if ((index.row() < rows) && (index.column() < cols)) {
            nix::NDSize offset(shape.size(), 0);
            offset[0] = index.row();
            if (shape.size() > 1) {
                offset[1] = index.column();
            }
            if (shape.size() > 2) {
                offset[2] = page;
            }
            dc.getData(this->src_info, nix::DataType::Double, &d, count, offset);
            return QVariant(d);
        }
    } else if (role == Qt::ToolTipRole) {/*
        std::string label = (array.label() ? *array.label() : "") +
                            (array.unit() ? " [" + *array.unit() + "]" : "") + " @ \n";
        label = label + headerData(index.row(), Qt::Vertical, Qt::ToolTipRole).toString().toStdString() + ": " +
                headerData(index.row(), Qt::Vertical, Qt::DisplayRole).toString().toStdString() + "\n";
        label = label + headerData(index.column(), Qt::Horizontal, Qt::ToolTipRole).toString().toStdString() + ": " +
                headerData(index.column(), Qt::Horizontal, Qt::DisplayRole).toString().toStdString() + "\n";
        return QString::fromStdString(label);
        */
        return QVariant("");
    }

    return QVariant();
}
