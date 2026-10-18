#include "telemetry/logging.h"
#include "telemetry/custom_exporter.h"

#include "opentelemetry/exporters/otlp/otlp_grpc_log_record_exporter_factory.h"
#include "opentelemetry/exporters/otlp/otlp_grpc_log_record_exporter_options.h"
#include "opentelemetry/logs/logger_provider.h"
#include "opentelemetry/sdk/logs/batch_log_record_processor_factory.h" // BatchLogRecordProcessorFactory
#include "opentelemetry/sdk/logs/batch_log_record_processor_options.h"
#include "opentelemetry/sdk/logs/logger_provider.h"
#include "opentelemetry/sdk/logs/logger_provider_factory.h"
#include "opentelemetry/sdk/logs/processor.h"
#include "opentelemetry/sdk/logs/provider.h"
#include "opentelemetry/sdk/logs/simple_log_record_processor_factory.h"
#include "opentelemetry/sdk/resource/resource.h"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

namespace logs_api = opentelemetry::logs;
namespace logs_sdk = opentelemetry::sdk::logs;

namespace
{

std::shared_ptr<logs_sdk::LoggerProvider> g_provider;
constexpr const char *otlp_endpoint_env = "OTEL_EXPORTER_OTLP_ENDPOINT";

bool otlp_enabled()
{
    const char *value = std::getenv(otlp_endpoint_env);
    return value != nullptr && *value != '\0';
}

}

namespace control::telemetry
{

void init_logging()
{

    std::vector<std::unique_ptr<logs_sdk::LogRecordProcessor>> processors;

    processors.push_back(logs_sdk::SimpleLogRecordProcessorFactory::Create(std::make_unique<custom_exporter>(std::cout)));

    if (otlp_enabled())
    {
        opentelemetry::exporter::otlp::OtlpGrpcLogRecordExporterOptions options; // reads OTEL_EXPORTER_OTLP_* env vars
        auto otlp_exporter = opentelemetry::exporter::otlp::OtlpGrpcLogRecordExporterFactory::Create(options);
        processors.push_back(
            logs_sdk::BatchLogRecordProcessorFactory::Create(std::move(otlp_exporter),
                                                             logs_sdk::BatchLogRecordProcessorOptions{}));
    }

    auto resource = opentelemetry::sdk::resource::Resource::Create({{"service.name", "led-control"}});

    g_provider = logs_sdk::LoggerProviderFactory::Create(std::move(processors), resource);

    std::shared_ptr<logs_api::LoggerProvider> api_provider = g_provider;
    logs_sdk::Provider::SetLoggerProvider(api_provider);
}

void shutdown_logging()
{
    if (g_provider)
    {
        g_provider->ForceFlush();
        g_provider->Shutdown();
    }
}

}
