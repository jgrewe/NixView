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

void create_tree_model_item() {

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

int count_sections(nix::Section s) {
   int count = s.sectionCount();
   for (nix::Section subsec : s.sections()) {
       count += count_sections(subsec);
   }
   return count;
}

FileInfo DataController::file_info() {
    FileInfo fi;
    if (!this->valid())
        return fi;
    fi.file_name = this->filename;
    fi.file_format = QString::fromStdString(this->file.format());
    QString version;
    for (int i : file.version())
        version.append(QString::fromStdString(nix::util::numToStr(i)) + ".");
    fi.format_version = version;

    fi.created_at = QString::fromStdString(nix::util::timeToStr(this->file.createdAt()));
    fi.updated_at = QString::fromStdString(nix::util::timeToStr(this->file.updatedAt()));

    fi.block_count = int(this->file.blockCount());
    fi.array_count = 0;
    fi.tag_count = 0;
    fi.group_count = 0;
    int section_count = 0;
    for (nix::Block b : this->file.blocks()) {
        fi.array_count += b.dataArrayCount();
        fi.tag_count += b.tagCount();
        fi.tag_count += b.multiTagCount();
        fi.group_count += b.groupCount();
    }
    section_count += file.sectionCount();
    for (nix::Section s : file.sections()) {
        section_count += count_sections(s);
    }
    fi.section_count = section_count;
    boost::filesystem::path p(this->filename.toStdString());
    fi.file_size = (int)(boost::filesystem::file_size(p) / 1000000.);

    fi.status = boost::filesystem::status(p);
    return fi;
}
