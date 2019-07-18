#include "tabledialog.hpp"
#include "ui_tabledialog.h"
#include "dialogs/csvexportdialog.h"

TableDialog::TableDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TableDialog)
{
    ui->setupUi(this);
}

TableDialog::~TableDialog()
{
    delete ui;
}

void TableDialog::set_entity(const EntityInfo &info) {
    ui->data_table->setDataSource(info);
}


void TableDialog::button_clicked(QAbstractButton *button) {
    if ((QPushButton *)button == ui->buttonBox->button(QDialogButtonBox::Close)) {
        this->close();
    }
}

void TableDialog::accept() {
    /*
    CSVExportDialog d(this);

    QModelIndexList indexes = ui->data_table->get_table()->selectionModel()->selection().indexes();
    d.setArray(ui->data_table->getArray());

    if(! indexes.isEmpty()) {
        nix::NDSize start(1,0);
        nix::NDSize extend(1,0);
        int size = ui->data_table->getArray().dataExtent().size();
        if(size == 1) {
            start  = {indexes[0].row()};
            extend = {indexes.last().row()-indexes[0].row()+1};
        } else if(size == 2) {
            start  = {indexes[0].row(),                         indexes[0].column()};
            extend = {indexes.last().row()-indexes[0].row()+1,  indexes.last().column()-indexes[0].column()+1};
        } else if(size == 3) {
            start  = {indexes[0].row(),                         indexes[0].column(),                           ui->data_table->currentPage()-1};
            extend = {indexes.last().row()-indexes[0].row()+1,  indexes.last().column()-indexes[0].column()+1, 1};
        }
        d.setSelection(start, extend);
    }
    d.exec();
    */
}
