#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

#include "graphics/render.h"
#include "math/vector2d.h"
#include "locked.h"
#include "modules/include.h"

#ifdef DEBUG
  #define PRINTF(...) Serial.printf(__VA_ARGS__)
  #define PRINTLN(...) Serial.println(__VA_ARGS__)
#else
  #define PRINTF(...)
  #define PRINTLN(...) 
#endif