#include "loadthread.h"
#include <QVector>
#include <chrono>
#include <thread>


LoadThread::LoadThread(QObject *parent, unsigned int chunksize):
    QThread(parent) {
    abort = false;
    restart = false;
    this->chunksize = chunksize;
}

LoadThread::~LoadThread() {
    mutex.lock();
    abort = true;
    condition.wakeOne();
    mutex.unlock();

    wait();
}


void LoadThread::run() {
    mutex.lock();
    restart = true;
    while (!abort) {
        if (restart) {
            restart = false;
        } else {
            condition.wait(&mutex);
            continue;
        }
        // loading stuff because of restart == true
        nix::DataArray array = this->array;
        nix::NDSize start = this->start;
        nix::NDSize extent = this->extent;
        unsigned int chunksize = this->chunksize;
        int graphIndex = this->graphIndex;
        int dimCount = this->dataExtent.size();
        unsigned int dimNumber;
        std::vector<int> index2D;

        if (dimCount > 1) {
            dimNumber = this->dimNumber;
            index2D = this->index2D;
        }
        mutex.unlock();

        if(dimCount == 1) {
            load1D(array, start, extent, chunksize, graphIndex);
        } else if(dimCount == 2) {
            load2D(array, start, extent, dimNumber, index2D, chunksize, graphIndex);
        }
        mutex.lock();
    }
    mutex.unlock();
}


void LoadThread::load1D(const nix::DataArray &array, nix::NDSize start, nix::NDSize extent, unsigned int chunksize, int graphIndex) {
    int dataLength = extent[0];
    unsigned int offset = start[0];
    int totalChunks = (dataLength / chunksize) + 1;

    if( dataLength / chunksize == static_cast<double>(dataLength) / chunksize) {
        totalChunks = dataLength / chunksize;
    }

    extent[0] = chunksize;
    std::vector<double> loadedData;
    std::vector<double> chunkdata(chunksize);

    bool brokenData = false;
    for (int i=0; i<totalChunks; i++) {
        mutex.lock();
        if(restart) {
            brokenData = true;
            mutex.unlock();
            break;
        }
        mutex.unlock();

        emit(progress(static_cast<double>(i)/totalChunks, graphIndex)); //starts with 0 ends with one step below 1

        if(i == totalChunks-1) {
            extent[0] = (dataLength - (totalChunks-1) * chunksize);
            chunkdata.resize((dataLength - (totalChunks-1) * chunksize));
        }
        start[0] = offset + i * chunksize;
        array.getData(chunkdata, extent, start);

        loadedData.insert(loadedData.end(), chunkdata.begin(), chunkdata.end());

        //loadedData.append(QVector<double>::fromStdVector(chunkdata));
    }

    if(! brokenData) {
        QVector<double> axis(0);
        getAxis(array.getDimension(1), axis, dataLength, offset);
        emit dataReady(QVector<double>::fromStdVector(loadedData), axis, graphIndex);
    }
}


void LoadThread::load2D(const nix::DataArray &array, nix::NDSize start, nix::NDSize extent, unsigned int xDim, std::vector<int> index2D, unsigned int chunksize, int graphIndex) {
    unsigned int xDimIndex = xDim-1;
    unsigned int dataLength = extent[xDimIndex];
    unsigned int offset = start[xDimIndex];

    int totalChunks;
    if( dataLength / chunksize == static_cast<double>(dataLength) / chunksize) {
        totalChunks = dataLength / chunksize;
    } else {
        totalChunks = dataLength / chunksize + 1;
    }

    extent[xDimIndex] = chunksize;
    std::vector<double> chunkdata(chunksize);

    //if index2D is empty do all.
    if(index2D.size() == 0) {
        index2D.resize(array.dataExtent()[1-xDimIndex]);
        for(unsigned int i=0; i< array.dataExtent()[1-xDimIndex]; i++) {
            index2D[i] = static_cast<int>(i);
        }
    }

    //iterate over given indices
    for(unsigned int j=0; j<index2D.size(); j++) {
        bool brokenData = false;
        std::vector<double> loadedData;

        start[1-xDimIndex] = index2D[j];

        mutex.lock();
        if(restart) {
            brokenData = true;
            mutex.unlock();
            break;
        }
        mutex.unlock();

        for (int i=0; i<totalChunks; i++) {
            mutex.lock();
            if(restart) {
                brokenData = true;
                mutex.unlock();
                break;
            }
            mutex.unlock();

            emit(progress(static_cast<double>(i)*j / (totalChunks*index2D.size()), graphIndex)); //starts with 0 ends with one step below 1

            if(i == totalChunks-1) {
                extent[xDimIndex] = (dataLength - (totalChunks-1) * chunksize);
                chunkdata.resize((dataLength - (totalChunks-1) * chunksize));
            }
            start[xDimIndex] = offset + i * chunksize;

            //array.getData(array.dataType(),chunkdata.data(),extent, start);
            array.getData(chunkdata, extent, start);
            loadedData.insert(loadedData.end(), chunkdata.begin(), chunkdata.end());
            }

        if(! brokenData) {
            QVector<double> axis(0);
            getAxis(array.getDimension(xDim), axis, dataLength, offset);

            emit dataReady(QVector<double>::fromStdVector(loadedData), axis, graphIndex + index2D[j]);
        }
    }
}



