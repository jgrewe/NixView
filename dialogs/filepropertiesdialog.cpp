#include "filepropertiesdialog.hpp"
#include "utils/datacontroller.h"
#include "ui_filepropertiesdialog.h"
#include <boost/filesystem.hpp>

namespace bfs = boost::filesystem;

FilePropertiesDialog::FilePropertiesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FilePropertiesDialog)
{
    ui->setupUi(this);
}

FilePropertiesDialog::~FilePropertiesDialog()
{
    delete ui;
}

void FilePropertiesDialog::refresh() {
    DataController &dc = DataController::instance();
    FileInfo fi = dc.file_info();
    ui->format->setText(fi.file_format);
    ui->version->setText(fi.format_version);
    ui->created->setText(fi.created_at);
    ui->updated->setText(fi.updated_at);
    ui->block_count->setText(QVariant(fi.block_count).toString());
    ui->group_count->setText(QVariant(fi.group_count).toString());
    ui->array_count->setText(QVariant(fi.array_count).toString());
    ui->tag_count->setText(QVariant(fi.tag_count).toString());
    ui->section_count->setText(QVariant(fi.section_count).toString());

    ui->file_path->setText(QString(fi.file_name));
    ui->file_size->setText(QString::fromStdString(nix::util::numToStr(fi.file_size)) + " MB");
    ui->owner_read->setChecked((bfs::perms::owner_read & fi.status.permissions()) == bfs::perms::owner_read);
    ui->owner_write->setChecked((bfs::perms::owner_write & fi.status.permissions()) == bfs::perms::owner_write);
    ui->owner_exec->setChecked((bfs::perms::owner_exe & fi.status.permissions()) == bfs::perms::owner_exe);

    ui->group_read->setChecked((bfs::perms::group_read & fi.status.permissions()) == bfs::perms::group_read);
    ui->group_write->setChecked((bfs::perms::group_write & fi.status.permissions()) == bfs::perms::group_write);
    ui->group_exec->setChecked((bfs::perms::group_exe & fi.status.permissions()) == bfs::perms::group_exe);

    ui->all_read->setChecked((bfs::perms::others_read & fi.status.permissions()) == bfs::perms::others_read);
    ui->all_write->setChecked((bfs::perms::others_write & fi.status.permissions()) == bfs::perms::others_write);
    ui->all_exec->setChecked((bfs::perms::others_exe & fi.status.permissions()) == bfs::perms::others_exe);
}

