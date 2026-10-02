/* pins.h — pick the XIAO variant from the IDF target. */
#ifndef PINS_H
#define PINS_H
#include "sdkconfig.h"
#if CONFIG_IDF_TARGET_ESP32S3
#include "pins_xiao_esp32s3.h"
#elif CONFIG_IDF_TARGET_ESP32C3
#include "pins_xiao_esp32c3.h"
#else
#error "maplet: no pin map for this target"
#endif
#endif
