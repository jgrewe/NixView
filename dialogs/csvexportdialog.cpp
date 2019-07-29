#include "csvexportdialog.h"
#include "ui_csvexportdialog.h"
#include <iostream>
#include <Qt>
#include <QFileDialog>
#include <QTextStream>
#include <QPushButton>
#include <nix/NDArray.hpp>

#include "utils/datacontroller.h"


CSVExportDialog::CSVExportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CSVExportDialog),
    start(1,0), extent(1,0)
{
    partLength = 5000;
    ui->setupUi(this);
    ui->progressBar->setValue(0);
    ui->progressBar->setVisible(false);
}


CSVExportDialog::~CSVExportDialog()
{
    delete ui;
}


void CSVExportDialog::dataSource(const EntityInfo &info) {
    this->data_src = info;
}


void CSVExportDialog::setSelection(nix::NDSize start, nix::NDSize extend) {
    this->start = start;
    this->extent = extend;

    if(testStartExtend()) {
        ui->export_selection->setChecked(true);
    }
}


void CSVExportDialog::setSelectionStatus(bool enabled) {
    ui->export_selection->setEnabled(enabled);
}


void CSVExportDialog::accept() {
    export_csv();
}


void CSVExportDialog::export_csv() {
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    ui->progressBar->setVisible(true);
    ui->progressBar->setValue(0);
    QFileDialog fd(this);
    fd.setAcceptMode(QFileDialog::AcceptSave);
    fd.setNameFilter(tr("CSV File (*.csv)"));
    fd.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (fd.exec())
        fileNames = fd.selectedFiles();
    if (fileNames.size() == 0)
        return;

    QFile outfile(fileNames[0]);
    outfile.open(QIODevice::WriteOnly);

    if(!outfile.isOpen()){
        std::cerr << "- Error, unable to open" << "outputFilename" << "for output";
    }
    QTextStream outStream(&outfile);
    QString sep = ui->separator_edit->text().append(" ");

    if(this->data_src.shape.size() == 1) {
        exportCsv1D(outStream, sep);
    } else if(this->data_src.shape.size() == 2) {
        exportCsv2D(outStream, sep);
    } else if(this->data_src.shape.size() == 3) {
        exportCsv3D(outStream, sep);
    } else if(this->data_src.shape.size() > 3) {
        std::cerr << "CSV-export of data with more than 3 dimensions currently not possible. Sorry!" << std::endl;
    }
    outfile.close();
    this->close();
}


void CSVExportDialog::exportCsv1D(QTextStream &outStream, QString &sep) {
    QStringList vheader;
    DataController &dc = DataController::instance();
    DataArrayInfo info = dc.getArrayInfo(this->data_src);
    nix::NDArray data = nix::NDArray(info.dtype, {partLength});

    if (ui->export_header->isChecked()) {
        outStream << " " << sep;
        QString label = QString::fromStdString(info.label);
        QString unit = QString::fromStdString(info.unit);
        label = label.append(" ").append(unit);
        outStream << label << "\n";
    }

    if(! (ui->export_selection->isChecked()) || !testStartExtend()) {
        start[0] = 0;
        extent[0] = info.shape[0];
    }

    int count = 0;
    int draw = 100;
    double step = 100. / extent[0];

    for(size_t x = start[0]; x < start[0]+extent[0]; ++x) {
        size_t x_ind = x-start[0];
        if(ui->export_header->isChecked()) {
            export1DHeader(vheader, outStream, sep, x);
        }

        if(x_ind % partLength == 0) {
            if( partLength <= extent[0] - x_ind) {
                dc.getData(this->data_src, info.dtype, data.data(), {partLength}, {x});
            } else {
                dc.getData(this->data_src, info.dtype, data.data(), {(extent[0]-x_ind)}, {x});
            }
        }
        nix::NDSize index = nix::NDSize(1,x_ind%partLength);
        //export depending on type:
        exportData(outStream, data, index, sep);
        outStream << "\n";
        count++;
        if(count%draw == 0) {
            ui->progressBar->setValue(rint(count * step));
            QCoreApplication::processEvents();
        }
    }
}

