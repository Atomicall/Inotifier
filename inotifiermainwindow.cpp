#include "inotifiermainwindow.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

InotifierMainwindow::InotifierMainwindow(QWidget *parent) : QMainWindow(parent),
    mainWidget(new QWidget(parent)),
    addButton(new QPushButton("Add")),
    stopButton(new QPushButton("Stop Service")),
    startButton(new QPushButton("Start Service")),
    pathLine(new QLineEdit()),
    eventsField(new QTextEdit("There will Appear Events")),
   statusLabel(new QLabel("Nothing to do there")),
   maskGroupBox(new QGroupBox("Mask"))
{
    mainWidget->resize(QGuiApplication::primaryScreen()->geometry().width()*0.5,
                      QGuiApplication::primaryScreen()->geometry().height()*0.4);
    QGridLayout* main_layout = new QGridLayout;
    main_layout->setContentsMargins(5, 5, 5, 5);
    QHBoxLayout* topLayout = new QHBoxLayout;
    QGridLayout* bottomLayout = new QGridLayout;
    topLayout->addWidget(eventsField);
    QGridLayout* MaskLayout =new QGridLayout();
    IN_ACCESS_box = createCheckBox("IN_ACCESS");
    MaskLayout->addWidget(IN_ACCESS_box, 0, 0);
    IN_ATTRIB_box = createCheckBox("IN_ATTRIB");
    MaskLayout->addWidget(IN_ATTRIB_box, 1, 0);
    IN_CLOSE_WRITE_box = createCheckBox("IN_CLOSE_WRITE");
    MaskLayout->addWidget(IN_CLOSE_WRITE_box, 2, 0);
    IN_CLOSE_NOWRITE_box = createCheckBox("IN_CLOSE_NOWRITE");
    MaskLayout->addWidget(IN_CLOSE_NOWRITE_box, 3, 0);
    IN_CLOSE_box = createCheckBox("IN_CLOSE");
    MaskLayout->addWidget(IN_CLOSE_box, 4, 0);
    IN_CREATE_box = createCheckBox("IN_CREATE");
    MaskLayout->addWidget(IN_CREATE_box, 5, 0);
    IN_DELETE_box = createCheckBox("IN_DELETE");
    MaskLayout->addWidget(IN_DELETE_box, 6, 0);
    IN_DELETE_SELF_box = createCheckBox("IN_DELETE_SELF");
    MaskLayout->addWidget(IN_DELETE_SELF_box, 7, 0);
    IN_MODIFY_box = createCheckBox("IN_MODIFY");
    MaskLayout->addWidget(IN_MODIFY_box, 8, 0);
    IN_MOVE_SELF_box = createCheckBox("IN_MOVE_SELF");
    MaskLayout->addWidget(IN_MOVE_SELF_box, 0, 1);
    IN_MOVED_FROM_box = createCheckBox("IN_MOVED_FROM");
    MaskLayout->addWidget(IN_MOVED_FROM_box, 1, 1);
    IN_MOVED_TO_box = createCheckBox("IN_MOVED_TO");
    MaskLayout->addWidget(IN_MOVED_TO_box, 2, 1);
    IN_MOVE_box = createCheckBox("IN_MOVE");
    MaskLayout->addWidget(IN_MOVE_box, 3, 1);
    IN_OPEN_box = createCheckBox("IN_OPEN");
    MaskLayout->addWidget(IN_OPEN_box, 4, 1);
    IN_ISDIR_box = createCheckBox("IN_ISDIR");
    MaskLayout->addWidget(IN_ISDIR_box, 5, 1);
    IN_UNMOUNT_box = createCheckBox("IN_UNMOUNT");
    MaskLayout->addWidget(IN_UNMOUNT_box, 6, 1);
    IN_Q_OVERFLOW_box = createCheckBox("IN_Q_OVERFLOW");
    MaskLayout->addWidget(IN_Q_OVERFLOW_box, 7, 1);
    IN_IGNORED_box = createCheckBox("IN_IGNORED");
    MaskLayout->addWidget(IN_IGNORED_box, 8, 1);
    IN_ALL_EVENTS_box = createCheckBox("IN_ALL_EVENTS");
    MaskLayout->addWidget(IN_ALL_EVENTS_box, 9, 0, 1, 2, Qt::AlignHCenter);
    maskGroupBox->setLayout(MaskLayout);
    topLayout->addWidget(maskGroupBox);
    QGridLayout* bottomPathStatusTable = new QGridLayout();
    QGridLayout* bottomButtonsTable = new QGridLayout();
    bottomPathStatusTable->addWidget((new QLabel("Path:")), 0, 0, 1, 1);
    bottomPathStatusTable->addWidget(pathLine, 0, 1, 1, -1);
    bottomPathStatusTable->addWidget(statusLabel, 1, 0, -1, 0);
    bottomButtonsTable->addWidget(startButton, 0, 0);
    bottomButtonsTable->addWidget(stopButton, 0, 1);
    bottomButtonsTable->addWidget(addButton, 1, 0, 1, 2);
    bottomLayout->addLayout(bottomPathStatusTable, 0, 0);
    bottomLayout->addLayout(bottomButtonsTable, 0, 1);
    main_layout->addLayout(topLayout,0, 0);
    main_layout->addLayout(bottomLayout,1, 0);
    mainWidget->setLayout(main_layout);
    eventsField->setReadOnly(1);
    startButton->setEnabled(false);
    stopButton->setEnabled(false);
    this->SetAllCheckBoxesUnchekable();
    this->IN_ALL_EVENTS_box->setCheckState(Qt::CheckState(2));
    QString pathLocation = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    if (!QFileInfo(pathLocation).isDir()){
        pathLocation = QDir::currentPath();
    }

    pathLine->setText(QDir::toNativeSeparators(pathLocation));
    connect(stopButton, &QAbstractButton::clicked, this, &InotifierMainwindow::stopButtonSlot);
    connect(startButton, &QAbstractButton::clicked, this, &InotifierMainwindow::startButtonSlot);
    connect(addButton, &QAbstractButton::clicked, this, &InotifierMainwindow::pathProcessingandFirstStarting);
}



