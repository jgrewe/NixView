#include "categoryplotter.h"
#include "ui_categoryplotter.h"
#include <nix.hpp>

CategoryPlotter::CategoryPlotter(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CategoryPlotter),
    cmap()
{
    ui->setupUi(this);
}

CategoryPlotter::~CategoryPlotter()
{
    delete ui;
}


void CategoryPlotter::draw(const EntityInfo &data_source) {
    if (data_source.shape.size() > 2) {
        std::cerr << "CategoryPlotter::draw cannot draw 3D!" << std::endl;
        return;
    }
    if (!Plotter::check_plottable_dtype(nix::string_to_data_type((data_source.dtype.toString().toStdString())))) {
        std::cerr << "CategoryPlotter::draw cannot handle data type " << data_source.dtype.toString().toStdString() << std::endl;
        return;
    }
    if (!check_dimensions(data_source)) {
        std::cerr << "CategroyPlotter::draw cannot handle dimensionality of the data" << std::endl;
        return;
    }
    if (data_source.shape.size() == 1) {
        draw_1d(data_source);
    } else {
        draw_2d(data_source);
    }
}


void CategoryPlotter::draw_1d(const EntityInfo &data_source) {
    QVector<double> x_axis = dc.axisData(data_source, 0);
    QVector<double> values(static_cast<int>(data_source.shape.nelms()), 0.0);
    nix::NDSize offset(data_source.shape.size(), 0);
    dc.getData(data_source, nix::DataType::Double, values.data(), data_source.shape, offset);
    QVector<QString> x_tick_labels;
    if (data_source.dim_types[0] == nix::DimensionType::Set) {
        x_tick_labels = dc.axisStringData(data_source, 0);
    }

    std::vector<std::string> labels = dc.axisLabels(data_source);
    QString y_label;
    QVector<QString> ax_labels;
    add_bar_plot(x_tick_labels, values, QString::fromStdString(data_source.name));
    set_label(data_source.name);
    //set_ylabel(data_source.(array.label() ? *array.label() : "") + (array.unit() ? (" [" + *array.unit() + "]") : ""));
}


void CategoryPlotter::draw_2d(const EntityInfo &data_source) {
    size_t best_dim = guess_best_xdim(data_source);
    std::vector<std::string> axis_labels = dc.axisLabels(data_source);
    QVector<double> xaxis = dc.axisData(data_source, best_dim);
    QVector<double> yaxis = dc.axisData(data_source, 1-best_dim);

    double ymin = 0.0;
    double ymax = 0.0;
    QCPBarsGroup *group = new QCPBarsGroup(ui->plot);
    nix::NDSize count(2, 1);
    count[best_dim] = data_source.shape[best_dim];
    nix::NDSize offset(2, 0);
    for (size_t i = 0; i < data_source.shape[1-best_dim]; i++) {
        offset[1-best_dim] = i;
        QVector<double> data(static_cast<int>(count[best_dim]), 0.0);
        dc.getData(data_source, nix::DataType::Double, data.data(), count, offset);

        QCPBars *bars = new QCPBars(ui->plot->xAxis, ui->plot->yAxis);
        bars->setData(xaxis, data);
        QColor c = cmap.next();
        bars->setPen(c);
        c.setAlpha(50);
        bars->setBrush(c);
        bars->setWidth(0.15);
        bars->setBarsGroup(group);
        //bars->setName(ylabels[i]);
        double mi = *std::min_element(std::begin(data), std::end(data));
        double ma = *std::max_element(std::begin(data), std::end(data));
        if (mi < ymin)
            ymin = mi;
        if (ma > ymax)
            ymax= ma;
    }

    //QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
    //textTicker->setSubTickCount(0);
    //textTicker->addTicks(xaxis,xlabels);
    //ui->plot->xAxis->setTicker(textTicker);

    ui->plot->xAxis->grid()->setVisible(true);
    ui->plot->legend->setVisible(true);

    ui->plot->xAxis->setRange(-0.5, xaxis.size());
    ui->plot->yAxis->setRange(1.5 * ymin, 1.5 * ymax);
    ui->plot->xAxis->setTickLength(0, 4);
    ui->plot->xAxis->setTickLabelRotation(60);

    set_label(data_source.name);

    /*
    std::string unit = "";
    if (array.label())
        set_ylabel(*array.label() + (array.unit() ? (" [" + *array.unit() + "]") : ""));
        */
        /*
    QVector<QString> xlabels, ylabels;
    get_data_array_axis(array, xaxis, xlabels, best_dim);
    get_data_array_axis(array, yaxis, ylabels, 3-best_dim);

        */
}


