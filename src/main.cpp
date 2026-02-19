#include "include.h"
/*
  RAM:   [=         ]   6.8% (used 22144 bytes from 327680 bytes)
  Flash: [==        ]  24.0% (used 314225 bytes from 1310720 bytes)

  RAM:   [=         ]   6.7% (used 22096 bytes from 327680 bytes)
  Flash: [==        ]  24.0% (used 315013 bytes from 1310720 bytes)
*/

#if defined DEBUG
auto heap_size = 0u;
#endif

enum RenderTask : uint8_t
{
  Clock = 0u,
  Alarms,
  AlarmsAdd,
  AlarmEdit,
  __Max
};
core::locked<RenderTask> render_task{RenderTask::Clock};
core::locked<int8_t> alarms_menu_id{-1};

void _render_task(void *);
void sensor_task(void *);
void button_task(void *);

//
//  Main
//
void setup()
{
#if defined DEBUG
  const auto start_time = esp_log_timestamp();
#endif
  Serial.begin(115200ul); // ESP32 default
  PRINTLN("Setup");

  if (!Wire.setPins(21, 22))
  {
    Serial.println("Wire set pins error");
    abort();
  }

  if (!Wire.begin(21, 22))
  {
    Serial.println("Wire begin error");
    abort();
  }

  if (!rtc_module.setup())
  {
    Serial.println("RTC not found");
    abort();
  }

  if (!rtc_module.is_set())
  {
    rtc_module.set_date(RTCDateTime()); // CT date
    PRINTLN("RTC: update time");
  }

  render_task.init();
  alarms_menu_id.init();

  button_click.setup();
  button_up.setup();
  button_down.setup();

  buzzer_module.setup();
  dht_module.setup();

  display_module = new DisplayModule({128, 64}, -1);
  if (!display_module->begin())
  {
    Serial.println("SSD1306 allocation failed");
    abort();
  }

  display_module->clear();

#if defined DEBUG
  heap_size = ESP.getHeapSize();
  char buffer[64] = {};
  const auto time_load = esp_log_timestamp();
  sprintf(buffer, "Init time: %u ms\n\nHEAP: %u KB", time_load - start_time, heap_size / 1024u);
  display_module->draw_text({0, 0}, buffer, 1U);
  display_module->end();
#endif
  delay(2000u);

  xTaskCreate(sensor_task, "SENSOR_TASK", 2048u, nullptr, 1u, nullptr);
  xTaskCreate(_render_task, "RENDER_TASK", 8192u, nullptr, 1u, nullptr);
  xTaskCreate(button_task, "BUTTON_TASK", 2048u, nullptr, 2u, nullptr);

  Serial.println("Init complete");
}

void loop()
{
  vTaskDelay(pdMS_TO_TICKS(1000));
}

//
//  Tasks
//

// Sensor
void sensor_task(void *)
{
  while (1)
  {
#ifdef DEBUG
    const auto now_ms = esp_log_timestamp();
#endif
    dht_module.update();

    PRINTF("dht update: %u ms\r\n", esp_log_timestamp() - now_ms);

    vTaskDelay(pdMS_TO_TICKS(5000u));
  }
}

