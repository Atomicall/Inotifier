#include <QMainWindow>
#include <QtWidgets>
#include <string>
#include <signal.h>

#include "Inotifier/models/FilesystemEv.h"

class InotifierMainwindow : public QMainWindow
{
    Q_OBJECT
public:
    enum COMMANDS {START = 100, STOP=200, SEND_STATUS=300, ADD_DIR=400};
    explicit InotifierMainwindow(QWidget *parent = nullptr);
    QWidget* mainWidget;
    bool getWasStarted();
    void setWasStarted(bool s);
    QTextEdit* eventsField;

    void setDataPipe(int* from);
    void setServiceControlPipe(int* from);
    void setServiceResponcePipe(int* from);

    int* getDataPipe(){return dataPipe;}
    int* getServiceControlPipe(){return serviceControlPipe;};
    int* getServiceResponcePipe(){return serviceResponcePipe;};




private:
   QPushButton* addButton;
   QPushButton* stopButton;
   QLineEdit* pathLine;
   QLabel* statusLabel;

   std::string path;
   bool wasStarted = 0;

   int dataPipe[2];
   int serviceControlPipe[2];
   int serviceResponcePipe[2];




protected slots:
   void pathProcessing();
   void startButton();
   void statusButton();
   void stopButtonSlot();

public slots:
   void newFSEventInDataPipe();
   void pipeResponceReady();


};

