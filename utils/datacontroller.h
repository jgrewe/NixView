#ifndef DATACONTROLLER_H
#define DATACONTROLLER_H

#include <nix.hpp>
#include <QString>
#include <QVariant>
#include <unordered_map>
#include <boost/filesystem.hpp>
#include "utils/entitydescriptor.h"
#include "plotter/plotter.h"

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

struct DataArrayInfo;
struct EntityInfo;
/*
 *
 * DataController singleton class. The controller should be the only class that interacts with the nix file to avoid parallel access to the file.
 * It provides lots of convenience methods to access data dimension descriptors etc.
 *
 */
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

    /*
     * Methods to build the tree model
     */
    void blocks_to_items(NixTreeModelItem *parent);
    void sections_to_items(NixTreeModelItem *parent);
    void fetchBlock(NixTreeModelItem *parent);
    void fetchDataArray(NixTreeModelItem *parent);
    void fetchTag(NixTreeModelItem* parent);
    void fetchMtag(NixTreeModelItem *parent);
    void fetchSource(NixTreeModelItem *parent);
    void fetchGroup(NixTreeModelItem *parent);
    void fetchSection(NixTreeModelItem *parent);

    template<typename T>
    void append_items(const std::vector<T> &entities, NixTreeModelItem *parent, std::vector<std::string> parent_path, QString subdir);
    void append_items(const std::vector<nix::Dimension> &entities, NixTreeModelItem *parent, std::vector<std::string> parent_path, QString subdir);

    /*
     *
     */
    DataArrayInfo getArrayInfo(const EntityInfo &src);
    void getData(const EntityInfo &src, nix::DataType dtype, void *buffer, const nix::NDSize &count, const nix::NDSize &offset);
    QStringList dimensionLabels(const EntityInfo &info, size_t dim, size_t start_index = 0, size_t count = 0);

    /*
     * Returns a axis label for each dimension of the data.
     *
     * @parameter: EntityInfo
     *
     * returns: std::vector<std::string>
     *
     */
    std::vector<std::string> axisLabels(const EntityInfo &info);

    QVector<double> axisData(const EntityInfo &info, nix::ndsize_t dim);
    QVector<double> axisData(const EntityInfo &info, nix::ndsize_t dim, nix::ndsize_t start, nix::ndsize_t count=1);
    QVector<QString> axisStringData(const EntityInfo &info, nix::ndsize_t dim);
    bool axisIsAlias(const EntityInfo &info, nix::ndsize_t dim);

    std::vector<EntityInfo> featureList(const EntityInfo &tag_info);
    std::vector<EntityInfo> referenceList(const EntityInfo &tag_info);

    std::vector<double> tagPosition(const EntityInfo &info);
    std::vector<double> tagExtent(const EntityInfo &info);
    std::vector<std::string> tagUnits(const EntityInfo &info);

    nix::NDArray mtagPositions(const EntityInfo &info);
    nix::NDArray mtagExtents(const EntityInfo &info);
    std::vector<std::string> mtagUnits(const EntityInfo &info);

    /*
    * find buffered entity info for the passed entity id.
    *
    * returns: bool, if the information was found in buffer, false otherwise.
    */
    bool findEntityInfo(const std::string &id, EntityInfo &info);
    void addEntityInfo(const std::string &id, EntityInfo &info);

private:
   DataController();
   DataController(const DataController&);
   DataController& operator = (const DataController &);

   std::unordered_map<std::string, EntityInfo> *info_buffer;
   QString filename;
   nix::File file;
   NixTreeModel *tree_model, *mdata_tree;

   //void create_tree_model_item();
   std::vector<std::string> sectionPath(const nix::Section &s) const;
   nix::DataArray getDataArray(const EntityInfo &info);
   nix::Tag getTag(const EntityInfo &info);
   nix::MultiTag getMTag(const EntityInfo &info);

};

struct DataArrayInfo {
    nix::NDSize shape;
    nix::DataType dtype;
    std::string name, id, unit, label;
    std::vector<std::string> dimension_labels;

    DataArrayInfo() {}
    DataArrayInfo(const nix::DataArray &array) {
        shape = array.dataExtent();
        dtype = array.dataType();
        name = array.name();
        id = array.id();
        unit = array.unit() ? array.unit().get() : "";
        label = array.label() ? array.label().get() : "";
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
            dimension_labels.push_back(dlabel);
        }
    }
};

struct EntityInfo {
    std::string name, type, id;
    QVariant created_at, updated_at, value;
    nix::DataType dtype;
    nix::NDSize shape;
    NixType nix_type;
    bool has_metadata = false;
    nix::ndsize_t max_child_count;
    std::vector<std::string> parent_path;
    std::string description, metadata_name, metadata_id;
    PlotterType suggested_plotter;
    std::vector<nix::DimensionType> dim_types;