void InotifierMainwindow::pathProcessingandFirstStarting(){
    QString path = pathLine->text().trimmed();
    if (path.isEmpty()) return;
    if (!QFileInfo(path).isDir()&&!QFile::exists(path)){
        path = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        eventsField->append("Seems like this is not a valid folder. ");  eventsField->append (getWasStarted() ? "Service did not start" : "Folder hasn't been added");
        pathLine->setText(path);
        return;
    }
    if(added_paths.count(path)){this->eventsField->append("This folder is already in WatchList: "); this->eventsField->append(path); return;}
    std::string tmp_p (path.toStdString());
    if (mask<=0) {std::cerr<<"Mask Must be not 0 In pathProcessingandFirstStarting"<<std::endl;
        eventsField->append ("Seems like Mask is 0. Please set it correctly");
        this->IN_ALL_EVENTS_box->setCheckState(Qt::CheckState::Unchecked);
        this->IN_ALL_EVENTS_box->setCheckable(1);
        return;
    };
    if (!getWasStarted()){
        this->setMaskSlot();
        this->addDirSlot(tmp_p);
        this->startButtonSlot();
        setWasStarted(1);
        this->SetAllCheckBoxesUnchekable();
        startButton->setEnabled(false);
        stopButton->setEnabled(true);
    }
    else {
        this->addDirSlot(tmp_p);
    }
    added_paths.insert(path);
    return;



}

void InotifierMainwindow::newFSEventInDataPipe(){

    char msg_in[100];
    ssize_t len = read(this->getDataPipe()[0], msg_in, sizeof (msg_in));
    if (len == -1) {std::cerr<<"Failed while: reading from DataPipe"<<std::endl; return;}
    FSEvent* newfsev = (FSEvent*)msg_in;
    std::stringstream ss;
    ss << *newfsev;
    this->eventsField->append(QString(ss.str().c_str()));
}



void InotifierMainwindow::pipeResponceReady(){
    char msg_in[100];
    std::cerr<<"!!STARTING READ FROM ServiceResponcePipe @STAUS?@ in pipeResponceReady"<<std::endl;
    ssize_t len = read(this->getServiceResponcePipe()[0], msg_in, sizeof (msg_in));
    if (len == -1) {std::cerr<<"Failed while: reading from"
"Responce Pipe\n"; return;}
    std::cerr<<"!!ENDING READ FROM ServiceResponcePipe @STAUS?@ in pipeResponceReady"<<std::endl;
    std::clog<<"recieved status from pipe: "<< msg_in<<std::endl;
    this->statusLabel->setText(QString(msg_in));
}


