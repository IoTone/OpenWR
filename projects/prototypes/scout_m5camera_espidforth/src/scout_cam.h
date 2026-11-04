/* scout_cam.h — Scout's OV2640, dithered frames and flash roll (scout_cam.c) */
#ifndef SCOUT_CAM_H
#define SCOUT_CAM_H
#include <stdbool.h>
void scout_cam_init(void);
void scout_register_words(void);
bool scout_cam_up(void);
int  scout_roll_count(void);
#endif
