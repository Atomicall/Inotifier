#include "inotifiermainwindow.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

InotifierMainwindow::InotifierMainwindow(QWidget *parent) : QMainWindow(parent),
    mainWidget(new QWidget(parent)),
    addButton(new QPushButton("Add")),
    stopButton(new QPushButton("Start Service")),
    pathLine(new QLineEdit()),
    statusLabel(new QLabel("Tmp status")),
    eventsField(new QTextEdit("Events"))
{

    mainWidget->resize(QGuiApplication::primaryScreen()->geometry().width()*0.5,
                      QGuiApplication::primaryScreen()->geometry().height()*0.4);

    QGridLayout* main_layout = new QGridLayout;
    main_layout->setContentsMargins(5, 5, 5, 5);

    QHBoxLayout* topLayout = new QHBoxLayout;
    QHBoxLayout* bottomLayout = new QHBoxLayout;
    eventsField->setReadOnly(1);
    topLayout->addWidget(eventsField);


    bottomLayout->addWidget((new QLabel("Path:")));
    bottomLayout->addWidget(pathLine);


    bottomLayout->addWidget(addButton);
    bottomLayout->addWidget(stopButton);


    main_layout->addLayout(topLayout,0, 0);
    main_layout->addLayout(bottomLayout,1, 0);
    main_layout->addWidget(statusLabel, 2, 0);
    mainWidget->setLayout(main_layout);

    QString pathLocation = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    if (!QFileInfo(pathLocation).isDir()){
        pathLocation = QDir::currentPath();
    }

    pathLine->setText(QDir::toNativeSeparators(pathLocation));
    connect(addButton, &QAbstractButton::clicked, this, &InotifierMainwindow::stopButtonSlot);
    connect(stopButton, &QAbstractButton::clicked, this, &InotifierMainwindow::startButton);


}


bool InotifierMainwindow::getWasStarted(){
    return wasStarted;
}

void InotifierMainwindow::setWasStarted(bool s){
    wasStarted = s;
}

void InotifierMainwindow::pathProcessing(){
    QString tmppath = pathLine->text().trimmed();
    if (tmppath.isEmpty()) return;
    if (!QFileInfo(tmppath).isDir()){
        tmppath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        eventsField->append("Seems like this is not a valid folder. ");  eventsField->append (getWasStarted() ? "Service did not start" : "Folder hasn't been added");
        pathLine->setText(tmppath);
        return;
    }
    if (!getWasStarted()){
    eventsField->append("Trying start Service with path: "+ tmppath);
    if (1) setWasStarted(1);
    }
    else {
        if (1){
        eventsField->append("Trying to add Folder to Service with path: "+ tmppath);
        }
        else {
             eventsField->append("ERRO: Service not working, but it is unexpected");
        }
    }
    return;


}

void InotifierMainwindow::newFSEventInDataPipe(){

    char msg_in[100];
    ssize_t len = read(this->getDataPipe()[0], msg_in, sizeof (msg_in));
    if (len == -1) {std::cerr<<"Failed while: reading from"
"data Pipe\n"; return;}
    FSEvent* newfsev = (FSEvent*)msg_in;
    // BIG TO DO красивый вывод
    std::stringstream ss;
    ss << *newfsev;
    this->eventsField->append(QString(ss.str().c_str()));
}



void InotifierMainwindow::pipeResponceReady(){

    char msg_in[100];
    ssize_t len = read(this->getServiceResponcePipe()[0], msg_in, sizeof (msg_in));
    if (len == -1) {std::cerr<<"Failed while: reading from"
"Responce Pipe\n"; return;}
    this->statusLabel->setText(QString(msg_in));
}


void InotifierMainwindow::startButton(){
    std::clog<<"StartButton Clicked, sending START command\n";
    int command = InotifierMainwindow::COMMANDS::START;
    int status = (write(this->getServiceControlPipe()[1], &command, sizeof(int)));
    if(-1== status){
        std::cerr<<"Failed while: sendind START command into ServiceControlPipe in startButton";
    }
    else {std::cerr<<"Wrote Bytes: "<<status;}


    kill(getpid(), SIGUSR1);
    return;
}

void InotifierMainwindow::statusButton(){
    std::clog<<"StatusButton Clicked, sending STAUS command\n";
    int command = InotifierMainwindow::COMMANDS::SEND_STATUS;
    int status = (write(this->getServiceControlPipe()[1], &command, sizeof(int)));
    if(-1== status){
        std::cerr<<"Failed while: sendind STATUS command into ServiceControlPipe in startButton";
    }
    else {std::cerr<<"Wrote Bytes: "<<status;}
    kill(getpid(), SIGUSR1);
    return;
}

void InotifierMainwindow::stopButtonSlot(){
    std::clog<<"StopButton Clicked, sending STOP command\n";
    int command = InotifierMainwindow::COMMANDS::STOP;
    int status = (write(this->getServiceControlPipe()[1], &command, sizeof(int)));
    if(-1== status){
        std::cerr<<"Failed while: sendind STOP command into ServiceControlPipe in startButton";
    }
    else {std::cerr<<"Wrote Bytes: "<<status;}
    kill(getpid(), SIGUSR1);
    return;
}

void InotifierMainwindow::setDataPipe(int *from){
    dataPipe[0] = from[0];
    dataPipe[1] = from[1];
}

void InotifierMainwindow::setServiceControlPipe(int* from){
    serviceControlPipe[0] = from[0];
    serviceControlPipe[1] = from[1];
}

void InotifierMainwindow::setServiceResponcePipe(int *from){
    serviceResponcePipe[0] = from[0];
    serviceResponcePipe[1] = from[1];
}
