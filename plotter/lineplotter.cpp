#include "lineplotter.h"
#include "ui_lineplotter.h"
#include "utils/datacontroller.h"
#include <nix/NDArray.hpp>
#include <QMenu>
#include <limits>

LinePlotter::LinePlotter(QWidget *parent, size_t numOfPoints) :
    QWidget(parent), ui(new Ui::LinePlotter), cmap(), totalXRange(0,0), totalYRange(0,0) {
    ui->setupUi(this);
    // connect slot that ties some axis selections together (especially opposite axes):
    connect(ui->plot, SIGNAL(selectionChangedByUser()), this, SLOT(selection_changed()));
    // connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
    connect(ui->plot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mouse_press()));
    connect(ui->plot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouse_wheel()));
    // make bottom and left axes transfer their ranges to top and right axes:
    connect(ui->plot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->plot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->plot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->plot->yAxis2, SLOT(setRange(QCPRange)));
    // setup policy and connect slot for context menu popup:
    ui->plot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->plot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(context_menu_request(QPoint)));
    ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables | QCP::iSelectAxes);

    connect(ui->plot->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(xAxisNewRange(QCPRange)));
    connect(ui->plot->yAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(yAxisNewRange(QCPRange)));

    qRegisterMetaType<QVector<double>>();
    connect(ui->plot->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(testThreads(QCPRange)));

    this->numOfPoints = numOfPoints; // standard 100 000
    this->totalYRange = QCPRange(0, 0);
}

LinePlotter::~LinePlotter()
{
    delete ui;
}


PlotterType LinePlotter::plotter_type() const {
    return PlotterType::Line;
}


void LinePlotter::set_label(const std::string &label) {
    //ui->label->setText(QString::fromStdString(label));
}


void LinePlotter::set_xlabel(const std::string &label) {
    set_xlabel(QString::fromStdString(label));
}


void LinePlotter::set_xlabel(const QString &label){
    ui->plot->xAxis->setLabel(label);
    ui->plot->replot();
}


void LinePlotter::set_ylabel(const std::string &label) {
    set_ylabel(QString::fromStdString(label));
}


void LinePlotter::set_ylabel(const QString &label){
    ui->plot->yAxis->setLabel(label);
    ui->plot->replot();
}


void LinePlotter::draw(const EntityInfo &info) {
    nix::DataType dt = nix::string_to_data_type(info.dtype.toString().toStdString());
    if (!Plotter::check_plottable_dtype(dt)) {
        std::cerr << "LinePlotter::draw cannot handle data type " << info.dtype.toString().toStdString() << std::endl;
        return;
    }
    if (!check_dimensions(info)) {
        std::cerr << "LinePlotter::draw cannot handle dimensionality of the data" << std::endl;
        return;
    }
    data_sources.append(info);
    //arrays.append(array);
    //loaders.append(new LoadThread());

    //LoadThread *loader = loaders.last();

    //connect(loader, SIGNAL(dataReady(const QVector<double> &, const QVector<double> &, int)), this, SLOT(drawThreadData(const QVector<double> &, const QVector<double> &, int)));
    //connect(loader, SIGNAL(progress(double)), this, SLOT(printProgress(double)));

    if (info.shape.size() == 1) {
        draw_1d(info);
    } else {
        draw_2d(info);
    }
}


