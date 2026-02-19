#pragma once

#include "structs/dht.h"
#include "structs/display.h"
#include "structs/button.h"
#include "structs/buzzer.h"
#include "structs/rtc.h"

inline DisplayModule *display_module = nullptr;

inline auto dht_module = DHTModule(15u, DHT22);
inline auto buzzer_module = BuzzerModule(25u, 0u);
inline auto rtc_module = RTCModule();

inline auto button_up = ButtonModule(19u);
inline auto button_click = ButtonModule(23u);
inline auto button_down = ButtonModule(18u);