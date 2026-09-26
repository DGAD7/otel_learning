#include "led.h"

namespace led
{

bool led::set_state(bool state)
{
    if (state)
    {
        return true;
    }
    return false;
}

} // namespace led
