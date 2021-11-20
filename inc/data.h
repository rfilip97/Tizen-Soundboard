#if !defined(_DATA_H)
#define _DATA_H

/*
 * Initialize the data component
 */
void data_initialize(void);

/*
 * Finalize the data component
 */
void data_finalize(void);

void data_get_full_path(const char *file_path, char *full_path, int path_max);
char *data_get_image_path(const char *part_name);
#endif

