#include "datacontroller.h"
#include "model/nixtreemodelitem.h"
#include "model/nixtreemodel.h"
#include <nix/NDArray.hpp>

DataController::DataController() {
    this->info_buffer = new std::unordered_map<std::string, EntityInfo>();
}

DataController::~DataController() {
    delete info_buffer;
    delete tree_model;
}

bool DataController::nix_file(const QString &nix_file) {
    if (!this->file.isNone() && this->file.isOpen() && this->filename != nix_file)
        this->file.close();
    this->filename = nix_file;
    this->file = nix::File::open(filename.toStdString(), nix::FileMode::ReadOnly);
    this->info_buffer->clear();
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


void DataController::fetchBlock(NixTreeModelItem *parent) {
    nix::Block b = this->file.getBlock(parent->entityInfo().name.toString().toStdString());
    std::vector<std::string> parent_path = {b.name()};
    append_items(b.dataArrays(), parent, parent_path, "DataArrays");
    append_items(b.groups(), parent, parent_path, "Groups");
    append_items(b.tags(), parent, parent_path, "Tags");
    append_items(b.multiTags(), parent, parent_path, "MultiTags");
    append_items(b.sources(), parent, parent_path, "Sources");
}


void DataController::fetchDataArray(NixTreeModelItem *parent) {
    std::vector<std::string> p = parent->entityInfo().parent_path;
    nix::Block b = this->file.getBlock(p[0]);
    nix::DataArray da = b.getDataArray(parent->entityInfo().name.toString().toStdString());
    p.push_back(da.name());
    append_items(da.dimensions(), parent, p, "Dimensions");
}


void DataController::fetchTag(NixTreeModelItem *parent) {
    std::vector<std::string> p = parent->entityInfo().parent_path;
    nix::Block b = this->file.getBlock(p[0]);
    nix::Tag tag = b.getTag(parent->entityInfo().name.toString().toStdString());
    append_items(tag.references(), parent, p, "References");
    append_items(tag.features(), parent, p, "Features");
}


void DataController::fetchMtag(NixTreeModelItem *parent) {
    std::vector<std::string> p = parent->entityInfo().parent_path;
    nix::Block b = this->file.getBlock(p[0]);
    nix::MultiTag tag = b.getMultiTag(parent->entityInfo().name.toString().toStdString());
    append_items(tag.references(), parent, p, "References");
    append_items(tag.features(), parent, p, "Features");
}

void DataController::fetchSource(NixTreeModelItem *parent) {
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


void DataController::fetchGroup(NixTreeModelItem *parent) {
    std::vector<std::string> p = parent->entityInfo().parent_path;
    nix::Block b = this->file.getBlock(p[0]);
    std::cerr << b << std::endl;
    for (auto s : parent->entityInfo().parent_path)
        std::cerr << s<< std::endl;
    std::cerr << "group name " << parent->entityInfo().name.toString().toStdString() << std::endl;
    nix::Group g = b.getGroup(parent->entityInfo().name.toString().toStdString());

    append_items(g.dataArrays(), parent, p, "DataArrays");
    append_items(g.tags(), parent, p, "Tags");
    append_items(g.multiTags(), parent, p, "MultiTrags");
    append_items(g.sources(), parent, p, "Sources");
}


void DataController::fetchSection(NixTreeModelItem *parent) {
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
        std::string id;
        for (nix::Section sec : s.sections()) {
            EntityInfo si;
            id = sec.id();
            if (!findEntityInfo(id, si)) {
                si = EntityInfo(sec, p);
                addEntityInfo(id, si);
            }
            NixTreeModelItem *itm = new NixTreeModelItem(si, parent);
            parent->appendChild(itm);
        }
        for (nix::Property prop : s.properties()) {
            EntityInfo pi;
            id = prop.id();
            if (!findEntityInfo(id, pi)) {
                pi = EntityInfo(prop, p);
                addEntityInfo(id, pi);
            }
            NixTreeModelItem *itm = new NixTreeModelItem(pi, parent);
            parent->appendChild(itm);
        }
    }
}

QStringList DataController::dimensionLabels(const EntityInfo &info, size_t dim, size_t start_index, size_t count) {
    QStringList headers;
    nix::DataArray da = getDataArray(info);
    size_t c = count == 0 ? da.dataExtent()[dim-1] - start_index : count;
    if (da.getDimension(dim).dimensionType() == nix::DimensionType::Sample) {
        double si = da.getDimension(dim).asSampledDimension().samplingInterval();
        for (size_t i=start_index; i<start_index + c; i++) {
            headers.append(QString::number(i*si));
        }
    } else if (da.getDimension(dim).dimensionType() == nix::DimensionType::Range) {
        std::vector<double> ticks = da.getDimension(dim).asRangeDimension().ticks();
        for (size_t i = start_index; i < start_index + c; i++) {
            if (i >= ticks.size()){
                break;
            }
            headers.append(QString::number(ticks[i]));
        }
    } else if (da.getDimension(dim).dimensionType() == nix::DimensionType::Set) {
        std::vector<std::string> labels = da.getDimension(dim).asSetDimension().labels();
        for (size_t i = start_index; i < start_index + c; i++) {
            if (i >= labels.size()) {
                break;
            }
            headers.append(QString::fromStdString(labels[i]));
        }
    }

    if (headers.isEmpty()) {
        for (size_t i = start_index; i < start_index + c; i++) {
            headers.append(QString::number(i));
        }
    }
    return headers;
}

std::vector<std::string> DataController::axisLabels(const EntityInfo &info) {
    std::vector<std::string> labels;
    nix::DataArray array = getDataArray(info);

    if (array) {
        for (nix::Dimension d : array.dimensions()) {
            std::string dlabel;
            if (d.dimensionType() == nix::DimensionType::Range) {
                nix::RangeDimension rd = d.asRangeDimension();
                dlabel = rd.label() ? *rd.label() : "";
                if (rd.unit())
                    dlabel += (" [" + *rd.unit() + "]");
            } else if (d.dimensionType() == nix::DimensionType::Sample) {
                nix::SampledDimension sd = d.asSampledDimension();
                dlabel = sd.label() ? *sd.label() : "";
                if (sd.unit())
                    dlabel += (" [" + *sd.unit() + "]");
            }
            labels.push_back(dlabel);
        }
    }
    return labels;
}


QVector<double> DataController::axisData(const EntityInfo &info, nix::ndsize_t dim, nix::ndsize_t start, nix::ndsize_t count) {
    QVector<double> xdata;
    nix::DataArray da = getDataArray(info);

    if (da && dim <= da.dimensionCount()) {
        nix::Dimension d = da.getDimension(dim + 1);  //FIXME this would probably not work for any other than the first dimension...
        if (d.dimensionType() == nix::DimensionType::Sample) {
            nix::SampledDimension dim = d.asSampledDimension();
            std::vector<double> ax = dim.axis(count, start);
            xdata = QVector<double>::fromStdVector(ax);
        } else if (d.dimensionType() == nix::DimensionType::Range) {
            nix::RangeDimension dim = d.asRangeDimension();
            std::vector<double> ax = dim.axis(count, start);
            xdata = QVector<double>::fromStdVector(ax);
        } else if (d.dimensionType() == nix::DimensionType::Set) {
            nix::SetDimension dim = d.asSetDimension();
            std::vector<std::string> labels = dim.labels();
            for (size_t i = start; i < start + count; ++i) {
                if (i < labels.size())
                    xdata.push_back(static_cast<double>(i));
                else
                    break;
            }
            if (labels.size() == 0) {
                for (nix::ndsize_t i = start; i < start + count; ++i) {
                    xdata.push_back(static_cast<double>(i));
                }
            }
        } else {
            std::cerr << "unsupported dimension type" << std::endl;
        }
    }
    return xdata;
}


QVector<double> DataController::axisData(const EntityInfo &info, nix::ndsize_t dim) {
    nix::ndsize_t count = info.shape[dim];
    return axisData(info, dim, 0, count);
}


QVector<QString> DataController::axisStringData(const EntityInfo &info, nix::ndsize_t dim) {
    QVector<QString> data;
    nix::DataArray da = getDataArray(info);
    if (da && dim <= da.dimensionCount()) {
        nix::Dimension d = da.getDimension(dim + 1);
        if (d.dimensionType() != nix::DimensionType::Set) {
            return data;
        }
        nix::SetDimension sd = d.asSetDimension();
        std::vector<std::string> sd_labels = sd.labels();
        if (sd_labels.size() > 0) {
            for (auto s : sd_labels) {
                data.push_back(QString::fromStdString(s));
            }
        } else {
            for (nix::ndsize_t i = 0; i < da.dataExtent()[dim]; ++i)
                data.push_back(QString::fromStdString(nix::util::numToStr(i)));
        }
    }
    return data;
}


bool DataController::axisIsAlias(const EntityInfo &info, nix::ndsize_t dim) {
    if (dim >= info.dim_types.size() || info.dim_types[dim] != nix::DimensionType::Range)
        return false;
    nix::DataArray da = getDataArray(info);
    if (da) {
        nix::RangeDimension d = da.getDimension(dim + 1).asRangeDimension();
        return d.alias();
    }
    return false;
}


std::vector<EntityInfo> DataController::featureList(const EntityInfo &tag_info) {
    std::vector<nix::Feature>  feats;
    std::vector<EntityInfo> feat_info;
    if (tag_info.nix_type == NixType::NIX_TAG){
        feats = getTag(tag_info).features();
    } else if (tag_info.nix_type == NixType::NIX_MTAG) {
        feats = getMTag(tag_info).features();
    }
    for (nix::Feature f : feats) {
       feat_info.push_back(EntityInfo(f, tag_info.parent_path));
    }
    return feat_info;
}


std::vector<EntityInfo> DataController::referenceList(const EntityInfo &tag_info) {
    std::vector<nix::DataArray>  refs;
    std::vector<EntityInfo> ref_info;
    if (tag_info.nix_type == NixType::NIX_TAG){
        refs = getTag(tag_info).references();
    } else if (tag_info.nix_type == NixType::NIX_MTAG) {
        refs = getMTag(tag_info).references();
    }
    for (nix::DataArray da : refs) {
        EntityInfo info(da, tag_info.parent_path);
        ref_info.push_back(info);
    }
    return ref_info;
}


DataArrayInfo DataController::getArrayInfo(const EntityInfo &src) {
    nix::DataArray da = getDataArray(src);
    if (da)
        return DataArrayInfo(da);
    else
        return DataArrayInfo();
}


nix::DataArray DataController::getDataArray(const EntityInfo &info) {
    nix::DataArray da;
    if (info.nix_type == NixType::NIX_DATA_ARRAY || info.nix_type == NixType::NIX_FEAT) {
        if (info.parent_path.size() == 1) {
            nix::Block b = this->file.getBlock(info.parent_path[0]);
            da = b.getDataArray(info.name.toString().toStdString());
        }
    }
    return da;
}

nix::Tag DataController::getTag(const EntityInfo &info) {
    nix::Tag tag;
    if (info.nix_type == NixType::NIX_TAG) {
        if (info.parent_path.size() == 1) {
            nix::Block b = this->file.getBlock(info.parent_path[0]);
            tag = b.getTag(info.name.toString().toStdString());
        }
    }
    return tag;
}

nix::MultiTag DataController::getMTag(const EntityInfo &info) {
    nix::MultiTag tag;
    if (info.nix_type == NixType::NIX_MTAG) {
        if (info.parent_path.size() == 1) {
            nix::Block b = this->file.getBlock(info.parent_path[0]);
            tag = b.getMultiTag(info.name.toString().toStdString());
        }
    }
    return tag;
}


void DataController::getData(const EntityInfo &src, nix::DataType dtype, void *buffer, const nix::NDSize &count, const nix::NDSize &offset) {
    nix::DataArray array = getDataArray(src);
    if (array)
        array.getData(dtype, buffer, count, offset);
}


std::vector<double> DataController::tagPosition(const EntityInfo &info) {
    std::vector<double> positions;
    nix::Tag t = getTag(info);
    if (t) {
        positions = t.position();
    }
    return positions;
}


std::vector<double> DataController::tagExtent(const EntityInfo &info) {
    std::vector<double> ext;
    nix::Tag t = getTag(info);
    if (t) {
        ext = t.extent();
    }
    return ext;
}


std::vector<std::string> DataController::tagUnits(const EntityInfo &info) {
    std::vector<std::string> units;
    nix::Tag t = getTag(info);
    if  (t) {
        units = t.units();
    }
    return units;
}


nix::NDArray DataController::mtagPositions(const EntityInfo &info) {
    nix::MultiTag mtag = getMTag(info);
    nix::NDArray a(nix::DataType::Double,{0});
    if (mtag) {
        nix::DataArray array = mtag.positions();
        a = nix::NDArray(array.dataType(), array.dataExtent());
        nix::NDSize offset(a.rank(), 0);
        array.getData(array.dataType(), a.data(), a.size(), offset);
    }
    return a;
}


nix::NDArray DataController::mtagExtents(const EntityInfo &info) {
    nix::MultiTag mtag = getMTag(info);
    nix::NDArray a(nix::DataType::Double, {0});
    if (mtag) {
        nix::DataArray array = mtag.extents();
        if (array) {
            a = nix::NDArray(array.dataType(), array.dataExtent());
            nix::NDSize offset(a.rank(), 0);
            array.getData(array.dataType(), a.data(), a.size(), offset);
        }
    }
    return a;
}


std::vector<std::string> DataController::mtagUnits(const EntityInfo &info) {
    std::vector<std::string> units;
    nix::MultiTag t = getMTag(info);
    if  (t) {
        units = t.units();
    }
    return units;
}


void DataController::sections_to_items(NixTreeModelItem *parent) {
    for (nix::Section s : this->file.sections()) {
        EntityInfo info;
        if (!findEntityInfo(s.id(), info)) {
            info = EntityInfo(s, {});
            addEntityInfo(s.id(), info);
        }
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
            EntityInfo info;
            if (!findEntityInfo(section.id(), info)) {
                info = EntityInfo(section, path);
                addEntityInfo(section.id(), info);
            }
            //EntityInfo info(section, path);
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


void DataController::append_items(const std::vector<nix::Dimension> &dimensions, NixTreeModelItem *parent, std::vector<std::string> parent_path, QString subdir) {
    NixTreeModelItem *p;
    if (subdir.size() > 0 && dimensions.size() > 0) {
        EntityInfo info(subdir);
        info.max_child_count = dimensions.size();
        p = new NixTreeModelItem(info, parent);
        parent->appendChild(p);
    } else {
        p = parent;
    }
    std::string parent_id = parent->entityInfo().id.toString().toStdString();
    std::string dim_id;
    for (size_t i = 0; i< dimensions.size(); ++i) {
        nix::Dimension dim = dimensions[i];
        EntityInfo info;
        dim_id = parent_id + "_" + nix::util::numToStr(i);
        if (!findEntityInfo(dim_id, info)) {
            info = EntityInfo(dim, parent_path);
            addEntityInfo(dim_id, info);
        }
        NixTreeModelItem *itm = new NixTreeModelItem(info, p);
        p->appendChild(itm);
    }
}


void DataController::append_items(const std::vector<nix::Feature> &features, NixTreeModelItem *parent, std::vector<std::string> parent_path, QString subdir) {
    NixTreeModelItem *p;
    std::string id;
    if (subdir.size() > 0 && features.size() > 0) {
        EntityInfo info(subdir);
        info.max_child_count = features.size();
        p = new NixTreeModelItem(info, parent);
        parent->appendChild(p);
    } else {
        p = parent;
    }
    std::string parent_id = parent->entityInfo().id.toString().toStdString();
    std::string feat_id;
    for (size_t i = 0; i< features.size(); ++i) {
        nix::Feature feat = features[i];
        EntityInfo info;
        feat_id = parent_id + "_" + nix::util::numToStr(i);
        if (!findEntityInfo(feat_id, info)) {
            info = EntityInfo(feat, parent_path);
            addEntityInfo(feat_id, info);
        }
        NixTreeModelItem *itm = new NixTreeModelItem(info, p);
        p->appendChild(itm);
    }

}


template<typename T>
void DataController::append_items(const std::vector<T> &entities, NixTreeModelItem *parent, std::vector<std::string> parent_path, QString subdir) {
    NixTreeModelItem *p;
    std::string id;
    if (subdir.size() > 0 && entities.size() > 0) {
        EntityInfo info(subdir);
        info.max_child_count = entities.size();
        p = new NixTreeModelItem(info, parent);
        parent->appendChild(p);
    } else {
        p = parent;
    }
    for (T entity : entities) {
        EntityInfo info;
        id = entity.id();
        if (!findEntityInfo(id, info)) {
            info = EntityInfo(entity, parent_path);
            addEntityInfo(id, info);
        }
        NixTreeModelItem *itm = new NixTreeModelItem(info, p);
        p->appendChild(itm);
    }
}

bool DataController::findEntityInfo(const std::string &id, EntityInfo &info) {
    std::unordered_map<std::string, EntityInfo>::iterator it = this->info_buffer->find(id);
    if (it == this->info_buffer->end()) {
        return false;
    } else {
        info = it->second;
        return true;
    }
}

void DataController::addEntityInfo(const std::string &id, EntityInfo &info) {
    this->info_buffer->insert(std::pair<std::string, EntityInfo>(id, info));
}
