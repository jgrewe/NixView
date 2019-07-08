#include "datacontroller.h"
#include "model/nixtreemodelitem.h"
#include "model/nixtreemodel.h"


DataController::~DataController() {
    delete tree_model;
}

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


nix::ndsize_t DataController::block_count() {
    return this->valid() ? this->file.blockCount() : 0;
}

nix::ndsize_t DataController::section_count() {
    return this->valid() ? this->file.sectionCount() : 0;
}

void DataController::blocks_to_items(NixTreeModelItem *parent) {
    for (nix::Block b : this->file.blocks()) {
        EntityInfo info(b, {});
        NixTreeModelItem *itm = new NixTreeModelItem(info, parent);
        parent->appendChild(itm);
    }
}


void DataController::fetch_block(NixTreeModelItem *parent) {
    nix::Block b = this->file.getBlock(parent->getEntityInfo().name.toString().toStdString());
    std::vector<std::string> parent_path = {b.name()};
    append_items(b.dataArrays(), parent, parent_path);
    append_items(b.groups(), parent, parent_path);
    append_items(b.tags(), parent, parent_path);
    append_items(b.multiTags(), parent, parent_path);
    append_items(b.sources(), parent, parent_path);
}


void DataController::fetch_data_array(NixTreeModelItem *parent) {
    std::vector<std::string> p = parent->getEntityInfo().parent_path;
    nix::Block b = this->file.getBlock(p[0]);
    nix::DataArray da = b.getDataArray(parent->getEntityInfo().name.toString().toStdString());
    p.push_back(da.name());
    for (nix::Dimension d : da.dimensions()) {
        EntityInfo ei(d, p);
        NixTreeModelItem *itm = new NixTreeModelItem(ei, parent);
        parent->appendChild(itm);
    }
}


void DataController::fetch_tag(NixTreeModelItem *parent) {
    std::vector<std::string> p = parent->getEntityInfo().parent_path;
    nix::Block b = this->file.getBlock(p[0]);
    nix::Tag tag = b.getTag(parent->getEntityInfo().name.toString().toStdString());
    p.push_back(tag.name());
    for (nix::DataArray d : tag.references()) {
        EntityInfo ei(d, p);
        NixTreeModelItem *itm = new NixTreeModelItem(ei, parent);
        parent->appendChild(itm);
    }
    for (nix::Feature f : tag.features()) {
        EntityInfo fi(f, p);
        NixTreeModelItem *itm = new NixTreeModelItem(fi, parent);
        parent->appendChild(itm);
    }
}


void DataController::fetch_mtag(NixTreeModelItem *parent) {
    std::vector<std::string> p = parent->getEntityInfo().parent_path;
    nix::Block b = this->file.getBlock(p[0]);
    nix::MultiTag tag = b.getMultiTag(parent->getEntityInfo().name.toString().toStdString());
    p.push_back(tag.name());
    for (nix::DataArray d : tag.references()) {
        EntityInfo ei(d, p);
        NixTreeModelItem *itm = new NixTreeModelItem(ei, parent);
        parent->appendChild(itm);
    }
    for (nix::Feature f : tag.features()) {
        EntityInfo fi(f, p);
        NixTreeModelItem *itm = new NixTreeModelItem(fi, parent);
        parent->appendChild(itm);
    }
}

void DataController::fetch_source(NixTreeModelItem *parent) {
    std::vector<std::string> p = parent->getEntityInfo().parent_path;
    nix::Block b = this->file.getBlock(p[0]);
    nix::Source s;
    if (p.size() > 1) {
        s = b.getSource(p[1]);
        for (size_t i = 2; i< p.size(); ++i) {
            s = s.getSource(p[i]);
        }
        s = s.getSource(parent->getEntityInfo().name.toString().toStdString());
    } else {
       s = b.getSource(parent->getEntityInfo().name.toString().toStdString());
    }
    if (s) {
        EntityInfo si(s, p);
        NixTreeModelItem *itm = new NixTreeModelItem(si, parent);
        parent->appendChild(itm);
    }//TODO is this correct? Do we need to update the parent?
}


void DataController::fetch_group(NixTreeModelItem *parent) {
    std::vector<std::string> p = parent->getEntityInfo().parent_path;
    nix::Block b = this->file.getBlock(p[0]);
    nix::Group g = b.getGroup(p[0]);
    for (nix::DataArray d : g.dataArrays()) {
        EntityInfo i(d, p);
        NixTreeModelItem *itm = new NixTreeModelItem(i, parent);
        parent->appendChild(itm);
    }
    for (nix::Tag t : g.tags()) {
        EntityInfo i(t, p);
        NixTreeModelItem *itm = new NixTreeModelItem(i, parent);
        parent->appendChild(itm);
    }
    for (nix::MultiTag mt : g.multiTags()) {
        EntityInfo i(mt, p);
        NixTreeModelItem *itm = new NixTreeModelItem(i, parent);
        parent->appendChild(itm);
    }
    for (nix::Source s : g.sources()) {
        EntityInfo i(s, p);
        NixTreeModelItem *itm = new NixTreeModelItem(i, parent);
        parent->appendChild(itm);
    }
}


void DataController::fetch_section(NixTreeModelItem *parent) {
    std::vector<std::string> p = parent->getEntityInfo().parent_path;

    nix::Section s;
    if (p.size() > 0) {
        std::cerr << p[0] << std::endl;
        s = this->file.getSection(p[0]);
        for (size_t i = 1; i < p.size(); ++i) {
            std::cerr << "\t" << p[i] << std::endl;
            s = s.getSection(p[i]);
        }
        s = s.getSection(parent->getEntityInfo().name.toString().toStdString());
    } else {
        s = this->file.getSection(parent->getEntityInfo().name.toString().toStdString());
    }
    p.push_back(parent->getEntityInfo().name.toString().toStdString());

    if (s) {
        for (nix::Section sec : s.sections()) {
            EntityInfo si(sec, p);
            NixTreeModelItem *itm = new NixTreeModelItem(si, parent);
            parent->appendChild(itm);
        }
        for (nix::Property prop : s.properties()) {
            EntityInfo pi(prop, p);
            NixTreeModelItem *itm = new NixTreeModelItem(pi, parent);
            parent->appendChild(itm);
        }
    }
}


void DataController::sections_to_items(NixTreeModelItem *parent) {
    for (nix::Section s : this->file.sections()) {
        EntityInfo info(s, {});
        NixTreeModelItem *itm = new NixTreeModelItem(info, parent);
        parent->appendChild(itm);
    }
}


NixTreeModel* DataController::create_tree_model() {
    if (tree_model != nullptr) {
        delete tree_model;
        tree_model = nullptr;
    }
    this->tree_model = new NixTreeModel();
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


template<typename T>
void DataController::append_items(const std::vector<T> &entities, NixTreeModelItem *parent, std::vector<std::string> parent_path) {
    for (T entity : entities) {
        EntityInfo info(entity, parent_path);
        NixTreeModelItem *itm = new NixTreeModelItem(info, parent);
        parent->appendChild(itm);
    }
}
