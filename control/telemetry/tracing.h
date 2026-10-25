#pragma once

#include "opentelemetry/nostd/shared_ptr.h"
#include "opentelemetry/trace/provider.h"
#include "opentelemetry/trace/tracer.h"

namespace control::telemetry
{

void init_tracing();
void shutdown_tracing();

inline opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> get_tracer(const char *name)
{
    return opentelemetry::trace::Provider::GetTracerProvider()->GetTracer(name);
}

}
