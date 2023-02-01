#include <cassert>
#include <iostream>
#include <string>
#include <string.h>
#include <unistd.h>

#include "tools.hpp"
#include "proxy-event.hpp"

#ifdef __cplusplus
extern "C" {
#endif

int exec_connector();

#ifdef __cplusplus
}
#endif

extern int IN_FD;
extern int OUT_FD;

namespace booba {

    static ProxyEvent getEvent()
    {
        std::cerr << "Client awaiting event\n";	
        ProxyEvent ev = {};
	size_t res = read(IN_FD, (char*)&ev, sizeof(ProxyEvent));
        if (res != sizeof(ProxyEvent)) {
            assert(false);
        }
        if (!APPCONTEXT) {
            APPCONTEXT = new ApplicationContext;
        }
        *APPCONTEXT = ev.context;
        std::cerr << "Client got event " << ev.function << "\n";
        return ev;
    }

    static void sendEvent(ProxyEvent ev)
    {
	assert(ev.function < ProxyEvent::updateContext);

        write(OUT_FD, (char*)&ev, sizeof(ProxyEvent));
        std::cerr << "Client sent event " << ev.function << "\n";
    }

    class Proxy : public Tool {
    private:
        Tool *tool = nullptr;
        Image *image = nullptr;
        char *texture = NULL;

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

            ProxyEvent pev;
            pev.function = ProxyEvent::apply;
            pev.event = *ev;
            pev.tool = tool;
            sendEvent(pev);

            handleApplyEvents(im);
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
                uint64_t id = -179;
                if (ev.function == ProxyEvent::createButton) {
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
                if (id != -179) {
                    ev.widgetId = id;
                    ev.tool = tool;
                    sendEvent(ev);
                }
            }
        }
    };

    void init_module()
    {
	exec_connector();

	system("qvm-copy ../UntrustedPlugins");

        ProxyEvent ev;
        ev.function = ProxyEvent::init_module;
        sendEvent(ev);

        while ((ev = getEvent()).function != ProxyEvent::init_module_finished) {
	    if (ev.function == ProxyEvent::addTool) {
                assert(ev.tool);
                Proxy *tool = new Proxy(ev.tool);
                addTool(tool);
	    } else if (ev.function == ProxyEvent::setToolBarSize) {
                ev.ok = setToolBarSize(ev.w, ev.h);
                sendEvent(ev);
	    } else {
                std::cerr << "ERROR: wrong proxy event (" << ev.function << ")\n";
                assert(false);
                exit(-1);
	    }
        }
    }
}
