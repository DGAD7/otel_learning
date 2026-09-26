#pragma once

#include "grpc/generated/led_control.grpc.pb.h"
#include "grpc/generated/led_control.pb.h"
#include "led/led.h"

#include "grpcpp/server_context.h"
#include "grpcpp/support/status.h"

#include <memory>

namespace led_control
{

class led_control : public ledcontrol::LedControl::Service
{
    public:
        explicit led_control(std::shared_ptr<led::led> led);

        grpc::Status SetLed(grpc::ServerContext *context,
                            const ledcontrol::SetLedRequest *request,
                            ledcontrol::SetLedResponse *response) override;

    private:
        std::shared_ptr<led::led> m_led;
};

} // namespace led_control
