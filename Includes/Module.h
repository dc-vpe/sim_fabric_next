//
// Created by krw10 on 10/18/2023.
//

#ifndef DSL_CPP_MODULE_H
#define DSL_CPP_MODULE_H

#include "U8String.h"

/// \desc Module defines a single module i.e. class in the DSL language.
class Module
{
private:
    U8String *_n;
    U8String *_f;
    U8String *_s;
    bool     _l;
public:

    /// \desc Module, i.e. class name, of this module.
    U8String *Name() { return _n; }

    /// \desc File path name of the script for this module.
    [[maybe_unused]] U8String *File() { return _f; }

    /// \desc Gets the script associated with this module.
    U8String *Script() { return _s; }

    /// \desc Get the lexed state of the module.
    [[maybe_unused]] [[nodiscard]] bool IsLexed() const { return _l; }

    /// \desc Creates a new Module with default values.
    Module() : _n(new U8String("")), _f(new U8String("")), _s(new U8String("")), _l(false) { };

    /// \desc Creates a new Module with the provided _n, _f, _s.
    /// \param name Name of the module.
    /// \param file Complete _f _n path of the _s _f.
    /// \param script Raw _s _f, may be in UTF8 or ASCII format.
    Module(const char *name, const char *file, const char *script) :
        _n(new U8String(name)), _f(new U8String(file)), _s(new U8String(script)), _l(false) { };

    /// \desc Releases the memory used by the module.
    ~Module()
    {
        delete _n;
        delete _f;
        delete _s;
    }
};


#endif //DSL_CPP_MODULE_H
