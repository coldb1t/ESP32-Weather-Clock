#pragma once

class DisplayModule
{
public:
    explicit DisplayModule(const Vec2 size,
                           const uint32_t frequency = 0u,
                           const int8_t oled_reset = -1) : size(size),
                                                           oled_reset(oled_reset) {};

    [[nodiscard]] bool begin()
    {
        display = Adafruit_SSD1306(size.x, size.y, &Wire, oled_reset);
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

    void draw_rect(Vec2 pos, Vec2 size, Color color = Color::white)
    {
        display.drawRect(pos.x, pos.y, size.x, size.y, color);
    }

    void draw_text(Vec2 pos, const char *str, uint8_t text_size = 1,
                   bool ln = true, Color color = Color::white)
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

    inline void set_font_size(uint8_t sz)
    {
        display.setTextSize(sz);
    }

    [[nodiscard]] Vec2 get_text_size(const char *str, uint8_t font_size = 1u)
    {
        display.setTextSize(font_size);

        static Vec2 xy = Vec2(0, 0);
        uint16_t size_x = 0u, size_y = 0u;

        display.getTextBounds(str, 0, 0, &xy.x, &xy.y, &size_x, &size_y);

        return {static_cast<int16_t>(size_x), static_cast<int16_t>(size_y)};
    }

    Adafruit_SSD1306 display;
    const Vec2 size = Vec2();

private:
    int8_t oled_reset = -1;
};