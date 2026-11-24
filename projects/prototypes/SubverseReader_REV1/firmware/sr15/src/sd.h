/* sd.h: the microSD card, mounted at /sd. Apache-2.0 */
#ifndef SR15_SD_H
#define SR15_SD_H
#include <stdbool.h>
#include "esp_err.h"
esp_err_t sd_mount(void);
void sd_unmount(void);   /* and power the card off */
bool sd_ok(void);
#endif
