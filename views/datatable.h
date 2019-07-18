#ifndef DATATABLE_H
#define DATATABLE_H

#include <QWidget>
#include <QModelIndex>
#include <nix.hpp>
#include "model/NixModelItem.hpp"
#include "model/nixarraytablemodel.h"
#include <QStandardItemModel>
#include <QStringList>
#include "utils/entitydescriptor.h"
#include "utils/datacontroller.h"
#include <QTableView>


namespace Ui {
class DataTable;
}


struct DataSource {
    EntityInfo src_info;
    nix::NDSize offset, count, shape;

    DataSource(){};
    DataSource(const EntityInfo &info);
};


class DataTable : public QWidget
{
    Q_OBJECT

public:
    explicit DataTable(QWidget *parent = 0);
    ~DataTable();

    void dataSource(const EntityInfo &info);
    bool canDraw(const EntityInfo &info) const;
    QTableView* get_table();
    EntityInfo dataSource();
    int currentPage();

private:
    Ui::DataTable *ui;
    NixArrayTableModel *model;
    DataSource data_source;
    nix::ndsize_t page_count;
    static const int max_points = 5000000;

    void build_model(int page = 0);

public slots:
    void select_page();
    void previous_page();
    void next_page();
};

#endif // DATATABLE_H
