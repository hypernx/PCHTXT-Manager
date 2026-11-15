#include <switch.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#define MANAGED_DIR "sdmc:/switch/PCHTXT-Manager"
#define MAX_FILES 64
#define NAME_LEN 256

typedef struct { char name[NAME_LEN]; bool enabled; } PatchFile;

static PatchFile g_files[MAX_FILES];
static int g_file_count;
static int g_selected;

static const char *tr(const char *ja, const char *en, const char *zh) {
    (void)ja;
    (void)zh;
    return en;
}

static bool ends_with(const char *text, const char *suffix) {
    size_t a = strlen(text), b = strlen(suffix);
    return a >= b && strcmp(text + a - b, suffix) == 0;
}

static void scan_files(void) {
    g_file_count = 0;
    DIR *dir = opendir(MANAGED_DIR);
    if (!dir) return;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL && g_file_count < MAX_FILES) {
        bool enabled = ends_with(entry->d_name, ".pchtxt");
        bool disabled = ends_with(entry->d_name, ".pchtxt.disabled");
        if (!enabled && !disabled) continue;
        strncpy(g_files[g_file_count].name, entry->d_name, NAME_LEN - 1);
        g_files[g_file_count].name[NAME_LEN - 1] = '\0';
        g_files[g_file_count].enabled = enabled;
        g_file_count++;
    }
    closedir(dir);
    if (g_selected >= g_file_count) g_selected = g_file_count > 0 ? g_file_count - 1 : 0;
}

static void set_selected_enabled(bool enable) {
    if (g_file_count == 0 || g_selected < 0 || g_selected >= g_file_count) return;
    if (g_files[g_selected].enabled == enable) return;

    char old_path[NAME_LEN * 2], new_path[NAME_LEN * 2];
    const char *name = g_files[g_selected].name;
    size_t len = strlen(name);
    const char *disabled_suffix = ".disabled";
    size_t suffix_len = strlen(disabled_suffix);

    snprintf(old_path, sizeof(old_path), "%s/%s", MANAGED_DIR, name);
    if (enable) {
        if (len <= suffix_len || !ends_with(name, disabled_suffix)) return;
        snprintf(new_path, sizeof(new_path), "%s/%.*s", MANAGED_DIR,
                 (int)(len - suffix_len), name);
    } else {
        if (!ends_with(name, ".pchtxt")) return;
        snprintf(new_path, sizeof(new_path), "%s/%s.disabled", MANAGED_DIR, name);
    }

    // Never overwrite another patch file if the target name already exists.
    struct stat st;
    if (stat(new_path, &st) == 0) {
        printf("\nCannot change state: target file already exists.\n");
        consoleUpdate(NULL);
        svcSleepThread(1500ULL * 1000 * 1000);
        return;
    }

    if (rename(old_path, new_path) == 0) {
        scan_files();
    } else {
        printf("\nRename failed. Check the SD card and file name.\n");
        consoleUpdate(NULL);
        svcSleepThread(1500ULL * 1000 * 1000);
    }
}

static void toggle_selected(void) {
    if (g_file_count == 0 || g_selected < 0 || g_selected >= g_file_count) return;
    set_selected_enabled(!g_files[g_selected].enabled);
}

static void draw_ui(void) {
    consoleClear();
    printf("\x1b[1;37;41m  PCHTXT-MANAGER  \x1b[0m  \x1b[90mHOMEbrew utility\x1b[0m\n");
    printf("\x1b[31m============================================================\x1b[0m\n");
    printf("\n");
    printf("  \x1b[1;31mFILE LIBRARY\x1b[0m                                      \x1b[90m01 / 03\x1b[0m\n");
    printf("  %s\n", tr("SDカード内の管理フォルダー", "MANAGED SD FOLDER", "SD 卡管理目录"));
    printf("  \x1b[36mswitch/PCHTXT-Manager/\x1b[0m\n\n");
    printf("  \x1b[90m----------------------------------------------------------\x1b[0m\n");
    if (g_file_count == 0) {
        printf("\n  \x1b[1;33m◇\x1b[0m  %s\n",
               tr("ファイルがまだありません", "Your library is empty", "文件列表为空"));
        printf("     %s\n", tr("このフォルダーに .pchtxt を配置してください。",
                           "Place your .pchtxt files in this folder.",
                           "请将 .pchtxt 文件放入此文件夹。"));
        printf("\n");
    } else {
        for (int i = 0; i < g_file_count; i++) {
            if (i == g_selected) printf("  \x1b[1;31m>\x1b[0m ");
            else printf("    ");
            if (g_files[i].enabled)
                printf("\x1b[1;32m[ ON ]\x1b[0m ");
            else
                printf("\x1b[90m[ OFF ]\x1b[0m ");
            printf("%s\n", g_files[i].name);
        }
        printf("\n  \x1b[90m%d %s\x1b[0m\n", g_file_count,
               tr("件のファイル", "FILES", "个文件"));
    }
    printf("  \x1b[90m----------------------------------------------------------\x1b[0m\n");
    printf("\n  \x1b[1;31mCONTROLS\x1b[0m\n");
    printf("  \x1b[37m[↑/↓]\x1b[0m %s    \x1b[37m[A]\x1b[0m %s\n",
           tr("選択", "Navigate", "选择"),
           tr("有効 / 無効", "Toggle", "切换"));
    printf("  [UP/DOWN] Navigate    [A] Toggle ON/OFF\n");
    printf("  [B] Exit\n");
    printf("\n  LANGUAGE: ENGLISH ONLY\n");
    printf("\x1b[1;31m============================================================\x1b[0m\n");
    printf("  \x1b[90mPCHTXT-Manager  |  SD card file organizer\x1b[0m\n");
    consoleUpdate(NULL);
}

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    consoleInit(NULL);
    printf("PCHTXT-Manager starting...\n");
    consoleUpdate(NULL);

    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    PadState pad;
    padInitializeDefault(&pad);
    // libnx normally mounts sdmc during startup. Only mount it here
    // if startup did not already make the device available.
    Result rc = 0;
    if (fsdevGetDeviceFileSystem("sdmc") == NULL)
        rc = fsdevMountSdmc();
    if (R_FAILED(rc)) {
        printf("Failed to mount SD card (0x%08X).\n", (unsigned)rc);
        printf("Press PLUS to exit.\n");
        consoleUpdate(NULL);
        while (appletMainLoop()) {
            padUpdate(&pad);
            if (padGetButtonsDown(&pad) & HidNpadButton_Plus) break;
        }
        consoleExit(NULL);
        return 1;
    }

    mkdir("sdmc:/switch", 0777);
    mkdir(MANAGED_DIR, 0777);
    scan_files();
    draw_ui();

    while (appletMainLoop()) {
        padUpdate(&pad);
        u64 keys = padGetButtonsDown(&pad);
        if ((keys & HidNpadButton_B) || (keys & HidNpadButton_Plus)) break;
        if ((keys & HidNpadButton_Up) && g_file_count > 0 && g_selected > 0) g_selected--;
        if ((keys & HidNpadButton_Down) && g_file_count > 0 && g_selected < g_file_count - 1) g_selected++;
        if (keys & HidNpadButton_X) set_selected_enabled(true);
        if (keys & HidNpadButton_Y) set_selected_enabled(false);
        if (keys & HidNpadButton_A) toggle_selected();
        draw_ui();
        svcSleepThread(16 * 1000 * 1000);
    }
    fsdevUnmountDevice("sdmc");
    consoleExit(NULL);
    return 0;
}
