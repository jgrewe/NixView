#ifndef DATACONTROLLER_H
#define DATACONTROLLER_H

#include <nix.hpp>
#include <QString>
#include <QVariant>
#include <boost/filesystem.hpp>
#include "utils/entitydescriptor.h"

class NixTreeModelItem;
class NixTreeModel;

namespace bfs = boost::filesystem;

enum class NixType {
    NIX_BLOCK,
    NIX_DATA_ARRAY,
    NIX_TAG,
    NIX_MTAG,
    NIX_GROUP,
    NIX_FEAT,
    NIX_SOURCE,
    NIX_SECTION,
    NIX_PROPERTY,
    NIX_DIMENSION,
    NIX_UNKNOWN
};

struct FileInfo {
    QString file_name, file_location, file_format, format_version, created_at, updated_at;
    int file_size;
    nix::ndsize_t block_count, group_count, array_count, section_count, tag_count;
    bfs::file_status status;

    FileInfo(){}

    FileInfo(const nix::File &file, const QString &filename) {
        file_name = filename;
        file_format = QString::fromStdString(file.format());
        QString version;
        for (int i : file.version())
            version.append(QString::fromStdString(nix::util::numToStr(i)) + ".");
        format_version = version;

        created_at = QString::fromStdString(nix::util::timeToStr(file.createdAt()));
        updated_at = QString::fromStdString(nix::util::timeToStr(file.updatedAt()));

        block_count = file.blockCount();
        array_count = 0;
        tag_count = 0;
        group_count = 0;
        section_count = 0;
        for (nix::Block b : file.blocks()) {
            array_count += b.dataArrayCount();
            tag_count += b.tagCount();
            tag_count += b.multiTagCount();
            group_count += b.groupCount();
        }
        section_count += file.sectionCount();
        for (nix::Section s : file.sections()) {
            section_count += count_sections(s);
        }
        boost::filesystem::path p(filename.toStdString());
        file_size = (int)(boost::filesystem::file_size(p) / 1000000.);

        status = boost::filesystem::status(p);
    }

    nix::ndsize_t count_sections(nix::Section s) {
       nix::ndsize_t count = s.sectionCount();
       for (nix::Section subsec : s.sections()) {
           count += count_sections(subsec);
       }
       return count;
    }
};


class DataController
{
public:
    static DataController& instance()
    {
      static DataController _instance;
      return _instance;
    }

    ~DataController();

    bool nix_file(const QString &filename);
    QString nix_file();
    FileInfo file_info();
    void close();
    bool valid();
    nix::ndsize_t block_count();
    nix::ndsize_t section_count();
    NixTreeModel *create_tree_model();
    NixTreeModel *create_metadata_treemodel(NixTreeModelItem *parent);

    void blocks_to_items(NixTreeModelItem *parent);
    void sections_to_items(NixTreeModelItem *parent);

    void fetch_block(NixTreeModelItem *parent);
    void fetch_data_array(NixTreeModelItem *parent);
    void fetch_tag(NixTreeModelItem* parent);
    void fetch_mtag(NixTreeModelItem *parent);
    void fetch_source(NixTreeModelItem *parent);
    void fetch_group(NixTreeModelItem *parent);
    void fetch_section(NixTreeModelItem *parent);

    template<typename T>
    void append_items(const std::vector<T> &entities, NixTreeModelItem *parent, std::vector<std::string> parent_path);

private:
   DataController(){}
   DataController(const DataController&);
   DataController& operator = (const DataController &);

   QString filename;
   nix::File file;
   NixTreeModel *tree_model, *mdata_tree;

   //void create_tree_model_item();
   std::vector<std::string> sectionPath(const nix::Section &s) const;
};


struct EntityInfo {
    QVariant created_at, updated_at, name, dtype, id, type, value;
    NixType nix_type;
    bool has_metadata = false;
    nix::ndsize_t max_child_count;
    std::vector<std::string> parent_path;
    std::string description, metadata_name, metadata_id;


    EntityInfo(const QString &nam){
        name = QVariant(nam);
        parent_path = {};
        nix_type = NixType::NIX_UNKNOWN;

        DataController &dc = DataController::instance();
        if (name == "Root")
            max_child_count = 2;
        else if (name == "Data") {
            max_child_count = dc.block_count();
        } else if (name == "Metadata") {
            max_child_count = dc.section_count();
        }
    }

