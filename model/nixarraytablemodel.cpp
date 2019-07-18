#include "nixarraytablemodel.h"
#include "views/datatable.h"
#include <stdint.h>
#include "utils/datacontroller.h"

NixArrayTableModel::NixArrayTableModel(QObject *parent)
    : QAbstractTableModel(parent), h_labels(), v_labels(), src_info(""), array_info() {
}

void NixArrayTableModel::set_source(const EntityInfo &info, size_t page) {
    this->src_info = info;
    DataController &dc = DataController::instance();
    array_info = dc.getArrayInfo(info);
    this->page = page;
    shape = this->src_info.shape;
    cols = shape.size() > 1 ?  shape[1] : 1;
    rows = shape[0];

    this->insertColumns(0, cols);
    this->insertRows(0, rows);
    v_labels = dc.dimensionLabels(this->src_info, 1);
    if (shape.size() > 1) {
        h_labels = dc.dimensionLabels(this->src_info, 2);
    }
}


QVariant NixArrayTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole && role != Qt::ToolTipRole) {
        return QVariant();
    }
    if (orientation == Qt::Orientation::Horizontal) { // column
        if (shape.size() == 1) {
            return QString::fromStdString( array_info.label + (array_info.unit.size() > 0 ? " [" + array_info.unit + "]" : ""));
        } else {
            return QVariant(h_labels[section]);
        }
    } else if (orientation == Qt::Orientation::Vertical) { // row
        return QVariant(v_labels[section]);
    }
    return "1";
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
