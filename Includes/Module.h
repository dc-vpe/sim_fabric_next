//
// Created by krw10 on 10/18/2023.
//

#ifndef DSL_CPP_MODULE_H
#define DSL_CPP_MODULE_H

#include "U8String.h"
#include "ComponentData.h"

/// \desc Module defines a single module i.e. class in the DSL language.
class Module
{
public:

    Module() = default;
    ~Module() = default;

    U8String name = {};
    U8String file = {};
    U8String script = {};

    /// \desc Run time system events that have handlers for this module.
    /// \remark system events are kept in the same order and range as the system error handlers enumeration.
    List<U8String> systemEvents = {};

    /// \desc User events are events that the program can raise to send a message to another part
    ///       of the program. This allows cross script communication.
    List<U8String> userEvents = {};
};


#endif //DSL_CPP_MODULE_H
