#include "datacontroller.h"


bool DataController::nix_file(const QString &nix_file) {
    if (!this->file.isNone() && this->file.isOpen() && this->filename != nix_file)
        this->file.close();
    this->filename = nix_file;
    this->file = nix::File::open(filename.toStdString(), nix::FileMode::ReadOnly);

    return this->file.isOpen();
}

QString DataController::nix_file() {
    return this->filename;
}

void DataController::close() {
    if(!this->file.isNone() && this->file.isOpen()) {
        this->file.close();
    }
    if (tree_model != nullptr) {
        delete tree_model;
        tree_model = nullptr;
    }
}

bool DataController::valid() {
    return !this->file.isNone() && this->file.isOpen();
}

void DataController::blocks_to_items(NixTreeModelItem *parent) {
    for (nix::Block b : this->file.blocks()) {
        NixTreeModelItem *itm = new NixTreeModelItem(QVariant::fromValue(b.name()), parent);

        parent->appendChild(itm);
    }
}

void DataController::sections_to_items(NixTreeModelItem *parent) {
    for (nix::Section s : this->file.sections()) {

    }
}


NixTreeModel* DataController::create_tree_model() {
    if (tree_model != nullptr) {
        delete tree_model;
        tree_model = nullptr;
    }
    this->tree_model = new NixTreeModel();
    this->tree_model->reset();
    return this->tree_model;
}


FileInfo DataController::file_info() {
    if (!this->valid()) {
        FileInfo fi;
        return fi;
    }
    FileInfo fi(this->file, this->filename);
    return fi;
}
