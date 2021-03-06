#include "projectnavigator.hpp"
#include "ui_projectnavigator.h"
#include "common/Common.hpp"
#include <QSettings>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <QSqlQuery>
#include <QInputDialog>


ProjectNavigator::ProjectNavigator(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProjectNavigator)
{
    ui->setupUi(this);
}

ProjectNavigator::~ProjectNavigator()
{
    delete ui;
}

/*
void ProjectNavigator::open_projects_db() {
    QString db;

    QSettings *settings = new QSettings();
    if (settings->contains(PROJECT_LIST)) {
        db = settings->value(PROJECT_LIST).toString();
        QFile f(db);
        if (!f.exists()) {
            QMessageBox msgBox;
            msgBox.setText("The project database could not be found at location " + db);
            msgBox.exec();
            QString dir_name = QFileDialog::getExistingDirectory(this, "Select location for project database.");
            if (!dir_name.isEmpty()) {
                db = QDir::cleanPath(dir_name + QDir::separator() + "projects.db");
                settings->setValue(PROJECT_LIST, db);
                settings->sync();
            }
        }
    } else {
        QString dir_name = QFileDialog::getExistingDirectory(this, "Select location for project database.");
        if (!dir_name.isEmpty()) {
            db = QDir::cleanPath(dir_name + QDir::separator() + "projects.db");
            settings->setValue(PROJECT_LIST, db);
            settings->sync();
        }
    }
    delete settings;
    if (!db.isEmpty())
        pm = ProjectManager(db);
}
*/

bool ProjectNavigator::set_project(const QString &project) {
    QStringList name = project.split('/');
    open_project(name[name.size()-1], project);
    return true;
}


void ProjectNavigator::clear() {
    ui->treeWidget->clear();
}


void ProjectNavigator::open_project(const QString &name, const QString &path) {
    ui->treeWidget->clear();
    ProjectIndex pi = ProjectIndex(path);
    QStringList file_list = pi.get_file_list();
    QTreeWidgetItem * item = new QTreeWidgetItem(ui->treeWidget, QStringList(name));
    for (int i = 0; i < file_list.size(); i++) {
        QTreeWidgetItem *it = new QTreeWidgetItem(QStringList(file_list[i]));
        it->setData(0, Qt::UserRole, pi.get_file_path(file_list[i]));
        item->addChild(it);
    }
    ui->treeWidget->insertTopLevelItem(0, item);
    ui->treeWidget->expandAll();
    this->project_name = name;
    this->project_path = path;
}

/*
void ProjectNavigator::refresh_projects() {
    ui->treeWidget->clear();
    QList<QTreeWidgetItem *> items;
    QSqlQuery q = pm.project_list();
    while (q.next()) {
        items.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(q.value(0).toString())));
    }
    ui->treeWidget->insertTopLevelItems(0, items);
}
*/
/*
void ProjectNavigator::select_project(int index) {
    std::cerr << "selected index: " << index << std::endl;
}
*/

void ProjectNavigator::item_selected(QTreeWidgetItem *item, int column) {
    emit file_selected(item->data(0, Qt::UserRole).toString());
}


void ProjectNavigator::add_file() {
    QFileDialog fd(this);
    QDir dir(this->project_path);
    fd.setDirectory(dir.dirName());
    fd.setAcceptMode(QFileDialog::AcceptOpen);
    fd.setFileMode(QFileDialog::ExistingFile);
    fd.setNameFilter(tr("NIX File (*.nix *.h5)"));
    fd.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (fd.exec())
        fileNames = fd.selectedFiles();
    if (fileNames.size() == 0)
        return;
    ProjectIndex pi(this->project_path);
    for (auto f : fileNames) {
        pi.add_file(f);
    }
    open_project(this->project_name, this->project_path);
}


void ProjectNavigator::remove_file() {
    QList<QTreeWidgetItem*> items = ui->treeWidget->selectedItems();
    if (items.size() <= 0) {
        return;
    }
    QString file_name = items[0]->data(0, Qt::UserRole).toString();
    ProjectIndex pi(this->project_path);
    pi.remove_file(file_name);
    emit close_file();
    open_project(this->project_name, this->project_path);
}

void ProjectNavigator::find() {
    ProjectIndex pi(this->project_path);
    pi.find("Deltaf 10 AND chirp");
}

/*
void ProjectNavigator::rename_project() {
    bool ok;
    //QString old_name = ui->comboBox->currentText();
    QList<QTreeWidgetItem*> selection = ui->treeWidget->selectedItems();
    if (selection.size() == 0)
        return;
    QString old_name = selection[0]->data(0, 0).toString();
    QString text = QInputDialog::getText(this, tr("QInputDialog::getText()"),
                                         tr("Give the new name:"), QLineEdit::Normal,
                                         QDir::home().dirName(), &ok);
    if (ok && !text.isEmpty()) {
        pm.rename_project(old_name, text);
    }
    refresh_projects();
}
*/

void ProjectNavigator::new_project() {
    QString fileName = QFileDialog::getSaveFileName(this, tr("new project"),
                                "", tr("nixview project index (*.npi)"));
    if (fileName.isEmpty())
        return;
    pm.add_project(fileName);
    //refresh_projects();
}

/*
void ProjectNavigator::delete_project() {
    QString curr_project = ui->treeWidget->currentItem()->text(0);
    pm.remove_project(curr_project);
    refresh_projects();
}
*/
