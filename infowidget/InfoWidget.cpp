#include "InfoWidget.hpp"
#include "ui_InfoWidget.h"
#include <sstream>
#include "common/Common.hpp"
#include "views/MainViewWidget.hpp"
#include <QDebug>
#include "utils/datacontroller.h"
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>


InfoWidget::InfoWidget(QWidget *parent) :
    QWidget(parent), ui(new Ui::InfoWidget) {
    ui->setupUi(this);

    mp = new MetaDataPanel(this);
    ui->verticalLayout_page_meta->addWidget(mp);

    tp = new TagPanel(this);
    ui->verticalLayout_page_tag->addWidget(tp);

    //tv = new TagView(this);
    //ui->tagViewLayout->addWidget(tv);

    dp = new DescriptionPanel(this);
    ui->verticalLayout_page_info->addWidget(dp);

    //pw = new PlotWidget(this);

    ui->tabWidget->setCurrentIndex(2);

    connect_widgets();
}


void InfoWidget::update_info_widget(QModelIndex qml) {
    mp->updateMetadataPanel(qml);
    tp->update_tag_panel(qml);
    dp->update_description_panel(qml);
    // tv->setEntity(qml);
}


void InfoWidget::metadata_column_state_change(QString column, bool visible){
    mp->setColumnState(column, visible);
}


void InfoWidget::plotEntity(const EntityInfo &info) {
    QtCharts::QChartView *cv = new QtCharts::QChartView();
    //QChartView *cv = new QChartView();
    ui->plotWidget->layout()->addWidget(cv);//dataSource(info);
    QtCharts::QLineSeries* series = new QtCharts::QLineSeries();
    series->append(0, 6);
    series->append(2, 4);
    cv->chart()->addSeries(series);
    cv->chart()->createDefaultAxes();
}


void InfoWidget::connect_widgets() {
//    QObject::connect(mp->get_tree_widget(), SIGNAL(expanded(QModelIndex)), mp, SLOT(resize_to_content(QModelIndex)));
//    QObject::connect(mp->get_tree_widget(), SIGNAL(collapsed(QModelIndex)), mp, SLOT(resize_to_content(QModelIndex)));
    QObject::connect(tp->get_tag_table(), SIGNAL(currentCellChanged(int,int,int,int)), tp, SLOT(tag_item_requested(int, int, int, int)));

    QObject::connect(tp->get_reference_tree(), SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
                     tp, SLOT(currentItemChanged_reference_helper(QTreeWidgetItem*,QTreeWidgetItem*)));
    QObject::connect(tp->get_feature_tree(), SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
                     tp, SLOT(currentItemChanged_feature_helper(QTreeWidgetItem*,QTreeWidgetItem*)));
    QObject::connect(tp->get_tag_table(), SIGNAL(currentCellChanged(int,int,int,int)),
                     tp, SLOT(tag_item_requested(int,int,int,int)));
}


const MetaDataPanel* InfoWidget::get_metadata_panel() {
    return mp;
}


InfoWidget::~InfoWidget() {
    delete ui;
}


