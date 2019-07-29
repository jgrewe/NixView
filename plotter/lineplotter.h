#ifndef LINEPLOTTER_H
#define LINEPLOTTER_H

#include <QWidget>
#include <QVector>
#include "plotter.h"
#include <nix.hpp>
#include "colormap.hpp"
#include "utils/loadthread.h"
#include "utils/datacontroller.h"

namespace Ui {
    class LinePlotter;
}

struct EntityInfo;

class LinePlotter : public QWidget, public Plotter {
    Q_OBJECT

public:
    explicit LinePlotter(QWidget *parent = 0, int numofPoints = 100000);
    ~LinePlotter();

    void draw(const EntityInfo &info);

    void set_label(const std::string &label) override;

    void add_line_plot(const QVector<double> &x_data, const QVector<double> &y_data, const QString &name);

    void set_ylabel(const std::string &label);
    void set_ylabel(const QString &label);

    void set_xlabel(const QString &label);
    void set_xlabel(const std::string &label);

    void add_events(const QVector<double> &x_data, const QVector<double> &y_data, const QString &name, bool y_scale) override;

    void add_events(const QVector<double> &x_data, const QString &name, bool y_scale);

    void add_segments(const QVector<double> &positions, const QVector<double> &extents, const QString &name) override;

    PlotterType plotter_type() const override;

    void save(QString filename) {}

private:
    Ui::LinePlotter *ui;
    ColorMap cmap;
    int numOfPoints;
    QCPRange totalXRange;
    QCPRange totalYRange;
    QVector<EntityInfo> data_sources;
    QVector<nix::DataArray> arrays;
    QVector<LoadThread*> loaders;
    QVector<int> working;
    DataController &dc = DataController::instance();

    void draw_1d(const EntityInfo &info);

    void draw_2d(const EntityInfo &info);

    QCustomPlot* get_plot() override;
    void expandXRange(const EntityInfo &info, int xDim);
    void setXRange(QVector<double> xData);
    void expandYRange(QVector<double> yData);
    void setYRange(QVector<double> yData);
    bool check_dimensions(const EntityInfo &info) const;
    int guess_best_xdim(const EntityInfo &info) const; //tries to find the best x-dimension needs to be optional at some point...

    //void calcStartExtent(const nix::DataArray &array, nix::NDSize &start_size, nix::NDSize& extent_size, int xDim);
    //bool checkForMoreData(int arrayIndex, double currentExtreme, bool higher);

signals:
    void xAxisChanged(QCPRange xNow, QCPRange xComplete);
    void yAxisChanged(QCPRange yNow, QCPRange yComplete);

public slots:
    void drawThreadData(const QVector<double> &data, const QVector<double> &axis, int graphIndex);
    void testThreads(QCPRange range);
    void printProgress(double progress);
    //void checkGraphsPerArray(QCPRange range);
    void resetView();

    void xAxisNewRange(QCPRange newRange);
    void yAxisNewRange(QCPRange newRange);

    void changeXAxisPosition(double newCenter);
    void changeYAxisPosition(double newCenter);
    void changeXAxisSize(double ratio);


    void selection_changed();
    void mouse_wheel();
    void mouse_press();
    void show_legend();
    void context_menu_request(QPoint pos);
    void remove_selected_graph();
    void clear_selection();
    void set_pen_style(QString style);
    void set_pen_none();
    void set_pen_solid();
    void set_pen_dashed();
    void set_pen_dashdotted();
    void set_pen_dotted();
    void set_marker(QString marker);
    void marker_none();
    void marker_square();
    void marker_diamond();
    void marker_circle();
    void marker_cross();
    void marker_dot();
    void marker_plus();

};

#endif // LINEPLOTTER_H
