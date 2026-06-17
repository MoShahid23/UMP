/**
 * @file files.c
 * @brief File manager: browse STORAGE_MOUNT_PATH with selectable rows.
 */

#include "files.h"

#include "esp_log.h"
#include "storage.h"
#include "ui_kit.h"

#include <ctype.h>
#include <dirent.h>
#include <strings.h>
#include <string.h>

static const char *TAG = "files";

#define FILES_MAX_ENTRIES  32
#define FILES_NAME_MAX     64
#define FILES_SCAN_MAX     64

typedef struct {
    char name[FILES_NAME_MAX];
    uint8_t type;
} files_entry_t;

static lv_obj_t *s_list;
static lv_obj_t *s_rows[FILES_MAX_ENTRIES];
static char s_names[FILES_MAX_ENTRIES][FILES_NAME_MAX];
static int s_count;
static int s_sel;

static void files_trim_name(char *name)
{
    size_t len = strlen(name);
    while (len > 0 && name[len - 1] == ' ') {
        name[--len] = '\0';
    }
}

static bool files_is_macos_junk(const char *name, uint8_t type)
{
    if (name[0] == '.') {
        return true;
    }
    if (strncmp(name, "._", 2) == 0) {
        return true;
    }
    if (strchr(name, '~') != NULL) {
        return true;
    }

    static const char *const junk[] = {
        "FSEVEN-1", "SPOTL-1", "TRASH-1",
        "FSEVENTS-D", "SPOTLIGHT-V100",
        "System Volume Information",
        "Temporary Items", ".DS_Store", "DS_Store",
        NULL,
    };
    for (int i = 0; junk[i] != NULL; i++) {
        if (strcasecmp(name, junk[i]) == 0) {
            return true;
        }
    }

    /* VFAT short names for hidden macOS folders (usually DT_DIR). */
    size_t len = strlen(name);
    if (len >= 3 && name[len - 2] == '-' && isdigit((unsigned char)name[len - 1])) {
        if (type == DT_DIR || type == DT_UNKNOWN) {
            return true;
        }
    }

    return false;
}

static bool files_is_shadow_of(const char *name, const files_entry_t *entries, int count)
{
    const char *base = NULL;

    if (strncmp(name, "._", 2) == 0) {
        base = name + 2;
    } else if (name[0] == '_') {
        base = name + 1;
    } else {
        return false;
    }

    if (base[0] == '\0') {
        return true;
    }

    for (int i = 0; i < count; i++) {
        if (strcasecmp(entries[i].name, name) == 0) {
            continue;
        }
        if (strcasecmp(entries[i].name, base) == 0) {
            return true;
        }
    }

    return false;
}

static void files_clear_list(void)
{
    lv_obj_clean(s_list);
    s_count = 0;
    s_sel = 0;
}

static void files_show_message(const char *msg)
{
    files_clear_list();
    lv_obj_t *row = ui_list_row_create(s_list, msg);
    ui_list_row_set_selected(row, true);
    s_count = 0;
}

static void files_highlight_update(void)
{
    for (int i = 0; i < s_count; i++) {
        ui_list_row_set_selected(s_rows[i], i == s_sel);
    }
    if (s_count > 0) {
        lv_obj_scroll_to_view(s_rows[s_sel], LV_ANIM_OFF);
    }
}

static void files_move_selection(int delta)
{
    if (s_count <= 1) {
        return;
    }

    int next = s_sel + delta;
    if (next < 0 || next >= s_count) {
        return;
    }

    s_sel = next;
    files_highlight_update();
}

static void files_refresh_list(void)
{
    if (!storage_is_ready()) {
        files_show_message("SD card not available.");
        return;
    }

    DIR *dir = opendir(STORAGE_MOUNT_PATH);
    if (!dir) {
        files_show_message("Could not open SD card.");
        return;
    }

    files_entry_t scan[FILES_SCAN_MAX];
    int scan_count = 0;
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL && scan_count < FILES_SCAN_MAX) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char name[FILES_NAME_MAX];
        strncpy(name, entry->d_name, FILES_NAME_MAX - 1);
        name[FILES_NAME_MAX - 1] = '\0';
        files_trim_name(name);

        strncpy(scan[scan_count].name, name, FILES_NAME_MAX);
        scan[scan_count].type = entry->d_type;
        scan_count++;
    }
    closedir(dir);

    files_clear_list();

    for (int i = 0; i < scan_count && s_count < FILES_MAX_ENTRIES; i++) {
        const char *name = scan[i].name;

        if (files_is_macos_junk(name, scan[i].type)) {
            ESP_LOGI(TAG, "skip junk: %s", name);
            continue;
        }
        if (files_is_shadow_of(name, scan, scan_count)) {
            ESP_LOGI(TAG, "skip shadow: %s", name);
            continue;
        }

        strncpy(s_names[s_count], name, FILES_NAME_MAX - 1);
        s_names[s_count][FILES_NAME_MAX - 1] = '\0';
        s_rows[s_count] = ui_list_row_create(s_list, s_names[s_count]);
        s_count++;
    }

    if (s_count == 0) {
        files_show_message("(empty)");
        return;
    }

    s_sel = 0;
    files_highlight_update();
}

lv_obj_t *files_screen_create(void)
{
    ui_page_t page = ui_page_create("Files", "Back: menu");
    s_list = ui_list_create(page.content);
    return page.screen;
}

void files_on_show(void)
{
    files_refresh_list();
}

void files_on_nav(nav_event_t event)
{
    switch (event) {
    case NAV_EVT_LEFT:
        files_move_selection(-1);
        break;
    case NAV_EVT_RIGHT:
        files_move_selection(1);
        break;
    case NAV_EVT_SELECT:
        /* Future: open dir, play file, rename, move, etc. */
        break;
    default:
        break;
    }
}

const char *files_selected_name(void)
{
    if (s_count == 0 || s_sel < 0 || s_sel >= s_count) {
        return NULL;
    }
    return s_names[s_sel];
}
