#ifndef DATACONTROLLER_H
#define DATACONTROLLER_H

#include <nix.hpp>
#include <QString>
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
