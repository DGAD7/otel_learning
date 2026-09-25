#pragma once

#include "grpc/generated/led_control.grpc.pb.h"
#include "led/led.h"

#include <memory>

namespace led_control
{

class led_control : public ledcontrol::LedControl::Service
{
    public:
        explicit led_control(std::shared_ptr<led::led> led);

    private:
        std::shared_ptr<led::led> m_led;
};

} // namespace led_control
