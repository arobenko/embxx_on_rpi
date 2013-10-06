//
// Copyright 2013 (C). Alex Robenko. All rights reserved.
//

#include "System.h"

#include <functional>

#include "embxx/util/Assert.h"

namespace
{

const unsigned LedChangeStateTimeout = 500; // 1/2 second

template <typename TTimer>
void ledOff(
    TTimer& timer,
    device::Led& led);

template <typename TTimer>
void ledOn(
    TTimer& timer,
    device::Led& led)
{
    led.on();

    timer.asyncWait(
        LedChangeStateTimeout,
        [&timer, &led](embxx::driver::ErrorStatus status)
        {
            static_cast<void>(status);
            ledOff(timer, led);
        });
}

template <typename TTimer>
void ledOff(
    TTimer& timer,
    device::Led& led)
{
    led.off();

    timer.asyncWait(
        LedChangeStateTimeout,
        [&timer, &led](embxx::driver::ErrorStatus status)
        {
            static_cast<void>(status);
            ledOn(timer, led);
        });
}

class LedOnAssert : public embxx::util::Assert
{
public:
    LedOnAssert(device::Led& led)
        : led_(led)
    {
    }

    virtual void fail(
        const char* expr,
        const char* file,
        unsigned int line,
        const char* function)
    {
        static_cast<void>(expr);
        static_cast<void>(file);
        static_cast<void>(line);
        static_cast<void>(function);

        led_.on();
        while (true) {;}
    }

private:
    device::Led& led_;
};

}  // namespace

int main() {
    auto& system = System::instance();
    auto& led = system.led();
    embxx::util::EnableAssert<LedOnAssert> assertion(std::ref(led));

    auto& timerMgr = system.timerMgr();
    auto timer = timerMgr.allocTimer();
    GASSERT(timer.isValid());
    device::interrupt::enable();

    ledOff(timer, led);

    auto& el = system.eventLoop();
    el.run();
    GASSERT(0); // Mustn't exit
	return 0;
}
