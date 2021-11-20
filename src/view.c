#include <player.h>
#include <tizen.h>
#include <dlog.h>
#include <app.h>
#include <efl_extension.h>
#include <Elementary.h>
#include <stdio.h>
#include <string.h>
#include "soundboard.h"
#include "view.h"

static struct view_info {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *layout;

	Evas_Object *entry;
	Ecore_Timer *timer;

	int mouse_down_dial_num;
	player_h player;
} s_info = {
	.win = NULL,
	.conform = NULL,
	.layout = NULL,

	.entry = NULL,
	.timer = NULL,
	.mouse_down_dial_num = -1,
	.player = NULL
};

#define TRACK0 "Applause.mp3"
#define TRACK1 "Laugh.mp3"
#define TRACK2 ""
#define TRACK3 ""
#define TRACK4 ""
#define TRACK5 ""
#define TRACK6 ""
#define TRACK7 ""
#define TRACK8 ""
#define TRACK9 ""

static void _win_delete_request_cb(void *data, Evas_Object *obj, void *event_info);
static void _rectangle_mouse_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _rectangle_mouse_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _rectangle_mouse_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _soundboard_layout_cb(void *data, Evas_Object *obj, void *event_info);

/*
 * @brief Create Essential Object window, conformant and layout
 */
void view_create(void)
{

	/* Create player */
    int error_code = 0;
    error_code = player_create(&s_info.player);
    if (error_code != PLAYER_ERROR_NONE)
        dlog_print(DLOG_ERROR, LOG_TAG, "failed to create");

	/* Create window */
	s_info.win = view_create_win(PACKAGE);
	if (s_info.win == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to create a window.");
		return;
	}

	/* Create conformant */
	s_info.conform = view_create_conformant_without_indicator(s_info.win);
	if (s_info.conform == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to create a conformant");
		return;
	}

	/* Show window after main view is set up */
	evas_object_show(s_info.win);
}

/*
 * @brief: Get absolute path to the track specified by the index
 * @param[index]: Index of the track
 * @param[in] result_path Entry object to be modified by this function
 */
static void _get_track_path_by_index(uint32_t index, char *result_path) {
    char *res_path = app_get_shared_resource_path();
    char *file_in;

    switch(index) {
    case 0: file_in = TRACK0; break;
    case 1: file_in = TRACK1; break;
    case 2: file_in = TRACK2; break;
    case 3: file_in = TRACK3; break;
    case 4: file_in = TRACK4; break;
    case 5: file_in = TRACK5; break;
    case 6: file_in = TRACK6; break;
    case 7: file_in = TRACK7; break;
    case 8: file_in = TRACK8; break;
    case 9: file_in = TRACK9; break;
    default: file_in = TRACK0;
    }

    if (res_path) {
        snprintf(result_path, 128, "%s%s", res_path, file_in);
        free(res_path);
    }
}

/*
 * @brief: Play a music that is displayed at screen
 * @param[file_path]: File path of music
 * @param[user_data]: Data to pass to function when it is called
 */
static void _play_current_music(uint32_t id)
{
	player_state_e player_state;

	if (player_get_state(s_info.player, &player_state) != PLAYER_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to get player state");
		return;
	}

	if (player_state != PLAYER_STATE_IDLE) {
		if (player_unprepare(s_info.player) != PLAYER_ERROR_NONE) {
			dlog_print(DLOG_ERROR, LOG_TAG, "unprepare error");
			return;
		}
	}

	char icon_path[128] = {0,};
	_get_track_path_by_index(id, icon_path);

	if (player_set_uri(s_info.player, icon_path) != PLAYER_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "set uri error");
		return;
	}

	if (player_prepare(s_info.player) != PLAYER_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "prepare error");
		return;
	}

	if (player_start(s_info.player) != PLAYER_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "start error");
		return;
	}
}

/*
 * @brief Make a basic window named package
 * @param[in] pkg_name Name of the window
 */