void InotifierMainwindow::startButtonSlot(){
    usleep(1000);
    std::clog<<"Sendin START FROM ++startButtonSlot++"<<std::endl;
    std::cerr<<"##LOCK MUTEX## in ++startButtonSlot++"<<std::endl;
    pthread_mutex_lock(this->getControlPipeMutex());
    std::cerr<<"!!STARTING WRITE to ServiceCoontrolPipe @COMMAND_START@ in ++startButtonSlot++"<<std::endl;
    int command = this->START;
    int stat = (write(this->getServiceControlPipe()[1], &command, sizeof(int)));
    if(-1== stat){
        std::cerr<<"Failed while: sendind START command into ServiceControlPipe in ++startButton++";
    }
    else {std::clog<<"Wrote Bytes: "<<stat<<std::endl;}
    std::cerr<<"!!ENDING WRITE to ServiceCoontrolPipe @COMMAND_START@ in ++startButtonSlot++"<<std::endl;

    kill(getpid(), SIGUSR1);
    std::cerr<<"##SIGNAL_COND## in ++startButtonSlot++"<<std::endl;
    pthread_cond_signal(this->getCondVar());
    std::cerr<<"##UNLOCK MUTEX## in ++startButtonSlot++"<<std::endl;
    pthread_mutex_unlock(this->getControlPipeMutex());
    this->SetAllCheckBoxesUnchekable();
    this->IN_ALL_EVENTS_box->setEnabled(0);
    if(getWasStopped()){this->startButton->setEnabled(0);}
    return;
}

void InotifierMainwindow::addDirSlot(std::string& tmppath){
    usleep(1000);
    std::cerr<<"Sendin PATH FROM ++addDirSlot++"<<std::endl;
     std::cerr<<"##LOCK MUTEX## in ++addDirSlot++"<<std::endl;
    pthread_mutex_lock(this->getControlPipeMutex());
    std::cerr<<"!!STARTING WRITE to ServiceControl @ADD_DIR_COMMAND@ in ++addDirSlot++"<<std::endl;
    int command = this->ADD_DIR;
    std::cerr<<"!!ENDING WRITE to ServiceControlPipe @ADD_DIR_COMMAND@ in ++addDirSlot++"<<std::endl;
    int  stat =  write(this->getServiceControlPipe()[1], &command, sizeof(command));
    if (-1 == stat){std::cerr<<"Failed while: writing SET_MAST command into Control Pipe";};
    std::cerr<<"!!STARTING WRITE to ServiceConfigurePipe @DIR_FILE_PATH@ in ++addDirSlot++"<<std::endl;
    std::clog<<"Path Sended to ConfigurePipe is:  "<< tmppath.c_str()<<std::endl;
    stat = write(this->getServiceConfigurePipe()[1], tmppath.c_str(), tmppath.size());
    if (-1==stat) {std::cerr<<"Failed while: writing Mask into DataPipe";};
    std::cerr<<"!!ENDING WRITE to ServiceConfigurePipe @DIR_FILE_PATH@ in ++addDirSlot++"<<std::endl;
    kill(getpid(), SIGUSR1);
    std::cerr<<"##SIGNAL_COND## in ++addDirSlot++"<<std::endl;
    pthread_cond_signal(this->getCondVar());
    std::cerr<<"##UNLOCK MUTEX## in ++addDirSlot++"<<std::endl;
    pthread_mutex_unlock(this->getControlPipeMutex());
    this->eventsField->append("Folder added to WatchList: ");
    QTextCursor cursor = eventsField->textCursor();
    cursor.movePosition(eventsField->textCursor().PreviousBlock);
    eventsField->setTextCursor(cursor);
    //ТУТ
    this->eventsField->append(tmppath.c_str());
}

void InotifierMainwindow::statusButtonSlot(){
    std::clog<<"StatusButton Clicked, sending STAUS command\n";
    pthread_mutex_lock(this->getControlPipeMutex());
    int command = InotifierMainwindow::COMMANDS::SEND_STATUS;
    int status = (write(this->getServiceControlPipe()[1], &command, sizeof(int)));
    if(-1== status){
        std::cerr<<"Failed while: sendind STATUS command into ServiceControlPipe in ++statusButtonSlot++";
    }
    kill(getpid(), SIGUSR1);
    pthread_cond_signal(this->getCondVar());
    pthread_mutex_unlock(this->getControlPipeMutex());
    return;
}