    EntityInfo() {
        name = "";
        type = "";
        id = "";
        parent_path = {};
        nix_type = NixType::NIX_UNKNOWN;
        suggested_plotter = PlotterType::Unsupported;
        dim_types = {};
        max_child_count = 0;
    }

    EntityInfo(const std::string &nam){
        name = nam;
        type = "";
        id = "";
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
        suggested_plotter = PlotterType::Unsupported;
        dim_types = {};
    }

    EntityInfo(const nix::Section &section, std::vector<std::string> path) {
        name = section.name();
        type = section.type();
        nix_type = NixType::NIX_SECTION;
        id = section.id();
        created_at = QVariant(nix::util::timeToStr(section.createdAt()).c_str());
        updated_at = QVariant(nix::util::timeToStr(section.updatedAt()).c_str());
        dtype = nix::DataType::Nothing;
        max_child_count = section.sectionCount() + section.propertyCount();
        description = EntityDescriptor::describe(section);
        parent_path = path;
        suggested_plotter = PlotterType::Unsupported;
        dim_types = {};
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
        name = property.name();
        type = "";
        nix_type = NixType::NIX_PROPERTY;
        id = property.id();
        created_at = QVariant(nix::util::timeToStr(property.createdAt()).c_str());
        updated_at = QVariant(nix::util::timeToStr(property.updatedAt()).c_str());
        dtype = property.dataType();
        max_child_count = 0;
        value = getPropertyValue(property);
        description = EntityDescriptor::describe(property);
        parent_path = path;
        suggested_plotter = PlotterType::Unsupported;
        dim_types = {};
    }

    EntityInfo(const nix::DataArray &array, std::vector<std::string> path) {
        name = array.name();
        type = array.type();
        nix_type = NixType::NIX_DATA_ARRAY;
        id = array.id();
        created_at = QVariant(nix::util::timeToStr(array.createdAt()).c_str());
        updated_at = QVariant(nix::util::timeToStr(array.updatedAt()).c_str());
        dtype = array.dataType();
        max_child_count = array.dimensionCount();
        description = EntityDescriptor::describe(array);
        parent_path = path;
        QString val = "shape [";
        shape = array.dataExtent();
        for (size_t i = 0; i < shape.size(); ++i)
            val.append((nix::util::numToStr(shape[i]) + (i < shape.size() - 1 ? ", " : "")).c_str());
        val.append("]");
        val.append(" dtype: " + QString::fromStdString(nix::data_type_to_string(dtype)));
        value = QVariant(val);
        nix::Section s = array.metadata();
        has_metadata = s != nix::none;
        if (has_metadata) {
            metadata_name = s.name();
            metadata_id = s.id();
        }
        suggested_plotter = Plotter::suggested_plotter(array);
        for (nix::Dimension d : array.dimensions())
            dim_types.push_back(d.dimensionType());

    }

    EntityInfo(const nix::Block &block, std::vector<std::string> path) {
        name = block.name();
        type = block.type();
        nix_type = NixType::NIX_BLOCK;
        id = block.id();
        created_at = QVariant(nix::util::timeToStr(block.createdAt()).c_str());
        updated_at = QVariant(nix::util::timeToStr(block.updatedAt()).c_str());
        dtype = nix::DataType::Nothing;
        description = EntityDescriptor::describe(block);
        max_child_count = 0;
        max_child_count += block.tagCount() > 0 ? 1 : 0;
        max_child_count += block.multiTagCount() > 0 ? 1 : 0;
        max_child_count += block.dataArrayCount() > 0 ? 1 : 0;
        max_child_count += block.sourceCount() > 0 ? 1 : 0;
        max_child_count += block.groupCount() > 0 ? 1 : 0;

        parent_path = path;
        nix::Section s = block.metadata();
        has_metadata = s != nix::none;
        if (has_metadata) {
            metadata_name = s.name();
            metadata_id = s.id();
        }
        suggested_plotter = PlotterType::Unsupported;
        dim_types = {};
    }

