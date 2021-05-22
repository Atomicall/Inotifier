#pragma once
#include <QMainWindow>
#include <QtWidgets>
#include <string>
#include <signal.h>
#include <unordered_set>

#include "Inotifier/models/FilesystemEv.h"

class InotifierMainwindow : public QMainWindow
{
    Q_OBJECT
public:
    enum COMMANDS {START = 100, STOP=200, SEND_STATUS=300, ADD_DIR=400, SET_MASK=500} ;
    explicit InotifierMainwindow(QWidget *parent = nullptr);
    QWidget* mainWidget;
    bool getWasStarted();
    void setWasStarted(bool s);
    bool getWasStopped(){return wasStopped;}
    void setWasStopped(bool s){wasStopped=s;}
    QTextEdit* eventsField;

    void initPipes(int* dP, int* sP, int* rP, int* cP);
    void setControlPipeMutexAndCondVar(pthread_mutex_t from, pthread_mutex_t from1, pthread_cond_t from3);



    int* getDataPipe(){return dataPipe;}
    int* getServiceControlPipe(){return serviceControlPipe;};
    int* getServiceResponcePipe(){return serviceResponcePipe;};
    int* getServiceConfigurePipe(){return serviceConfigurePipe;};
    pthread_mutex_t* getControlPipeMutex(){return &controlPipeMutex;}
    pthread_mutex_t* getNewMutex(){return NULL;}
    pthread_cond_t* getCondVar(){return &condVar;}



private:
   QPushButton* addButton;
   QPushButton* stopButton;
   QPushButton* startButton;
   QLineEdit* pathLine;
   QLabel* statusLabel;
   QString path;
   QGroupBox* maskGroupBox;

   QCheckBox* IN_ACCESS_box;
   QCheckBox* IN_ATTRIB_box;
   QCheckBox* IN_CLOSE_WRITE_box;
   QCheckBox* IN_CLOSE_NOWRITE_box;
   QCheckBox* IN_CLOSE_box;

   QCheckBox* IN_CREATE_box;
   QCheckBox* IN_DELETE_box;
   QCheckBox* IN_DELETE_SELF_box;
   QCheckBox* IN_MODIFY_box;
   QCheckBox* IN_MOVE_SELF_box;
   QCheckBox* IN_MOVED_FROM_box;
   QCheckBox* IN_MOVED_TO_box;
   QCheckBox* IN_MOVE_box;

   QCheckBox* IN_OPEN_box;
   QCheckBox* IN_ISDIR_box;
   QCheckBox* IN_UNMOUNT_box;
   QCheckBox* IN_Q_OVERFLOW_box;
   QCheckBox* IN_IGNORED_box;//18
   QCheckBox* IN_ALL_EVENTS_box;//19

   bool wasStarted = 0;
   bool wasStopped;
   uint32_t mask = IN_ALL_EVENTS;
  std::unordered_set<QString> added_paths;

   QCheckBox* createCheckBox(const QString& text);
   void SetAllCheckBoxesUnchekable();
   void SetAllCheckBoxesChekable();
   void updateMask();

   int dataPipe[2];
   int serviceControlPipe[2];
   int serviceResponcePipe[2];
   int serviceConfigurePipe[2];

   pthread_mutex_t controlPipeMutex;
   pthread_mutex_t newMutex;
   pthread_cond_t condVar;

protected slots:
   void pathProcessingandFirstStarting();
   void startButtonSlot();
   void statusButtonSlot();
   void stopButtonSlot();
   void setMaskSlot();
   void addDirSlot(std::string& tmppath);
public slots:
   void newFSEventInDataPipe();
   void pipeResponceReady();
};

