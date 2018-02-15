#ifndef TAGVIEWLISTED_H
#define TAGVIEWLISTED_H

#include <QWidget>
#include "model/NixModelItem.hpp"
#include "utils/entitydescriptor.h"
#include <nix.hpp>
#include "utils/tagcontainer.h"

namespace Ui {
class TagViewListed;
}

class TagViewListed : public QWidget
{
    Q_OBJECT

public:
    explicit TagViewListed(QWidget *parent = 0);
    ~TagViewListed();

    void setEntity(QVariant var);
    void clear();

public slots:
    //void show_tag_info();

private:
    Ui::TagViewListed *ui;

    TagContainer tag;

    void processTag();
    void plotData();
    void plotTags();
    void plotFeatures();
};

#endif // TAGVIEWLISTED_H
