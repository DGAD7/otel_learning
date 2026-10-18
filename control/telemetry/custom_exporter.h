#pragma once

#include "opentelemetry/nostd/span.h"
#include "opentelemetry/sdk/common/exporter_utils.h"
#include "opentelemetry/sdk/logs/exporter.h"
#include "opentelemetry/sdk/logs/recordable.h"

#include <chrono>
#include <memory>
#include <ostream>

namespace control::telemetry
{

class custom_exporter final : public opentelemetry::sdk::logs::LogRecordExporter
{
    public:
        explicit custom_exporter(std::ostream &out);

        std::unique_ptr<opentelemetry::sdk::logs::Recordable> MakeRecordable() noexcept override;
        opentelemetry::sdk::common::ExportResult Export(const opentelemetry::nostd::span<std::unique_ptr<opentelemetry::sdk::logs::Recordable>> &) noexcept override;

        bool ForceFlush(std::chrono::microseconds timeout) noexcept override;
        bool Shutdown(std::chrono::microseconds timeout) noexcept override;

    private:
        std::ostream &m_out;
};

}