void LoadThread::getAxis(nix::Dimension dim, QVector<double> &axis, unsigned int count, unsigned int offset) {

    if(dim.dimensionType() == nix::DimensionType::Sample) {
        axis = QVector<double>::fromStdVector(dim.asSampledDimension().axis(count, offset));
    } else if(dim.dimensionType() == nix::DimensionType::Range) {
        axis = QVector<double>::fromStdVector(dim.asRangeDimension().axis(count, offset));
    } else {
        axis.resize(count);
        for (unsigned int i=0; i<count; i++) {
            axis[0] = i+offset;
        }
    }
}


void LoadThread::setVariables(const std::string &arrayId, const nix::Block &block, nix::NDSize start, nix::NDSize extent, std::vector<int> index2D, unsigned int dimNumber, int graphIndex) {
    QMutexLocker locker(&mutex); // locks the members and unlocks them when it goes out of scope.

    this->array = block.getDataArray(arrayId);
    this->dataExtent = this->array.dataExtent();
    this->xDimension = this->array.getDimension(dimNumber);
    this->dimType = this->xDimension.dimensionType();
    if (this->dimType == nix::DimensionType::Range) {
        this->ticks = xDimension.asRangeDimension().ticks();
    }
    if (this->dimType == nix::DimensionType::Sample) {
        nix::SampledDimension sd = xDimension.asSampledDimension();
        this->offset = sd.offset() ? *sd.offset() : 0.0;
        this->samplingInterval = sd.samplingInterval();
    }

    this->start = start;
    this->extent = extent;
    this->graphIndex = graphIndex;
    this->index2D = index2D;
    this->dimNumber = dimNumber;

    if (!testInput(array, start, extent)) {
        std::cerr << "LoadThread::setVariables(): Input not correct." << std::endl;
        return;
    }
    if (!isRunning()) {
        QThread::start(LowPriority);
    } else {
        this->restart = true;
        condition.wakeOne();
    }
}

void LoadThread::setVariables1D(const std::string &arrayId, const nix::Block &block, nix::NDSize start, nix::NDSize extent, int graphIndex) {
    QMutexLocker locker(&mutex);

    this->array = block.getDataArray(arrayId);
    this->dataExtent = this->array.dataExtent();
    this->xDimension = this->array.getDimension(1);
    this->dimType = this->xDimension.dimensionType();

    if (this->dimType == nix::DimensionType::Range) {
        this->ticks = xDimension.asRangeDimension().ticks();
    }
    if (this->dimType == nix::DimensionType::Sample) {
        nix::SampledDimension sd = xDimension.asSampledDimension();
        this->offset = sd.offset() ? *sd.offset() : 0.0;
        this->samplingInterval = sd.samplingInterval();
    }

    this->start = start;
    this->extent = extent;
    this->graphIndex = graphIndex;

    if (dataExtent.size() != 1) {
        std::cerr << "LoadThread::setVariables1D() given array has more than 1 dimension." << std::endl;
    }

    if (!testInput(array, start, extent)) {
        std::cerr << "LoadThread::setVariables1D(): Input not correct." << std::endl;
        return;
    }

    if (!isRunning()) {
        QThread::start(LowPriority);
    } else {
        this->restart = true;
        condition.wakeOne();
    }
}

void LoadThread::setChuncksize(unsigned int size) {
    if(size == 0) {
        std::cerr << "LoadThread::setChunksize(): Size can't be zero." << std::endl;
        return;
    }

    mutex.lock();
    chunksize = size;
    mutex.unlock();
}


void LoadThread::restartThread(nix::NDSize start, nix::NDSize extent) {
    QMutexLocker locker(&mutex);

    this->start = start;
    this->extent = extent;

    if (!isRunning()) {
        std::cerr << "You have to call Loadthread::setVariables[1D]() first to set all needed members." << std::endl;
    } else {
        this->restart = true;
        condition.wakeOne();
    }
}


void LoadThread::startLoadingIfNeeded(QCPRange range, int xDim, double dataMin, double dataMax, double samplerate) {
    mutex.lock();
    double numOfPoints = static_cast<double>(this->chunksize) / 3;
    mutex.unlock();

    nix::NDSize start, extent;
    if (dataMin == dataMax) {
        calcStartExtent(array, start, extent, range, xDim);
        std::cerr << "Restarting 0\n";

        restartThread(start, extent);
        return;
    }

    if ((range.lower - dataMin) * samplerate < numOfPoints/4) {
        if (checkForMoreData(array, dataMin, false, xDim)) {
            calcStartExtent(array, start, extent, range, xDim);
            restartThread(start, extent);
        }
    }
    if ((dataMax - range.upper) * samplerate < numOfPoints / 4) {
        if (checkForMoreData(array, dataMax, true, xDim)) {
            calcStartExtent(array, start, extent, range, xDim);
            restartThread(start, extent);
        }
    }
}


