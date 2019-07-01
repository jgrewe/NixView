#ifndef MAINVIEWWIDGET_HPP
#define MAINVIEWWIDGET_HPP

#include <QWidget>
#include <QtGui>
#include <infowidget/InfoWidget.hpp>
#include <boost/optional.hpp>
#include "views/RawTreeView.hpp"
#include "views/ColumnView.hpp"
#include "model/NixDataModel.hpp"
#include "filter/NixProxyModel.hpp"
#include "views/lazyloadview.h"
#include "model/nixtreemodel.h"
#include "QShortcut"

namespace Ui {
class MainViewWidget;
}

class MainViewWidget : public QWidget
{
    Q_OBJECT

private:
    static NixTreeModel *CURRENT_MODEL;

public:
    explicit MainViewWidget(QWidget *parent = 0);
    ~MainViewWidget();

    ColumnView *get_cv();
    LazyLoadView* getTreeView();
    static NixTreeModel* get_current_model() {return CURRENT_MODEL; }
    int get_scan_progress();
    bool refresh();
    void clear();

public slots:
    void set_view(int);
    void activate_info_widget();
    void emit_current_qml_worker_slot(QModelIndex qml);
    void emit_current_qml_worker_slot(QModelIndex qml, QModelIndex prev);
    void scan_progress();
    void close_nix_file();

signals:
    void emit_model_update(NixTreeModel*);
    void scan_progress_update();
    void emit_current_qml(QModelIndex);
    void update_file(QString);
    void close_file();

private:
    Ui::MainViewWidget *ui;
    NixTreeModel *nix_model;
    //QSortFilterProxyModel *nix_proxy_model;
    //NixDataModel *nix_model;
    NixProxyModel *nix_proxy_model;

    //RawTreeView *rtv;
    ColumnView *cv;
    LazyLoadView *tv;
    InfoWidget *iw;

    QShortcut *shortcut_filter;

    void populate_data_stacked_widget();
};

#endif // MAINVIEWWIDGET_HPP