    EntityInfo(const nix::Group &group, std::vector<std::string> path) {
        name = group.name();
        type = group.type();
        nix_type = NixType::NIX_GROUP;
        id = group.id();
        created_at = QVariant(nix::util::timeToStr(group.createdAt()).c_str());
        updated_at = QVariant(nix::util::timeToStr(group.updatedAt()).c_str());
        dtype = nix::DataType::Nothing;
        description = EntityDescriptor::describe(group);
        max_child_count = 0;
        max_child_count += group.tagCount() > 0 ? 1 : 0;
        max_child_count += group.multiTagCount() > 0 ? 1 : 0;
        max_child_count += group.sourceCount() > 0 ? 1 : 0;
        max_child_count += group.dataArrayCount() > 0 ? 1 : 0;
        parent_path = path;

        nix::Section s = group.metadata();
        has_metadata = s != nix::none;
        if (has_metadata) {
            metadata_id = s.id();
            metadata_name = s.name();
        }
        suggested_plotter = PlotterType::Unsupported;
        dim_types = {};
    }

    EntityInfo(const nix::Tag &tag, std::vector<std::string> path) {
        name = tag.name();
        type = tag.type();
        nix_type = NixType::NIX_TAG;
        id = tag.id();
        created_at = QVariant(nix::util::timeToStr(tag.createdAt()).c_str());
        updated_at = QVariant(nix::util::timeToStr(tag.updatedAt()).c_str());
        dtype = nix::DataType::Nothing;
        max_child_count = 0;
        max_child_count += tag.referenceCount() > 0 ? 1 : 0;
        max_child_count += tag.featureCount() > 0 ? 1 : 0;
        description = EntityDescriptor::describe(tag);
        parent_path = path;

        nix::Section s = tag.metadata();
        has_metadata = s != nix::none;
        if (has_metadata) {
            metadata_id = s.id();
            metadata_name = s.name();
        }
        suggested_plotter = PlotterType::Unsupported;
        dim_types = {};
    }

    EntityInfo(const nix::MultiTag &mtag, std::vector<std::string> path) {
        name = mtag.name();
        type = mtag.type();
        nix_type = NixType::NIX_MTAG;
        id = mtag.id();
        created_at = QVariant(nix::util::timeToStr(mtag.createdAt()).c_str());
        updated_at = QVariant(nix::util::timeToStr(mtag.updatedAt()).c_str());
        dtype = nix::DataType::Nothing;
        max_child_count = 0;
        max_child_count += mtag.referenceCount() > 0 ? 1 : 0;
        max_child_count += mtag.featureCount() > 0 ? 1 : 0;
        description = EntityDescriptor::describe(mtag);
        parent_path = path;

        nix::Section s = mtag.metadata();
        has_metadata = s != nix::none;
        if (has_metadata) {
            metadata_id = s.id();
            metadata_name = s.name();
        }
        suggested_plotter = PlotterType::Unsupported;
        dim_types = {};
    }

    EntityInfo(const nix::Feature &feat, std::vector<std::string> path) {
        name = feat.data().name();
        type = feat.data().type();
        nix_type = NixType::NIX_FEAT;
        dtype = feat.data().dataType();
        id = feat.id();
        created_at = QVariant(nix::util::timeToStr(feat.createdAt()).c_str());
        updated_at = QVariant(nix::util::timeToStr(feat.updatedAt()).c_str());
        description = EntityDescriptor::describe(feat);
        shape = feat.data().dataExtent();
        max_child_count = 0;
        parent_path = path;
        suggested_plotter = Plotter::suggested_plotter(feat.data());
        for (nix::Dimension d : feat.data().dimensions())
            dim_types.push_back(d.dimensionType());
    }

    EntityInfo(const nix::Dimension &dimension, std::vector<std::string> path) {
        if (dimension.dimensionType() == nix::DimensionType::Sample) {
            nix::SampledDimension sdim = dimension.asSampledDimension();
            name = sdim.label() ? *sdim.label() : "";
            type = "Sampled dimension";
        } else if (dimension.dimensionType() == nix::DimensionType::Range) {
            nix::RangeDimension rdim = dimension.asRangeDimension();
            name = rdim.label() ? *rdim.label() : "";
            type = "Range dimension";
        } else {
            name = "";
            type = "Set dimension";
        }
        description = EntityDescriptor::describe(dimension);
        nix_type = NixType::NIX_DIMENSION;
        dtype = nix::DataType::Nothing;
        id = "n.a.";

        created_at = QVariant("");
        updated_at = QVariant("");
        max_child_count = 0;
        parent_path = path;
        suggested_plotter = PlotterType::Unsupported;
        dim_types = {};
    }

    EntityInfo(const nix::Source &src, std::vector<std::string> path) {
        name = src.name();
        type = src.type();
        nix_type = NixType::NIX_SOURCE;
        dtype = nix::DataType::Nothing;
        id = src.id();
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
        suggested_plotter = PlotterType::Unsupported;
        dim_types = {};
    }
};

#endif // DATACONTROLLER_H
