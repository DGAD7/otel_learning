#include "led/led.h"
#include "led_control.h"

#include <iostream>
#include <memory>
#include <ostream>

int main()
{
    std::cout << "Hello OTEL" << std::endl;

    const auto led = std::make_shared<led::led>();
    const auto led_controller = std::make_shared<led_control::led_control>(led);
    return 0;
}
