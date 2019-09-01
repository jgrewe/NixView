#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include <QStackedWidget>

namespace Ui {
class ChartWidget;
}

struct EntityInfo;

class ChartWidget : public QStackedWidget
{
    Q_OBJECT

public:
    explicit ChartWidget(QWidget *parent = 0);
    ~ChartWidget();

    bool can_draw(const EntityInfo &data_source);
    bool add_data(const EntityInfo &data_source);

    void clear();

private:
    Ui::ChartWidget *ui;
    std::vector<QWidget*> widgets;

    void plot_series(const EntityInfo &data_source);
    void plot_events(const EntityInfo &data_source);
    void plot_tag(const EntityInfo &data_source);
};

#endif // CHARTWIDGET_H
