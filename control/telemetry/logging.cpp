#include "telemetry/logging.h"

#include "opentelemetry/exporters/ostream/log_record_exporter_factory.h"
#include "opentelemetry/logs/logger_provider.h"
#include "opentelemetry/sdk/logs/logger_provider.h"
#include "opentelemetry/sdk/logs/logger_provider_factory.h"
#include "opentelemetry/sdk/logs/provider.h"
#include "opentelemetry/sdk/logs/simple_log_record_processor_factory.h"
#include "opentelemetry/sdk/resource/resource.h"

#include <memory>
#include <utility>

namespace logs_api = opentelemetry::logs;
namespace logs_sdk = opentelemetry::sdk::logs;

namespace
{
std::shared_ptr<logs_sdk::LoggerProvider> g_provider;
}

namespace control::telemetry
{

void init_logging()
{
    auto exporter = opentelemetry::exporter::logs::OStreamLogRecordExporterFactory::Create();
    auto processor = logs_sdk::SimpleLogRecordProcessorFactory::Create(std::move(exporter));
    auto resource = opentelemetry::sdk::resource::Resource::Create({{"service.name", "led-control"}});

    g_provider = logs_sdk::LoggerProviderFactory::Create(std::move(processor), resource);

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
