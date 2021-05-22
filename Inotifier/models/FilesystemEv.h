#pragma once
#include <filesystem>
#include <iostream>
#include <string>
#include "../InotifierService/MaskDecodeServiceComponent.h"
#include <string>
// Модель для общения между Ui и сервисом
class FSEvent {
 public:
    int wd;
    uint32_t mask;
    std::filesystem::path path;
    std::string eventString;


  FSEvent(){};
  FSEvent(const int wd, uint32_t mask, const std::filesystem::path& path)
      : wd(wd), mask(mask), path(path){
      eventString = MaskDecodeServiceComponent(this->mask).decodeMask();
  };

  FSEvent(FSEvent* old) {
    wd = old->wd;
    mask = old->mask;
    path = old->path;
    eventString = old->eventString;
  };
  friend std::ostream& operator<<(std::ostream& o, const FSEvent& e) {
    o << "Event for: " << e.path << " with mask ";
    o<<e.eventString;
    return o;
  }
  FSEvent operator=(FSEvent e){
    this->mask = e.mask;
    this->wd = e.wd;
    this->path = e.path;
    this->eventString = e.eventString;
    return this;
  }
  bool isEmpty(){
      return eventString.empty();
  }
};