void LinePlotter::draw_1d(const EntityInfo &info) {
    DataArrayInfo ai = dc.getArrayInfo(info);

    QString y_label = QString::fromStdString(ai.label);
    QVector<QString> ax_labels;
    for (std::string s : ai.dimension_labels) {
        ax_labels.push_back(QString::fromStdString(s));
    }

    this->set_ylabel(y_label);
    this->set_xlabel(ax_labels[0]);
    this->set_label(ai.name);

    if (info.dim_types[0] == nix::DimensionType::Set) {
        QVector<double> x_data = dc.axisData(info, 0);
        QVector<double> y_data(x_data.size(), 0.0);
        QVector<QString> x_tick_labels;
        this->add_line_plot(x_data, y_data, info.name.toString());
        // plot 1d set data in a meaningfull way.
        //   ?? this->add_line_plot(x_axis, y_axis, QString::fromStdString(array.name()));
    } else {
        // int newGraphIndex = ui->plot->graphCount();
        expandXRange(info, 1);
        ui->plot->addGraph();
        ui->plot->graph()->setPen(QPen(cmap.next()));
        nix::NDSize start(1);
        start[0] = 0;

        nix::ndsize_t length = info.shape[0];
        if(length > numOfPoints) {
            length = numOfPoints;
        }
        nix::NDSize extent(1);
        extent[0] = length;
        std::vector<double> data(info.shape.nelms(), 0.0);
        nix::NDSize count = info.shape;
        nix::NDSize offset(info.shape.size(), 0);
        dc.getData(info, nix::string_to_data_type(info.dtype.toString().toStdString()), data.data(), count, offset);
        QVector<double> xdata = dc.axisData(info, 0);
        QVector<double> ydata = QVector<double>::fromStdVector(data);

        this->add_line_plot(xdata, ydata, info.name.toString());
    }
}


void LinePlotter::draw_2d(const EntityInfo &info) {
    QVector<double> x_data, y_data;
    nix::ndsize_t best_dim = guess_best_xdim(info);

    x_data = dc.axisData(info, best_dim-1);
    QVector<QString> axis_labels;
    std::vector<std::string> l = dc.axisLabels(info);
    for (auto s : l) {
        axis_labels.push_back(QString::fromStdString(s));
    }

    QVector<QString> line_labels = dc.axisStringData(info, 2-best_dim);
    nix::NDSize count(2, 1);
    count[best_dim - 1] = info.shape[best_dim - 1];
    nix::NDSize offset(2, 0);

    for (nix::ndsize_t i=0; i < info.shape[2-best_dim]; i++) {
        y_data.resize(info.shape[best_dim - 1]);
        offset[2-best_dim] = i;
        dc.getData(info, nix::DataType::Double, y_data.data(), count, offset);
        add_line_plot(x_data, y_data, line_labels[i]);
    }
    /*
    QString y_label;
    QVector<QString> ax_labels;
    data_array_ax_labels(array, y_label, ax_labels);
    this->set_ylabel(y_label);
    this->set_xlabel(ax_labels[best_dim-1]);
    */

    //int firstGraphIndex = ui->plot->graphCount();

    //expandXRange(info, best_dim);
}


nix::ndsize_t LinePlotter::guess_best_xdim(const EntityInfo &info) const {
    if(info.shape.size() == 0) {
        throw nix::IncompatibleDimensions("Array has dataExtent().size 0.", "guess_best_xdim");
    }

    if(info.shape.size() > 2) {
        throw nix::IncompatibleDimensions("Array has more than two dimensions.", "guess_best_xdim");
    }

    if(info.shape.size() == 1) {
        return 1;
    } else { //(array.dataExtent().size() == 2) {
        nix::DimensionType d_1 = info.dim_types[0];
        nix::DimensionType d_2 = info.dim_types[1];

        if(d_1 == nix::DimensionType::Sample) {
            return 1;
        } else if (d_2 == nix::DimensionType::Sample) {
            return 2;
        } else {
            if(d_1 == nix::DimensionType::Set && d_2 == nix::DimensionType::Range) {
                return 2;
            } else if (d_1 == nix::DimensionType::Range && d_2 == nix::DimensionType::Set){
                return 1;
            } else {
                std::cerr << "How did you get with 2D Set Data to guess_best_xdims() in the Lineplotter?" << std::endl;
                throw nix::IncompatibleDimensions("Array contains 2D set data.", "guess_best_xdim");
            }
        }
    }
}