    EntityInfo(const nix::Section &section, std::vector<std::string> path) {
        name = QVariant(section.name().c_str());
        type = QVariant(section.type().c_str());
        nix_type = NixType::NIX_SECTION;
        id = QVariant(section.id().c_str());
        created_at = QVariant(nix::util::timeToStr(section.createdAt()).c_str());
        updated_at = QVariant(nix::util::timeToStr(section.updatedAt()).c_str());
        dtype = QVariant("n.a.");
        max_child_count = section.sectionCount() + section.propertyCount();
        description = EntityDescriptor::describe(section);
        parent_path = path;
    }

    QVariant getPropertyValue(const nix::Property &p) {
        std::string vals;
        if (p.valueCount() > 1) {
            vals = "[ ";
        }
        for (nix::Value v : p.values()) {
            vals = vals + EntityDescriptor::value_to_str(v, p.dataType());
        }
        if (p.valueCount() > 1) {
            vals = vals + "]";
        }
        return QVariant(vals.c_str());
    }

    EntityInfo(const nix::Property &property, std::vector<std::string> path) {
        name = QVariant(property.name().c_str());
        type = QVariant("");
        nix_type = NixType::NIX_PROPERTY;
        id = QVariant(property.id().c_str());
        created_at = QVariant(nix::util::timeToStr(property.createdAt()).c_str());
        updated_at = QVariant(nix::util::timeToStr(property.updatedAt()).c_str());
        dtype = QVariant(nix::data_type_to_string(property.dataType()).c_str());
        max_child_count = 0;
        value = getPropertyValue(property);
        description = EntityDescriptor::describe(property);
        parent_path = path;
    }

    EntityInfo(const nix::DataArray &array, std::vector<std::string> path) {
        name = QVariant(array.name().c_str());
        type = QVariant(array.type().c_str());
        nix_type = NixType::NIX_DATA_ARRAY;
        id = QVariant(array.id().c_str());
        created_at = QVariant(nix::util::timeToStr(array.createdAt()).c_str());
        updated_at = QVariant(nix::util::timeToStr(array.updatedAt()).c_str());
        dtype = QVariant(nix::data_type_to_string(array.dataType()).c_str());
        max_child_count = array.dimensionCount();
        description = EntityDescriptor::describe(array);
        parent_path = path;
        QString val = "shape [";
        nix::NDSize extent = array.dataExtent();
        for (size_t i = 0; i < extent.size(); ++i)
            val.append((nix::util::numToStr(extent[i]) + (i < extent.size() - 1 ? ", " : "")).c_str());
        val.append("]");
        value = QVariant(val);
        nix::Section s = array.metadata();
        has_metadata = s != nix::none;
        if (has_metadata) {
            metadata_name = s.name();
            metadata_id = s.id();
        }
    }

    EntityInfo(const nix::Block &block, std::vector<std::string> path) {
        name = QVariant(block.name().c_str());
        type = QVariant(block.type().c_str());
        nix_type = NixType::NIX_BLOCK;
        id = QVariant(block.id().c_str());
        created_at = QVariant(nix::util::timeToStr(block.createdAt()).c_str());
        updated_at = QVariant(nix::util::timeToStr(block.updatedAt()).c_str());
        dtype = QVariant("n.a.");
        description = EntityDescriptor::describe(block);
        max_child_count = block.dataArrayCount() + block.groupCount() + block.tagCount() + block.multiTagCount() +
                block.sourceCount();
        parent_path = path;
        nix::Section s = block.metadata();
        has_metadata = s != nix::none;
        if (has_metadata) {
            metadata_name = s.name();
            metadata_id = s.id();
        }
    }

    EntityInfo(const nix::Group &group, std::vector<std::string> path) {
        name = QVariant(group.name().c_str());
        type = QVariant(group.type().c_str());
        nix_type = NixType::NIX_GROUP;
        id = QVariant(group.id().c_str());
        created_at = QVariant(nix::util::timeToStr(group.createdAt()).c_str());
        updated_at = QVariant(nix::util::timeToStr(group.updatedAt()).c_str());
        dtype = QVariant("n.a.");
        description = EntityDescriptor::describe(group);
        max_child_count = group.dataArrayCount() + group.multiTagCount() + group.tagCount() + group.sourceCount();
        parent_path = path;

        nix::Section s = group.metadata();
        has_metadata = s != nix::none;
        if (has_metadata) {
            metadata_id = s.id();
            metadata_name = s.name();
        }
    }

