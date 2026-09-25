
#include "led_control.h"
#include "led/led.h"

#include <memory>
#include <utility>

namespace led_control
{

led_control::led_control(std::shared_ptr<led::led> led)
    : m_led{std::move(led)}
{
}

} // namespace led_control
