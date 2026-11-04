#pragma once

#include "grpc/generated/led_control.grpc.pb.h"
#include "grpc/generated/led_control.pb.h"
#include "led/led.h"

#include "grpcpp/server_context.h"
#include "grpcpp/support/status.h"
#include "opentelemetry/metrics/sync_instruments.h"
#include "opentelemetry/nostd/unique_ptr.h"

#include <cstddef>
#include <cstdint>
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
        std::size_t m_toggle_count = 0;
        opentelemetry::nostd::unique_ptr<opentelemetry::metrics::Counter<std::uint64_t>> m_switch_on_counter;
};

} // namespace led_control