void CSVExportDialog::exportCsv2D(QTextStream &outStream, QString &sep) {
    QStringList vheader, hheader;
    DataController &dc = DataController::instance();
    DataArrayInfo info = dc.getArrayInfo(this->data_src);

    if (! ui->export_selection->isChecked() || ! testStartExtend()) {
        start = nix::NDSize(2,0);
        extent = nix::NDSize(2, info.shape[0]);
        extent[1] = info.shape[1];
    }
    nix::NDArray data = nix::NDArray(info.dtype, {(int)partLength, (int)extent[1]});

    int count = 0;
    int draw = 100;
    double step = 100. / extent[0];

    if (ui->export_header->isChecked()) {
        export2DHeader(hheader, outStream, sep);
    }

    for (size_t x = start[0]; x < start[0] + extent[0]; ++x) {
        size_t x_ind = x - start[0];
        if (ui->export_header->isChecked()) {
            export1DHeader(vheader, outStream, sep, x);
        }

        if (x_ind % partLength == 0) {
            nix::NDSize offset = {(int)x, (int)start[1]};
            if (partLength <= extent[0] - x_ind) {
                nix::NDSize chunk = {(int)partLength, (int)extent[1]};
                dc.getData(this->data_src, info.dtype, data.data(), chunk, offset);
            } else {
                nix::NDSize chunk = {(int)(extent[0]-x_ind), (int)extent[1]};
                dc.getData(this->data_src, info.dtype, data.data(), chunk, offset);
            }
        }

        for(size_t y = 0; y < extent[1]; ++y) {
            nix::NDSize yIndex;
            yIndex = nix::NDSize(2,x_ind % partLength);
            yIndex[1] = y;
            exportData(outStream, data, yIndex, sep);
        }
        outStream << "\n";

        count++;
        if (count % draw == 0) {
            ui->progressBar->setValue(rint(count * step));
            QCoreApplication::processEvents();
        }
    }
}


void CSVExportDialog::exportCsv3D(QTextStream &outStream, QString &sep) {
    QStringList vheader, hheader, dheader;
    DataController &dc = DataController::instance();
    DataArrayInfo info = dc.getArrayInfo(this->data_src);

    if (!ui->export_selection->isChecked() || !testStartExtend()) {
        start = nix::NDSize(3,0);
        extent = nix::NDSize(3, info.shape[0]);
        extent[1] = info.shape[1];
        extent[2] = info.shape[2];
    }

    nix::NDArray data = nix::NDArray(info.dtype, {(int)partLength, (int) extent[1], 1});

    int count =0;
    int draw = 100;
    double step = 100. / extent[2]*extent[0]; // number of lines with data.

    for (size_t z = start[2] ; z < start[2]+extent[2]; ++z) {
        if (ui->export_header->isChecked()) {
            // 3rd dimension header
            dheader = dc.dimensionLabels(this->data_src, 3, z, 1);
            outStream << dheader[0];
            outStream << "\n";
            // 2nd dimension header
            export2DHeader(hheader, outStream, sep);
        }

        for (size_t x=start[0]; x < start[0]+extent[0]; ++x) {
            size_t x_ind = x-start[0];
            if (ui->export_header->isChecked()) {
                export1DHeader(vheader, outStream, sep, x);
            }
            if (x_ind % partLength == 0) {
                nix::NDSize offset = {(int)x, (int)start[1], (int)z};
                if (partLength <= extent[0]-x_ind) {
                    nix::NDSize chunk = {(int) partLength, (int) extent[1], 1};
                    dc.getData(this->data_src, info.dtype, data.data(), chunk, offset);
                    //array.getDataDirect(type, data.data(), {(int) partLength,       (int) extent[1], 1}, {});
                } else {
                    nix::NDSize chunk = {(int)(extent[0]-x_ind), (int) extent[1], 1};
                    dc.getData(this->data_src, info.dtype, data.data(), chunk, offset);
                    //array.getDataDirect(type, data.data(), {(int)(extent[0]-x_ind), (int) extent[1], 1}, {x, (int) start[1], z });
                }
            }

            for (size_t y=0; y<extent[1]; y++) {
                nix::NDSize yIndex;
                yIndex = nix::NDSize(3,x_ind % partLength);
                yIndex[1] = y;
                yIndex[2] = 0;
                exportData(outStream, data, yIndex, sep);
            }

            outStream << "\n";
            count++;
            if (count%draw == 0) {
                ui->progressBar->setValue(rint(count * step));
                QCoreApplication::processEvents();
            }
        }
    }
}