Evas_Object *view_create_win(const char *pkg_name)
{
	Evas_Object *win = NULL;

	/*
	 * Window
	 * Create and initialize elm_win.
	 * elm_win is mandatory to manipulate window
	 */
	win = elm_win_util_standard_add(pkg_name, pkg_name);
	elm_win_conformant_set(win, EINA_TRUE);
	elm_win_autodel_set(win, EINA_TRUE);

	/* Rotation setting */
	if (elm_win_wm_rotation_supported_get(win)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(win, (const int *)(&rots), 4);
	}

	evas_object_smart_callback_add(win, "delete,request", _win_delete_request_cb, NULL);

	return win;
}

/*
 * @brief Make a conformant without indicator for wearable app
 * @param[in] win The object to which you want to set this conformant
 * Conformant is mandatory for base GUI to have proper size
 */
Evas_Object *view_create_conformant_without_indicator(Evas_Object *win)
{
	/*
	 * Conformant
	 * Create and initialize elm_conformant.
	 * elm_conformant is mandatory for base GUI to have proper size
	 * when indicator or virtual keypad is visible.
	 */
	Evas_Object *conform = NULL;

	if (win == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "window is NULL.");
		return NULL;
	}

	conform = elm_conformant_add(win);
	evas_object_size_hint_weight_set(conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(win, conform);

	evas_object_show(conform);

	return conform;
}

/*
 * @brief Make essential object for the this app, like conformant and layout
 * @param[in] file_path File path of EDJ file will be used
 */
void view_soundboard_create(const char *file_path)
{
	s_info.layout = view_create_layout_for_conformant(s_info.conform, file_path, GRP_MAIN, _soundboard_layout_cb, NULL);
	if (s_info.layout == NULL) {
		evas_object_del(s_info.win);
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to create a content.");
		return;
	}
}

/*
 * @brief Function will be operated when window is deleted
 * @param[in] data The data to be passed to the callback function
 * @param[in] obj The Evas object handle to be passed to the callback function
 * @param[in] event_info The system event information
 */
static void _win_delete_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	ui_app_exit();
}

/*
 * @brief Make a layout to target parent object with edje file
 * @param[in] parent The object to which you want to add this layout
 * @param[in] file_path File path of EDJ file will be used
 * @param[in] group_name Name of group in EDJ you want to set to
 * @param[in] cb_function The function will be called when back event is detected
 * @param[in] user_data The user data to be passed to the callback functions
 */
Evas_Object *view_create_layout(Evas_Object *parent, const char *file_path, const char *group_name, Eext_Event_Cb cb_function, void *user_data)
{
	Evas_Object *layout = NULL;

	if (parent == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "parent is NULL.");
		return NULL;
	}

	/* Create layout by EDC(edje file) */
	layout = elm_layout_add(parent);
	elm_layout_file_set(layout, file_path, group_name);

	/* Layout size setting */
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	if (cb_function)
		eext_object_event_callback_add(layout, EEXT_CALLBACK_BACK, cb_function, user_data);

	evas_object_show(layout);

	return layout;
}

/*
 * @brief Make and set a layout to conformant
 * @param[in] parent Target conformant object
 * @param[in] file_path File path of EDJ will be used
 * @param[in] group_name Group name in EDJ you want to set to layout
 * @param[in] cb_function The function will be called when the back event is detected
 * @param[in] user_data The user data to be passed to the callback functions
 */
Evas_Object *view_create_layout_for_conformant(Evas_Object *parent, const char *file_path, const char *group_name, Eext_Event_Cb cb_function, void *user_data)
{
	Evas_Object *layout = NULL;

	if (parent == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "parent is NULL.");
		return NULL;
	}

	/* Create layout for conformant */
	if (file_path == NULL) {
		layout = view_create_layout_by_theme(parent, "layout", "application", "default");
	} else {
		layout = view_create_layout(parent, file_path, group_name, cb_function, user_data);
	}

	if (layout == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "layout is NULL.");
		return NULL;
	}

	elm_object_content_set(parent, layout);

	return layout;
}

