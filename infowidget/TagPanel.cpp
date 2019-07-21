#include "TagPanel.hpp"
#include "ui_TagPanel.h"
#include "common/Common.hpp"
#include "model/NixDataModel.hpp"
#include "model/NixModelItem.hpp"
#include "views/MainViewWidget.hpp"

#include <iostream>
#include <sstream>
#include <ostream>
#include <iomanip>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/array.hpp>
#include <boost/multi_array.hpp>
#include <nix/NDArray.hpp>


TagPanel::TagPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TagPanel) {
    ui->setupUi(this);
    ui->tableWidget->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
}

// TODO set all labels to "-" if empty item is emitted

void TagPanel::update_tag_panel(QModelIndex qml) {
    clear_tag_panel();
    if (!qml.isValid()) {
        return;
    }
    NixTreeModelItem *item = static_cast<NixTreeModelItem*>(qml.internalPointer());

    if (item->entityInfo().nix_type == NixType::NIX_TAG || item->entityInfo().nix_type == NixType::NIX_MTAG) {
        this->data_src = item->entityInfo();
    } else {
        return;
    }
    if (data_src.nix_type == NixType::NIX_MTAG) {
        extract_multitag_info();
    } else {
        extract_tag_info();
    }
    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
}


std::string TagPanel::extract_tag_info() {
    std::stringstream ss;

    std::vector<double> pos = dc.tagPosition(this->data_src);
    std::ostringstream oss_pos;
    if(!pos.empty()) {
        for (int i = 0; i < (int)pos.size(); ++i) {
            oss_pos << std::setprecision(5) << pos[i];
            if (i < (int)pos.size()-1)
                oss_pos << ", ";
        }
    }
    ui->tableWidget->insertRow(0);
    QTableWidgetItem* item_pos = new QTableWidgetItem(QString::fromStdString(oss_pos.str()));
    ui->tableWidget->setItem(0, 0, item_pos);

    std::vector<double> ext = dc.tagExtent(this->data_src);
    std::ostringstream oss_ext;
    if(!ext.empty()) {
        for (int i = 0; i < (int)ext.size(); ++i) {
            oss_ext << std::setprecision(5) << ext[i];
            if (i < (int)ext.size()-1)
                oss_ext << ", ";
        }
    }
    QTableWidgetItem* item_ext = new QTableWidgetItem(QString::fromStdString(oss_ext.str()));
    ui->tableWidget->setItem(0, 1, item_ext);

    std::vector<std::string> units = dc.tagUnits(this->data_src);
    QTableWidgetItem* item_unit = nullptr;
    if(!units.empty())
        item_unit = new QTableWidgetItem(QString::fromStdString(units[0]));
    ui->tableWidget->setItem(0, 2, item_unit);

    references = dc.referenceList(this->data_src);
    fill_tree(ui->treeWidget_references, references);

    features = dc.featureList(this->data_src);
    fill_tree(ui->treeWidget_features, features);

    return ss.str();
}


