#include "chartwidget.h"
#include "ui_chartwidget.h"
#include "utils/datacontroller.h"
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QVBoxLayout>

ChartWidget::ChartWidget(QWidget *parent) :
    QStackedWidget(parent),
    ui(new Ui::ChartWidget)
{
    ui->setupUi(this);
    QVBoxLayout *vb = new QVBoxLayout();
    vb->setContentsMargins(0, 0, 0, 0);
    ui->charts->setLayout(vb);
}

ChartWidget::~ChartWidget()
{
    delete ui;
}


bool ChartWidget::can_draw(const EntityInfo &data_source) {
    bool drawable = data_source.nix_type == NixType::NIX_DATA_ARRAY || data_source.nix_type == NixType::NIX_FEAT ||
            data_source.nix_type == NixType::NIX_TAG || data_source.nix_type == NixType::NIX_MTAG;
    if (data_source.nix_type == NixType::NIX_DATA_ARRAY || data_source.nix_type == NixType::NIX_FEAT)
        drawable = drawable && data_source.shape.size() < 2;
    return drawable;
}


void ChartWidget::clear() {
    while(this->widgets.size() > 0) {
        delete widgets.back();
        this->widgets.pop_back();
    }
}


QVector<QPointF> toPoints(std::vector<double> &xdata, std::vector<double> &ydata) {
    assert(xdata.size() == ydata.size());
    QVector<QPointF> points(xdata.size());

    for (size_t i = 0; i < xdata.size(); ++i) {
        points.push_back({xdata[i], ydata[i]});
    }
    return points;
}


bool ChartWidget::add_data(const EntityInfo &data_source) {
    clear();
    if (!can_draw(data_source)) {
        this->setCurrentIndex(0);
        return false;
    }
    this->setCurrentIndex(1);

    switch (data_source.suggested_plotter) {
    case PlotterType::Line:
        plot_series(data_source);
        break;
    case PlotterType::Event:
        plot_events(data_source);
        break;
    default:
        setCurrentIndex(0);
        break;
    }

    return true;
}


void ChartWidget::plot_tag(const EntityInfo &data_source) {
    if (data_source.nix_type == NixType::NIX_TAG || data_source.nix_type == NixType::NIX_MTAG) {
        setCurrentIndex(2);
    } else {
        setCurrentIndex(0);
    }
}


void ChartWidget::plot_series(const EntityInfo &data_source) {
    QtCharts::QChartView *cv = new QtCharts::QChartView();
    ui->charts->layout()->addWidget(cv);//dataSource(info);
    this->widgets.push_back(cv);
    QtCharts::QLineSeries* series = new QtCharts::QLineSeries();
    DataController &dc = DataController::instance();
    std::vector<double> data(data_source.shape.nelms(), 0.0);
    nix::NDSize count = data_source.shape;
    nix::NDSize offset(data_source.shape.size(), 0);
    dc.getData(data_source, data_source.dtype, data.data(), count, offset);
    std::vector<double> xdata = dc.axisData(data_source, 0).toStdVector();
    QVector<QPointF> points = toPoints(xdata, data);

    series->replace(points);
    cv->chart()->addSeries(series);
    cv->chart()->createDefaultAxes();
}


void ChartWidget::plot_events(const EntityInfo &data_source) {

}
