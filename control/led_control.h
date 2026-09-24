#pragma once

#include "grpc/generated/led_control.grpc.pb.h"

namespace led_control {

class led_control : public ledcontrol::LedControl::Service {
public:
private:
};

} // namespace led_control