bool LinePlotter::check_dimensions(const EntityInfo &info) const {
    if (info.shape.size() > 2) {
        std::cerr << "LinePlotter::check_dimensions cannot draw 3D!" << std::endl;
        return false;
    }

    if (info.shape.size() == 0 || info.shape.size() > 2) {
        return false;
    }

    if (info.shape.size() == 1) {
        return true;
    }

    if (info.dim_types.size() != info.shape.size()) {
        std::cerr << "LinePlotter: mismatch of dimensionality of data and number of dimension descriptors!" << std::endl;
            return false;
     }

    nix::DimensionType dt_1 = info.dim_types[0];
    nix::DimensionType dt_2 = info.dim_types[1];

    if ((dt_1 == nix::DimensionType::Sample || dt_1 == nix::DimensionType::Range) && dt_2 == nix::DimensionType::Set) {
        return true;
    }

    if (dt_1 == nix::DimensionType::Set && (dt_2 == nix::DimensionType::Range || dt_2 == nix::DimensionType::Sample)) {
        return true;
    }
    if (dt_1 == nix::DimensionType::Set && dt_2 == nix::DimensionType::Set) {
        std::cerr << "LinePlotter should draw 2D Set data? You serious?" << std::endl;
        return false;
    }
    return false;
}


void LinePlotter::add_line_plot(const QVector<double> &xData, const QVector<double> &yData, const QString &name) {
    ui->plot->addGraph();
    QPen pen;
    pen.setColor(cmap.next());
    ui->plot->graph()->setPen(pen);
    ui->plot->graph()->addData(xData, yData);

    setXRange(xData);
    setYRange(yData);

    ui->plot->graph()->setName(name);
    ui->plot->replot();
}


void LinePlotter::add_events(const QVector<double> &xData, const QVector<double> &yData, const QString &name, bool yScale) {
    ui->plot->addGraph();
    ui->plot->graph()->addData(xData, yData);
    ui->plot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
    ui->plot->graph()->setLineStyle(QCPGraph::LineStyle::lsNone);
    ui->plot->graph()->setName(name);
    ui->plot->graph()->setPen(QPen(Qt::red));
    if (yScale) {
        setYRange(yData);
    }
    setXRange(xData);
}


void LinePlotter::add_events(const QVector<double> &xData, const QString &name, bool yScale) {
    double max = ui->plot->yAxis->range().upper * 0.9;
    QVector<double> yData(xData.size(), max);
    add_events(xData, yData, name, yScale);
}


void LinePlotter::add_segments(const QVector<double> &positions, const QVector<double> &extents, const QString &name) {
    if (extents.size() > 0 && (positions.size() != extents.size())) {
        std::cerr << "Cannot draw segments, number of positions and extents does not match!" << std::endl;
        return;
    }
    for (int i = 0; i<positions.size(); i++) {
        QCPItemRect *rect = new QCPItemRect(ui->plot);
        double x_min, x_max, y_min, y_max;
        y_max = ui->plot->yAxis->range().upper;
        y_min = ui->plot->yAxis->range().lower;
        x_min = positions[i];
        if (extents.size() > 0)
            x_max = x_min + extents[i];
        else
            x_max = positions[i];

        rect->position("topLeft")->setCoords(x_min, y_max);
        rect->position("bottomRight")->setCoords(x_max, y_min);
        rect->setPen(QPen(Qt::red));
        rect->setBrush(QBrush(QColor(255, 10, 10, 50)));
    }
}


QCustomPlot* LinePlotter::get_plot() {
    return ui->plot;
}

void LinePlotter::expandXRange(const EntityInfo &info, nix::ndsize_t xDim) {
    nix::ndsize_t dimI = xDim-1;

    nix::ndsize_t maxLoad = numOfPoints;
    if (info.shape[dimI] < numOfPoints) {
        maxLoad = info.shape[dimI];
    }
    double min, max;
    min = dc.axisData(info, dimI, 0)[0];
    max = dc.axisData(info, dimI, maxLoad-1)[0];
    QCPRange range(min, max);
    ui->plot->xAxis->setRange(range);
}


void LinePlotter::setXRange(QVector<double> xData) {
    totalXRange.expand(QCPRange(xData[0],xData.last()));
    if (numOfPoints < std::numeric_limits<int>::max()) {
        int nop = static_cast<int>(numOfPoints);
        if (nop < xData.size() && nop != 0) {
            ui->plot->xAxis->setRange(xData[0], xData[nop]);
        } else {
            ui->plot->xAxis->setRange(totalXRange);
        }
    } else {
        throw std::overflow_error("size_t value cannot be stored in a variable of type int.");
    }
}


