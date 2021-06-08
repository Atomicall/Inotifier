
#include"inotifiermainwindow.h"
#include "interProcessingCommunications.h"
#include "Inotifier/InotifyServiceInterface.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app (argc, argv);
    InotifierMainwindow mainw;
    signal (SIGUSR1, ServiceControlSignal);
    int erro = pthread_mutex_init(&controlPipeMutex, NULL);
    if (erro != 0)
    { std::cerr<<"Error creating ControlPipe Mutex"<<std::endl;exit(-1);}
    erro = pthread_mutex_init(&newMut, NULL);
    if (erro != 0)
    {std::cerr<<"Error creating newMut Mutex"<<std::endl;  exit(-1);}
    erro = pthread_cond_init(&condVar, NULL);
    if (erro != 0){ std::cerr<<"Error creating condVar"<<std::endl;exit(-1); }


    if (-1 == pipe(dataPipe)) {std::cerr<<"Data pipe problem"<<std::endl; exit(1);}
    if (-1 == pipe(serviceControlPipe)) {std::cerr<<"ServiceControl pipe problem"<<std::endl; exit(1);}
    if (-1 == pipe(serviceResponcePipe)) {std::cerr<<"ServiceResponce pipe problem"<<std::endl; exit(1);}
    if (-1 == pipe(serviceConfigurePipe)) {std::cerr<<"ServiceResponce pipe problem"<<std::endl; exit(1);}
    mainw.initPipes(dataPipe, serviceControlPipe, serviceResponcePipe, serviceConfigurePipe);
    mainw.setControlPipeMutexAndCondVar(controlPipeMutex, newMut, condVar);
    arg_st args= {dataPipe,serviceControlPipe, serviceResponcePipe,  serviceConfigurePipe, controlPipeMutex, newMut, condVar, &mainw};


    pthread_t serviceThreadId;
    pthread_create(&serviceThreadId, NULL, inotifierThread, (void*)&args);
    mainw.mainWidget->show();
    return app.exec();;
}

