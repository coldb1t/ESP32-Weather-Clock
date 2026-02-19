#pragma once

class DHTModule
{
public:
    explicit DHTModule(const uint8_t pin, const int8_t type)
        : pin(pin), type(type), dht(pin, type)
    {
    }

    void setup()
    {
        dht.begin();

        temperature.init();
        humidity.init();
    }

    inline void update()
    {
        if (const auto v = dht.readTemperature(); !isnan(v))
        {
            temperature.set(v);
        }

        if (const auto v = dht.readHumidity(); !isnan(v))
        {
            humidity.set(v);
        }
    }

    core::locked<float> temperature{0.f};
    core::locked<float> humidity{0.f};

private:
    uint8_t pin = 0u;
    uint8_t type = 0u;
    DHT dht;
};