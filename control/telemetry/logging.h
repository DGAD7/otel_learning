#pragma once

#include "opentelemetry/common/attribute_value.h"
#include "opentelemetry/common/key_value_iterable_view.h"
#include "opentelemetry/logs/logger.h"
#include "opentelemetry/logs/provider.h"
#include "opentelemetry/nostd/shared_ptr.h"
#include "opentelemetry/nostd/string_view.h"

#include <initializer_list>
#include <utility>

namespace control::telemetry
{

void init_logging();
void shutdown_logging();

inline opentelemetry::nostd::shared_ptr<opentelemetry::logs::Logger> get_logger(const char* name)
{
    return opentelemetry::logs::Provider::GetLoggerProvider()->GetLogger(name);
}

inline auto With(
    std::initializer_list<std::pair<opentelemetry::nostd::string_view, opentelemetry::common::AttributeValue>> attributes)
{
    return opentelemetry::common::MakeAttributes(attributes);
}

}
