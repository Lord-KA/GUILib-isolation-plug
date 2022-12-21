#include <iostream>
#include <cassert>
#include <string>
#include <filesystem>
#include <dlfcn.h>
#include <string.h>

#include "tools.hpp"
#include "proxy-event.hpp"
#include "proxy-image.hpp"

namespace booba {
    ApplicationContext *APPCONTEXT = nullptr;

    uint64_t createButton(size_t x, size_t y, size_t w, size_t h, const char *text)
    {
        ProxyEvent ev;
        ev.function = ProxyEvent::createButton;
        ev.x = x;
        ev.y = y;
        ev.w = w;
        ev.h = h;
        strncpy(ev.text, text, 63);
        sendEvent(ev);

        ev = getEvent();
        assert(ev.function == ProxyEvent::createButton);
        return ev.widgetId;
    }

    uint64_t createLabel(size_t x, size_t y, size_t w, size_t h, const char *text)
    {
        ProxyEvent ev;
        ev.function = ProxyEvent::createLabel;
        ev.x = x;
        ev.y = y;
        ev.w = w;
        ev.h = h;
        strncpy(ev.text, text, 63);
        sendEvent(ev);

        ev = getEvent();
        assert(ev.function == ProxyEvent::createLabel);
        return ev.widgetId;
    }

    uint64_t createSlider(size_t x, size_t y, size_t w, size_t h, long min, long max, long start)
    {
        ProxyEvent ev;
        ev.function = ProxyEvent::createLabel;
        ev.x = x;
        ev.y = y;
        ev.w = w;
        ev.h = h;
        ev.min = min;
        ev.max = max;
        ev.start = start;
        sendEvent(ev);

        ev = getEvent();
        assert(ev.function == ProxyEvent::createLabel);
        return ev.widgetId;
    }

    uint64_t createCanvas(size_t x, size_t y, size_t w, size_t h)
    {
        ProxyEvent ev;
        ev.function = ProxyEvent::createCanvas;
        ev.x = x;
        ev.y = y;
        ev.w = w;
        ev.h = h;
        sendEvent(ev);

        ev = getEvent();
        assert(ev.function == ProxyEvent::createCanvas);
        return ev.widgetId;
    }

    void putPixel (uint64_t canvas, size_t x, size_t y, uint32_t color)
    {
        ProxyEvent ev;
        ev.function = ProxyEvent::putPixel;
        ev.x = x;
        ev.y = y;
        ev.color = color;
        ev.canvasId = canvas;
        sendEvent(ev);
    }

    void putSprite(uint64_t canvas, size_t x, size_t y, size_t w, size_t h, const char* texture)
    {
        ProxyEvent ev;
        ev.function = ProxyEvent::putSprite;
        ev.x = x;
        ev.y = y;
        ev.w = w;
        ev.h = h;
        strncpy(ev.texture, texture, 63);
        ev.canvasId = canvas;
        sendEvent(ev);
    }

    GUID getGUID()
    {
        assert(!"no GUID yet");
    }

    bool setToolBarSize(size_t w, size_t h)
    {
        ProxyEvent ev;
        ev.function = ProxyEvent::setToolBarSize;
        ev.w = w;
        ev.h = h;
        sendEvent(ev);

        ev = getEvent();
        assert(ev.function = ProxyEvent::setToolBarSize);
        return ev.ok;
    }

    void cleanCanvas(uint64_t canvasId, uint32_t color)
    {
        ProxyEvent ev;
        ev.function = ProxyEvent::cleanCanvas;
        ev.canvasId = canvasId;
        ev.color = color;
        sendEvent(ev);
    }

    void addTool(Tool* tool)
    {
        assert(tool);
        ProxyEvent ev;
        ev.function = ProxyEvent::addTool;
        ev.tool = tool;
        sendEvent(ev);
    }

    void addFilter(Tool* tool)
    {
        addTool(tool);
    }

    static ProxyImage *IMAGE = nullptr;

    void handleToolEvents()
    {
        ProxyEvent ev;
        while ((ev = getEvent()).function != ProxyEvent::none) {
            if (ev.function == ProxyEvent::apply) {
                ev.tool->apply(IMAGE, &ev.event);
                ev.function = ProxyEvent::apply_finished;
                sendEvent(ev);
            } else {
                std::cerr << "ERROR: wrong proxy event (" << ev.function << ")\n";
                assert(false);
                exit(-1);
            }
        }
    }

    void init_DLLs(std::string dir)
    {
        ProxyEvent ev = getEvent();
        assert(ev.function == ProxyEvent::init_module);
        for (auto file : std::filesystem::directory_iterator(dir)) {
            std::cerr << file.path().string() << "\n";
            if (file.is_directory() or not file.path().string().ends_with(".aboba.so"))
                continue;

            void* dlHandler = dlopen(file.path().c_str(), RTLD_LAZY);

            if (dlHandler) {
                void (*init)()     = nullptr;
                *((void**)(&init)) = dlsym(dlHandler, "init_module");
                (*init)();
                for (size_t i = 0; i < 2; ++i) {
                    ev = getEvent();
                    if (ev.function == ProxyEvent::getTexture) {
                        const char *t = ev.tool->getTexture();
                        strncpy(ev.texture, t, 63);
                        sendEvent(ev);
                    } else if (ev.function == ProxyEvent::buildSetupWidget) {
                        ev.tool->buildSetupWidget();
                        ev.function = ProxyEvent::buildSetupWidget_finished;
                        sendEvent(ev);
                    } else {
                        std::cerr << "ERROR: wrong proxy event (" << ev.function << ")\n";
                        assert(false);
                        exit(-1);
                    }
                }

            } else {
                fprintf(stderr, "ERROR: Unable to open plugin: %s\n", dlerror());
            }
        }
        ev.function = ProxyEvent::init_module_finished;
        sendEvent(ev);
    }
}

int main()
{
    booba::APPCONTEXT = new booba::ApplicationContext;
    booba::IMAGE = new booba::ProxyImage();
    booba::init_DLLs("/home/user/QubesIncoming/Dev-fedora/UntrustedPlugins/");

    booba::handleToolEvents();
}
