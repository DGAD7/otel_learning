#include "led.h"
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <linux/gpio.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace led
{

class led::impl
{
    public:
        impl(int gpio_line, int chip, polarity_t polarity);
        ~impl();
        void set_state(state_t);

    private:
        int m_line;
        int m_line_fd = -1;
};

led::impl::~impl()
{
    if (m_line_fd > 0)
    {
        ::close(m_line_fd);
    }
}

led::impl::impl(int gpio_line, int chip, polarity_t polarity)
    : m_line{gpio_line}
{
    const std::string chip_path = "/dev/gpiochip" + std::to_string(chip);
    const int chip_fd = ::open(chip_path.c_str(), O_RDWR | O_CLOEXEC);
    if (chip_fd < 0)
    {
        const std::string err = "failed to open gpio chip:";
        throw std::runtime_error{err};
    }

    struct gpio_v2_line_request gpio_req{};

    constexpr int first_line_index = 0;
    gpio_req.offsets[first_line_index] = m_line;
    gpio_req.num_lines = 1;

    std::uint64_t line_flags = 0;
    line_flags |= GPIO_V2_LINE_FLAG_OUTPUT;

    std::strncpy(gpio_req.consumer, "led_control", sizeof(gpio_req.consumer) - 1);
    if (polarity == polarity_t::active_low)
    {
        line_flags |= GPIO_V2_LINE_FLAG_ACTIVE_LOW;
    }

    gpio_req.config.flags = line_flags;

    auto response = ioctl(chip_fd, GPIO_V2_GET_LINE_IOCTL, &gpio_req);
    if (response < 0)
    {
        const std::string err = "failed to request gpio line from linux kernel. chip:" + chip_path + " line:" + std::to_string(m_line) + " error:" + std::string(std::strerror(errno));
        ::close(chip_fd);
        throw std::runtime_error{err};
    }

    m_line_fd = gpio_req.fd;
    ::close(chip_fd);
}

void led::impl::set_state(state_t to_state)
{
    struct gpio_v2_line_values line_value{};
    line_value.mask = 1ULL;
    line_value.bits = (to_state == state_t::active) ? 1ULL : 0ULL;

    const auto response = ioctl(m_line_fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &line_value);
    if (response < 0)
    {
        const std::string err = "failed to set gpio line:";
        throw std::runtime_error{err};
    }
}

led::led(led_config config)
    : m_impl{std::make_unique<impl>(config.line, config.chip, config.polarity)}
{
}

void led::set_state(bool state)
{
    if (state)
    {
        return m_impl->set_state(state_t::active);
    }
    return m_impl->set_state(state_t::inactive);
}

led::~led() = default;

} // namespace led
