#ifndef TAGVIEWLISTED_H
#define TAGVIEWLISTED_H

#include <QWidget>

namespace Ui {
class TagViewListed;
}

class TagViewListed : public QWidget
{
    Q_OBJECT

public:
    explicit TagViewListed(QWidget *parent = 0);
    ~TagViewListed();

private:
    Ui::TagViewListed *ui;
};

#endif // TAGVIEWLISTED_H
