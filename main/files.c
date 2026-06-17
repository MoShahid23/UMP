/**
 * @file files.c
 * @brief File manager: list SD card root via VFS.
 */

#include "files.h"

#include "board.h"
#include "storage.h"

#include <dirent.h>
#include <stdio.h>
#include <string.h>

#define FILES_LIST_BUF  2048
#define FILES_MAX_NAMES 48

static lv_obj_t *s_list_label;

static void files_refresh_list(void)
{
    static char listing[FILES_LIST_BUF];

    if (!storage_is_ready()) {
        lv_label_set_text(s_list_label, "SD card not available.");
        return;
    }

    DIR *dir = opendir(STORAGE_MOUNT_PATH);
    if (!dir) {
        lv_label_set_text(s_list_label, "Could not open SD card.");
        return;
    }

    listing[0] = '\0';
    size_t len = 0;
    int count = 0;
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL && count < FILES_MAX_NAMES) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        size_t name_len = strlen(entry->d_name);
        if (len + name_len + 2 >= sizeof(listing)) {
            break;
        }

        if (len > 0) {
            listing[len++] = '\n';
        }
        memcpy(listing + len, entry->d_name, name_len);
        len += name_len;
        listing[len] = '\0';
        count++;
    }
    closedir(dir);

    if (count == 0) {
        lv_label_set_text(s_list_label, "(empty)");
    } else {
        lv_label_set_text(s_list_label, listing);
    }
}

lv_obj_t *files_screen_create(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    lv_obj_t *heading = lv_label_create(scr);
    lv_label_set_text(heading, "Files");
    lv_obj_set_style_text_color(heading, lv_color_white(), 0);
    lv_obj_align(heading, LV_ALIGN_TOP_MID, 0, 24);

    s_list_label = lv_label_create(scr);
    lv_obj_set_style_text_color(s_list_label, lv_color_white(), 0);
    lv_obj_set_width(s_list_label, LCD_WIDTH - 40);
    lv_label_set_long_mode(s_list_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(s_list_label, LV_ALIGN_TOP_LEFT, 20, 56);

    lv_obj_t *hint = lv_label_create(scr);
    lv_label_set_text(hint, "Back: menu");
    lv_obj_set_style_text_color(hint, lv_color_hex(0x888888), 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -16);

    lv_label_set_text(s_list_label, "...");
    return scr;
}

void files_on_show(void)
{
    files_refresh_list();
}

void files_on_nav(nav_event_t event)
{
    (void)event;
    /* Future: scroll list, enter directories, select file to play. */
}
