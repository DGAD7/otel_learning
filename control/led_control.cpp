
#include "led_control.h"
#include "grpc/generated/led_control.pb.h"
#include "led/led.h"

#include "grpcpp/server_context.h"
#include "grpcpp/support/status.h"
#include "opentelemetry/context/propagation/global_propagator.h"
#include "opentelemetry/context/propagation/text_map_propagator.h"
#include "opentelemetry/context/runtime_context.h"
#include "opentelemetry/logs/logger.h"
#include "opentelemetry/metrics/provider.h"
#include "opentelemetry/nostd/shared_ptr.h"
#include "opentelemetry/nostd/string_view.h"
#include "opentelemetry/trace/context.h"
#include "opentelemetry/trace/span_metadata.h"
#include "opentelemetry/trace/span_startoptions.h"
#include "telemetry/logging.h"
#include "telemetry/tracing.h"

#include <exception>
#include <grpcpp/support/status.h>
#include <grpcpp/support/string_ref.h>
#include <memory>
#include <unistd.h>
#include <utility>

namespace
{

using control::telemetry::With;

inline opentelemetry::nostd::shared_ptr<opentelemetry::logs::Logger> logger()
{
    return control::telemetry::get_logger("led_control");
}

class grpc_server_carrier : public opentelemetry::context::propagation::TextMapCarrier
{
    public:
        explicit grpc_server_carrier(const grpc::ServerContext *context)
            : m_context{context}
        {
        }

        opentelemetry::nostd::string_view Get(opentelemetry::nostd::string_view key) const noexcept override
        {
            const auto &metadata = m_context->client_metadata();
            const auto it = metadata.find(grpc::string_ref{key.data(), key.size()});
            if (it == metadata.end())
            {
                return "";
            }
            return {it->second.data(), it->second.size()};
        }

        void Set(opentelemetry::nostd::string_view, opentelemetry::nostd::string_view) noexcept override
        {
        }

    private:
        const grpc::ServerContext *m_context;
};

}

namespace led_control
{

led_control::led_control(std::shared_ptr<led::led> led)
    : m_led{std::move(led)}
{
    auto meter = opentelemetry::metrics::Provider::GetMeterProvider()->GetMeter("led_control");
    m_switch_on_counter = meter->CreateUInt64Counter(
        "led.switch_on",
        "Number of times the LED was switched on",
        "{switch}");
}

grpc::Status led_control::SetLed(grpc::ServerContext *context,
                                 const ledcontrol::SetLedRequest *request,
                                 ledcontrol::SetLedResponse *response)
{
    auto propagator = opentelemetry::context::propagation::GlobalTextMapPropagator::GetGlobalPropagator();
    grpc_server_carrier carrier{context};
    auto current = opentelemetry::context::RuntimeContext::GetCurrent();
    auto parent = propagator->Extract(carrier, current);

    opentelemetry::trace::StartSpanOptions options;
    options.kind = opentelemetry::trace::SpanKind::kServer;
    options.parent = opentelemetry::trace::GetSpan(parent)->GetContext();
    auto tracer = control::telemetry::get_tracer("led_control");
    auto span = tracer->StartSpan("ledcontrol.LedControl/SetLed",
                                  {{"rpc.system", "grpc"},
                                   {"rpc.service", "ledcontrol.LedControl"},
                                   {"rpc.method", "SetLed"},
                                   {"led.requested_state", request->on()}},
                                  options);
    auto scope = tracer->WithActiveSpan(span);
    try
    {
        m_led->set_state(request->on());
        response->set_on(request->on());
        if (request->on())
        {
            m_toggle_count++;
            m_switch_on_counter->Add(1);
        }
        response->set_total_times_switch_on(m_toggle_count);
        logger()->Info("change the state of LED", With({{"new state", request->on()}}));
        span->End();
        return grpc::Status::OK;
    }
    catch (const std::exception &e)
    {
        logger()->Error("failed to change led state");
        span->SetStatus(opentelemetry::trace::StatusCode::kError, e.what());
        span->End();
        return grpc::Status::CANCELLED;
    }
}

} // namespace led_control
