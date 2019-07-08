#ifndef NIXTREEMODELITEM_H
#define NIXTREEMODELITEM_H
#include <QVariant>
#include <QList>
#include <QVector>
#include <nix.hpp>
#include "utils/datacontroller.h"


class NixTreeModelItem {

public:
    explicit NixTreeModelItem(EntityInfo &info, NixTreeModelItem *parentItem = 0);
    ~NixTreeModelItem();

    static const QVector<QString> columns;
    static const std::map<NixType, QVariant> type_name_map;

    void appendChild(NixTreeModelItem *child);
    NixTreeModelItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    NixTreeModelItem *parentItem();
    QString getHeader(int column);
    NixType nixType() const;
    QVariant& itemData();
    QVariant toolTip() const;
    EntityInfo getEntityInfo() const;

private:
    NixTreeModelItem *parent_item;
    EntityInfo info;
    QList<NixTreeModelItem*> children;
};

#endif // NIXTREEMODELITEM_H
