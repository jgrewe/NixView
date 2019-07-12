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
    nix::Block b = this->file.getBlock(parent->entityInfo().name.toString().toStdString());
    std::vector<std::string> parent_path = {b.name()};
    append_items(b.dataArrays(), parent, parent_path);
    append_items(b.groups(), parent, parent_path);
    append_items(b.tags(), parent, parent_path);
    append_items(b.multiTags(), parent, parent_path);
    append_items(b.sources(), parent, parent_path);
}


void DataController::fetch_data_array(NixTreeModelItem *parent) {
    std::vector<std::string> p = parent->entityInfo().parent_path;
    nix::Block b = this->file.getBlock(p[0]);
    nix::DataArray da = b.getDataArray(parent->entityInfo().name.toString().toStdString());
    p.push_back(da.name());
    for (nix::Dimension d : da.dimensions()) {
        EntityInfo ei(d, p);
        NixTreeModelItem *itm = new NixTreeModelItem(ei, parent);
        parent->appendChild(itm);
    }
}


void DataController::fetch_tag(NixTreeModelItem *parent) {
    std::vector<std::string> p = parent->entityInfo().parent_path;
    nix::Block b = this->file.getBlock(p[0]);
    nix::Tag tag = b.getTag(parent->entityInfo().name.toString().toStdString());
    p.push_back(tag.name());

    if (tag.referenceCount() > 0) {
        EntityInfo rinfo("References");
        NixTreeModelItem *refs = new NixTreeModelItem(rinfo, parent);
        parent->appendChild(refs);
        for (nix::DataArray d : tag.references()) {
            EntityInfo ei(d, p);
            NixTreeModelItem *itm = new NixTreeModelItem(ei, refs);
            refs->appendChild(itm);
        }
    }
    if (tag.featureCount() > 0) {
        EntityInfo finfo("Features");
        NixTreeModelItem *feats = new NixTreeModelItem(finfo, parent);
        parent->appendChild(feats);
        for (nix::Feature f : tag.features()) {
            EntityInfo fi(f, p);
            NixTreeModelItem *itm = new NixTreeModelItem(fi, feats);
            feats->appendChild(itm);
        }
    }
}


void DataController::fetch_mtag(NixTreeModelItem *parent) {
    std::vector<std::string> p = parent->entityInfo().parent_path;
    nix::Block b = this->file.getBlock(p[0]);
    nix::MultiTag tag = b.getMultiTag(parent->entityInfo().name.toString().toStdString());
    p.push_back(tag.name());
    if (tag.referenceCount() > 0) {
        EntityInfo rinfo("References");
        NixTreeModelItem *refs = new NixTreeModelItem(rinfo, parent);
        parent->appendChild(refs);
        for (nix::DataArray d : tag.references()) {
            EntityInfo ei(d, p);
            NixTreeModelItem *itm = new NixTreeModelItem(ei, refs);
            refs->appendChild(itm);
        }
    }
    if (tag.featureCount() > 0) {
        EntityInfo finfo("Features");
        NixTreeModelItem *feats = new NixTreeModelItem(finfo, parent);
        parent->appendChild(feats);
        for (nix::Feature f : tag.features()) {
            EntityInfo fi(f, p);
            NixTreeModelItem *itm = new NixTreeModelItem(fi, feats);
            feats->appendChild(itm);
        }
    }
}

void DataController::fetch_source(NixTreeModelItem *parent) {
    std::vector<std::string> p = parent->entityInfo().parent_path;
    nix::Block b = this->file.getBlock(p[0]);
    nix::Source s;
    if (p.size() > 1) {
        s = b.getSource(p[1]);
        for (size_t i = 2; i< p.size(); ++i) {
            s = s.getSource(p[i]);
        }
        s = s.getSource(parent->entityInfo().name.toString().toStdString());
    } else {
       s = b.getSource(parent->entityInfo().name.toString().toStdString());
    }
    if (s) {
        EntityInfo si(s, p);
        NixTreeModelItem *itm = new NixTreeModelItem(si, parent);
        parent->appendChild(itm);
    }//TODO is this correct? Do we need to update the parent?
}


void DataController::fetch_group(NixTreeModelItem *parent) {
    std::vector<std::string> p = parent->entityInfo().parent_path;
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
    std::vector<std::string> p = parent->entityInfo().parent_path;

    nix::Section s;
    if (p.size() > 0) {
        s = this->file.getSection(p[0]);
        for (size_t i = 1; i < p.size(); ++i) {
            s = s.getSection(p[i]);
        }
        s = s.getSection(parent->entityInfo().name.toString().toStdString());
    } else {
        s = this->file.getSection(parent->entityInfo().name.toString().toStdString());
    }
    p.push_back(parent->entityInfo().name.toString().toStdString());

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


NixTreeModel *DataController::create_tree_model() {
    if (tree_model != nullptr) {
        delete tree_model;
        tree_model = nullptr;
    }
    this->tree_model = new NixTreeModel();
    return this->tree_model;
}


std::vector<std::string> DataController::sectionPath(const nix::Section &s) const {
    std::vector<std::string> path;
    while (s.parent() != nix::none) {
        path.insert(path.begin(), {s.parent().name()});
    }
    return path;
}


NixTreeModel *DataController::create_metadata_treemodel(NixTreeModelItem *parent) {
    if(mdata_tree != nullptr) {
        delete mdata_tree;
        mdata_tree = nullptr;
    }
    EntityInfo parent_info = parent->entityInfo();
    if (parent_info.has_metadata) {
        std::vector<nix::Section> sections = this->file.findSections(nix::util::NameFilter<nix::Section>(parent_info.metadata_name));
        nix::Section section;
        if (sections.size() == 1) {
            section = sections[0];
        } else {
            for (nix::Section s : sections) {
                if(s.id() == parent_info.metadata_id) {
                    section = s;
                    break;
                }
            }
        }
        if (section != nix::none) {
            std::vector<std::string> path = sectionPath(section);
            EntityInfo info(section, path);
            NixTreeModelItem *itm = new NixTreeModelItem(info);
            mdata_tree = new NixTreeModel(itm, nullptr);
        }
    }
    return mdata_tree;
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
