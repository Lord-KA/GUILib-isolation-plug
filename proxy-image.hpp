#include <cassert>
#include <iostream>

#include "tools.hpp"
#include "proxy-event.hpp"

namespace booba {
    static ProxyEvent getEvent() //TODO move from cin/cout to other streams
    {
        std::cerr << "Server awaiting event\n";
        ProxyEvent ev;
        std::cin.read((char*)&ev, sizeof(ProxyEvent));
        if (std::cin.gcount() != sizeof(ProxyEvent)) {
            assert(false);
        }
        std::cerr << "Server got event " << ev.function << "\n";
        return ev;
    }

    static void sendEvent(ProxyEvent ev)
    {
        assert(APPCONTEXT);
        ev.context = *APPCONTEXT;
        std::cout.write((char*)&ev, sizeof(ProxyEvent));
        std::cout.flush();      //TODO maybe do flush() on many buffered events.
        std::cerr << "Server sent event " << ev.function << "\n";
    }

    class ProxyImage : public Image {
        virtual size_t getH() override
        {
            ProxyEvent ev;
            ev.function = ProxyEvent::getH;
            sendEvent(ev);

            ev = getEvent();
            assert(ev.function == ProxyEvent::getH);
            return ev.h;
        }

        virtual size_t getW() override
        {
            ProxyEvent ev;
            ev.function = ProxyEvent::getW;
            sendEvent(ev);

            ev = getEvent();
            assert(ev.function == ProxyEvent::getW);
            return ev.w;
        }

        virtual uint32_t getPixel(size_t x, size_t y) override
        {
            ProxyEvent ev;
            ev.function = ProxyEvent::getPixel;
            ev.x = x;
            ev.y = y;
            sendEvent(ev);

            ev = getEvent();
            assert(ev.function == ProxyEvent::getPixel);
            return ev.color;
        }

        virtual void setPixel(size_t x, size_t y, uint32_t color) override
        {
            ProxyEvent ev;
            ev.function = ProxyEvent::setPixel;
            ev.x = x;
            ev.y = y;
            ev.color = color;
            sendEvent(ev);
        }
    };
}
