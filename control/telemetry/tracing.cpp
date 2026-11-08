#include "telemetry/tracing.h"
#include "telemetry/otlp.h"

#include "opentelemetry/context/propagation/global_propagator.h"
#include "opentelemetry/exporters/otlp/otlp_grpc_exporter_factory.h"
#include "opentelemetry/exporters/otlp/otlp_grpc_exporter_options.h"
#include "opentelemetry/sdk/resource/resource.h"
#include "opentelemetry/sdk/trace/batch_span_processor_factory.h"
#include "opentelemetry/sdk/trace/batch_span_processor_options.h"
#include "opentelemetry/sdk/trace/provider.h"
#include "opentelemetry/sdk/trace/tracer_provider.h"
#include "opentelemetry/sdk/trace/tracer_provider_factory.h"
#include "opentelemetry/trace/propagation/http_trace_context.h"

#include <memory>
#include <utility>

namespace trace_api = opentelemetry::trace;
namespace trace_sdk = opentelemetry::sdk::trace;
namespace otlp = opentelemetry::exporter::otlp;
namespace propagation = opentelemetry::context::propagation;

namespace
{

std::shared_ptr<trace_sdk::TracerProvider> g_provider;

}

namespace control::telemetry
{

void init_tracing()
{
    // W3C traceparent, the same format the Go UI sends
    propagation::GlobalTextMapPropagator::SetGlobalPropagator(
        opentelemetry::nostd::shared_ptr<propagation::TextMapPropagator>(
            new trace_api::propagation::HttpTraceContext()));

    if (!otlp_enabled())
    {
        return;
    }

    otlp::OtlpGrpcExporterOptions options; // reads OTEL_EXPORTER_OTLP_* env vars
    auto exporter = otlp::OtlpGrpcExporterFactory::Create(options);
    auto processor = trace_sdk::BatchSpanProcessorFactory::Create(std::move(exporter), trace_sdk::BatchSpanProcessorOptions{});

    auto resource = opentelemetry::sdk::resource::Resource::Create({{"service.name", "led-control-cpp"}});
    g_provider = trace_sdk::TracerProviderFactory::Create(std::move(processor), resource);

    std::shared_ptr<trace_api::TracerProvider> api_provider = g_provider;
    trace_sdk::Provider::SetTracerProvider(api_provider);
}

void shutdown_tracing()
{
    if (g_provider)
    {
        g_provider->ForceFlush();
        g_provider->Shutdown();
    }
}

}