void CSVExportDialog::export1DHeader(QStringList& vheader, QTextStream&  outStream, QString& sep, size_t x) {
    DataController &dc = DataController::instance();

    size_t x_ind = x - start[0];
    if (x_ind % partLength == 0) {
        if ( partLength <= extent[0] - x_ind) {
            vheader = dc.dimensionLabels(this->data_src, 1, x, partLength);
        } else {
            vheader = dc.dimensionLabels(this->data_src, 1, x, extent[0] - x_ind);
        }
    }
    outStream << vheader[x_ind % partLength] << sep;
}


void CSVExportDialog::export2DHeader(QStringList& hheader, QTextStream&  outStream, QString& sep) {
    DataController &dc = DataController::instance();
    outStream << " " << sep;
    hheader = dc.dimensionLabels(this->data_src, 2, start[1], extent[1]);
    for (int i = 0; i < static_cast<int>(extent[1]); i++) {
        outStream << hheader[i] << sep;
    }
    outStream << "\n";
}


void CSVExportDialog::exportData(QTextStream &outStream, nix::NDArray &data, nix::NDSize &yIndex, QString &sep) {
    nix::DataType type = data.dtype();

    // how do i get the correct form of int to export depending on machine and compiler ?
    if(type == nix::DataType::Int64) {
        outStream << data.get<long long int>(yIndex) << sep;
    } else if(type == nix::DataType::Int32) {
        outStream << data.get<int>(yIndex) << sep;
    } else if(type == nix::DataType::Int16) {
        outStream << QString::number(data.get<short int>(yIndex)) << sep;
    } else if(type == nix::DataType::Int8) {
        outStream << (int)data.get<char>(yIndex) << sep;
    } else if(type == nix::DataType::Double) {
        outStream << data.get<double>(yIndex) << sep;
    } else if (type == nix::DataType::Float) {
        outStream << data.get<float>(yIndex) << sep;
    } else if(type == nix::DataType::UInt64) {
        outStream << data.get<unsigned long long int>(yIndex) << sep;
    } else if(type == nix::DataType::UInt32) {
        outStream << data.get<unsigned int>(yIndex) << sep;
    } else if(type == nix::DataType::UInt16) {
        outStream << data.get<unsigned short int>(yIndex) << sep;
    } else if(type == nix::DataType::UInt8) {
        outStream << (unsigned int)data.get<unsigned char>(yIndex) << sep;
    } else if(type == nix::DataType::Bool) {
        outStream << data.get<bool>(yIndex) << sep;
    } else if(type == nix::DataType::Char) {
        outStream << data.get<char>(yIndex) << sep;
    } else if(type == nix::DataType::String) {
        outStream << QString::fromStdString(data.get<std::string>(yIndex)) << sep;
    } else if ((type == nix::DataType::Nothing) | (type == nix::DataType::Opaque))
        //error message ?
        return;
}

bool CSVExportDialog::testStartExtend() {
    nix::NDSize shape = this->data_src.shape;

    if( (shape.size() != start.size()) || (shape.size() != extent.size()) ) {
        return false;
    } else if( start.size() != extent.size()) {
        return false;
    }
    for(size_t i=0; i<extent.size(); i++) {
        if(extent[i] == 0) {
            return false;
        }
        if(start[i]+ extent[i] > shape[i]) {
            return false;
        }
    }
    return true;
}






