#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include <QStackedWidget>
#include <QtCharts/QLineSeries>

#include "utils/datacontroller.h"
#include "nixchartview.h"

namespace Ui {
class ChartWidget;
}

class ChartWidget : public QStackedWidget
{
    Q_OBJECT

public:
    explicit ChartWidget(QWidget *parent = nullptr);
    ~ChartWidget();

    bool can_draw(const EntityInfo &data_source);
    bool add_data(const EntityInfo &data_source);

    void clear();

private:
    Ui::ChartWidget *ui;
    std::vector<QWidget*> widgets;
    DataController &dc = DataController::instance();

    void plot_series(const EntityInfo &data_source);
    void plot_series_1D(const EntityInfo &data_source, NixChartView *cv);
    void plot_series_2D(const EntityInfo &data_source, NixChartView *cv);
    QtCharts::QLineSeries* do_plot_series_data(std::vector<double> &xdata, std::vector<double> &ydata, const std::string & series_label);
    void plot_series_data(const EntityInfo &data_source, NixChartView *cv, nix::ndsize_t dim,
                          nix::NDSize count, nix::NDSize offset, const std::string &series_label);
    void plot_series_data(const EntityInfo &data_source, NixChartView *cv, nix::NDSize count, nix::NDSize offset,
                          std::vector<double> xdata, const std::string &series_label);

    void plot_events(const EntityInfo &data_source);

    void plot_tag(const EntityInfo &data_source);

    void plot_bars(const EntityInfo &data_source);

    nix::ndsize_t guess_best_xdim(const EntityInfo &info) const {
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

    bool check_dimensions(const EntityInfo &info) const {
        if (info.shape.size() > 2) {
            std::cerr << "check_dimensions cannot draw 3D!" << std::endl;
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
};

#endif // CHARTWIDGET_H
