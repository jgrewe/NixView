#ifndef DATACONTROLLER_H
#define DATACONTROLLER_H

#include <nix.hpp>
#include <QString>
#include <QVariant>
#include <boost/filesystem.hpp>
#include "model/nixtreemodel.h"

namespace bfs = boost::filesystem;

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

struct EntityInfo {

    QVariant created_at, updated_at, name, dtype, id, store_type, type, value;
    NixType nix_type;

    EntityInfo(nix::base::INamedEntity &item, const NixType &ntype) {
        name = QVariant(item.name().c_str());
        type = QVariant(item.type().c_str());
        nix_type = ntype;
        entity_props(item);
        if (ntype == NixType::NIX_FEAT) {
            nix::Property &p = dynamic_cast<nix::Property&>(item);
            dtype = QVariant(nix::data_type_to_string(p.dataType()).c_str());
        } else if (ntype == NixType::NIX_DATA_ARRAY) {
            nix::DataArray &da = dynamic_cast<nix::DataArray&>(item);
            dtype = QVariant(nix::data_type_to_string(da.dataType()).c_str());
        } else {
            dtype = QVariant("n.a.");
        }
    }

    EntityInfo(nix::Feature &feat) {
        name = QVariant(feat.data().name().c_str());
        type = QVariant(feat.data().type().c_str());
        nix_type = NixType::NIX_FEAT;
        dtype = QVariant(nix::data_type_to_string(feat.data().dataType()).c_str());
        entity_props(dynamic_cast<nix::base::IEntity&>(feat));
    }

    void entity_props(const nix::base::IEntity &ent) {
        id = QVariant(ent.id().c_str());
        created_at = QVariant(nix::util::timeToStr(ent.createdAt()).c_str());
        updated_at = QVariant(nix::util::timeToStr(ent.updatedAt()).c_str());
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

    ~DataController() {
        delete tree_model;
    }

    bool nix_file(const QString &filename);
    QString nix_file();
    FileInfo file_info();
    void close();
    bool valid();

    NixTreeModel* create_tree_model();
    void blocks_to_items(NixTreeModelItem *parent);
    void sections_to_items(NixTreeModelItem *parent);

private:
   DataController(){}
   DataController(const DataController&);
   DataController& operator = (const DataController &);

   QString filename;
   nix::File file;
   NixTreeModel *tree_model;

   void create_tree_model_item();
};
#endif // DATACONTROLLER_H
