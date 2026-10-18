#include "telemetry/custom_exporter.h"

#include "opentelemetry/exporters/ostream/common_utils.h"
#include "opentelemetry/logs/severity.h"
#include "opentelemetry/nostd/span.h"
#include "opentelemetry/sdk/common/exporter_utils.h"
#include "opentelemetry/sdk/logs/read_write_log_record.h"
#include "opentelemetry/sdk/logs/recordable.h"

#include <chrono>
#include <cstddef>
#include <ctime>
#include <iomanip>
#include <ios>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>

namespace control::telemetry
{

namespace logs_sdk = opentelemetry::sdk::logs;
using opentelemetry::exporter::ostream_common::print_value;

custom_exporter::custom_exporter(std::ostream &out)
    : m_out{out}
{
}

std::unique_ptr<logs_sdk::Recordable> custom_exporter::MakeRecordable() noexcept
{
    return std::make_unique<logs_sdk::ReadWriteLogRecord>();
}

opentelemetry::sdk::common::ExportResult custom_exporter::Export(
    const opentelemetry::nostd::span<std::unique_ptr<logs_sdk::Recordable>> &records) noexcept
{
    for (const auto &record : records)
    {
        // every record came from our MakeRecordable(), so this cast is safe
        const auto *r = static_cast<const logs_sdk::ReadWriteLogRecord *>(record.get());

        std::ostringstream line;

        // timestamp: 15:04:05.000
        const std::chrono::system_clock::time_point tp = r->GetTimestamp();
        const std::time_t secs = std::chrono::system_clock::to_time_t(tp);
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count() % 1000;
        std::tm local{};
        localtime_r(&secs, &local);
        line << std::put_time(&local, "%H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << ms
             << std::setfill(' ');

        // severity, scope, body
        const auto severity = opentelemetry::logs::SeverityNumToText[static_cast<std::size_t>(r->GetSeverity())];
        line << ' ' << std::left << std::setw(5) << std::string(severity.data(), severity.size())
             << " [" << r->GetInstrumentationScope().GetName() << "] ";
        print_value(r->GetBody(), line);

        // attributes
        for (const auto &[key, value] : r->GetAttributes())
        {
            line << ' ' << key << '=';
            print_value(value, line);
        }

        m_out << line.str() << '\n';
    }
    return opentelemetry::sdk::common::ExportResult::kSuccess;
}

bool custom_exporter::ForceFlush(std::chrono::microseconds) noexcept
{
    m_out.flush();
    return true;
}

bool custom_exporter::Shutdown(std::chrono::microseconds) noexcept
{
    return true;
}

} // namespace control::telemetry