/*
 * @brief Make a layout with theme.
 * @param[in] parent Object to which you want to add this layout
 * @param[in] class_name The class of the group
 * @param[in] group_name Group name in EDJ that you want to set to layout
 * @param[in] style The style to use
 */
Evas_Object *view_create_layout_by_theme(Evas_Object *parent, const char *class_name, const char *group_name, const char *style)
{
	/*
	 * Layout
	 * Create and initialize elm_layout.
	 * view_create_layout_by_theme() is used to create layout by using premade edje file.
	 */
	Evas_Object *layout = NULL;

	if (parent == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "parent is NULL.");
		return NULL;
	}

	layout = elm_layout_add(parent);
	elm_layout_theme_set(layout, class_name, group_name, style);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	evas_object_show(layout);

	return layout;
}

/*
 * @brief Destroy window and free important data to finish this application
 */
void view_destroy(void)
{
	if (s_info.win == NULL) {
		return;
	}

	evas_object_del(s_info.win);

/*
	 * Destroys the media player handle and releases all its resources
	 */
	if (player_destroy(s_info.player) != PLAYER_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "Can not destroy player.");
	}
}

/*
 * @brief Set image to given part
 * @param[in] parent Object has part to which you want to set this image
 * @param[in] part_name Part name to which you want to set this image
 * @param[in] image_path Path of the image file
 */
void view_set_image(Evas_Object *parent, const char *part_name, const char *image_path)
{
	Evas_Object *image = NULL;

	if (parent == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "parent is NULL.");
		return;
	}

	image = elm_object_part_content_get(parent, part_name);
	if (image == NULL) {
		image = elm_image_add(parent);
		if (image == NULL) {
			dlog_print(DLOG_ERROR, LOG_TAG, "failed to create an image object.");
			return;
		}

		elm_object_part_content_set(parent, part_name, image);
	}

	if (EINA_FALSE == elm_image_file_set(image, image_path, NULL)) {
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to set image.");
		return;
	}

	evas_object_show(image);

	return;
}

/*
 * @brief Set text to the part
 * @param[in] parent Object has part to which you want to set text
 * @param[in] part_name Part name to which you want to set the text
 * @param[in] text Text you want to set to the part
 */
void view_set_text(Evas_Object *parent, const char *part_name, const char *text)
{
	if (parent == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "parent is NULL.");
		return;
	}

	/* Set text of target part object */
	elm_object_part_text_set(parent, part_name, text);
}

/*
 * @brief Set color of the part
 * @param[in] parent Object has part to which you want to set color
 * @param[in] part_name Name of part to which you want to set color
 * @param[in] r R of RGBA you want to set to the part
 * @param[in] g G of RGBA you want to set to the part
 * @param[in] b B of RGBA you want to set to the part
 * @param[in] a A of RGBA you want to set to the part
 */
void view_set_color(Evas_Object *parent, const char *part_name, int r, int g, int b, int a)
{
	Evas_Object *obj = NULL;

	if (parent == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "parent is NULL.");
		return;
	}

	obj = elm_object_part_content_get(parent, part_name);
	if (obj == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to get parent.");
		return;
	}

	/* Set color of target part object */
	evas_object_color_set(obj, r, g, b, a);
}

/*
 * @brief Make and set button.
 * @param[in] parent Object to which you want to set the button
 * @param[in] part_name Name of part to which you want to set the button
 * @param[in] style Style of the button
 * @param[in] image_path Path of image file will be used as button icon
 * @param[in] text The text will be written on the button
 * @param[in] down_cb Function will be operated when the button is pressed
 * @param[in] up_cb Function will be operated when the button is released
 * @param[in] clicked_cb Function will be operated when the button is clicked
 * @param[in] user_data Data passed to the 'clicked_cb' function
 */
