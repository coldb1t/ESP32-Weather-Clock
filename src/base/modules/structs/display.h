#pragma once

class DisplayModule
{
public:
    DisplayModule() = delete;
    explicit DisplayModule(const Vec2 size,
                           const int8_t oled_reset = -1) : size(size),
                                                           oled_reset(oled_reset),
                                                           display(size.x, size.y, &Wire, oled_reset) {};

    [[nodiscard]] bool begin()
    {
        return display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    }

    inline void clear()
    {
        display.clearDisplay();
    }

    inline void end()
    {
        display.display();
    }

    void draw_rect(const Vec2 &pos, const Vec2 &size, Color color = Color::white)
    {
        display.drawRect(pos.x, pos.y, size.x, size.y, color);
    }

    void draw_text(const Vec2 &pos, const char *str, const uint8_t text_size = 1,
                   const bool ln = true, const Color color = Color::white)
    {
        display.setTextSize(text_size);
        display.setTextColor(color);
        if (pos.is_valid())
        {
            display.setCursor(pos.x, pos.y);
        }

        display.print(str);
        if (ln)
        {
            display.print('\n');
        }
    }

    inline void set_font_size(const uint8_t sz)
    {
        display.setTextSize(sz);
    }

    [[nodiscard]] Vec2 get_text_size(const char *str, const uint8_t font_size = 1u)
    {
        display.setTextSize(font_size);

        int16_t x = 0, y = 0;
        uint16_t size_x = 0u, size_y = 0u;

        display.getTextBounds(str, 0, 0, &x, &y, &size_x, &size_y);

        return {static_cast<int16_t>(size_x), static_cast<int16_t>(size_y)};
    }

    const Vec2 size = Vec2();

private:
    int8_t oled_reset = -1;
    Adafruit_SSD1306 display;
};