#include "telemetry/metrics.h"
#include "opentelemetry/metrics/meter_provider.h"
#include "telemetry/otlp.h"

#include "opentelemetry/exporters/otlp/otlp_grpc_metric_exporter_factory.h"
#include "opentelemetry/exporters/otlp/otlp_grpc_metric_exporter_options.h"
#include "opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader_factory.h"
#include "opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader_options.h"
#include "opentelemetry/sdk/metrics/meter_context_factory.h"
#include "opentelemetry/sdk/metrics/meter_provider.h"
#include "opentelemetry/sdk/metrics/meter_provider_factory.h"
#include "opentelemetry/sdk/metrics/provider.h"
#include "opentelemetry/sdk/metrics/view/view_registry_factory.h"
#include "opentelemetry/sdk/resource/resource.h"

#include <chrono>
#include <memory>
#include <utility>

namespace metrics_api = opentelemetry::metrics;
namespace metrics_sdk = opentelemetry::sdk::metrics;
namespace otlp = opentelemetry::exporter::otlp;

namespace
{

std::shared_ptr<metrics_sdk::MeterProvider> g_provider;

}

namespace control::telemetry
{

void init_metrics()
{
    if (!otlp_enabled())
    {
        return; // no provider set -> the API hands out no-op meters
    }

    otlp::OtlpGrpcMetricExporterOptions exporter_options; // reads OTEL_EXPORTER_OTLP_* env vars
    auto exporter = otlp::OtlpGrpcMetricExporterFactory::Create(exporter_options);

    metrics_sdk::PeriodicExportingMetricReaderOptions reader_options;
    reader_options.export_interval_millis = std::chrono::milliseconds(10000);
    reader_options.export_timeout_millis = std::chrono::milliseconds(5000);
    auto reader = metrics_sdk::PeriodicExportingMetricReaderFactory::Create(std::move(exporter), reader_options);

    auto resource = opentelemetry::sdk::resource::Resource::Create({{"service.name", "led-control"}});
    auto context = metrics_sdk::MeterContextFactory::Create(metrics_sdk::ViewRegistryFactory::Create(), resource);
    context->AddMetricReader(std::move(reader));

    g_provider = metrics_sdk::MeterProviderFactory::Create(std::move(context));

    std::shared_ptr<metrics_api::MeterProvider> api_provider = g_provider;
    metrics_sdk::Provider::SetMeterProvider(api_provider);
}

void shutdown_metrics()
{
    if (g_provider)
    {
        g_provider->ForceFlush();
        g_provider->Shutdown();
    }
}

}
