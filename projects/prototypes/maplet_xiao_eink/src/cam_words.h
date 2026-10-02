#ifndef CAM_WORDS_H
#define CAM_WORDS_H
void cam_init_all(void);        /* camera + SD card; quiet no-ops when absent or not an S3 */
void cam_register_words(void);
#endif
