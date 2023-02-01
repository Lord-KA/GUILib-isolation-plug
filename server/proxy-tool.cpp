#include <cassert>
#include <iostream>
#include <string>
#include <string.h>

#include "tools.hpp"
#include "proxy-event.hpp"

namespace booba {

    static ProxyEvent getEvent() //TODO move from cin/cout to other streams
    {
        ProxyEvent ev = {};
        std::cin.read((char*)&ev, sizeof(ProxyEvent));
        if (std::cin.gcount() != sizeof(ProxyEvent)) {
            assert(false);
        }
        if (!APPCONTEXT) {
            APPCONTEXT = new ApplicationContext;
        }
        *APPCONTEXT = ev.context;
        return ev;
    }

    static void sendEvent(ProxyEvent ev)
    {
        std::cout.write((char*)&ev, sizeof(ProxyEvent));
        std::cout.flush();      //TODO maybe do flush() on many buffered events.
    }

    class Proxy : public Tool {
    private:
        Tool *tool;
        Image *image;
        char *texture;

        void handleApplyEvents(Image *im)
        {
            ProxyEvent ev;
            while ((ev = getEvent()).function != ProxyEvent::apply_finished) {
                if (ev.function == ProxyEvent::getH) {
                    ev.h = im->getH();
                    sendEvent(ev);
                } else if (ev.function == ProxyEvent::getW) {
                    ev.w = im->getW();
                    sendEvent(ev);
                } else if (ev.function == ProxyEvent::getPixel) {
                    ev.color = im->getPixel(ev.x, ev.y);
                    sendEvent(ev);
                } else if (ev.function == ProxyEvent::setPixel) {
                    im->setPixel(ev.x, ev.y, ev.color);
                } else if (ev.function == ProxyEvent::putPixel) {
                    putPixel(ev.canvasId, ev.x, ev.y, ev.color);
                } else if (ev.function == ProxyEvent::putSprite) {
                    putSprite(ev.canvasId, ev.x, ev.y, ev.w, ev.h, ev.texture);
                } else if (ev.function == ProxyEvent::cleanCanvas) {
                    cleanCanvas(ev.canvasId, ev.color);
                } else {
                    std::cerr << "ERROR: wrong proxy event (" << ev.function << ")\n";
                    assert(false);
                    exit(-1);
                }
            }
        }

    public:
        Proxy(Tool *tool) : tool(tool) {}

        virtual ~Proxy() override { }

        virtual void apply(Image *im, const Event *ev) override
        {
            image = im;
            static bool firstRun = true;
            if (firstRun) {
                firstRun = false;
            }

            handleApplyEvents(im);

            ProxyEvent pev;
            pev.function = ProxyEvent::apply;
            pev.event = *ev;
            pev.tool = tool;
            sendEvent(pev);
        }

        virtual const char* getTexture() override
        {
            ProxyEvent ev;
            ev.function = ProxyEvent::getTexture;
            ev.tool = tool;
            sendEvent(ev);

            ev = getEvent();
            assert(ev.function == ProxyEvent::getTexture);
            free(texture);
            texture = strdup(ev.texture);
            return texture;
        }

        virtual void buildSetupWidget() override
        {
            ProxyEvent ev;
            ev.function = ProxyEvent::buildSetupWidget;
            ev.tool = tool;
            sendEvent(ev);

            while ((ev = getEvent()).function != ProxyEvent::buildSetupWidget_finished) {
                uint64_t id = -1;
                if (ev.function == ProxyEvent::setToolBarSize) {
                    ev.ok = setToolBarSize(ev.w, ev.h);
                    sendEvent(ev);
                } else if (ev.function == ProxyEvent::createButton) {
                    id = createButton(ev.x, ev.y, ev.w, ev.h, ev.text);
                } else if (ev.function == ProxyEvent::createLabel) {
                    id = createLabel(ev.x, ev.y, ev.w, ev.h, ev.text);
                } else if (ev.function == ProxyEvent::createSlider) {
                    id = createSlider(ev.x, ev.y, ev.w, ev.h, ev.min, ev.max, ev.start);
                } else if (ev.function == ProxyEvent::createCanvas) {
                    id = createCanvas(ev.x, ev.y, ev.w, ev.h);
                } else {
                    std::cerr << "ERROR: wrong proxy event (" << ev.function << ")\n";
                    assert(false);
                    exit(-1);
                }
                if (id != -1) {
                    ev.widgetId = id;
                    ev.tool = tool;
                    sendEvent(ev);
                }
            }
        }
    };

    void init_module()
    {
        const std::string dir = "../UntrustedPlugins";

        //TODO send DLLs to server

        ProxyEvent ev;
        ev.function = ProxyEvent::init_module;
        sendEvent(ev);

        while ((ev = getEvent()).function != ProxyEvent::init_module_finished) {
            assert(ev.tool);
            Proxy *tool = new Proxy(ev.tool);
            addTool(tool);
        }
    }
}
