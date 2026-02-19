#pragma once

constexpr auto DS1307_ADDRESS = 0x68; ///< I2C address for DS1307

// #include <RTClib.h>
class RTCDateTime
{
public:
    constexpr RTCDateTime(const uint16_t year, const uint8_t month, const uint8_t day, const uint8_t hour = 0u,
                          const uint8_t min = 0u, const uint8_t sec = 0u) : year_offset(static_cast<uint8_t>(year >= 2000u ? year - 2000u : year)),
                                                                            m(month), d(day),
                                                                            hh(hour), mm(min), ss(sec), dd(day_of_week()) {};

    // https://learn.microsoft.com/en-us/cpp/preprocessor/predefined-macros?view=msvc-170
    // The first character of date dd is a space if the value is less than 10. This macro is always defined
    // __DATE__ mmm dd yyyy
    // __TIME__ hh:mm:ss
    RTCDateTime(const char *date = __DATE__, const char *time = __TIME__)
    {
        // Date
        const uint16_t year = (date[7] - '0') * 1000u +
                              (date[8] - '0') * 100u +
                              (date[9] - '0') * 10u +
                              (date[10] - '0');
        year_offset = static_cast<uint8_t>(year >= 2000u ? year - 2000u : year);

        const char month_str[4] = {date[0], date[1], date[2], '\0'};
        m = month_from_str(month_str);

        d = (date[4] == ' ' ? 0u : (date[4] - '0')) * 10u + (date[5] - '0');

        // Time
        hh = (time[0] - '0') * 10u + (time[1] - '0');
        mm = (time[3] - '0') * 10u + (time[4] - '0');
        ss = (time[6] - '0') * 10u + (time[7] - '0');

        dd = day_of_week();
    }

    inline constexpr uint16_t year() const
    {
        return static_cast<uint16_t>(year_offset) + 2000u;
    }

    uint8_t year_offset; ///< Year offset from 2000
    uint8_t m;           ///< Month 1-12
    uint8_t d;           ///< Day 1-31
    uint8_t hh;          ///< Hours 0-23
    uint8_t mm;          ///< Minutes 0-59
    uint8_t ss;          ///< Seconds 0-59
    uint8_t dd;          ///< Day of week 1-7 ISO
private:
    uint8_t month_from_str(const char *m) const
    {
        if (!strcmp(m, "Jan"))
            return 1u;
        if (!strcmp(m, "Feb"))
            return 2u;
        if (!strcmp(m, "Mar"))
            return 3u;
        if (!strcmp(m, "Apr"))
            return 4u;
        if (!strcmp(m, "May"))
            return 5u;
        if (!strcmp(m, "Jun"))
            return 6u;
        if (!strcmp(m, "Jul"))
            return 7u;
        if (!strcmp(m, "Aug"))
            return 8u;
        if (!strcmp(m, "Sep"))
            return 9u;
        if (!strcmp(m, "Oct"))
            return 10u;
        if (!strcmp(m, "Nov"))
            return 11u;
        if (!strcmp(m, "Dec"))
            return 12u;
        return 1u;
    }

    // Zeller's congruence
    inline constexpr uint8_t day_of_week() const
    {
        auto y_ = year();
        auto m_ = m;
        auto d_ = d;

        if (m_ < 3)
        {
            m_ += 12;
            y_ -= 1;
        }

        const auto K = y_ % 100;
        const auto J = y_ / 100;

        const auto h =
            (d_ + (13 * (m_ + 1)) / 5 + K + K / 4 + J / 4 + 5 * J) % 7;

        // Convert to ISO
        auto dd = h == 0 ? 6 : (h == 1 ? 7 : h - 1);
        return min(max(1, dd) - 1, 6);
    }
};

class RTCModule
{
public:
    RTCModule() {};
    ~RTCModule()
    {
        if (device)
        {
            delete device;
        }
    }
    [[nodiscard]] bool setup()
    {
        if (device)
        {
            return false;
        }

        device = new Adafruit_I2CDevice(DS1307_ADDRESS, &Wire);
        return device->begin();
    }

    [[nodiscard]] inline RTCDateTime now()
    {
        uint8_t buffer[7];
        buffer[0] = 0;
        device->write_then_read(buffer, 1, buffer, 7);

        return RTCDateTime(bcd2bin(buffer[6]) + 2000U, bcd2bin(buffer[5]),
                           bcd2bin(buffer[4]), bcd2bin(buffer[2]), bcd2bin(buffer[1]),
                           bcd2bin(buffer[0] & 0x7F));
    }

    [[nodiscard]] bool is_set()
    {
        return !(read_register(0u) >> 7);
    }

    void set_date(const RTCDateTime &dt)
    {
        uint8_t buffer[8] = {0u,
                             bin2bcd(dt.ss),
                             bin2bcd(dt.mm),
                             bin2bcd(dt.hh),
                             0u,
                             bin2bcd(dt.d),
                             bin2bcd(dt.m),
                             bin2bcd(dt.year_offset)};

        device->write(buffer, 8u);
    }

private:
    [[nodiscard]] uint8_t read_register(const uint8_t reg)
    {
        uint8_t buffer[1];
        device->write(&reg, 1);
        device->read(buffer, 1);

        return buffer[0];
    }

    [[nodiscard]] inline uint8_t bin2bcd(const uint8_t val) { return val + 6 * (val / 10); }
    [[nodiscard]] inline uint8_t bcd2bin(const uint8_t val) { return val - 6 * (val >> 4); }

    Adafruit_I2CDevice *device = nullptr;
};