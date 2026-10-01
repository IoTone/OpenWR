#include <stddef.h>
#include "esp_err.h"
typedef void* spi_device_handle_t;
#define SPI_DMA_CH_AUTO 0
typedef struct { int mosi_io_num, miso_io_num, sclk_io_num, quadwp_io_num, quadhd_io_num, max_transfer_sz; } spi_bus_config_t;
typedef struct { int clock_speed_hz, mode, spics_io_num, queue_size; } spi_device_interface_config_t;
typedef struct { size_t length; const void *tx_buffer; } spi_transaction_t;
static inline esp_err_t spi_bus_initialize(int h, const spi_bus_config_t *b, int d){return 0;}
static inline esp_err_t spi_bus_add_device(int h, const spi_device_interface_config_t *d, spi_device_handle_t *o){*o=(void*)1;return 0;}
static inline esp_err_t spi_device_polling_transmit(spi_device_handle_t h, spi_transaction_t *t){return 0;}