void InotifierMainwindow::stopButtonSlot(){
    std::clog<<"StopButton Clicked, sending STOP command\n";

    std::cerr<<"##LOCK MUTEX## in ++stopButtonSlot++"<<std::endl;
    pthread_mutex_lock(this->getControlPipeMutex());

    int command = InotifierMainwindow::COMMANDS::STOP;
    int status = (write(this->getServiceControlPipe()[1], &command, sizeof(int)));
    if(-1== status){
        std::cerr<<"Failed while: sendind STOP command into ServiceControlPipe in startButton";
    }
    else {std::cerr<<"Wrote Bytes: "<<status;}
    kill(getpid(), SIGUSR1);
    std::cerr<<"##SIGNAL_COND## in ++stopButtonSlot++"<<std::endl;
    pthread_cond_signal(this->getCondVar());
    std::cerr<<"##UNLOCK MUTEX## in ++stopButtonSlot++"<<std::endl;
    pthread_mutex_unlock(this->getControlPipeMutex());
    startButton->setEnabled(true);
    setWasStopped(1);
    return;
}


void InotifierMainwindow::setMaskSlot(){
    std::cerr<<"##LOCK MUTEX## in ++pathProcessingandFirstStarting++"<<std::endl;
    pthread_mutex_lock(this->getControlPipeMutex());
    std::cerr<<"!!STARTING WRITE to ServiceConfigurePipe @MASK@ in ++pathProcessingandFirstStarting++"<<std::endl;
    int stat = write(this->getServiceConfigurePipe()[1], &mask, sizeof(mask));
    if (-1==stat) {std::cerr<<"Failed while: writing Mask into DataPipe";};
    std::cerr<<"!!ENDING WRITE to ServiceConfigurePipe @MASK@ in ++pathProcessingandFirstStarting++"<<std::endl;
    std::cerr<<"!!STARTING WRITE to ServiceCoontrolPipe @COMM_SET_MASK@ in ++pathProcessingandFirstStarting++"<<std::endl;
    int command = this->SET_MASK;
    stat =  write(this->getServiceControlPipe()[1], &command, sizeof(command));
    if (-1 == stat){std::cerr<<"Failed while: writing SET_MAST command into Control Pipe";};
    std::cerr<<"!!ENDING WRITE to ServiceCoontrolPipe @COMM_SET_MASK@ in ++pathProcessingandFirstStarting++"<<std::endl;
    kill(getpid(), SIGUSR1);
    std::cerr<<"##SIGNAL_COND## in ++pathProcessingandFirstStarting++"<<std::endl;
    pthread_cond_signal(this->getCondVar());
    std::cerr<<"##UNLOCK MUTEX## in ++pathProcessingandFirstStarting++"<<std::endl;
    pthread_mutex_unlock(this->getControlPipeMutex());
    return;
}

bool InotifierMainwindow::getWasStarted(){
    return wasStarted;
}

void InotifierMainwindow::setWasStarted(bool s){
    wasStarted = s;
}

void InotifierMainwindow::initPipes(int* dP, int* sP, int* rP, int* cP){
    dataPipe[0] = dP[0];
    dataPipe[1] = dP[1];
    serviceControlPipe[0] = sP[0];
    serviceControlPipe[1] = sP[1];
    serviceResponcePipe[0] = rP[0];
    serviceResponcePipe[1] = rP[1];
    serviceConfigurePipe[0] = cP[0];
    serviceConfigurePipe[1] = cP[1];
}


void InotifierMainwindow::setControlPipeMutexAndCondVar(pthread_mutex_t from, pthread_mutex_t from1, pthread_cond_t from3){
    controlPipeMutex = from;
    newMutex = from1;
    condVar = from3;
}


QCheckBox* InotifierMainwindow::createCheckBox(const QString &text)
{
    QCheckBox *checkBox = new QCheckBox(text);
    connect(checkBox, &QCheckBox::clicked,
            this, &InotifierMainwindow::updateMask);
    return checkBox;
}

