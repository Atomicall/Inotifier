#include "Inotifier.h"


Inotifier::Inotifier() {
   // InotEpEv.events = EPOLLIN | EPOLLET;
    // Edge-triggering, Связанный файл доступен для чтения (read)
    //
    epollEvent.events = EPOLLIN | EPOLLET;
    inotFd = inotify_init1(IN_NONBLOCK);  // Устанавливваем ф.д inotify в неблокирующий режим
    // Ввода-вывода (т.е не будем ждать, пока читается Inot, а возвращается
    // ошибка, что он не готов)
    if (inotFd == -1) {
        *this->cerr_stream << "_Failed to initialize Inotify" << std::endl;
        // To Do throw
    }
    // Сохраним fd, чтобы смогли им пользоваться при получении события
    /*InotEpEv.data.fd = inotFd;*/
    epollEvent.data.fd = inotFd;

    epollFd = epoll_create1(EPOLL_CLOEXEC);
    // O_CLOEXEC - значит, что при вызове execv дескриптор будет закрыт
    // monitoring multiple file descriptors to see if I/O is possible on any of
    // them
    if (epollFd == -1) {
        *this->cerr_stream << "_Failed to initialize epoll" << std::endl;
        // To Do throw
    }

    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, inotFd, &epollEvent) == -1) {
        *this->cerr_stream << "_Failed to add Inotify To Epoll_ctrl" << std::endl;
        // To Do throw
    }
    evBuff = new char [evBuffSize];
    if (!evBuff){*this->cerr_stream<<"Failed to allocate mem for buff with size:"<<evBuffSize; exit (-1);}
    isRunned = 0;
}

Inotifier::~Inotifier() {
    epoll_ctl(epollFd, EPOLL_CTL_DEL, inotFd, NULL);
    close(epollFd);
    close(inotFd);
    delete [] evBuff;
}

void Inotifier::addDirSRecursively(std::filesystem::path root_path) {
    if (std::filesystem::exists(root_path) &&
        std::filesystem::is_directory(root_path)) {
        sub_paths.push_back(root_path);
        std::error_code ec;
        std::filesystem::recursive_directory_iterator it( root_path, std::filesystem::directory_options::follow_directory_symlink, ec);
        std::filesystem::recursive_directory_iterator end;
        std::filesystem::path currentPath;
        while (it != end) {
            currentPath = *it;
            if (!std::filesystem::is_directory(currentPath) &&
                !std::filesystem::is_symlink(currentPath)) {
                it.increment(ec);
                continue;
            }
            sub_paths.push_back(currentPath);
            it.increment(ec);
        }
    } else {
        *this->cerr_stream << "_Unexpected: Failed to add Dirs Recursively "
                  << strerror(errno) << "|||" << errno << std::endl;
        return;
    }
    std::list<std::filesystem::path>::iterator list_it = sub_paths.begin();
    while (list_it != sub_paths.end()) {
        addDirToWatch((*list_it));
        list_it++;
    }
}

void Inotifier::addDirToWatch(std::filesystem::path dir_path) {
    int wd;
    if (!std::filesystem::exists(dir_path)) {
        *this->cerr_stream << "_Failed to add Dir to Inot_Watch: Dir doen't exists"
                  << std::endl;
        return;

    }
    if (eventsMask ==0){
        *this->cerr_stream << "_Failed to add Dir To WatchList:: Mask==0; errno: " << errno << std::endl;

        return;
    }
    wd = inotify_add_watch(inotFd, dir_path.c_str(), eventsMask);
    if (-1 == wd) {
        *this->cerr_stream << "_Failed to add Dir To WatchList:" <<dir_path<<" Errno: "<< errno << std::endl;

        return;
    }
    dirMap.insert(std::pair<int, std::filesystem::path>(wd, dir_path));
}

void Inotifier::removeDirFromWatch(std::filesystem::path dir_path) {

    auto find_cr = [&dir_path](std::pair<int, std::filesystem::path> elem) {
        return (elem.second.compare(dir_path) == 0) ? 1: 0;
    };


    std::map<int, std::filesystem::path>::iterator result =
            std::find_if(dirMap.begin(), dirMap.end(), find_cr);

    if (dirMap.end() == result) {
        *this->cerr_stream << "_Unexpected: path for delete wasn found in dirMap"
                  << std::endl;
        return;
    };

    if (-1 == inotify_rm_watch(inotFd, result->first)) {
        *this->cerr_stream << "_Unexpected: failed to delete dirPath from dirMap"
                  << std::endl;
    }
    return;
}

