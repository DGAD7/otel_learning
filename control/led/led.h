#pragma once

#include <memory>

namespace led
{

enum class polarity_t
{
    active_high,
    active_low,
};

enum class state_t
{
    active,
    inactive,
};

struct led_config
{
        int line;
        int chip;
        polarity_t polarity;
};

class led
{

    public:
        explicit led(led_config);
        ~led();
        void set_state(bool);

    private:
        class impl;
        std::unique_ptr<impl> m_impl;
};

} // namespace led
