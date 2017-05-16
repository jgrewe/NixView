#ifndef PROJECTINDEX_H
#define PROJECTINDEX_H
#include <QString>
#include <QtSql>
/**
 * @brief The ProjectIndex class manages the files within a project.
 * ProjectIndex database has (at the moment) two tables that manage the files.
 * 1. files merely stores file name and path in two separate columns (so far absolute paths...)
 * 2. data_index stores all the different NIX entity_names, ids, entity_types, and path.
 * 3. not implemented a entity_index, basically a triplestore associated with each entity....
 */
class ProjectIndex {

private:
    QString path;

    void index_file(const QString &file_path);

public:
    ProjectIndex(const QString &path);

    /**
     * @brief get_file_list
     * @return QStringList with the files associated with this project
     */
    QStringList get_file_list();

    /**
     * @brief get_file_path
     * @param file_name the filename
     * @return
     */
    QString get_file_path(const QString &file_name);

    /**
     * @brief add_file to project index.
     * @param file_path The full path to the file
     * @return false if operation was not successful, e.g. the file does not exsist.
     */
    bool add_file(const QString &file_path);

    /**
     * @brief remove_file from the project index.
     * @param file_name the name of the file (not its path)
     * @return true if operation was successful
     */
    bool remove_file(const QString &file_name);

    /**
     * @brief create_project_index
     * @param path
     * @return
     * TODO needs to check whether an existing file actually represents a project_index
     */
    static bool create_project_index(const QString &path);


};

#endif // PROJECTINDEX_H
