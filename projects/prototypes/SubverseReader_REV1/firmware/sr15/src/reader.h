/* reader.h: the SR-15 reading loop (reader.c). Apache-2.0 */
#ifndef SR15_READER_H
#define SR15_READER_H
#include <stdbool.h>
void reader_start(void);
void reader_activity(void);      /* console input counts as use: keeps the pass awake */
void reader_sleep_now(void);
int  reader_sleep_s(void);
int  reader_full_every(void);
/* console words: take the lock around any of these */
void reader_lock(void);
void reader_unlock(void);
void reader_cmd_shelf(void);
void reader_cmd_open(int i);
void reader_cmd_page(int n);     /* 1-based */
void reader_cmd_turn(int d);
void reader_cmd_test(void);
void reader_cmd_badge(void);
void reader_cmd_redraw(bool full);
void reader_set_sleep(int s);    /* idle seconds before deep sleep; 0 = never */
void reader_set_full_every(int n);
bool pack_path(int i, char *out, int len);   /* path of pack i from the last scan */
void reader_info(void (*out)(const char *fmt, ...));
#endif
