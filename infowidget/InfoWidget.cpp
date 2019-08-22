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


QVector<QPointF> toPoints(std::vector<double> &xdata, std::vector<double> &ydata) {
    assert(xdata.size() == ydata.size());
    QVector<QPointF> points(xdata.size());

    for (size_t i = 0; i < xdata.size(); ++i) {
        points.push_back({xdata[i], ydata[i]});
    }
    return points;
}


void InfoWidget::plotEntity(const EntityInfo &info) {
    /*
     * QtCharts::QChartView *cv = new QtCharts::QChartView();
    ui->plotWidget->layout()->addWidget(cv);//dataSource(info);
    QtCharts::QLineSeries* series = new QtCharts::QLineSeries();
    DataController &dc = DataController::instance();
    std::vector<double> data(info.shape.nelms(), 0.0);
    nix::NDSize count = info.shape;
    nix::NDSize offset(info.shape.size(), 0);
    dc.getData(info, info.dtype, data.data(), count, offset);
    std::vector<double> xdata = dc.axisData(info, 0).toStdVector();
    QVector<QPointF> points = toPoints(xdata, data);

    series->replace(points);
    cv->chart()->addSeries(series);
    cv->chart()->createDefaultAxes();
    */
}

void InfoWidget::showData(const EntityInfo &info) {
    std::cerr << "show data table\n";

    if (ui->table_page->canDraw(info)) {
        std::cerr << "show data table\n";
        ui->stackedWidget->setCurrentIndex(1);
        ui->table_page->dataSource(info);
    }
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


