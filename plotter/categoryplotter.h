#ifndef CATEGORYPLOTTER_H
#define CATEGORYPLOTTER_H

#include <QWidget>
#include "plotter.h"
#include "colormap.hpp"
#include "utils/datacontroller.h"

namespace Ui {
class CategoryPlotter;
}

class CategoryPlotter : public QWidget, public Plotter {
    Q_OBJECT

public:
    explicit CategoryPlotter(QWidget *parent = 0);
    ~CategoryPlotter();

    void draw(const EntityInfo &data_source);

    void set_label(const std::string &label) override;

    void set_ylabel(const QString &label);

    void set_xlabel(const QString &label);

    void set_ylabel(const std::string &label);

    void set_xlabel(const std::string &label);

    void add_bar_plot(QVector<QString> categories, QVector<double> y_data, const QString &name);

    void add_events(const QVector<double> &x_data, const QVector<double> &y_data, const QString &name, bool y_scale) override;

    void add_segments(const QVector<double> &positions, const QVector<double> &extents, const QString &name) override;

    PlotterType plotter_type() const override;

private:
    Ui::CategoryPlotter *ui;
    ColorMap cmap;
    DataController &dc = DataController::instance();

    QCustomPlot* get_plot() override;
    void draw_1d(const EntityInfo &data_source);
    void draw_2d(const EntityInfo &data_source);
    bool check_dimensions(const EntityInfo &data_source) const;
    size_t guess_best_xdim(const EntityInfo &data_source) const;
};

#endif // CATEGORYPLOTTER_H
