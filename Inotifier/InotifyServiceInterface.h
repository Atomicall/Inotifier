#pragma once
#include <iostream>
#include "InotifierService/Inotifier.h"
#include <list>
#include <iostream>

class InotifyServiceInterface {

public:
    enum COMMADS {START = 100, STOP=200, SEND_STATUS=300, ADD_DIR=400, SET_MASK=500};
    InotifyServiceInterface();


    InotifyServiceInterface* setDirs(std::list<std::filesystem::path> paths); //<-- Both recursively
    InotifyServiceInterface* setDir(std::filesystem::path path);// <--
    InotifyServiceInterface* setOneDir(std::filesystem::path path);//<--Not recursively
    InotifyServiceInterface* setMask(uint32_t mask);
    InotifyServiceInterface* run();
    InotifyServiceInterface* stop();
    bool getStatus ();
    void setDataPipe(int* from);
    void setServiceControlPipe(int* from);
    void setServiceResponcePipe(int* from);
    void setServiceConfigurePipe(int* from);

    int* getDataPipe(){return dataPipe;}
    int* getServiceControlPipe(){return serviceControlPipe;};
    int* getServiceResponcePipe(){return serviceResponcePipe;};
    int* getServiceConfigurePipe(){return serviceConfigurePipe;};
    std::ostream* getDebugStream(){return debug_stream;};
    void setDebugStream (std::ostream* from){debug_stream = from; ino1.setDebugStream(debug_stream);};
    std::shared_ptr<FSEvent> getEvent();
    uint32_t getMask(){return ino1.getMask();};
private:
    Inotifier ino1;
    std::ostream* debug_stream;
    int dataPipe[2];
    int serviceControlPipe[2];
    int serviceResponcePipe[2];
    int serviceConfigurePipe[2];
};

