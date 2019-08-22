#include "datatable.h"
#include "ui_datatable.h"
#include "common/Common.hpp"
#include "MainViewWidget.hpp"
#include <vector>

DataSource::DataSource(const EntityInfo &info) {
    src_info = info;
}

DataTable::DataTable(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DataTable), model(), page_count(0)
{
    ui->setupUi(this);
    ui->navigation_widget->setVisible(false);
    ui->back_btn->setEnabled(false);
    ui->next_btn->setEnabled(false);
}


DataTable::~DataTable()
{
    delete model;
    delete ui;
}


bool DataTable::canDraw(const EntityInfo &info) const {
    std::cerr << "can I show data!" << info.name << std::endl;

    if (info.nix_type != NixType::NIX_DATA_ARRAY && info.nix_type != NixType::NIX_FEAT) {
        std::cerr << "nixtype does not match"  << std::endl;
        return false;
    }
    if (info.shape.size() > 3){
        std::cerr << "shape does not match" << std::endl;
        return false;
    }
    if (!nix::data_type_is_numeric(info.dtype)) {
        std::cerr << info.dtype << std::endl;
        return false;
    }
    std::cerr << "can show data!" << std::endl;
    return true;
}


void DataTable::dataSource(const EntityInfo &info) {
    data_source = DataSource(info);
    if (data_source.src_info.shape.size() > 2) {
        page_count = data_source.src_info.shape[2];
        ui->navigation_widget->setVisible(true);
        ui->total_count_label->setText(QString::fromStdString(nix::util::numToStr(data_source.src_info.shape[2])));
        ui->current_page->setText(QVariant(1).toString());
        ui->current_page->setValidator(new QIntValidator(0, (int)data_source.src_info.shape[2]));
        ui->next_btn->setEnabled(true);
    } else {
        page_count = 1;
    }
    build_model();
    ui->description->setText(QString::fromStdString(info.description));
}


void DataTable::next_page() {
    int curr_page = QVariant(ui->current_page->text()).toInt();
    curr_page++;
    ui->current_page->setText(QVariant(curr_page).toString());
    select_page();
}


void DataTable::previous_page() {
    int curr_page = QVariant(ui->current_page->text()).toInt();
    curr_page--;
    ui->current_page->setText(QVariant(curr_page).toString());
    select_page();
}


void DataTable::select_page() {
    int curr_page = QVariant(ui->current_page->text()).toInt();
    ui->next_btn->setEnabled(curr_page < page_count);
    ui->back_btn->setEnabled(curr_page > 1);
    build_model(curr_page - 1);
}

int DataTable::currentPage() {
    return QVariant(ui->current_page->text()).toInt();
}


void DataTable::build_model(int page) {
    if (this->model != nullptr) {
        delete model;
        model = nullptr;
    }

    model = new NixArrayTableModel(this);
    model->set_source(data_source.src_info, page);
    ui->table->setModel(model);
    ui->table->setSelectionMode(QAbstractItemView::ContiguousSelection);
}


QTableView* DataTable::get_table() {
    return ui->table;
}


EntityInfo DataTable::dataSource() {
    return this->data_source.src_info;
}

/*
DataSource::DataSource(std::string name, std::string id, nix::NDSize shape)
    : offset(shape.size()), count(shape.size()) {
    this->name = name;
    this->id = id;
    this->shape = shape;
}
*/