void view_set_button(Evas_Object *parent, const char *part_name, const char *style, const char *image_path, const char *text,
		Evas_Object_Event_Cb down_cb, Evas_Object_Event_Cb up_cb, Evas_Smart_Cb clicked_cb, void *user_data)
{
	Evas_Object *btn = NULL;

	if (parent == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "parent is NULL.");
		return;
	}

	btn = elm_button_add(parent);
	if (btn == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to create button.");
		return;
	}

	if (style)
		elm_object_style_set(btn, style);

	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_content_set(parent, part_name, btn);

	if (text)
		elm_object_text_set(btn, text);

	if (image_path)
		view_set_image(btn, NULL, image_path);

	if (down_cb)
		evas_object_event_callback_add(btn , EVAS_CALLBACK_MOUSE_DOWN, down_cb, user_data);
	if (up_cb)
		evas_object_event_callback_add(btn, EVAS_CALLBACK_MOUSE_UP, up_cb, user_data);
	if (clicked_cb)
		evas_object_smart_callback_add(btn, "clicked", clicked_cb, user_data);

	evas_object_show(btn);
}

/*
 * @brief Make Rectangle Object to target window for hijacking touch event
 * @Add callback function will be operated when mouse down/up event is triggered
 */
Evas_Object *view_soundboard_create_rectangle()
{
	Evas_Object *rect = NULL;

	if (s_info.win == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "window is NULL.");
		return NULL;
	}

	if (s_info.layout == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "parent layout is NULL.");
		return NULL;
	}

	/* Add Rectangle object to parent */
	rect = evas_object_rectangle_add(evas_object_evas_get(s_info.layout));
	if (rect == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to add a rectangle");
		return NULL;
	}

	/* Set Rectangle color to transparent because this rectangle is just in charge of touch event hooking
	 * You can change color for other purpose e.g. color filter layer */
	evas_object_color_set(rect, 255, 255, 255, 0);

	/* Set event repeat mode */
	evas_object_repeat_events_set(rect, EINA_TRUE);

	/* Set size of Rectangle object */
	evas_object_size_hint_weight_set(rect, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(s_info.win, rect);

	evas_object_show(rect);

	/* Set callback for event about Rectangle */
	evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_DOWN, _rectangle_mouse_down_cb, NULL);
	evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_UP, _rectangle_mouse_up_cb, NULL);
	evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_MOVE, _rectangle_mouse_move_cb, NULL);

	return rect;
}

/*
 * @brief Create entry object and keep the object for handling.
 * @param[in] part_name Part of the layout which you want to locate Entry
 */
void view_soundboard_set_entry(const char *part_name)
{

}

/*
 * @brief Make a Entry Object to target window
 * @param[in] parent Object to which you want to set Entry
 * @param[in] part_name Part of the layout which you want to locate Entry
 * @param[in] clicked_cb The function will be called when the entry is clicked
 * @param[in] user_data The user data passed to the callback function
 * @Add callback function will be operated when mouse clicked event is triggered
 */
Evas_Object *view_create_entry(Evas_Object *parent, const char *part_name, Evas_Smart_Cb clicked_cb, void *user_data)
{
	Evas_Object *entry = NULL;

	if (parent == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "parent is NULL.");
		return NULL;
	}

	entry = elm_entry_add(parent);
	if (entry == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to add a entry");
		return NULL;
	}

	evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_input_panel_enabled_set(entry, EINA_FALSE);
	elm_entry_editable_set(entry, EINA_FALSE);
	elm_entry_context_menu_disabled_set(entry, EINA_TRUE);

	elm_entry_text_style_user_push(entry, DIAL_TEXT_STYLE_NORMAL);

	elm_object_part_content_set(parent, part_name, entry);

	if (clicked_cb) {
		evas_object_smart_callback_add(entry, "clicked", clicked_cb, user_data);
	}

	return entry;
}

/*
 * @brief Modify entry text of Entry object
 * @param[in] operation ENTRY_TEXT_CLEAR_ALL - clear all text, ENTRY_TEXT_ADD_TEXT - add specific text to current text, ENTRY_TEXT_BACKSPACE - delete one character by backspace
 * @param[in] text target text which will be added to current entry text
 */
