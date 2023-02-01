#pragma once

#include "tools.hpp"
#include <iostream>

namespace booba {
    struct ProxyEvent {
        enum {
            none = 0,
            // Tool functions.
            apply,
            apply_finished,
            getTexture,
            buildSetupWidget,
            buildSetupWidget_finished,
            // Global toolbar functions.
            setToolBarSize,
            createButton,
            createLabel,
            createSlider,
            createCanvas,
            // Global canvas funcs.
            putPixel,
            putSprite,
            cleanCanvas,
            // Global functions.
            init_module,
            init_module_finished,
            addTool,
            addFilter,
            getLibSymbol,
            // Image functions.
            getH,
            getW,
            getPixel,
            setPixel,
            // Non-standart helpers
            sendFile, //FIXME remove if unused?
            updateContext,
        } function = none;
        Event event;
        size_t x, y;
        size_t w, h;
        uint32_t color;
        char text[64];
        char texture[64];
        long min, max, start;
        uint64_t canvasId;
        uint64_t widgetId;
        Tool *tool;
        char name[64];
        GUID guid;
        ApplicationContext context;
        bool ok;
    };
}
