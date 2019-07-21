#include "TagView.hpp"
#include "ui_TagView.h"
#include "common/Common.hpp"
#include <QListWidgetItem>
#include "plotter/plotter.h"
#include "plotter/lineplotter.h"
#include "plotter/plotwidget.h"

TagView::TagView(QWidget *parent) :
    QScrollArea(parent),
    ui(new Ui::TagView) {
    ui->setupUi(this);
}

TagView::~TagView() {
    delete ui;
}


void TagView::dataSource(const EntityInfo &info) {
    if (info.nix_type == NixType::NIX_MTAG || info.nix_type == NixType::NIX_TAG) {
        //this->tag = TagContainer(info);
        this->data_src = info;
        ui->tagLabel->setText(info.name.toString() + " - " + info.type.toString());
        ui->tagLabel->setToolTip(info.description.c_str());
        fillReferences();
        fillFeatures();
    }
}


void TagView::clear() {
    ui->tagLabel->setText("");
    clearReferences();
    clearFeatures();
}


void TagView::clearReferences() {
    ui->referencesCombo->clear();
    reference_map.clear();
    for (int i = ui->referenceStack->count() -1; i == 0; i--) {
        QWidget *w = ui->referenceStack->widget(i);
        ui->referenceStack->removeWidget(w);
        delete w;
    }
    QWidget *w = new QWidget();
    w->setLayout(new QVBoxLayout());
    ui->referenceStack->addWidget(w);
    ui->referenceStack->setCurrentIndex(0);
}


void TagView::clearFeatures() {
    ui->featuresCombo->clear();
    feature_map.clear();
    for (int i = ui->featureStack->count() -1; i == 0; i--) {
        QWidget *w = ui->featureStack->widget(i);
        ui->featureStack->removeWidget(w);
        delete w;
    }
    QWidget *w = new QWidget();
    w->setLayout(new QVBoxLayout());
    ui->featureStack->addWidget(w);
    ui->featureStack->setCurrentIndex(0);
}


void TagView::fillReferences() {
    clearReferences();
    DataController &dc = DataController::instance();

    for (EntityInfo info : dc.referenceList(this->data_src)) {
        std::cerr << "info: "  << info.name.toString().toStdString() << std::endl;
        ui->referencesCombo->addItem(info.name.toString() + " [" + info.type.toString() + "]", QVariant::fromValue(info));
    }
    if (ui->referencesCombo->count() > 0)
        ui->referencesCombo->setCurrentIndex(0);
}


void TagView::fillFeatures() {
    clearFeatures();
    DataController &dc = DataController::instance();

    for (EntityInfo info : dc.featureList(this->data_src)) {
        ui->referencesCombo->addItem(info.name.toString() + " [" + info.type.toString() + "]", QVariant::fromValue(info));
    }
    if (ui->featuresCombo->count() > 0)
        ui->featuresCombo->setCurrentIndex(0);
}


void TagView::referenceSelected(int i) {
    if (reference_map.size() == 0) {
        QWidget *w = ui->referenceStack->widget(0);
        ui->referenceStack->removeWidget(w);
        delete w;
    }
    if (reference_map.contains(i)) {
        ui->referenceStack->setCurrentIndex(reference_map[i]);
    } else if(i >= 0) {
        QWidget *w = new QWidget();
        w->setLayout(new QVBoxLayout());
        PlotWidget *pw = new PlotWidget();
        w->layout()->addWidget(pw);
        /*
        QVariant var = this->tag.getEntity();
        if (var.canConvert<nix::Tag>())
            pw->process(var.value<nix::Tag>(), i);
        else if (var.canConvert<nix::MultiTag>())
            pw->process(var.value<nix::MultiTag>(), i);
         */
        ui->referenceStack->addWidget(w);
        reference_map[i] = ui->referenceStack->count() - 1;
        ui->referenceStack->setCurrentIndex(reference_map[i]);
    }
}


void TagView::featureSelected(int i) {
    if (feature_map.size() == 0) {
        QWidget *w = ui->featureStack->widget(0);
        ui->featureStack->removeWidget(w);
        delete w;
    }
    if (feature_map.contains(i)) {
        ui->featureStack->setCurrentIndex(feature_map[i]);
    } else if(i >= 0) {
        QWidget *w = new QWidget();
        w->setLayout(new QVBoxLayout());
        PlotWidget *pw = new PlotWidget();
        w->layout()->addWidget(pw);
        /*
        QVariant var = this->tag.getEntity();
        if (var.canConvert<nix::Tag>())
            pw->process(this->tag.features()[i], var.value<nix::Tag>());
        else if (var.canConvert<nix::MultiTag>())
            pw->process(this->tag.features()[i], var.value<nix::MultiTag>());
        */
        ui->featureStack->addWidget(w);
        feature_map[i] = ui->featureStack->count() - 1;
        ui->featureStack->setCurrentIndex(feature_map[i]);
    }
}


void TagView::showTagInfo() {
    QMessageBox msgBox;
    msgBox.setWindowFlags(Qt::FramelessWindowHint);
    msgBox.setText(data_src.description.c_str());
    msgBox.exec();
}
