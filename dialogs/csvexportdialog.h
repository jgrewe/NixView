#ifndef CSVEXPORTDIALOG_H
#define CSVEXPORTDIALOG_H

#include <QDialog>
#include <QTextStream>
#include "nix.hpp"
#include "model/nixtreemodelitem.h"
#include "utils/datacontroller.h"

namespace Ui {
class CSVExportDialog;
}

class CSVExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CSVExportDialog(QWidget *parent = 0);
    ~CSVExportDialog();

    void dataSource(const EntityInfo &info);
    void setSelection(nix::NDSize start, nix::NDSize extent);
    void setSelectionStatus(bool enabled);

private:
    Ui::CSVExportDialog *ui;
    EntityInfo data_src;
    nix::NDSize start, extent;
    size_t partLength;

    void accept();
    void export_csv();

    void exportCsv1D(QTextStream &outStream, QString &sep);
    void exportCsv2D(QTextStream &outStream, QString &sep);
    void exportCsv3D(QTextStream &outStream, QString &sep);

    void export1DHeader(QStringList& vheader, QTextStream&  outStream, QString &sep, size_t x_ind);
    void export2DHeader(QStringList& hheader, QTextStream&  outStream, QString &sep);
    void exportData(QTextStream &outStream, nix::NDArray &data, nix::NDSize &yIndex, QString &sep);
    bool testStartExtend();
};

#endif // CSVEXPORTDIALOG_H
