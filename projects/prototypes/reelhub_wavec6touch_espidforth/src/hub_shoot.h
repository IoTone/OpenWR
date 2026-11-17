/*
 * hub_shoot.h — what the touches do, and firing every camera at once.
 *
 *   swipe left / right   newer / older shot
 *   double-tap picture   show (or hide) the round shutter button; a tap on
 *                        it shoots; it hides itself after 6 s
 *   hold 3 s             shoot (a bar in the strip fills while you hold)
 *
 * Shooting flashes the screen white (this board has no speaker or LED),
 * prints `fire <n>` on every console (the glasses take theirs), then fetches
 * each camera's picture into the reel. Cameras so far: Scout over HTTP.
 *
 *   shoot        ( -- )          fire all
 *   cams?        ( -- )          cam <name> <how> <state>
 *   scout-url!   ( addr u -- )   Scout's base URL, e.g. http://10.0.0.216 (saved)
 *
 * Licensed under the Apache License, Version 2.0
 */
#ifndef HUB_SHOOT_H
#define HUB_SHOOT_H
void shoot_init(void);               /* after lcd, touch, reel */
void shoot_fire(const char *why);
void shoot_register_words(void);
#endif
