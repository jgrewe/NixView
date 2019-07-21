#ifndef TAGVIEW_H
#define TAGVIEW_H

#include <QScrollArea>
#include <nix.hpp>
#include <QModelIndex>
#include <QMap>
#include <QListWidget>
#include "model/NixModelItem.hpp"
#include "utils/entitydescriptor.h"
#include "utils/tagcontainer.h"
#include "utils/datacontroller.h"

namespace Ui {
class TagView;
}

class PlotWidget;

class TagView : public QScrollArea
{
    Q_OBJECT

public:
    explicit TagView(QWidget *parent = 0);
    ~TagView();

    void dataSource(const EntityInfo &info);
    void clear();

public slots:
    void referenceSelected(int i);
    void featureSelected(int i);
    void showTagInfo();

private:
    Ui::TagView *ui;
    QMap<int, int> feature_map, reference_map;
    //TagContainer tag;
    EntityInfo data_src;

    void fillReferences();
    void fillFeatures();
    void clearReferences();
    void clearFeatures();
};

#endif // TAGVIEW_H
