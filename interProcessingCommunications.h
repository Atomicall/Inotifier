#pragma once
#include "Inotifier/InotifyServiceInterface.h"
#include "inotifiermainwindow.h"
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>



int dataPipe[2];
int serviceControlPipe[2];
int serviceResponcePipe[2];
int serviceConfigurePipe[2];

pthread_mutex_t controlPipeMutex;
pthread_mutex_t newMut;
pthread_cond_t condVar;

struct arg_st{
    int* dataPipe;
    int* serviceControlPipe;
    int* serviceResponcePipe;
    int* serviceConfigurePipe;
    pthread_mutex_t controlPipeMutex;
    pthread_mutex_t newMut;
    pthread_cond_t condVar;
    InotifierMainwindow* mainw;
};

struct Control_args{
    InotifierMainwindow* mainw_ptr;
    InotifyServiceInterface* service_ptr;
};
volatile sig_atomic_t presentCommand = 0;
volatile sig_atomic_t exitf =0;

//void ServiceControlSignal (int signal);
//void* serviceControlThread(void* recevied_args);
//void* inotifierThread (void* arg);
void ServiceControlSignal (int signal){
    // Минное поле
    //https://habr.com/ru/post/141206/
    //@@malloc не реентерабельна.@@
    //@@string нельзя@@
    //не_атомик_нельзя@@
    //@@сигнал один для всего процесса@@
    //@@никакого праздника@@
    presentCommand=true;
    return;
}


void* serviceControlThread(void* recevied_args){
    Control_args* args = (Control_args*) recevied_args;
    int cmd;
    while (!exitf){
    pthread_mutex_lock(args->mainw_ptr->getControlPipeMutex());
    std::cerr<<"@@LOCK MUTEX@@ in ++serviceControlThread++"<<std::endl;

    if (!presentCommand){
        std::cerr<<"@@COND_WAIT@@ in ++serviceControlThread++"<<std::endl;
        pthread_cond_wait(args->mainw_ptr->getCondVar(), args->mainw_ptr->getControlPipeMutex());
        std::cerr<<"@@END COND WAIT@ in ++serviceControlThread++"<<std::endl;
    };
        std::cerr<<"!!STARTING READ from ServiceCoontrolPipe @COMMAND@ in ++serviceControlThread++"<<std::endl;
        int sta = read(args->service_ptr->getServiceControlPipe()[0], &cmd, sizeof(int));
        std::cerr<<"!!ENDING READ from ServiceCoontrolPipe @COMMAND@ in serviceControlThread"<<std::endl;
        if ( -1 ==sta ){std::cerr<<"///////////////Failed while: trying to read data from ServiceControlPipe in ++ControlThread++"<<std::endl;}
        std::cerr<<"Cmd reciever from Control Pipe is: "<<cmd<<std::endl;
        char msg [100] = {};
        int stat;
        switch (cmd) {
        case (InotifyServiceInterface::START):{


            std::cerr<<"!!STARTING WRITE to ServiceResponcePipe @STATUS@ in ++serviceControlThread++"<<std::endl;
            if(args->service_ptr->getStatus()){
               memcpy(msg , "Service is already runned", sizeof(msg));
            }
            else {
                args->service_ptr->run();
                memcpy(msg , "Inotifier service started", sizeof(msg));
            }
            stat =  write(args->service_ptr->getServiceResponcePipe()[1], &msg, sizeof(msg));
            if (-1==stat) {std::cerr<<"///////////////Failed while: trying to write data to Control Pipe in ++serviceControlThread++\n";}
            std::cerr<<"!!ENDING WRITE to ServiceResponcePipe @STATUS@ in ++serviceControlThread++"<<std::endl;
            break;
        }
        case (InotifyServiceInterface::STOP):{
            std::cerr<<"!!STARTING WRITE to ServiceResponcePipe @STATUS@ in ++serviceControlThread++"<<std::endl;
            if(!args->service_ptr->getStatus()){
               memcpy(msg , "Service is already stopped", sizeof(msg));
            }
            else {
                args->service_ptr->stop();
                memcpy(msg , "Service stopped", sizeof(msg));
                if (args->service_ptr->getStatus()){std::cerr<<"///////////////Failed while: trying to stop service from ++ControlThread++"<<std::endl;}
            }
            stat = write(args->service_ptr->getServiceResponcePipe()[1], &msg, sizeof(msg));
            if (-1==stat) {std::cerr<<"Failed while: trying to write data to Control Pipe in ControlThread\n";}
            std::cerr<<"!!ENDING WRITE to ServiceResponcePipe @STATUS@ in ++serviceControlThread++"<<std::endl;
        break;
        }
        case(InotifyServiceInterface::SEND_STATUS):{
            if (args->service_ptr->getStatus()){memcpy(msg , "Service is Runnig", sizeof(msg));}
            else{memcpy(msg , "Service has been stopped", sizeof(msg));}
            stat = write(args->service_ptr->getServiceResponcePipe()[1], &msg, sizeof(msg));
            if (-1==stat){std::cerr<<"//////////////Failed while: trying to write status service from ++ControlThread++"<<std::endl;}
            break;
        }
        case(InotifyServiceInterface::SET_MASK):{
        uint32_t tmp_mask;
        std::cerr<<"!!STARTING READ from ServiceConfigurePipe @MASK_@ in ++serviceControlThread++"<<std::endl;
        stat = read(args->service_ptr->getServiceConfigurePipe()[0], &tmp_mask, sizeof(tmp_mask));
        if (-1==stat){std::cerr<<"//////////////Failed while: trying to read Mask from Pipe in ++ControlThread++"<<std::endl;};
        std::cerr<<"!!ENDING READ from ServiceConfigurePipe @MASK_@ in ++serviceControlThread++"<<std::endl;

        std::cerr<<"Mask recieved from ConfigurePipe is:  "<<tmp_mask<<std::endl;

        args->service_ptr->setMask(tmp_mask);
        std::string tmp_stat_mask = MaskDecodeServiceComponent(args->service_ptr->getMask()).decodeMask();
        std::clog<< "+-+-+-+-Mask decode is: " << tmp_stat_mask <<std::endl;
        memcpy(msg , "Mask for Inotifier Set", sizeof(msg));
        std::cerr<<"!!STARTING WRITE to ServiceResponcePipe @MASK_SET_OK_RESP@ in ++serviceControlThread++"<<std::endl;
        stat = write(args->service_ptr->getServiceResponcePipe()[1], &msg, sizeof(msg));
        if (-1==stat){std::cerr<<"//////////////Failed while: trying to write status MASK_SET from ControlThread\n";}
        std::cerr<<"!!ENDING WRITE to ServiceResponcePipe @MASK_SET_OK_RESP@ in ++serviceControlThread++"<<std::endl;
        break;
        }
        case (InotifyServiceInterface::ADD_DIR):{
            char msg_rec[500]={};
            std::cerr<<"!!STARTING READ from ServiceConfigurePipe @DIR_FILE_PATH@ in ++serviceControlThread++"<<std::endl;
            int stat = read(args->service_ptr->getServiceConfigurePipe()[0], &msg_rec, sizeof(msg_rec));
            if (-1==stat){std::cerr<<"//////////////Failed while: trying to read DIR_FILE_Path from Pipe in ++ControlThread++"<<std::endl;};
            std::cerr<<"!!ENDING READ from ServiceConfigurePipe @DIR_FILE_PATH@ in ++serviceControlThread++"<<std::endl;
            std::string tmp_path (msg_rec);
            std::clog<<"Path recieved from ConfigurePipe is:  "<<tmp_path<<" Or "<<msg_rec<<std::endl;
            args->service_ptr->setDir(tmp_path);
            memcpy(msg , "Path for Inotifier Set", sizeof(msg));
            std::cerr<<"!!STARTING WRITE to ServiceResponcePipe @PATH_SET_OK@ in ++serviceControlThread++"<<std::endl;
            stat = write(args->service_ptr->getServiceResponcePipe()[1], &msg, sizeof(msg));
            if (-1==stat){std::cerr<<"Failed while: trying to write status MASK_SET from ++ControlThread++"<<std::endl;}
            std::cerr<<"!!ENDING WRITE to ServiceResponcePipe @PATH_SET_OK@ in ++serviceControlThread++"<<std::endl;
            break;
        }
        default: {
            std::cerr<<"Unknow command: "<< cmd<<std::endl;
        }
        }
        presentCommand =0;
        std::cerr<<"!!STARTING EMIT"<<std::endl;
        emit args->mainw_ptr->pipeResponceReady();
        std::cerr<<"!!ENDING EMIT"<<std::endl;
        std::cerr<<"@@UNLOCK MUTEX@@ in ++serviceControlThread++"<<std::endl;
        pthread_mutex_unlock(args->mainw_ptr->getControlPipeMutex());

    }
    std::cerr<<"Exit Control";
    return NULL;
}

