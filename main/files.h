/**
 * @file files.h
 * @brief File manager sub-app: browse STORAGE_MOUNT_PATH.
 */

#pragma once

#include "input.h"
#include "lvgl.h"

/** Build the Files screen (call from ui_init under lvgl_port_lock). */
lv_obj_t *files_screen_create(void);

/** Refresh directory listing (call when navigating to Files). */
void files_on_show(void);

/** Handle encoder/button events while Files is active. */
void files_on_nav(nav_event_t event);