void InotifierMainwindow::SetAllCheckBoxesChekable(){
    IN_ACCESS_box->setEnabled(true);
    IN_ATTRIB_box->setEnabled(true);
    IN_CLOSE_WRITE_box->setEnabled(true);
    IN_CLOSE_NOWRITE_box->setEnabled(true);
    IN_CLOSE_box->setEnabled(true);
    IN_CREATE_box->setEnabled(true);
    IN_DELETE_box->setEnabled(true);
    IN_DELETE_SELF_box->setEnabled(true);
    IN_MODIFY_box->setEnabled(true);
    IN_MOVE_SELF_box->setEnabled(true);
    IN_MOVED_FROM_box->setEnabled(true);
    IN_MOVED_TO_box->setEnabled(true);
    IN_MOVE_box->setEnabled(true);
    IN_OPEN_box->setEnabled(true);
    IN_ISDIR_box->setEnabled(true);
    IN_UNMOUNT_box->setEnabled(true);
    IN_Q_OVERFLOW_box->setEnabled(true);
    IN_IGNORED_box->setEnabled(true);
}

void InotifierMainwindow::SetAllCheckBoxesUnchekable(){
    IN_ACCESS_box->setEnabled(false);
    IN_ATTRIB_box->setEnabled(false);
    IN_CLOSE_WRITE_box->setEnabled(false);
    IN_CLOSE_NOWRITE_box->setEnabled(false);
    IN_CLOSE_box->setEnabled(false);
    IN_CREATE_box->setEnabled(false);
    IN_DELETE_box->setEnabled(false);
    IN_DELETE_SELF_box->setEnabled(false);
    IN_MODIFY_box->setEnabled(false);
    IN_MOVE_SELF_box->setEnabled(false);
    IN_MOVED_FROM_box->setEnabled(false);
    IN_MOVED_TO_box->setEnabled(false);
    IN_MOVE_box->setEnabled(false);
    IN_OPEN_box->setEnabled(false);
    IN_ISDIR_box->setEnabled(false);
    IN_UNMOUNT_box->setEnabled(false);
    IN_Q_OVERFLOW_box->setEnabled(false);
    IN_IGNORED_box->setEnabled(false);
}

void InotifierMainwindow::updateMask(){

    this->mask=0;
    if (IN_ALL_EVENTS_box->isChecked()){
        mask = IN_ALL_EVENTS;
       SetAllCheckBoxesUnchekable();
       std::clog<<"Mask after Checkboxes is: "<<mask<<std::endl;
       return;
    }
    SetAllCheckBoxesChekable();
    if (IN_ACCESS_box->isChecked())
        mask|=IN_ACCESS;
    if (IN_ATTRIB_box->isChecked()){
         mask|=IN_ATTRIB;
    }
    if (IN_CLOSE_WRITE_box->isChecked()){
         mask|=IN_CLOSE_WRITE;
    }
    if (IN_CLOSE_NOWRITE_box->isChecked()){
         mask|=IN_CLOSE_NOWRITE;
    }
    if (IN_CLOSE_box->isChecked()){
         mask|=IN_CLOSE;
    }
    if (IN_CREATE_box->isChecked()){
         mask|=IN_CREATE;
    }
    if (IN_DELETE_box->isChecked()){
         mask|=IN_DELETE;
    }
    if (IN_DELETE_SELF_box->isChecked()){
         mask|=IN_DELETE_SELF;
    }
    if (IN_MODIFY_box->isChecked()){
         mask|=IN_MODIFY;
    }

    if (IN_MOVE_SELF_box->isChecked()){
         mask|=IN_MOVE_SELF;
    }
    if (IN_MOVED_FROM_box->isChecked()){
         mask|=IN_MOVED_FROM;
    }
    if (IN_MOVED_TO_box->isChecked()){
         mask|=IN_MOVED_TO;
    }
    if (IN_MOVE_box->isChecked()){
         mask|=IN_MOVE;
    }
    if (IN_OPEN_box->isChecked()){
         mask|=IN_OPEN;
    }
    if (IN_ISDIR_box->isChecked()){
         mask|=IN_ISDIR;
    }
    if (IN_UNMOUNT_box->isChecked()){
         mask|=IN_UNMOUNT;
    }
    if (IN_Q_OVERFLOW_box->isChecked()){
         mask|=IN_Q_OVERFLOW;
    }
    if (IN_IGNORED_box->isChecked()){
         mask|=IN_IGNORED;
    }
    std::clog<<"Mask after Checkboxes is: "<<mask<<std::endl;


}
