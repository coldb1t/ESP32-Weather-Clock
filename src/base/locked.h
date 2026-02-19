#pragma once

namespace core
{
    template <typename T>
    class locked
    {
    public:
        explicit locked(const T &value, bool init = false)
            : mutex(init ? xSemaphoreCreateMutex() : nullptr),
              data(value),
              cache(value) {}

        ~locked()
        {
            if (mutex)
            {
                vSemaphoreDelete(mutex);
            }
        }

        locked(const locked &) = delete;
        locked &operator=(const locked &) = delete;
        locked(locked &&) = delete;
        locked &operator=(locked &&) = delete;

        void init()
        {
            if (!mutex)
            {
                mutex = xSemaphoreCreateMutex();
            }
        }

        [[nodiscard]] T get(TickType_t wait = portMAX_DELAY) const
        {
            T out = cache;
            if (!mutex)
            {
                return out;
            }

            if (xSemaphoreTake(mutex, wait) == pdTRUE)
            {
                out = data;
                cache = out;
                xSemaphoreGive(mutex);
            }
            return out;
        }

        [[nodiscard]] inline T fast_get() const
        {
            return get(0u);
        }

        void set(const T &value, TickType_t wait = portMAX_DELAY)
        {
            if (!mutex)
            {
                return;
            }

            if (xSemaphoreTake(mutex, wait) == pdTRUE)
            {
                data = value;
                cache = value;
                xSemaphoreGive(mutex);
            }
        }

    private:
        mutable SemaphoreHandle_t mutex = nullptr;
        T data{};
        mutable T cache{};
    };
}