void Inotifier::readEventSToEventQueue() {
    if (0 == IsRunned()) {
        *this->cerr_stream << "_Unexpected: Inotitfier must be stoppen, but you'r trying "
                     "read event to Queue "
                  << std::endl;
        return;
    };

    size_t read_size = 0;

    /// возвращает количество файловых дескрипторов, готовых для запросов
    ///ввода-вывода, или ноль, если ни один файловый дескриптор не стал готов за
    ///отведённые timeout миллисекунд
    std::clog << "___STARTING read EPOLL"<<std::endl;
    int ep = epoll_wait(epollFd, &epollEvent, 1, -1 /*500000*/);
    if (-1 == ep) {
        *this->cerr_stream << "_Unexpected: Epoll read error while trying to receive data "
                     "from Inot: "
                  << errno << std::endl;
        return;
    };
    std::clog << " ___ENDING read EPOLL"<<std::endl;
    std::clog << "___STARTING read READ"<<std::endl;
    read_size = read(epollEvent.data.fd,this->evBuff, this->evBuffSize);
    if (!(this->IsRunned())) {*this->cerr_stream <<"_____________________________________Stopped?\n"; return;};
    if (-1 == (read_size)) {
        *this->cerr_stream << "_Unexpected: Failed to read into buffer " << strerror(errno)
                  << "::" << errno << std::endl;
        return;
    } else {
        std::clog << "_OK read buff"<<std::endl;
    }
     std::clog << "___ENDING read READ"<<std::endl;
    size_t processed = 0;
    std::filesystem::path path;
    while (processed < read_size) {
        struct inotify_event *event = (struct inotify_event *)&this->evBuff[processed];
        if (event->mask & IN_IGNORED) {
            processed += EVENT_SIZE + event->len;
            continue;
        }
        path = dirMap.at(event->wd);
        if (std::filesystem::is_directory(path)) {
            path = path / std::string(event->name);
            //event->mask |= IN_ISDIR;
        }
        if ((event->mask & IN_CREATE) && std::filesystem::is_directory(path)){
            this->addDirToWatch(path);
            *this->cerr_stream <<"_Added New? Adding to WatchList\n";
        }
        FSEvent* fsEvent = new FSEvent(event->wd, event->mask, path);
        if (!fsEvent->path.empty()) {
            EventQueue.push(fsEvent);
        }
        delete fsEvent;
        processed += EVENT_SIZE + event->len;
    }
}


void Inotifier::runService() {isRunned = 1;}
void Inotifier::stopService(){isRunned=0; }
void Inotifier::setEventsMask(uint32_t evMask) {this->eventsMask = evMask;}
std::shared_ptr<FSEvent> Inotifier::getNextEvent() {
    if (!IsRunned()) {
        *this->cerr_stream << "_Inotifier must be stopped, but Yr trying to read event\n"<<std::endl;
        return 0;
    }
    while (EventQueue.empty() && IsRunned()) {
        readEventSToEventQueue();
    }
    if (!EventQueue.empty()){
    FSEvent* tmp = new FSEvent();
    std::shared_ptr<FSEvent> event = std::make_shared<FSEvent>(tmp);
    *event = EventQueue.front();
    EventQueue.pop();
    return event;
    }
    return NULL;
}

Inotifier::Inotifier(Inotifier *pInotifier) {

    this->inotFd = pInotifier->inotFd;
    this->epollFd = pInotifier->epollFd;
    this->dirMap = pInotifier->dirMap;
    this->epollEvent = pInotifier->epollEvent;
    this->isRunned = pInotifier->isRunned;
    this->EventQueue = pInotifier->EventQueue;
    this->sub_paths = pInotifier->sub_paths;
    *this = pInotifier;
}

int Inotifier::getEpollFd() {
    return this->epollFd;
}
int Inotifier::getInotifyFd() {
    return this->inotFd;
}

