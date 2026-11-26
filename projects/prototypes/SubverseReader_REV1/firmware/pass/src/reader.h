/* reader.h: the reading loop (reader.c). Apache-2.0 */
#ifndef PASS_READER_H
#define PASS_READER_H
#include <stdbool.h>
void reader_keys_init(void);     /* first thing at boot */
void reader_start(void);
void reader_activity(void);      /* console input counts as use: keeps the pass awake */
void reader_sleep_now(void);
int  reader_sleep_s(void);
int  reader_full_every(void);
int  reader_sync_s(void);        /* seconds between timed Bluetooth check-ins; 0 = none */
int  reader_keys(void);          /* 5, 3 or 0 */
/* console words: take the lock around any of these */
void reader_lock(void);
void reader_unlock(void);
void reader_cmd_shelf(void);
void reader_cmd_open(int i);
void reader_cmd_page(int n);     /* 1-based */
void reader_cmd_where(void);          /* page number bar for a few seconds */
void reader_cmd_turn(int d);
void reader_cmd_test(void);
void reader_cmd_badge(void);
void reader_cmd_redraw(bool full);
void reader_set_sleep(int s);    /* idle seconds before deep sleep; 0 = never */
void reader_set_full_every(int n);
void reader_set_auto(int s);     /* auto-turn: seconds a page (5 or more), 0 = off */
void reader_set_sync(int s);     /* timed check-in: seconds (15 or more), 0 = off */
void reader_set_keys(int n);     /* SR-16: 3 keys fitted, or 0; takes effect at the next start */
bool pack_path(int i, char *out, int len);   /* path of pack i from the last scan */
void reader_info(void (*out)(const char *fmt, ...));
#endif
