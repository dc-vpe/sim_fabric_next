//
// Created by krw10 on 2/15/2024.
//

#ifndef SIM_SYSTEMERRORHANDLERS_H
#define SIM_SYSTEMERRORHANDLERS_H

/// \desc IDs for the handlers handled by events.
enum SystemErrorHandlers
{
    ON_NONE = 0,
    /// \desc OnError handler. Each module (script file) can have its own on error handler.
    ON_ERROR = 1,
    /// \desc OnTick handler. Each module (script file) can have its own on tick handler.
    ON_TICK = 2,
    //The following events are reserved in case the design evolves to require their functionality.
    ON_KEY_DOWN = 3,
    ON_KEY_UP = 4,
    ON_LEFT_DRAG = 5,
    ON_LEFT_UP = 6,
    ON_LEFT_DOWN = 7,
    ON_RIGHT_DRAG = 8,
    ON_RIGHT_UP = 9,
    ON_RIGHT_DOWN = 10
};

#endif //SIM_SYSTEMERRORHANDLERS_H
