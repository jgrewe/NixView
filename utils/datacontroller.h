#ifndef DATACONTROLLER_H
#define DATACONTROLLER_H


#include <nix.hpp>
#include <QString>
#include <boost/filesystem.hpp>
#include "model/nixtreemodel.h"

namespace bfs = boost::filesystem;

struct FileInfo {
    QString file_name, file_location, file_format, format_version, created_at, updated_at;
    int file_size, block_count, group_count, array_count, section_count, tag_count;
    bfs::file_status status;
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
