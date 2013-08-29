#include "System.h"

int main() {
    auto& led = System::instance().led();
    static_cast<void>(led);

	while (true) {
	    led.on();
	    for (volatile int i = 0; i < 1000000; ++i) {}
	    led.off();
	    for (volatile int i = 0; i < 1000000; ++i) {}
	}

	return 0;
}
