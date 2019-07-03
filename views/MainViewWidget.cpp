#include <QMessageBox>
#include "views/MainViewWidget.hpp"
#include "ui_MainViewWidget.h"
#include "common/Common.hpp"
#include "model/nixtreemodel.h"
#include "utils/datacontroller.h"

NixTreeModel *MainViewWidget::CURRENT_MODEL = nullptr;

MainViewWidget::MainViewWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainViewWidget)
{
    ui->setupUi(this);
    tv = nullptr;
    nix_model = nullptr;
    nix_proxy_model = nullptr;
    iw = nullptr;
    cv = nullptr;
    populate_data_stacked_widget();
}


bool MainViewWidget::refresh() {
    bool result = false;
    if (tv == nullptr)
        populate_data_stacked_widget();

    DataController& dc = DataController::instance();
    if (!dc.valid()) {
        emit emit_model_update(nix_model);
        emit update_file(QString(""));
        return result;
    }
    nix_model = dc.tree_model();
    nix_proxy_model = new NixProxyModel(this);
    tv->getTreeView()->setModel(nix_proxy_model);
    tv->getTreeView()->setSortingEnabled(true);
    emit emit_model_update(nix_model);
    QObject::connect(tv->getTreeView(), SIGNAL(clicked(QModelIndex)), this, SLOT(emit_current_qml_worker_slot(QModelIndex)));
    QObject::connect(tv->getTreeView(), SIGNAL(expanded(QModelIndex)), tv, SLOT(resizeRequest()));
    QObject::connect(tv->getTreeView(), SIGNAL(collapsed(QModelIndex)), tv, SLOT(resizeRequest()));
    QObject::connect(tv->getTreeView()->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), this, SLOT(emit_current_qml_worker_slot(QModelIndex, QModelIndex)));
    QObject::connect(cv->get_column_view(), SIGNAL(clicked(QModelIndex)), this, SLOT(emit_current_qml_worker_slot(QModelIndex)));
    result = true;
    nix_proxy_model->setSourceModel(nix_model);
    MainViewWidget::CURRENT_MODEL = nix_model;
    cv->set_proxy_model(nix_proxy_model);
    tv->setColumns();
    return result;
}


void MainViewWidget::clear() {
    delete nix_proxy_model;
    nix_proxy_model = nullptr;
    emit emit_model_update(nix_model);
    delete cv;
    cv = nullptr;
    delete tv;
    tv = nullptr;
    populate_data_stacked_widget();
}


void MainViewWidget::populate_data_stacked_widget() {
    // order of adding views has to be consistent with integers defined in Common.hpp
    // 0
    tv = new LazyLoadView();
    ui->data_stacked_Widget->addWidget(tv);
    // 1
    cv = new ColumnView(this);
    ui->data_stacked_Widget->addWidget(cv);
    ui->data_stacked_Widget->setCurrentIndex(0);
}


ColumnView* MainViewWidget::get_cv() {
    return cv;
}


LazyLoadView *MainViewWidget::getTreeView() {
    return tv;
}

// slots
void MainViewWidget::set_view(int index) {
    ui->data_stacked_Widget->setCurrentIndex(index);
}


void MainViewWidget::activate_info_widget() {
    ui->horizontalLayout->addWidget(iw);
}


void MainViewWidget::emit_current_qml_worker_slot(QModelIndex qml) {
    emit emit_current_qml(nix_proxy_model->mapToSource(qml));
    tv->resizeRequest();
}


void MainViewWidget::emit_current_qml_worker_slot(QModelIndex qml, QModelIndex prev) {
    emit_current_qml_worker_slot(qml);
}

void MainViewWidget::scan_progress() {
    emit scan_progress_update();
}


void MainViewWidget::close_nix_file() {
    clear();
}

int MainViewWidget::get_scan_progress() {
    //return nix_model->progress();
    return 100;
}


MainViewWidget::~MainViewWidget() {
    delete nix_proxy_model;
    delete ui;
}
