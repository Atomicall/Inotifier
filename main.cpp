#include"inotifiermainwindow.h"
#include "Inotifier/InotifyServiceInterface.h"
#include <QApplication>
#include <sstream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <pthread.h>

#include <signal.h>
int dataPipe[2];
int serviceControlPipe[2];
int serviceResponcePipe[2];


struct arg_st{
    int* dataPipe;
    int* serviceControlPipe;
    int* serviceResponcePipe;
    InotifierMainwindow* mainw;
};
struct Control_args{
    InotifierMainwindow* mainw_ptr;
    InotifyServiceInterface* service_ptr;
};

volatile sig_atomic_t presentCommand = 0; // Есть ли команда
volatile sig_atomic_t exitf =0;


void ServiceControlSignal (int signal){
    // Минное поле
    //https://habr.com/ru/post/141206/
    //@@malloc не реентерабельна.@@
    //@@string нельзя@@
    //не_атомик_нельзя@@
    //@@сигнал один для всего процесса@@
    //@@никакого праздника@@
    std::cerr<<"SIGAL\n";
    presentCommand=true;
    return;
}

void ResponceDataReadySignal (int signal){


    return;
}


// 1 раз только умеем стартовать
void* serviceControlThread(void* recevied_args){
    Control_args* args = (Control_args*) recevied_args;
    // Мютекс бахнуть для getStatus
    int cmd=-1;
    while (!exitf){ // Пока запущен сервис, ждем команду
    while (presentCommand){
        int sta = read(args->service_ptr->getServiceControlPipe()[0], &cmd, sizeof(int));
        if ( -1 ==sta ){std::cerr<<"Failed while: trying to read data from ServiceControlPipe in"
                       " ControlThread\n";}
        std::clog<<"Cmd reciever from Control Pipe is: "<<cmd<<std::endl;
        switch (cmd) {
        case (InotifyServiceInterface::START):{
            if(args->service_ptr->getStatus()){
               const char msg[] = "Service is already running";
               int stat =  write(args->service_ptr->getServiceResponcePipe()[1], &msg, sizeof(msg)); // неблокирующий прием на стороне интерфейса, уведомление сигналом!!
               if (-1==stat) {std::cerr<<"Failed while: trying to write data to Control Pipe in ControlThread\n";}
            }
            else {
                args->service_ptr->run();
                //delay(1);
                const char msg[] = "Starting service";
                int stat = write(args->service_ptr->getServiceResponcePipe()[1], &msg, sizeof(msg));
                if (-1==stat) {std::cerr<<"Failed while: trying to write data to Control Pipe in ControlThread\n";}
                if (!args->service_ptr->getStatus()){std::cerr<<"Failed while: trying to start service from ControlThread\n";}

            }
            break;
        }
        case (InotifyServiceInterface::STOP):{
            if(!args->service_ptr->getStatus()){
               const char msg[] = "Service is already stopped";
               int stat = write(args->service_ptr->getServiceResponcePipe()[1], &msg, sizeof(msg));
               if (-1==stat) {std::cerr<<"Failed while: trying to write data to Control Pipe in ControlThread\n";}
            }
            else {
                args->service_ptr->stop();
                const char msg[] = "Stoppped service";
                int stat = write(args->service_ptr->getServiceResponcePipe()[1], &msg, sizeof(msg));
               if (-1==stat) {std::cerr<<"Failed while: trying to write data to Control Pipe in ControlThread\n";}
                if (args->service_ptr->getStatus()){std::cerr<<"Failed while: trying to stop service from ControlThread\n";}
            }

        break;
        }
        case(InotifyServiceInterface::SEND_STATUS):{
            bool status = args->service_ptr->getStatus();
                const char* msg;
                if (status){msg =  "Service is Runnig";}
                else {msg = "Service Stopped\n";};
                int stat = write(args->service_ptr->getServiceResponcePipe()[1], &msg, sizeof(msg));
                if (-1==stat){std::cerr<<"Failed while: trying to write status service from ControlThread\n";}
            break;
        }
        default: {
            std::cerr<<"Unknow command: "<< cmd<<std::endl;
        }
        }
        presentCommand =0;
        emit args->mainw_ptr->pipeResponceReady();

    }
    }
    std::cerr<<"Exit Control";
    return NULL;
}


void* inotifierThread (void* arg){ // запущен тред, запущен интерфейс. нарисуй их общение, где хранить пайпы, как обрабатыать сигналы и кто их оюработает. Как
    // сказать сервису прочитать инфу-команду из пайпа и где этот пайп взять
    // походу надо делать отдельные потоки
    // сделай бэкап обязательно
    arg_st* args_struct = (arg_st*) arg;
    InotifierMainwindow* mainw = args_struct->mainw;
    InotifyServiceInterface inter;

    inter.setDataPipe(args_struct->dataPipe);
    inter.setServiceControlPipe(args_struct->serviceControlPipe);
    inter.setServiceResponcePipe(args_struct->serviceResponcePipe);



    inter.setMask(IN_MODIFY | IN_CREATE| IN_DELETE| IN_DELETE_SELF)->setDir("/tmp/11/");
    // Отдельный тред для контроля сервиса
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
                   //Немного синтаксического сахара
                   emit mainw->newFSEventInDataPipe();
               }
            }
    }
    std::cerr<<"Exit Ser";
    return NULL;
}


int main(int argc, char *argv[])
{
    QApplication app (argc, argv);
    InotifierMainwindow mainw;
    signal (SIGUSR1, ServiceControlSignal);
    signal (SIGUSR2, ResponceDataReadySignal);



    if (-1 == pipe(dataPipe)) {std::cerr<<"Data pipe problem"<<std::endl; exit(1);}
    if (-1 == pipe(serviceControlPipe)) {std::cerr<<"ServiceControl pipe problem"<<std::endl; exit(1);}
   if (-1 == pipe(serviceResponcePipe)) {std::cerr<<"ServiceResponce pipe problem"<<std::endl; exit(1);}

    mainw.setDataPipe(dataPipe); // в одну ф-ю
    mainw.setServiceControlPipe(serviceControlPipe);
    mainw.setServiceResponcePipe(serviceResponcePipe);



    arg_st args= {dataPipe,serviceControlPipe, serviceResponcePipe, &mainw};
    pthread_t serviceThreadId;
    // Аргументы потока поумолчанию нас вполне устраивают
    pthread_create(&serviceThreadId, NULL, inotifierThread, (void*)&args);




    //делаем пайп, настраиваем сервис на вывод туда
    //запускаем гуи, ждем добавления, запускаем сервис
    //сигнал ф-и проверки пайпа на слот вывода данных из пайпа
    // добавляем данные через пайп (или раздел память)
    // получаем ивенты через другой пайп
    //через sigusr1 можем запросить статус сервиса







    mainw.mainWidget->show();
    return app.exec();;
}
