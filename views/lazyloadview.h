#ifndef LAZYLOADVIEW_H
#define LAZYLOADVIEW_H

#include <QWidget>
#include <QTreeView>

namespace Ui {
class LazyLoadView;
}

class LazyLoadView : public QWidget
{
    Q_OBJECT

public:
    explicit LazyLoadView(QWidget *parent = 0);
    ~LazyLoadView();

    QTreeView* getTreeView();

private:
    Ui::LazyLoadView *ui;
};

#endif // LAZYLOADVIEW_H