#ifndef NIXARRAYTABLEMODEL_H
#define NIXARRAYTABLEMODEL_H

#include <QAbstractTableModel>
#include <nix.hpp>
#include "utils/datacontroller.h"

class NixArrayTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit NixArrayTableModel(QObject *parent = 0);

    void set_source(const EntityInfo &info, int page = 0);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    nix::NDSize shape;
    std::vector<std::string> h_labels, v_labels;
    nix::ndsize_t rows, cols, page;
    EntityInfo src_info;

    QVariant get_dimension_label(int section, int role, Qt::Orientation orientation, const nix::Dimension &dim) const;
};

#endif // NIXARRAYTABLEMODEL_H