void* inotifierThread (void* arg){
    // ! Не баг, а фича. В случае моего сервиса его можно только приостановить (т.к можно только отменить процесс, в котором работает epoll_wait). Поэтому после запуска он выдает все ивенты,
    // что произошли в период простоя
    arg_st* args_struct = (arg_st*) arg;
    InotifierMainwindow* mainw = args_struct->mainw;
    InotifyServiceInterface inter;

    inter.setDataPipe(args_struct->dataPipe);
    inter.setServiceControlPipe(args_struct->serviceControlPipe);
    inter.setServiceResponcePipe(args_struct->serviceResponcePipe);
    inter.setServiceConfigurePipe((args_struct->serviceConfigurePipe));


    pthread_t serviceControlThreadId;
    Control_args args = {mainw, &inter};
    pthread_create(&serviceControlThreadId, NULL, serviceControlThread, (void*)&args);
    std::shared_ptr<FSEvent> temp_event=nullptr;
    while (!exitf){
        while (inter.getStatus()) {
               temp_event = inter.getEvent(); // Блокируется, пока не получит событие (т.е ф-д не изменится)
               if (temp_event==nullptr){continue;}
               if (!temp_event->isEmpty()){
                   FSEvent tmp = *temp_event;
                   write(inter.getDataPipe()[1], &tmp, sizeof(tmp)); //колхоз, пиши правильную сериализацию
                   emit mainw->newFSEventInDataPipe();
               }
            }
    }
    std::cerr<<"Exit Ser";
    return NULL;
}