void render_clock()
{
#ifdef DEBUG
  const auto heap_pcent = (1.f - static_cast<float>(ESP.getFreeHeap()) / static_cast<float>(heap_size)) * 100.f;

  char str_heap[32] = {};
  sprintf(str_heap, "H: %.0f%%", heap_pcent);

  const auto sz_heap_sz_str = display_module->get_text_size(str_heap, 1U);
#endif
  const auto tm = rtc_module.now();
  const auto &seconds = tm.ss;
  static constexpr const char *days[] = {
      "Mon", "Tue", "Wed", "Thu", "Fri", "SAT", "SUN"};

  char str_temp[16] = {};
  char str_hum[16] = {};
  char str_time[16] = {};
  char str_date[16] = {};

  sprintf(str_temp, "%.1fC", dht_module.temperature.fast_get());
  sprintf(str_hum, "%.0f%%", dht_module.humidity.fast_get());
  sprintf(str_time, "%02d%s%02d", tm.hh, seconds % 2 == 0 ? ":" : " ", tm.mm);
  sprintf(str_date, "%d.%02d.%04d", tm.d, tm.m, tm.year());

  const auto sz_temp = display_module->get_text_size(str_temp, 1u);
  const auto sz_hum = display_module->get_text_size(str_hum, 1u);
  const auto sz_time = display_module->get_text_size(str_time, 2u);

  display_module->draw_text(Vec2(0, 0), str_date, 1u, true);
  if (tm.dd > 0u)
  {
    display_module->draw_text({}, days[tm.dd]);
  }
  display_module->draw_text(Vec2(display_module->size.x, 0) - Vec2(sz_temp.x, 0), str_temp);
  display_module->draw_text(Vec2(display_module->size.x, sz_temp.y + 2) - Vec2(sz_hum.x, 0), str_hum);

#ifdef DEBUG
  display_module->draw_text(Vec2(display_module->size.x - sz_heap_sz_str.x,
                                 display_module->size.y / 2 - sz_heap_sz_str.y / 2),
                            str_heap, 1U, false);
#endif

  const auto pos_time = display_module->size / 2u - sz_time / 2u;
  display_module->draw_text(pos_time, str_time, 2u);

  const auto pos_seconds = pos_time + Vec2(0, sz_time.y + 2);
  display_module->draw_rect(pos_seconds, {static_cast<int16_t>((sz_time.x * seconds) / 60), 2});

  static constexpr const char *str_alarms = "\\/Alarms\\/";
  static const auto sz_alarms = display_module->get_text_size(str_alarms, 1u);
  static const auto pos_alarms = Vec2(display_module->size.x / 2 - sz_alarms.x / 2, display_module->size.y - sz_alarms.y);
  display_module->draw_text(pos_alarms, str_alarms, 1u, false);
}

void render_alarms()
{
  const auto idx = alarms_menu_id.get();

  static constexpr const char *str_clock = "/\\Clock/\\";
  static const auto sz_clock = display_module->get_text_size(str_clock, 1u);
  static const auto pos_clock = Vec2(display_module->size.x / 2 - sz_clock.x / 2, 0u);
  display_module->draw_text(pos_clock, str_clock, 1u, false);

  const auto text_add = idx == 0 ? "Add < " : "Add";
  const auto pos_add = Vec2(10, sz_clock.y);
  display_module->draw_text(pos_add, text_add);
}

// Render
void _render_task(void *)
{
  while (1)
  {
    display_module->clear();

    switch (render_task.get())
    {
    case RenderTask::Clock:
      render_clock();
      break;

    case RenderTask::Alarms:
      render_alarms();
      break;

    default:
      break;
    }

    display_module->end();
    vTaskDelay(pdMS_TO_TICKS(60u));
  }
}

// Button
void button_task(void *)
{
  while (1)
  {
    button_click.update();
    button_up.update();
    button_down.update();

    const auto current_render_task = render_task.get();
    switch (current_render_task)
    {
    case RenderTask::Alarms:
    {
      const auto alarm_ids = alarms_menu_id.get();
      /*if (button_click.is_clicked())
      {
        if (alarm_ids == 0)
        {
          render_task.set(RenderTask::AlarmsAdd);
          break;
        }
      }*/

      if (button_down.is_clicked())
      {
        if (alarm_ids < 0)
        {
          alarms_menu_id.set(alarm_ids + 1);
        }
      }
      else if (button_up.is_clicked())
      {
        if (alarm_ids <= -1)
        {
          render_task.set(RenderTask::Clock);
        }
        else
        {
          alarms_menu_id.set(alarm_ids - 1);
        }
      }
      break;
    }

    case RenderTask::Clock:
    {
      if (button_down.is_clicked())
      {
        render_task.set(RenderTask::Alarms);
      }
      break;
    }

    default:
      break;
    }

    vTaskDelay(pdMS_TO_TICKS(50u));
  }
}
