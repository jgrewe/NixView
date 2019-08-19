#ifndef INFOWIDGET_HPP
#define INFOWIDGET_HPP

#include <QWidget>
#include <QTreeWidget>
#include <QtGui>
#include <QVariant>
#include <boost/optional.hpp>
#include <nix.hpp>
#include "DescriptionPanel.hpp"
#include "MetaDataPanel.hpp"
#include "TagPanel.hpp"
#include "views/TagView.hpp"
#include "model/nixtreemodel.h"

//#include "utils/datacontroller.h"

struct EntityInfo;

namespace Ui {
class InfoWidget;
}

class InfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit InfoWidget(QWidget *parent =0);
    ~InfoWidget();

public slots:
    void update_info_widget(QModelIndex qml);
    void metadata_column_state_change(QString column, bool visible);
    void plotEntity(const EntityInfo &info);

private:
    Ui::InfoWidget *ui;

    DescriptionPanel* dp;
    MetaDataPanel* mp;
    TagPanel* tp;
    //TagView* tv;

    void update_meta_info(nix::Section);
    void add_children_to_item(QTreeWidgetItem*, nix::Section);
    void add_properties_to_item(QTreeWidgetItem*, nix::Section);
    void connect_widgets();

public:
    const MetaDataPanel* get_metadata_panel();


};
#endif // INFOWIDGET_HPP
