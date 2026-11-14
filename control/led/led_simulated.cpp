#include "led.h"

#include <iostream>
#include <memory>

namespace led
{

class led::impl
{
    public:
        impl(int line, int chip)
            : m_line{line}
            , m_chip{chip}
        {
            std::cout << "[sim] LED on gpiochip" << m_chip << " line " << m_line << '\n';
        }

        void set_state(bool on)
        {
            std::cout << "[sim] LED line " << m_line << (on ? " ON" : " OFF") << '\n';
        }

    private:
        int m_line;
        int m_chip;
};

led::led(led_config config)
    : m_impl{std::make_unique<impl>(config.line, config.chip)}
{
}

led::~led() = default;

void led::set_state(bool state)
{
    m_impl->set_state(state);
}

} // namespace led
