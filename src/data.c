#include <tizen.h>
#include <app.h>
#include <efl_extension.h>
#include <dlog.h>
#include "soundboard.h"
#include "data.h"

/*
 * @brief Initialization function for data module
 */
void data_initialize(void)
{
	/*
	 * If you need to initialize managing data,
	 * please use this function.
	 */
}

/*
 * @brief Finalization function for data module
 */
void data_finalize(void)
{
	/*
	 * If you need to finalize managing data,
	 * please use this function.
	 */
}

/*
 * @brief Get full path of resource
 * @param[in] file_path File path of target file
 * @param[out] full_path Full file path concatenated with resource path
 * @param[in] path_max Max length of full file path
 */
void data_get_full_path(const char *file_path, char *full_path, int path_max)
{
	char *res_path = app_get_resource_path();
	if (res_path) {
		snprintf(full_path, path_max, "%s%s", res_path, file_path);
		free(res_path);
	}
}

/*
 * @brief Get path of image file for part
 * @param[in] part_name Part name of the target image path
 */
char *data_get_image_path(const char *part_name)
{
	/*
	 * You can use this function to retrieve data.
	 */
	char *ret = NULL;
	char full_path[PATH_MAX] = { 0, };
	char *res_path = app_get_resource_path();

	if (res_path) {
		if (!strcmp("sw.button.bg", part_name))
			snprintf(full_path, sizeof(full_path) - 1, "%s%s", res_path, "images/wheel.png");
		else {
			dlog_print(DLOG_ERROR, LOG_TAG, "failed to get image.");
			free(res_path);
			return NULL;
		}

		ret = strdup(full_path);
		free(res_path);
	}

	return ret;
}
