#pragma once
#include <filesystem>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <string>
#include "../InotifierService/MaskDecodeServiceComponent.h"
#include <string>


struct FSEv_pod{
    int wd;
    uint32_t mask;
    char path_ch_m [512] ;
    char eventString_ch_m [512] ;
};

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

  void fill(FSEv_pod* pod){
      this->wd = pod->wd;
      this->mask = pod->mask;
      this->path = std::filesystem::path (pod->path_ch_m);
      this->eventString = MaskDecodeServiceComponent(this->mask).decodeMask();
  };


  ~FSEvent(){/*std::cerr<<"_one_"<<std::endl;*/};

  FSEvent (std::shared_ptr<FSEvent> old) {
    wd = old->wd;
    mask = old->mask;
    path = old->path;
    eventString = old->eventString;
  };

  FSEvent (FSEvent* old) {
    wd = old->wd;
    mask = old->mask;
    path = old->path;
    eventString.clear();
    eventString.append(old->eventString);
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
    return *this;
  }
  bool isEmpty(){
      return eventString.empty();
  }

  static FSEv_pod serialize(FSEvent* e){
      FSEv_pod pod = {};
      pod.mask = e->mask;
      pod.wd = e->wd;
      memcpy(pod.eventString_ch_m, e->eventString.c_str(), sizeof (pod.eventString_ch_m));
      memcpy(pod.path_ch_m, e->path.c_str(), sizeof (pod.path_ch_m));
      return pod;
    };
};