void TagPanel::extract_multitag_info() {
    std::stringstream ss;

    nix::NDArray pos = dc.mtagPositions(this->data_src);
    nix::NDArray ext = dc.mtagExtents(this->data_src);
    std::vector<std::string> units = dc.mtagUnits(this->data_src);

    if (pos.rank() == 1) {// 1-dimensional data
        std::vector<std::string> units = dc.mtagUnits(this->data_src);

        for (size_t i = 0; i < pos.shape()[0]; i++) {
            std::ostringstream ss;
            ss << std::setprecision(5) << pos.get<double>(i);
            std::string pos_str = ss.str();
            QTableWidgetItem* item = new QTableWidgetItem(QString::fromStdString(pos_str));

            int new_row = ui->tableWidget->rowCount();
            ui->tableWidget->insertRow(new_row);
            ui->tableWidget->setItem(new_row, 0, item);

            if(ext.num_elements() > 0) {
                std::ostringstream ss;
                ss << std::setprecision(5) << ext.get<double>(i);
                QTableWidgetItem* item = new QTableWidgetItem(QString::fromStdString(ss.str()));
                ui->tableWidget->setItem(new_row, 1, item);
            }

            if(units.size() > 0) {
                QTableWidgetItem* item = new QTableWidgetItem(QString::fromStdString(units[0]));
                ui->tableWidget->setItem(new_row, 2, item);
            }
        }
    }
    else { // TODO if (size_pos.size() > 2)  // = not 1-dimensional data
        size_t dim_1 = pos.size()[0];
        size_t dim_2 = pos.size()[1];

        //double* pos_array = new double[dim_1 * dim_2];
        //positions.getDataDirect(nix::DataType::Double, pos_array, pos_size, {0,0});
        //double* ext_array = new double[dim_1 * dim_2];
        //if (extents)
        //    extents.getDataDirect(nix::DataType::Double, ext_array, pos_size, {0,0});
        for (size_t i = 0; i < dim_1; ++i) {
            int new_row = ui->tableWidget->rowCount();
            ui->tableWidget->insertRow(new_row);

            std::ostringstream ss_pos;
            std::ostringstream ss_ext;
            std::ostringstream ss_units;
            nix::NDSize s(2, i);
            for (int j =  0; j < dim_2; ++j) {
                s[1] = j;
                ss_pos << std::setprecision(5) << pos.get<double>(s);
                if (j < dim_2 - 1)
                    ss_pos << ", ";

                if (ext.num_elements() > 0) {
                    ss_ext << std::setprecision(5) << ext.get<double>(s);
                    if (j < dim_2 - 1)
                        ss_ext << ", ";
                }

                if (units.size() > 0) {
                    ss_units << units[j];
                    if (j < dim_2 - 1)
                        ss_units << ", ";
                }

            }
            QTableWidgetItem* item = new QTableWidgetItem(QString::fromStdString(ss_pos.str()));
            ui->tableWidget->setItem(new_row, 0, item);

            if (ext.num_elements() > 0) {
                QTableWidgetItem* item = new QTableWidgetItem(QString::fromStdString(ss_ext.str()));
                ui->tableWidget->setItem(new_row, 1, item);
            }

            if(units.size() > 0) {
                QTableWidgetItem* item = new QTableWidgetItem(QString::fromStdString(ss_units.str()));
                ui->tableWidget->setItem(new_row, 2, item);
            }
        }
    }

    references = dc.referenceList(this->data_src);
    fill_tree(ui->treeWidget_references, references);


    features = dc.featureList(this->data_src);
    fill_tree(ui->treeWidget_features, features);
}


void TagPanel::fill_tree(QTreeWidget* tree, std::vector<EntityInfo> ar) {
    for (auto i : ar) {
        QTreeWidgetItem* item = new QTreeWidgetItem(tree, QStringList(i.name.toString()));
        item->setText(1, i.type.toString());
        item->setText(2, QString::fromStdString("Data Array"));
        item->setText(3, i.dtype.toString());
        std::stringstream s;
        s << i.shape;
        std::string shape = s.str();
        boost::algorithm::trim(shape);
        shape = shape.substr(7, shape.length()-1);
        item->setText(4, QString::fromStdString(shape));
    }
}

void TagPanel::clear_tag_panel() {
    int table_size  = ui->tableWidget->rowCount();
    for (int i = 0; i < table_size; ++i)
        ui->tableWidget->removeRow(0);
    this->ui->treeWidget_references->clear();
    this->ui->treeWidget_features->clear();
}

// slots
void TagPanel::reference_item_requested(QTreeWidgetItem* item, int) {
    if (!item) {
        emit references_cleared();
        return;
    }
    for (auto i : references) {
        //if(i.name == item->text(0).toStdString())
            //emit emit_reference(i);
    }
}

void TagPanel::feature_item_requested(QTreeWidgetItem* item, int) {
    if (!item) {
        emit features_cleared();
        return;
    }
    for (auto i : features) {
        //if(i.name == item->text(0).toStdString())
            //emit emit_feature(i);
    }
}

void TagPanel::tag_item_requested(int current_row, int, int, int) {
    //emit emit_tag(current_qml, current_row);
}

void TagPanel::currentItemChanged_reference_helper(QTreeWidgetItem* current, QTreeWidgetItem*) {
    reference_item_requested(current, 0);
}

void TagPanel::currentItemChanged_feature_helper(QTreeWidgetItem* current, QTreeWidgetItem*) {
    feature_item_requested(current, 0);
}

// getters
QTreeWidget* TagPanel::get_reference_tree() {
    return ui->treeWidget_references;
}

QTreeWidget* TagPanel::get_feature_tree() {
    return ui->treeWidget_features;
}

QTableWidget* TagPanel::get_tag_table() {
    return ui->tableWidget;
}

// destructor
TagPanel::~TagPanel() {
    delete ui;
}