void LinePlotter::expandYRange(QVector<double> yData) {
    double yMin = *std::min_element(yData.constBegin(), yData.constEnd());
    double yMax = *std::max_element(yData.constBegin(), yData.constEnd());
    if (std::abs(yMin - yMax) < std::numeric_limits<double>::epsilon())
        yMin = yMax-1;

    totalYRange.expand(QCPRange(yMin, yMax));
}


void LinePlotter::setYRange(QVector<double> yData) {
    double yMin = *std::min_element(yData.constBegin(), yData.constEnd());
    double yMax = *std::max_element(yData.constBegin(), yData.constEnd());
    if (std::abs(yMin - yMax) < std::numeric_limits<double>::epsilon())
        yMin = yMax-1;

    totalYRange.expand(QCPRange(yMin, yMax));
    ui->plot->yAxis->setRange(totalYRange.lower*1.05, totalYRange.upper*1.05);
}


void LinePlotter::drawThreadData(const QVector<double> &data, const QVector<double> &axis, int graphIndex) {
    if(totalYRange == QCPRange(0, 0)) {
        setYRange(data);
    } else {
        expandYRange(data);
    }
    ui->plot->graph(graphIndex)->setData(axis, data, true);
    ui->plot->replot();
}

void LinePlotter::printProgress(double progress) {
    std::cerr << "Loaded: " << progress*100 << "%" << std::endl;
}

void LinePlotter::resetView() {
    QCPDataContainer<QCPGraphData> data = *ui->plot->graph()->data().data();
    int nop;
    if (numOfPoints < std::numeric_limits<int>::max()) {
        nop = static_cast<int>(numOfPoints);
    } else {
        throw std::overflow_error("size_t value cannot be stored in a variable of type int.");
    }
    // reset x Range
    if(nop != 0 && nop < data.size()) {
        QCPGraphData firstPoint = *data.at(0);
        QCPGraphData lastPoint = *data.at(nop);
        QCPRange resetX = QCPRange(firstPoint.sortKey(), lastPoint.sortKey());
        ui->plot->xAxis->setRange(resetX);
    } else {
        ui->plot->xAxis->setRange(totalXRange);
    }

    ui->plot->yAxis->setRange(totalYRange.lower*1.05, totalYRange.upper*1.05);
}


void LinePlotter::testThreads(QCPRange range) {
    if(ui->plot->graphCount() == 0) {
        return;
    }
    /*
    int graphIndex = 0;
    for(int i=0; i<arrays.size(); i++) {
        nix::ndsize_t xDim = guess_best_xdim(data_sources[i]);
        QCPGraph *graph = ui->plot->graph(graphIndex);
        EntityInfo src = data_sources[i];
        if(graph->dataCount() == 0) {
            // with setVariables ?
            //loaders[i]->setVariables();

            if(src.shape.size() == 1) {
                graphIndex += 1;
            } else if(src.shape.size() == 2) {
                graphIndex += src.shape[2-xDim];
            }
            continue;
        }

        double max = graph->dataMainKey(graph->dataCount()-1);
        double min = graph->dataMainKey(0);
        double mean = graph->dataCount() / (max-min);

        //loaders[i]->startLoadingIfNeeded(range, xDim, min, max, mean);

        if(src.shape.size() == 1) {
            graphIndex += 1;
        } else if(src.shape.size() == 2) {
            graphIndex += src.shape[2-xDim];
        }
    }
    */
}


void LinePlotter::xAxisNewRange(QCPRange range) {
    emit xAxisChanged(range,totalXRange);
}

void LinePlotter::yAxisNewRange(QCPRange range) {
    emit yAxisChanged(range, totalYRange);
}