bool CategoryPlotter::check_dimensions(const EntityInfo &data_source) const {
    if (data_source.dim_types.size() == 0) {
        return false;
    }
    if (data_source.dim_types.size() == 1) {
        return true;
    }
    nix::DimensionType dt_1 = data_source.dim_types[0];
    nix::DimensionType dt_2 = data_source.dim_types[1];
    if (((dt_1 == nix::DimensionType::Sample || dt_1 == nix::DimensionType::Range) && dt_2 == nix::DimensionType::Set) ||
            (dt_1 == nix::DimensionType::Set && (dt_2 == nix::DimensionType::Range || dt_2 == nix::DimensionType::Range))) {
        std::cerr << "CategoryPlotter should draw 2D data with Range or SampleDimension? You serious? I can do this, but seems suboptimal..." << std::endl;
        return true;
    }
    if (dt_1 == nix::DimensionType::Set && dt_2 == nix::DimensionType::Set) {
        return true;
    }
    return false;
}


size_t CategoryPlotter::guess_best_xdim(const EntityInfo &data_source) const {
    if (data_source.shape[0] > data_source.shape[1])
        return 0;
    return 1;
}


PlotterType CategoryPlotter::plotter_type() const {
    return PlotterType::Category;
}


void CategoryPlotter::set_label(const std::string &label) {
    ui->label->setText(QString::fromStdString(label));
}


void CategoryPlotter::set_xlabel(const QString &label){
    ui->plot->xAxis->setLabel(label);
    ui->plot->replot();
}

void CategoryPlotter::set_xlabel(const std::string &label) {
    set_xlabel(QString::fromStdString(label));
}

void CategoryPlotter::set_ylabel(const QString &label){
    ui->plot->yAxis->setLabel(label);
    ui->plot->replot();
}

void CategoryPlotter::set_ylabel(const std::string &label) {
    set_ylabel(QString::fromStdString(label));
}

void CategoryPlotter::add_bar_plot(QVector<QString> categories, QVector<double> y_data, const QString &name){
    QCPBars *bars = new QCPBars(ui->plot->xAxis, ui->plot->yAxis);

    QPen pen;
    pen.setColor(QColor(150, 222, 0));
    bars->setPen(pen);
    bars->setBrush(QColor(150, 222, 0, 70));
    QVector<double> ticks;
    for (int i = 0; i < categories.size(); ++i)
        ticks.push_back(i);

    QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
    textTicker->setSubTickCount(0);
    textTicker->addTicks(ticks,categories);
    ui->plot->xAxis->setTicker(textTicker);

    ui->plot->xAxis->grid()->setVisible(true);
    ui->plot->legend->setVisible(true);

    ui->plot->xAxis->setTickLabelRotation(60);
    ui->plot->xAxis->setTickLength(0, 4);
    ui->plot->xAxis->grid()->setVisible(true);
    ui->plot->xAxis->setRange(-0.5, categories.size() - 0.5);

    ui->plot->yAxis->setPadding(5);
    ui->plot->yAxis->grid()->setSubGridVisible(true);
    double y_max = *std::max_element(std::begin(y_data), std::end(y_data));
    ui->plot->yAxis->setRange(0, 1.05 * y_max);

    QPen gridPen;
    gridPen.setStyle(Qt::SolidLine);
    gridPen.setColor(QColor(0, 0, 0, 25));
    ui->plot->yAxis->grid()->setPen(gridPen);
    gridPen.setStyle(Qt::DotLine);
    ui->plot->yAxis->grid()->setSubGridPen(gridPen);

    // Add data:
    bars->setData(ticks, y_data);
    bars->setName(name);
}



void CategoryPlotter::add_events(const QVector<double> &x_data, const QVector<double> &y_data, const QString &name, bool y_scale) {
    /*
    ui->plot->addGraph();
    ui->plot->graph()->addData(x_data, y_data);
    ui->plot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
    ui->plot->graph()->setLineStyle(QCPGraph::LineStyle::lsNone);
    ui->plot->graph()->setName(name);
    ui->plot->graph()->setPen(QPen(Qt::red));
    if (y_scale) {
        double y_min = *std::min_element(std::begin(y_data), std::end(y_data));
        double y_max = *std::max_element(std::begin(y_data), std::end(y_data));
        if (y_min == y_max)
            y_min = 0.0;

        ui->plot->yAxis->setRange(1.05*y_min, 1.05*y_max);
    }
    ui->plot->xAxis->setRange(x_data[0], x_data.last());
    */
}


void CategoryPlotter::add_segments(const QVector<double> &positions, const QVector<double> &extents, const QString &name) {
    /*
    if (positions.size() != extents.size()) {
        std::cerr << "Cannot draw segments, number of positions and extents does not match!" << std::endl;
    }
    for (int i = 0; i<positions.size(); i++) {
        QCPItemRect *rect = new QCPItemRect(ui->plot);
        double x_min, x_max, y_min, y_max;
        y_max = ui->plot->yAxis->range().upper;
        y_min = ui->plot->yAxis->range().lower;
        x_min = positions[i];
        x_max = x_min + extents[i];
        rect->position("topLeft")->setCoords(x_min, y_max);
        rect->position("bottomRight")->setCoords(x_max, y_min);
        rect->setPen(QPen(Qt::red));
        rect->setBrush(QBrush(QColor(255, 10, 10, 50)));
        ui->plot->addItem(rect);
    }
    */
}


QCustomPlot* CategoryPlotter::get_plot() {
    return ui->plot;
}