int view_soundboard_set_entry_text(int operation, const char *text)
{
	return 1;
}

/*
 * @brief Get dial number of user's touch event position by calculating distance and slope from center
 * @param[in] evt_x X-coordinate value of event position
 * @param[in] evt_y Y-coordinate value of event position
 */
static int _get_btn_dial_number(int evt_x, int evt_y)
{
	int result = -1;

	/* Calculation x and y from CENTER_REF point */
	int x = evt_x - CENTER_REF_X;
	int y = CENTER_REF_Y - evt_y;

	/* Calculation the slope and radius from CENTER_REF point */
	float slope = (float)y / (float)x;
	float radius = sqrt(x*x + y*y);

	if (radius > RADIUS_MIN) {
		if (x == 0) {
			if (y >= 0)
				result = 0;
			else
				result = 5;
		} else if (y == 0) {
			if (x >= 0)
				result = 2;
			else
				result = 8;
		} else {
			if (slope > SLOPE_72_DEGREE) {
				if (y > 0)
					result = 0;
				else
					result = 5;
			} else if (slope > SLOPE_36_DEGREE) {
				if (y > 0)
					result = 1;
				else
					result = 6;
			} else if (slope > SLOPE_180_DEGREE) {
				if (y > 0)
					result = 2;
				else
					result = 7;
			} else if (slope > SLOPE_144_DEGREE) {
				if (y > 0)
					result = 8;
				else
					result = 3;
			} else if (slope > SLOPE_108_DEGREE) {
				if (y > 0)
					result = 9;
				else
					result = 4;
			} else {
				if (y > 0)
					result = 0;
				else
					result = 5;
			}
		}
	} else {
		dlog_print(DLOG_DEBUG, LOG_TAG, "Ignore touch event under min radius");
	}

	return result;
}

/*
 * @brief Function will be operated when mouse move event is triggered
 * @param[in] data The data to be passed to the callback function
 * @param[in] e The handle to an Evas canvas to be passed to the callback function
 * @param[in] obj The Evas object handle to be passed to the callback function
 * @param[in] event_info The system event information
 */
static void _rectangle_mouse_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{

}

/*
 * @brief Function will be operated when mouse down event is triggered
 * @param[in] data The data to be passed to the callback function
 * @param[in] e The handle to an Evas canvas to be passed to the callback function
 * @param[in] obj The Evas object handle to be passed to the callback function
 * @param[in] event_info The system event information
 */
static void _rectangle_mouse_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Down *ev = (Evas_Event_Mouse_Down*) event_info;
	s_info.mouse_down_dial_num = _get_btn_dial_number(ev->output.x, ev->output.y);

	if (s_info.mouse_down_dial_num != -1) {
		char signal[9] = {0, };

		/* Trigger touch animation */
		snprintf(signal, sizeof(signal), "%s%d", "button.", s_info.mouse_down_dial_num);
		elm_layout_signal_emit(s_info.layout, "button.dial.touch", signal);
	}

	/* Play sound effect */
	if (s_info.mouse_down_dial_num != -1) {
		_play_current_music(s_info.mouse_down_dial_num);
	}
}

/*
 * @brief Function will be operated when mouse up event is triggered
 * @param[in] data The data to be passed to the callback function
 * @param[in] e The handle to an Evas canvas to be passed to the callback function
 * @param[in] obj The Evas object handle to be passed to the callback function
 * @param[in] event_info The system event information
 */
static void _rectangle_mouse_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{

}

/*
 * @brief Layout back key event callback function
 * @param[in] data The data to be passed to the callback function
 * @param[in] obj The Evas object handle to be passed to the callback function
 * @param[in] event_info The system event information
 */
static void _soundboard_layout_cb(void *data, Evas_Object *obj, void *event_info)
{

}

/*
 * @brief This function just return static layout object.
 */
Evas_Object *view_soundboard_get_layout_object(void)
{
	return s_info.layout;
}
