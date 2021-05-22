#include "InotifyServiceInterface.h"
#include <cassert>

InotifyServiceInterface::InotifyServiceInterface() {
    std::clog<<"___Creating Inotify Service"<<std::endl;
    assert(ino1.getEpollFd()!=0 && ino1.getInotifyFd());
    std::clog<<"___Created"<<std::endl;

}

InotifyServiceInterface* InotifyServiceInterface::setDir(std::filesystem::path path) {
    ino1.addDirSRecursively(path);
    return this;
}
InotifyServiceInterface* InotifyServiceInterface::setDirs(std::list<std::filesystem::path> paths){
    auto it = paths.begin();
    while (it!= paths.end()){
        ino1.addDirSRecursively(*it);
        it++;
    }
    return this;
}

InotifyServiceInterface* InotifyServiceInterface::setOneDir(std::filesystem::path path){
    ino1.addDirToWatch(path);
    return this;
}


InotifyServiceInterface* InotifyServiceInterface::setMask(uint32_t mask) {
    ino1.setEventsMask(mask);
    return this;
}

bool InotifyServiceInterface::getStatus() {

    return ino1.IsRunned();
}

std::shared_ptr<FSEvent> InotifyServiceInterface::getEvent() {


   // return (NULL==(ino1.getNextEvent())? NULL: *ino1.getNextEvent());

    return ino1.getNextEvent();
}

InotifyServiceInterface *InotifyServiceInterface::run() {
    ino1.runService();
    if (ino1.IsRunned()){std::clog<<"___Inotifier Started Succesfully"<<std::endl;}
    else {std::cerr<<"___Failed while: starting Inotifier in run()\n";};
    return this;
}
InotifyServiceInterface *InotifyServiceInterface::stop() {
    ino1.stopService();
    if (!ino1.IsRunned()){std::clog<<"___Inotifier Stopped Succesfully"<<std::endl;}
    else {std::cerr<<"___Failed while: stoping Inotifier in stop()\n";};
    return this;

}



void InotifyServiceInterface::setDataPipe(int *from){
    dataPipe[0] = from[0];
    dataPipe[1] = from[1];
}

void InotifyServiceInterface::setServiceControlPipe(int* from){
    serviceControlPipe[0] = from[0];
    serviceControlPipe[1] = from[1];
}

void InotifyServiceInterface::setServiceResponcePipe(int *from){
    serviceResponcePipe[0] = from[0];
    serviceResponcePipe[1] = from[1];
}

void InotifyServiceInterface::setServiceConfigurePipe(int *from){
    serviceConfigurePipe[0] = from[0];
    serviceConfigurePipe[1] = from[1];
}

