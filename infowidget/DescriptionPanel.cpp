#include "DescriptionPanel.hpp"
#include "ui_DescriptionPanel.h"
#include <common/Common.hpp>
#include <time.h>
#include <boost/algorithm/string.hpp>
#include "views/MainViewWidget.hpp"

DescriptionPanel::DescriptionPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DescriptionPanel)
{
    ui->setupUi(this);
}

void DescriptionPanel::update_description_panel(QModelIndex qml) {
    if(!qml.isValid()) {
        clear_description_panel();
        return;
    }
    NixTreeModelItem *item = static_cast<NixTreeModelItem*>(qml.internalPointer());
    ui->info_text_edit->setText(QString::fromStdString(item->entityInfo().description));
}


void DescriptionPanel::clear_description_panel() {
    ui->info_text_edit->setText("");
}


DescriptionPanel::~DescriptionPanel() {
    delete ui;
}
