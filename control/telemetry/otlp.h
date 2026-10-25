#pragma once

#include <cstdlib>

namespace control::telemetry
{
constexpr const char *otlp_endpoint_env = "OTEL_EXPORTER_OTLP_ENDPOINT";

bool otlp_enabled()
{
    const char *value = std::getenv(otlp_endpoint_env);
    return value != nullptr && *value != '\0';
}

}
