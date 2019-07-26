#ifndef TAGCONTAINER_H
#define TAGCONTAINER_H

#include <nix.hpp>
#include "utils/entitydescriptor.h"
#include "utils/datacontroller.h"


class TagContainer {
public:
    TagContainer();
    TagContainer(const EntityInfo &info);

private:
    EntityInfo data_src;

public:
    EntityInfo dataSource();
    std::string name();
    std::string type();
    //std::vector<nix::DataArray> references();
    std::vector<nix::Feature> features();
    QVector<double> positions(unsigned int index);
    bool hasExtents();
    QVector<double> extents(unsigned int index);
    std::string description();
    QVariant entity;

    void refLabels(QString &ylabel, QVector<QString> &xlabels, unsigned int index);
    void tagLabels(QString &ylabel, QVector<QString> &xlabels, unsigned int index);
    void featureLabels(QString &ylabel, QVector<QString> &xlabels, unsigned int index);

    unsigned int refCount();
    unsigned int tagCount();
    unsigned int featureCount();
};
#endif // TAGCONTAINER_H
