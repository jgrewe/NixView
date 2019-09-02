#include "chartwidget.h"
#include "ui_chartwidget.h"
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
        drawable = drawable && data_source.shape.size() < 3;
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
    if (data_source.nix_type == NixType::NIX_MTAG || data_source.nix_type == NixType::NIX_MTAG)
        plot_tag(data_source);
    else {
        switch (data_source.suggested_plotter) {
        case PlotterType::Line:
            plot_series(data_source);
            break;
        case PlotterType::Event:
            plot_events(data_source);
            break;
        case PlotterType::Category:
            plot_bars(data_source);
            break;
        default:
            setCurrentIndex(0);
            break;
        }
    }
    return true;
}


void ChartWidget::plot_tag(const EntityInfo &data_source) {
    if (data_source.nix_type == NixType::NIX_TAG || data_source.nix_type == NixType::NIX_MTAG) {
        setCurrentIndex(2);
    } else {
        setCurrentIndex(0);
    }
    // FIXME
}

void ChartWidget::plot_bars(const EntityInfo &data_source) {

}


void ChartWidget::plot_series(const EntityInfo &data_source) {
    if (data_source.shape.size() > 2) {
         std::cerr << "chartwidget cannot lineplot more than 2D" << std::endl;
         return;
    }
    if (!check_dimensions(data_source)) {
        std::cerr << "problem with data dimensionality" << std::endl;
        return;
    }
    QtCharts::QChartView *cv = new QtCharts::QChartView();
    cv->setRubberBand(QtCharts::QChartView::RubberBand::RectangleRubberBand);
    ui->charts->layout()->addWidget(cv);
    this->widgets.push_back(cv);


    if (data_source.shape.size() == 1) {
        plot_series_1D(data_source, cv);
    } else if (data_source.shape.size() == 2) {
        plot_series_2D(data_source, cv);
    }
}


void ChartWidget::plot_series_data(const EntityInfo &data_source, QtCharts::QChartView *cv, nix::ndsize_t dim, nix::NDSize count,
                                   nix::NDSize offset, std::string &series_label) {
    std::vector<double> data(count.nelms(), 0.0);
    std::vector<double> xdata = dc.axisData(data_source, dim -1 ).toStdVector();

    dc.getData(data_source, data_source.dtype, data.data(), count, offset);
    QVector<QPointF> points = toPoints(xdata, data);

    QtCharts::QLineSeries* series = new QtCharts::QLineSeries();
    series->replace(points);
    series->setName(QString::fromStdString(series_label));

    cv->chart()->addSeries(series);
    cv->chart()->createDefaultAxes();
}


void ChartWidget::plot_series_data(const EntityInfo &data_source, QtCharts::QChartView *cv, nix::ndsize_t dim, nix::NDSize count,
                                   nix::NDSize offset, std::vector<double> xdata, std::string &series_label) {
    std::vector<double> data(count.nelms(), 0.0);
    dc.getData(data_source, data_source.dtype, data.data(), count, offset);
    QVector<QPointF> points = toPoints(xdata, data);

    QtCharts::QLineSeries* series = new QtCharts::QLineSeries();
    series->replace(points);
    series->setName(QString::fromStdString(series_label));

    cv->chart()->addSeries(series);
    cv->chart()->createDefaultAxes();
}


void ChartWidget::plot_series_1D(const EntityInfo &data_source, QtCharts::QChartView *cv) {
    nix::ndsize_t dim = guess_best_xdim(data_source);
    nix::NDSize count = data_source.shape;
    nix::NDSize offset(data_source.shape.size(), 0);
    plot_series_data(data_source, cv, dim, count, offset, data_source.name);
}


void ChartWidget::plot_series_2D(const EntityInfo &data_source, QtCharts::QChartView *cv) {
    nix::ndsize_t dim = guess_best_xdim(data_source);
    nix::NDSize count(2, 1);
    count[dim - 1] = data_source.shape[dim - 1];
    nix::NDSize offset(2, 0);
    std::vector<double> xdata = dc.axisData(data_source, dim - 1).toStdVector();

    for (nix::ndsize_t i = 0; i < data_source.shape[2 - dim]; i++) {
        offset[2 - dim] = i;
        label = data_source.name + "" + nix::util::numToStr(i); // FIXME is SET Dimension with labels? Use them
        plot_series_data(data_source, cv, dim, count, offset, xdata, label);
    }
}


void ChartWidget::plot_events(const EntityInfo &data_source) {

}
