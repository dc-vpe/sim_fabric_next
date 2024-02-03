//
// Created by krw10 on 10/18/2023.
//

#ifndef DSL_CPP_MODULE_H
#define DSL_CPP_MODULE_H

#include "U8String.h"

/// \desc Module defines a single module i.e. class in the DSL language.
class Module
{
public:
    U8String name = {};
    U8String file = {};
    U8String script = {};

    //After lexer system on event functions names. Parser reads these and writes EFI instructions
    //after the end of the code with the actual event function information.
    U8String onError = {};
    U8String onKeyUp = {};
    U8String onKeyDown = {};
    U8String onLeftDrag = {};
    U8String onLeftUp = {};
    U8String onLeftDown = {};
    U8String onRightDrag = {};
    U8String onRightUp = {};
    U8String onRightDown = {};
    U8String onTick = {};

    //User events are events that the program can raise to send a message to another part
    //of the program. This allows cross script communication.
    List<U8String *> userEvents = {};
};


#endif //DSL_CPP_MODULE_H
