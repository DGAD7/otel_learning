#pragma once

#include "opentelemetry/logs/logger.h"
#include "opentelemetry/logs/provider.h"
#include "opentelemetry/nostd/shared_ptr.h"

namespace control::telemetry
{

void init_logging();
void shutdown_logging();

inline opentelemetry::nostd::shared_ptr<opentelemetry::logs::Logger> get_logger(const char* name)
{
    return opentelemetry::logs::Provider::GetLoggerProvider()->GetLogger(name);
}

}
