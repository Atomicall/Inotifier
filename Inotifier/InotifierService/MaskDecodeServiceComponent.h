#pragma once
#include <cstdint>
#include <iostream>
#include <string>
#include <sstream>

#include <sys/inotify.h>

class MaskDecodeServiceComponent{

public:
    uint32_t mvalue;

    MaskDecodeServiceComponent(uint32_t value){
        mvalue = value;
    }

    enum Masks : std::uint32_t {
        access = IN_ACCESS,
        attrib = IN_ATTRIB,
        close_write = IN_CLOSE_WRITE,
        close_nowrite = IN_CLOSE_NOWRITE,
        close = IN_CLOSE,
        create = IN_CREATE,
        remove = IN_DELETE,
        remove_self = IN_DELETE_SELF,
        modify = IN_MODIFY,
        move_self = IN_MOVE_SELF,
        moved_from = IN_MOVED_FROM,
        moved_to = IN_MOVED_TO,
        move = IN_MOVE,
        open = IN_OPEN,
        is_dir = IN_ISDIR,
        unmount = IN_UNMOUNT,
        q_overflow = IN_Q_OVERFLOW,
        ignored = IN_IGNORED,
        oneshot = IN_ONESHOT,
        all = IN_ALL_EVENTS
    };
    friend std::ostream& operator<<(std::ostream& stream, MaskDecodeServiceComponent& e){

        uint32_t& value = e.mvalue;
        std::string maskString = "";

        // перемещение и закрытие состоит из нескольних частей, проверим по отдельности
        if ((value & Masks::close_write) && (value & Masks::close_nowrite)) {
            maskString.append("Close (Close_write  | Close_nowrite) ");
        } else {
            if ((value & close_write))
                maskString.append("Close_write ");
            if ((value & close_nowrite))
                maskString.append("Close_nowrite ");
        }

        if ((value & Masks::moved_from) && (value & Masks::moved_to)) {
            maskString.append("Moved (Moved_from  | Moved_to) ");
        } else {
            if ((value & Masks::moved_from))
                maskString.append("Moved_from ");
            if ((value & Masks::moved_to))
                maskString.append("Moved_to ");
        }
        if ((value & Masks::access))
            maskString.append("IN_Access ");
        if ((value & Masks::attrib))
            maskString.append("IN_Attrib ");
        if ((value & Masks::create))
            maskString.append("Created ");
        if ((value & Masks::remove))
            maskString.append("Removed ");
        if ((value & Masks::remove_self))
            maskString.append("Removed_self ");
        if ((value & Masks::modify))
            maskString.append("Modify ");
        if ((value & Masks::move_self))
            maskString.append("Move_self ");
        if ((value & Masks::open))
            maskString.append("Open ");
        if ((value & Masks::is_dir))
            maskString.append("Is_dir ");
        if ((value & Masks::unmount))
            maskString.append("Unmount ");
        if ((value & Masks::q_overflow))
            maskString.append("Q_overflow ");
        if ((value & Masks::ignored))
            maskString.append("Ignored ");
        if ((value & Masks::oneshot))
            maskString.append("Oneshot ");
        stream << maskString;
        return stream;
    }
    std::string decodeMask(){
        std::stringstream the_stream;
        the_stream<< *this;
        return the_stream.str();
    }
};