    EntityInfo(const nix::Tag &tag, std::vector<std::string> path) {
        name = QVariant(tag.name().c_str());
        type = QVariant(tag.type().c_str());
        nix_type = NixType::NIX_TAG;
        id = QVariant(tag.id().c_str());
        created_at = QVariant(nix::util::timeToStr(tag.createdAt()).c_str());
        updated_at = QVariant(nix::util::timeToStr(tag.updatedAt()).c_str());
        dtype = QVariant("n.a.");
        max_child_count = tag.referenceCount() + tag.featureCount();
        description = EntityDescriptor::describe(tag);
        parent_path = path;

        nix::Section s = tag.metadata();
        has_metadata = s != nix::none;
        if (has_metadata) {
            metadata_id = s.id();
            metadata_name = s.name();
        }
    }

    EntityInfo(const nix::MultiTag &mtag, std::vector<std::string> path) {
        name = QVariant(mtag.name().c_str());
        type = QVariant(mtag.type().c_str());
        nix_type = NixType::NIX_MTAG;
        id = QVariant(mtag.id().c_str());
        created_at = QVariant(nix::util::timeToStr(mtag.createdAt()).c_str());
        updated_at = QVariant(nix::util::timeToStr(mtag.updatedAt()).c_str());
        dtype = QVariant("n.a.");
        max_child_count = mtag.referenceCount() + mtag.featureCount();
        description = EntityDescriptor::describe(mtag);
        parent_path = path;

        nix::Section s = mtag.metadata();
        has_metadata = s != nix::none;
        if (has_metadata) {
            metadata_id = s.id();
            metadata_name = s.name();
        }
    }

    EntityInfo(const nix::Feature &feat, std::vector<std::string> path) {
        name = QVariant(feat.data().name().c_str());
        type = QVariant(feat.data().type().c_str());
        nix_type = NixType::NIX_FEAT;
        dtype = QVariant(nix::data_type_to_string(feat.data().dataType()).c_str());
        id = QVariant(feat.id().c_str());
        created_at = QVariant(nix::util::timeToStr(feat.createdAt()).c_str());
        updated_at = QVariant(nix::util::timeToStr(feat.updatedAt()).c_str());
        description = EntityDescriptor::describe(feat);
        max_child_count = 0;
        parent_path = path;
    }

    EntityInfo(const nix::Dimension &dimension, std::vector<std::string> path) {
        if (dimension.dimensionType() == nix::DimensionType::Sample) {
            nix::SampledDimension sdim = dimension.asSampledDimension();
            name = QVariant(sdim.label() ? sdim.label()->c_str() : "");
            type = QVariant("Sampled dimension");
        } else if (dimension.dimensionType() == nix::DimensionType::Range) {
            nix::RangeDimension rdim = dimension.asRangeDimension();
            name = QVariant(rdim.label() ? rdim.label()->c_str() : "");
            type = QVariant("Range dimension");
        } else {
            name = QVariant("");
            type = QVariant("Set dimension");
        }
        description = EntityDescriptor::describe(dimension);
        nix_type = NixType::NIX_DIMENSION;
        dtype = QVariant("n.a.");
        id = QVariant("n.a.");

        created_at = QVariant("");
        updated_at = QVariant("");
        max_child_count = 0;
        parent_path = path;
    }

    EntityInfo(const nix::Source &src, std::vector<std::string> path) {
        name = QVariant(src.name().c_str());
        type = QVariant(src.type().c_str());
        nix_type = NixType::NIX_SOURCE;
        dtype = QVariant("n.a.");
        id = QVariant(src.id().c_str());
        created_at = QVariant(nix::util::timeToStr(src.createdAt()).c_str());
        updated_at = QVariant(nix::util::timeToStr(src.updatedAt()).c_str());
        max_child_count = src.sourceCount();
        description = EntityDescriptor::describe(src);
        parent_path = path;

        nix::Section s = src.metadata();
        has_metadata = s != nix::none;
        if (has_metadata) {
            metadata_id = s.id();
            metadata_name = s.name();
        }
    }
};

#endif // DATACONTROLLER_H
