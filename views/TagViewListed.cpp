#include "TagViewListed.h"
#include "ui_TagViewListed.h"

TagViewListed::TagViewListed(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TagViewListed)
{
    ui->setupUi(this);
}

TagViewListed::~TagViewListed()
{
    delete ui;
}
