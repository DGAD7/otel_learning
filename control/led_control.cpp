
#include "led_control.h"
#include "grpc/generated/led_control.pb.h"
#include "led/led.h"

#include "grpcpp/server_context.h"
#include "grpcpp/support/status.h"
#include "opentelemetry/logs/logger.h"
#include "opentelemetry/nostd/shared_ptr.h"
#include "telemetry/logging.h"

#include <exception>
#include <grpcpp/support/status.h>
#include <memory>
#include <utility>

namespace
{

using control::telemetry::With;

inline opentelemetry::nostd::shared_ptr<opentelemetry::logs::Logger> logger()
{
    return control::telemetry::get_logger("led_control");
}

}

namespace led_control
{

led_control::led_control(std::shared_ptr<led::led> led)
    : m_led{std::move(led)}
{
}

grpc::Status led_control::SetLed(grpc::ServerContext *,
                                 const ledcontrol::SetLedRequest *request,
                                 ledcontrol::SetLedResponse *response)
{
    try
    {
        m_led->set_state(request->on());
        response->set_on(request->on());
        if (request->on())
        {
            m_toggle_count++;
        }
        response->set_total_times_switch_on(m_toggle_count);
        logger()->Info("change the state of LED", With({{"new state", request->on()}}));
        return grpc::Status::OK;
    }
    catch (const std::exception &)
    {
        logger()->Error("failed to change led state");
        return grpc::Status::CANCELLED;
    }
}

} // namespace led_control