void LoadThread::calcStartExtent(const nix::DataArray &array, nix::NDSize &start_size, nix::NDSize &extent_size, QCPRange curRange, int xDim) {
    nix::Dimension d = array.getDimension(xDim);
    mutex.lock();
    nix::NDSize tempDataExtent = this->dataExtent;
    nix::DataArray a = this->array;
    nix::Dimension d = this->xDimension;
    double samplingInterval = this->samplingInterval;
    double offset = this->offset;
    nix::DimensionType dimType = this->dimType;
    std::vector<double> ticks = this->ticks;
    mutex.unlock();

    double start, extent;
    if (d.dimensionType() == nix::DimensionType::Set) {
        start = 0;
        extent = tempDataExtent[xDim-1];
    } else {
        double pInRange;
        double startIndex;

        if (dimType == nix::DimensionType::Sample) {
            startIndex = (curRange.lower - offset) / samplingInterval;
            pInRange = curRange.size() / samplingInterval; // number of data points in the x_range
        } else { // rangeDimension
            startIndex = std::distance(ticks.cbegin(), std::lower_bound(ticks.cbegin(), ticks.cend(), curRange.lower));
            pInRange   = std::distance(ticks.cbegin(), std::upper_bound(ticks.cbegin(), ticks.cend(), curRange.upper)) - startIndex;
        }

        double numOfPoints = static_cast<double>(chunksize) / 3;

        if (pInRange <= numOfPoints) {
            start  = startIndex - numOfPoints;
            extent =  numOfPoints + pInRange + numOfPoints;

        } else if (pInRange > numOfPoints && pInRange < numOfPoints * 3) {
            start  = startIndex - numOfPoints / 2;
            extent = pInRange + numOfPoints; //numOfPoints/2 + pInRange + numOfPoints/2
        } else {
            start  = startIndex;
            extent = pInRange + 1;
        }
    }

    start = std::floor(start);
    extent = std::ceil(extent);

    if (start < 0) {
        start = 0;
    }

    if (extent > dataExtent[xDim-1] - start) {
        extent = dataExtent[xDim-1] - start;
    } else if(extent < 1) {
        extent = 1;
    }

    if (dataExtent.size() == 1) {
        start_size = nix::NDSize({static_cast<int>(start)});
        extent_size = nix::NDSize({static_cast<int>(extent)});
    } else {
        start_size = nix::NDSize({0, 0});
        start_size[xDim-1] = static_cast<int>(start);
        extent_size = nix::NDSize({1,1});
        extent_size[xDim-1] = static_cast<int>(extent);
    }
}


bool LoadThread::checkForMoreData(const nix::DataArray &array, double currentExtreme, bool higher, int xDim) {
    nix::Dimension d = array.getDimension(xDim);
    mutex.lock();
    nix::Dimension d = this->xDimension;
    nix::NDSize dataExtent = this->dataExtent;
    nix::DimensionType dimType = this->dimType;
    double samplingInterval = this->samplingInterval;
    double offset = this->offset;
    std::vector<double> ticks = this->ticks;
    mutex.unlock();

    if (dimType == nix::DimensionType::Set) {
        std::cerr << "LoadThread::CheckForMoreData(): check set dim... no! Not yet." << std::endl;
        return false;
    } else if (dimType == nix::DimensionType::Sample) {
        if (higher) {
            return (dataExtent[xDim-1] - 1 * samplingInterval - offset > currentExtreme);
        } else {
            return (offset < currentExtreme);
        }
    } else if (dimType == nix::DimensionType::Range) {
        if (higher) {
            return (ticks.back() > currentExtreme);
        } else {
            return (ticks.front() < currentExtreme);
        }
    } else {
        std::cerr << "LoadThread::CheckForMoreData(): unsupported dimension type.\t" << dimType << std::endl;
        return false;
    }
}

bool LoadThread::testInput(const nix::DataArray &array, nix::NDSize start, nix::NDSize extent) {
    nix::NDSize size = array.dataExtent();
    if( ! (size.size() == start.size() && size.size() == extent.size())) {
    size_t rank = dataExtent.size();
    if( ! (rank == start.size() && rank == extent.size())) {
        std::cerr << "DataThread::testInput(): start and/or extent don't have the same dimensionality as the array." << std::endl;
        return false;
    }

    bool Dataload1d = false;
    for(size_t i = 0; i < rank; ++i) {
        if(dataExtent[i] < start[i] + extent[i]) {
            std::cerr << "DataThread::testInput(): start + extent bigger than length of array in dimension " << i << std::endl;
            return false;
        }

        if(extent[i] > 1) { // extent has to define 1d data (all 1 exept for one entry)
            if (Dataload1d) {
                std::cerr << "DataThread::testInput(): extent defines data that is more than one dimensional." << std::endl;
                return false;
            } else {
                Dataload1d = true;
            }
        }
    }
    if(! Dataload1d) {
        std::cerr << "DataThread::testInput(): using DataThread to load a single datum." << std::endl;
    }
    return true;
}