void LinePlotter::changeXAxisPosition(double newCenter) {
    if(std::abs(ui->plot->xAxis->range().center() - newCenter) > std::numeric_limits<double>::epsilon()) {
        ui->plot->xAxis->setRange(newCenter, ui->plot->xAxis->range().size(), Qt::AlignCenter);
        ui->plot->replot();
    }
}

void LinePlotter::changeYAxisPosition(double newCenter) {
    if(std::abs(ui->plot->xAxis->range().center() - newCenter) > std::numeric_limits<double>::epsilon()) {
        ui->plot->yAxis->setRange(newCenter, ui->plot->yAxis->range().size(), Qt::AlignCenter);
        ui->plot->replot();
    }
}

void LinePlotter::changeXAxisSize(double ratio) {
    double xNewSize = totalXRange.size() * ratio;
    if(std::abs(ui->plot->xAxis->range().size() - xNewSize) > std::numeric_limits<double>::epsilon()) {
        ui->plot->xAxis->setRange(ui->plot->xAxis->range().center(), xNewSize, Qt::AlignCenter);
        ui->plot->replot();
    }
}


void LinePlotter::selection_changed() {
    // make top and bottom axes be selected synchronously, and handle axis and tick labels as one selectable object:
    if (ui->plot->xAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->plot->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
            ui->plot->xAxis->selectedParts().testFlag(QCPAxis::spAxisLabel) || ui->plot->xAxis2->selectedParts().testFlag(QCPAxis::spAxis) ||
            ui->plot->xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels) || ui->plot->xAxis2->selectedParts().testFlag(QCPAxis::spAxisLabel))
    {
        ui->plot->xAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels|QCPAxis::spAxisLabel);
        ui->plot->xAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels|QCPAxis::spAxisLabel);
    }
    // make left and right axes be selected synchronously, and handle axis and tick labels as one selectable object:
    if (ui->plot->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->plot->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
            ui->plot->yAxis->selectedParts().testFlag(QCPAxis::spAxisLabel) || ui->plot->yAxis2->selectedParts().testFlag(QCPAxis::spAxis) ||
            ui->plot->yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels) || ui->plot->yAxis2->selectedParts().testFlag(QCPAxis::spAxisLabel))
    {
        ui->plot->yAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels|QCPAxis::spAxisLabel);
        ui->plot->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels|QCPAxis::spAxisLabel);
    }

    // synchronize selection of graphs with selection of corresponding legend items:
    for (int i=0; i<ui->plot->graphCount(); ++i) {
        QCPGraph *graph = ui->plot->graph(i);
        QCPPlottableLegendItem *item = ui->plot->legend->itemWithPlottable(graph);
        if (item->selected() || graph->selected()) {
            item->setSelected(true);
            QCPDataRange wholeGraph = QCPDataRange(0, graph->dataCount());
            QCPDataSelection selection = QCPDataSelection(wholeGraph);
            graph->setSelection(selection);
        }
    }
}


