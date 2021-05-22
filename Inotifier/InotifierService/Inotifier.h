#pragma once
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <unistd.h>


#include <algorithm>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <queue>
#include <memory>

#include "../models/FilesystemEv.h"
#define EVENT_SIZE (sizeof(inotify_event))

class Inotifier {
 public:
    Inotifier();
    Inotifier (Inotifier *pInotifier);
    ~Inotifier();

  void addDirToWatch(std::filesystem::path dir_path);
  void addDirSRecursively(std::filesystem::path root_path);
  void removeDirFromWatch(std::filesystem::path dir_path);
  void readEventSToEventQueue();
  void setEventsMask(uint32_t evMask);
  void runService();
  void stopService();
  std::shared_ptr<FSEvent> getNextEvent();
  bool IsRunned() { return isRunned; };
  int getEpollFd();
  int getInotifyFd();
  uint32_t getMask(){return eventsMask;};


 private:
 //?? std::atomic<bool> isRunned ();
  bool isRunned;
  uint32_t eventsMask= IN_ALL_EVENTS; // Маска отслеживаемых событий, общая для ВСЕХ (под)директорий в данном экземпляре Inotifier
  int inotFd;   //Храним экземпляр inotify
  int epollFd;  // Ф.д экземпляра poll для чтения событий файловых дескрипторов
  std::map<int, std::filesystem::path> dirMap;  //Для каждой поддиректории - свой wd от inotify и путь
  std::list<std::filesystem::path> sub_paths; //Все поддиректории, вкл корневую
  std::vector<std::string> IgnoredDirs;// Что игнорируем, TO Do
  std::queue<FSEvent> EventQueue;//Очередь ивентов, либо пуста, либо, если не пуста, не заполняется, пока не очистится
  epoll_event epollEvent;
  char evBuff[4096] = {};


};
