#include "nixtreemodel.h"
#include <common/Common.hpp>
#include "utils/datacontroller.h"


NixTreeModel::NixTreeModel(QObject *parent)
    : QAbstractItemModel(parent) {
    EntityInfo rinfo("Root");
    EntityInfo dinfo("Data");
    EntityInfo minfo("Metadata");

    root_node = new NixTreeModelItem(rinfo);
    data_node = new NixTreeModelItem(dinfo, root_node);
    metadata_node = new NixTreeModelItem(minfo, root_node);
    root_node->appendChild(data_node);
    root_node->appendChild(metadata_node);
    DataController& dc = DataController::instance();
    dc.blocks_to_items(data_node);
    dc.sections_to_items(metadata_node);
}


NixTreeModel::NixTreeModel(NixTreeModelItem *root_item, QObject *parent)
    :QAbstractItemModel(parent) {
    EntityInfo rinfo("Root");
    root_node = new NixTreeModelItem(rinfo);
    EntityInfo section_info = root_item->entityInfo();
    NixTreeModelItem *section_node = new NixTreeModelItem(section_info, root_node);
    root_node->appendChild(section_node);
}


NixTreeModel::~NixTreeModel() {
    if (root_node != nullptr) {
        delete root_node;
    }
}


QVariant NixTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Orientation::Horizontal) {
        return QVariant(this->root_node->getHeader(section));
    }
    return QVariant("");
}


QModelIndex NixTreeModel::index(int row, int column, const QModelIndex &parent) const {
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    NixTreeModelItem *parentItem;

    if (!parent.isValid())
        parentItem = root_node;
    else
        parentItem = static_cast<NixTreeModelItem*>(parent.internalPointer());

    NixTreeModelItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}


QModelIndex NixTreeModel::parent(const QModelIndex &index) const{
    if (!index.isValid())
          return QModelIndex();

    NixTreeModelItem *childItem = static_cast<NixTreeModelItem*>(index.internalPointer());
    NixTreeModelItem *parentItem = childItem->parentItem();
    if (parentItem == root_node || parentItem == nullptr)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}


int NixTreeModel::rowCount(const QModelIndex &parent) const {
    NixTreeModelItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = root_node;
    else
        parentItem = static_cast<NixTreeModelItem*>(parent.internalPointer());

    return parentItem->childCount();
}


int NixTreeModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return static_cast<NixTreeModelItem*>(parent.internalPointer())->columnCount();
    else
        return root_node->columnCount();
}


QVariant NixTreeModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || (role != Qt::DisplayRole && role != Qt::ToolTipRole)) {
        return QVariant();
    }
    NixTreeModelItem *item = static_cast<NixTreeModelItem*>(index.internalPointer());
    if (role == Qt::DisplayRole) {
        return item->data(index.column());
    }
    return item->toolTip();
}


bool NixTreeModel::hasChildren(const QModelIndex &parent) const {
    if (!parent.isValid()) {
        return true;
    }

    NixTreeModelItem *item = static_cast<NixTreeModelItem*>(parent.internalPointer());
    return item->entityInfo().max_child_count > 0;
}


bool NixTreeModel::canFetchMore(const QModelIndex &parent) const {
    if (!parent.isValid())
        return false;
    NixTreeModelItem *itm = static_cast<NixTreeModelItem*>(parent.internalPointer());
    return itm->entityInfo().max_child_count > rowCount(parent);
}


void NixTreeModel::fetchMore(const QModelIndex &parent) {
    if (!parent.isValid()) {
        return;
    }
    NixTreeModelItem *itm = static_cast<NixTreeModelItem*>(parent.internalPointer());
    DataController &dc = DataController::instance();
    switch (itm->nixType()) {
        case NixType::NIX_BLOCK: {
            dc.fetch_block(itm);
            break;
        }
        case NixType::NIX_DATA_ARRAY: {
            dc.fetch_data_array(itm);
            break;
        }
        case NixType::NIX_TAG: {
            dc.fetch_tag(itm);
            break;
        }
        case NixType::NIX_SOURCE: {
            dc.fetch_source(itm);
            break;
        }
        case NixType::NIX_MTAG: {
            dc.fetch_mtag(itm);
            break;
        }
        case NixType::NIX_GROUP: {
            dc.fetch_group(itm);
            break;
        }
        case NixType::NIX_SECTION: {
            dc.fetch_section(itm);
        }
        default:
            break;
    }
}