void LinePlotter::mouse_press() {
    // if an axis is selected, only allow the direction of that axis to be dragged
    // if no axis is selected, both directions may be dragged
    if (ui->plot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->plot->axisRect()->setRangeDrag(ui->plot->xAxis->orientation());
    else if (ui->plot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->plot->axisRect()->setRangeDrag(ui->plot->yAxis->orientation());
    else
        ui->plot->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
}


void LinePlotter::mouse_wheel() {
    // if an axis is selected, only allow the direction of that axis to be zoomed
    // if no axis is selected, both directions may be zoomed
    if (ui->plot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->plot->axisRect()->setRangeZoom(ui->plot->xAxis->orientation());
    else if (ui->plot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->plot->axisRect()->setRangeZoom(ui->plot->yAxis->orientation());
    else
        ui->plot->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);

}


void LinePlotter::context_menu_request(QPoint pos) {
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    QAction *legend_action = menu->addAction("Show legend", this, SLOT(show_legend()));
    legend_action->setCheckable(true);
    legend_action->setChecked(ui->plot->legend->visible());
    menu->addAction("Clear selection", this, SLOT(clear_selection()));
    if (ui->plot->selectedGraphs().size() > 0) {
        menu->addAction("Remove selected graph", this, SLOT(remove_selected_graph()));
        QMenu *line_style_menu = menu->addMenu("Line style");
        line_style_menu->addAction("none", this, SLOT(set_pen_none()));
        line_style_menu->addAction("solid", this, SLOT(set_pen_solid()));
        line_style_menu->addAction("dashed", this, SLOT(set_pen_dashed()));
        line_style_menu->addAction("dash-dotted", this, SLOT(set_pen_dashed()));
        line_style_menu->addAction("dotted", this, SLOT(set_pen_dashed()));

        QMenu *marker_menu = menu->addMenu("Marker");
        marker_menu->addAction("none", this, SLOT(marker_none()));
        marker_menu->addAction("circle", this, SLOT(marker_circle()));
        marker_menu->addAction("cross", this, SLOT(marker_cross()));
        marker_menu->addAction("dot", this, SLOT(marker_dot()));
        marker_menu->addAction("diamond", this, SLOT(marker_diamond()));
        marker_menu->addAction("plus", this, SLOT(marker_plus()));
        marker_menu->addAction("Square", this, SLOT(marker_square()));
    }
    menu->popup(ui->plot->mapToGlobal(pos));
}


void LinePlotter::remove_selected_graph() {
    if (ui->plot->selectedGraphs().size() > 0) {
        ui->plot->removeGraph(ui->plot->selectedGraphs().first());
        ui->plot->replot();
    }
}


void LinePlotter::show_legend() {
    ui->plot->legend->setVisible(!ui->plot->legend->visible());
    ui->plot->replot();
}


void LinePlotter::clear_selection(){
    ui->plot->deselectAll();
    ui->plot->replot();
}


void LinePlotter::set_marker(QString marker){
    QCPGraph *graph = ui->plot->selectedGraphs().first();
    if (marker == "none") {
        graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, 0));
    } else if (marker == "circle") {
        graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 10));
    } else if (marker == "diamond") {
        graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDiamond, 10));
    } else if (marker == "dot") {
        graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDot, 10));
    } else if (marker == "cross") {
        graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross, 10));
    } else if (marker == "square") {
        graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssSquare, 10));
    } else if (marker == "plus") {
        graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssPlus, 10));
    } else {
        graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, 10));
    }
    clear_selection();
}


void LinePlotter::marker_circle() {
    set_marker("circle");
}


void LinePlotter::marker_cross() {
    set_marker("cross");
}


void LinePlotter::marker_dot() {
    set_marker("dot");
}


void LinePlotter::marker_diamond() {
    set_marker("diamond");
}

void LinePlotter::marker_square() {
    set_marker("square");
}

void LinePlotter::marker_none(){
    set_marker("none");
}

void LinePlotter::marker_plus(){
    set_marker("plus");
}


void LinePlotter::set_pen_style(QString style) {
    QCPGraph *graph = ui->plot->selectedGraphs().first();
    QPen pen = graph->pen();
    if (style == "none"){
        pen.setStyle(Qt::PenStyle::NoPen);
    } else if (style == "solid") {
        pen.setStyle(Qt::PenStyle::SolidLine);
    } else if (style == "dashed") {
        pen.setStyle(Qt::PenStyle::DashLine);
    } else if (style == "dotted") {
        pen.setStyle(Qt::PenStyle::DotLine);
    } else if (style == "dashdot") {
        pen.setStyle(Qt::PenStyle::DashDotLine);
    } else {
        pen.setStyle(Qt::PenStyle::SolidLine);
    }
    graph->setPen(pen);
    clear_selection();
}


void LinePlotter::set_pen_none() {
    set_pen_style("none");
}


void LinePlotter::set_pen_solid() {
    set_pen_style("solid");
}


void LinePlotter::set_pen_dashed() {
    set_pen_style("dashed");
}


void LinePlotter::set_pen_dotted() {
    set_pen_style("dotted");
}


void LinePlotter::set_pen_dashdotted() {
    set_pen_style("dashdot");
}
