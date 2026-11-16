#pragma once

#include "grpc/generated/led_control.grpc.pb.h"
#include "grpc/generated/led_control.pb.h"
#include "led/led.h"

#include "grpcpp/server_context.h"
#include "grpcpp/support/status.h"
#include "opentelemetry/metrics/async_instruments.h"
#include "opentelemetry/metrics/observer_result.h"
#include "opentelemetry/metrics/sync_instruments.h"
#include "opentelemetry/nostd/shared_ptr.h"
#include "opentelemetry/nostd/unique_ptr.h"

#include <atomic>
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
        static void observe_led_status(opentelemetry::metrics::ObserverResult result, void *state);
        std::shared_ptr<led::led> m_led;
        std::atomic<bool> m_led_state{false};
        std::size_t m_toggle_count = 0;
        opentelemetry::nostd::unique_ptr<opentelemetry::metrics::Counter<std::uint64_t>> m_switch_on_counter;
        opentelemetry::nostd::shared_ptr<opentelemetry::metrics::ObservableInstrument> m_led_state_gauge;
};

} // namespace led_control
