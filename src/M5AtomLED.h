#ifndef _M5_ATOM_LED_H_
#define _M5_ATOM_LED_H_

#if __has_include(<FastLED.h>)

#include <FastLED.h>

template <uint8_t DATA_PIN, size_t LED_W, size_t LED_H> class M5AtomLED {
private:
  CRGB _ledbuff[LED_W * LED_H];
  SemaphoreHandle_t _xSemaphore = nullptr;
  bool _updated;

  uint8_t _brightness;

public:
  M5AtomLED() {
    FastLED.addLeds<WS2812, DATA_PIN, GRB>(_ledbuff, LED_W * LED_H);
    _xSemaphore = xSemaphoreCreateMutex();

    setBrightness(20);
    memset(_ledbuff, 0, sizeof(_ledbuff));
  };
  ~M5AtomLED(){};

  void begin(void) {
    FastLED.addLeds<WS2812, DATA_PIN, GRB>(_ledbuff, LED_W * LED_H);
    _xSemaphore = xSemaphoreCreateMutex();

    setBrightness(20);
    clear();
  }

  void update(void) {
    if (_updated) {
      xSemaphoreTake(_xSemaphore, portMAX_DELAY);
      FastLED.show();
      _updated = false;
      xSemaphoreGive(_xSemaphore);
    }
  }

  void setBrightness(uint8_t brightness) {
    xSemaphoreTake(_xSemaphore, portMAX_DELAY);
    brightness = (brightness > 100) ? 100 : brightness;
    brightness = (40 * brightness / 100);
    _brightness = brightness;
    FastLED.setBrightness(_brightness);

    _updated = true;
    xSemaphoreGive(_xSemaphore);
  };

  void drawpix(uint8_t xpos, uint8_t ypos, CRGB Color) {
    if ((xpos < LED_W) && (ypos < LED_H)) {
      drawpix(xpos + ypos * LED_W, Color);
    }
  };

  void drawpix(uint8_t Number, CRGB Color) {
    if (Number < LED_W * LED_H) {
      xSemaphoreTake(_xSemaphore, portMAX_DELAY);
      _ledbuff[Number] = Color;
      _updated = true;
      xSemaphoreGive(_xSemaphore);
    }
  };

  void fillpix(CRGB Color) {
    xSemaphoreTake(_xSemaphore, portMAX_DELAY);
    for (uint_fast8_t i = 0; i < (LED_W * LED_H); i++) {
      _ledbuff[i] = Color;
    }
    _updated = true;
    xSemaphoreGive(_xSemaphore);
  }

  void clear() {
    xSemaphoreTake(_xSemaphore, portMAX_DELAY);
    memset(_ledbuff, 0, sizeof(_ledbuff));
    _updated = true;
    xSemaphoreGive(_xSemaphore);
  }
};

#endif /* __has_include(<FastLED.h>) */

#ifdef ARDUINO_M5Stack_ATOM
#if !__has_include(<FastLED.h>)
#error "FastLED.h missing."
#endif

#define M5_ATOM_NEOPIXEL_PIN 27
extern M5AtomLED<M5_ATOM_NEOPIXEL_PIN, 5, 5> M5_dis;
#endif

#endif /* _M5_ATOM_LED_H_ */
