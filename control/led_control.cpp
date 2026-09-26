
#include "led_control.h"
#include "grpc/generated/led_control.pb.h"
#include "led/led.h"

#include "grpcpp/server_context.h"
#include "grpcpp/support/status.h"

#include <exception>
#include <grpcpp/support/status.h>
#include <memory>
#include <utility>

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
        const auto new_state = m_led->set_state(request->on());
        response->set_on(new_state);
        return grpc::Status::OK;
    }
    catch (const std::exception &)
    {
        return grpc::Status::CANCELLED;
    }
}

} // namespace led_control
