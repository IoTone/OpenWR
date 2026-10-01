typedef struct { unsigned long long pin_bit_mask; int mode; int pull_up_en; } gpio_config_t;
#define GPIO_MODE_OUTPUT 1
static inline int gpio_config(const gpio_config_t*c){return 0;}
static inline int gpio_set_level(int p,int l){return 0;}
