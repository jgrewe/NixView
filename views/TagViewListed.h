#ifndef TAGVIEWLISTED_H
#define TAGVIEWLISTED_H

#include <QWidget>
#include "model/NixModelItem.hpp"
#include "utils/entitydescriptor.h"
#include <nix.hpp>

namespace Ui {
class TagViewListed;
}

class TagContainerTemp {
public:
    TagContainerTemp();
    TagContainerTemp(QVariant entity);

private:
    QVariant entity;

public:
    QVariant getEntity();
    std::string name();
    std::string type();
    std::vector<nix::DataArray> references();
    std::vector<nix::Feature> features();
    std::string description();
    QVector<QVector<double>> positions();
    bool hasExtents();
    QVector<QVector<double>> extents();
    QVector<QVector<QString>> completeDescription();
};

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

    TagContainerTemp tag;

    void processTag();
    void plotData();
    void plotTags();
    void plotFeatures();
};

#endif // TAGVIEWLISTED_H
