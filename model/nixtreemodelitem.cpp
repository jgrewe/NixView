#include "nixtreemodelitem.h"
#include "utils/entitydescriptor.h"
#include "common/Common.hpp"
#include <QString>

const QVector<QString> NixTreeModelItem::columns = {MODEL_HEADER_NAME, MODEL_HEADER_NIXTYPE, MODEL_HEADER_STORAGETYPE,
                                                    MODEL_HEADER_DATATYPE, MODEL_HEADER_ID, MODEL_HEADER_VALUE,
                                                    MODEL_HEADER_CREATEDAT, MODEL_HEADER_UPDATEDAT};

const std::map<NixType, QVariant> NixTreeModelItem::type_name_map = {
    { NixType::NIX_BLOCK, QVariant(NIX_STRING_BLOCK) },
    { NixType::NIX_DATA_ARRAY, QVariant(NIX_STRING_DATAARRAY) },
    { NixType::NIX_GROUP, QVariant(NIX_STRING_GROUP) },
    { NixType::NIX_FEAT, QVariant(NIX_STRING_FEATURE) },
    { NixType::NIX_TAG, QVariant(NIX_STRING_TAG) },
    { NixType::NIX_MTAG, QVariant(NIX_STRING_MULTITAG) },
    { NixType::NIX_SOURCE, QVariant(NIX_STRING_SOURCE) },
    { NixType::NIX_SECTION, QVariant(NIX_STRING_SECTION) },
    { NixType::NIX_DIMENSION, QVariant(NIX_STRING_DIMENSION) },
    { NixType::NIX_PROPERTY, QVariant(NIX_STRING_PROPERTY) }
};


NixTreeModelItem::NixTreeModelItem(EntityInfo &info, NixTreeModelItem *parent)
    : parent_item(parent), info(info) {

}


NixTreeModelItem::~NixTreeModelItem() {
    qDeleteAll(children);
}



void NixTreeModelItem::appendChild(NixTreeModelItem *item) {
    children.append(item);
}


NixTreeModelItem* NixTreeModelItem::child(int row) {
    if (row < children.size()) {
        return children.value(row);
    }
    return nullptr;
}


int NixTreeModelItem::childCount() const {
    return children.count();
}



int NixTreeModelItem::columnCount() const {
    return this->columns.size();
}


QVariant NixTreeModelItem::data(int column) const {
    if (column < this->columns.count()) {
        switch (column) {
            case 0:
                return QVariant(QString::fromStdString(info.name));
            case 1:
                return QVariant(QString::fromStdString(info.type));
            case 2:
                if (info.nix_type != NixType::NIX_UNKNOWN)
                    return QVariant(type_name_map.at(info.nix_type));
                else
                    return QVariant();
            case 3:
                return info.dtype;
            case 4:
                return QVariant(QString::fromStdString(info.id));
            case 5:
                return info.value;
            case 6:
                return info.created_at;
            case 7:
                return info.updated_at;
            default:
                return QVariant();
        }
    }
    return QVariant();
}


QVariant NixTreeModelItem::toolTip() const {
    return QVariant(QString::fromStdString(entityInfo().description));
}


QString NixTreeModelItem::getHeader(int column) {
    return this->columns[column];
}


NixType NixTreeModelItem::nixType() const {
    return info.nix_type;
}


EntityInfo NixTreeModelItem::entityInfo() const {
    return this->info;
}


int NixTreeModelItem::row() const {
    if (parent_item) {
       return parent_item->children.indexOf(const_cast<NixTreeModelItem*>(this));
    }
    return 0;
}


NixTreeModelItem* NixTreeModelItem::parentItem() {
    return parent_item;
}
