#include <switch.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#define MANAGED_DIR "sdmc:/switch/PCHTXT-Manager"
#define MAX_FILES 64
#define NAME_LEN 256

typedef enum { LANG_JA = 0, LANG_EN, LANG_ZH, LANG_COUNT } Language;
typedef struct { char name[NAME_LEN]; bool enabled; } PatchFile;

static PatchFile g_files[MAX_FILES];
static int g_file_count;
static int g_selected;
static Language g_language = LANG_JA;

static const char *tr(const char *ja, const char *en, const char *zh) {
    if (g_language == LANG_EN) return en;
    if (g_language == LANG_ZH) return zh;
    return ja;
}

static bool ends_with(const char *text, const char *suffix) {
    size_t a = strlen(text), b = strlen(suffix);
    return a >= b && strcmp(text + a - b, suffix) == 0;
}

static void load_language(void) {
    char path[NAME_LEN];
    snprintf(path, sizeof(path), "%s/settings.cfg", MANAGED_DIR);
    FILE *fp = fopen(path, "r");
    if (!fp) return;
    int value = 0;
    if (fscanf(fp, "%d", &value) == 1 && value >= 0 && value < LANG_COUNT)
        g_language = (Language)value;
    fclose(fp);
}

static void save_language(void) {
    char path[NAME_LEN];
    snprintf(path, sizeof(path), "%s/settings.cfg", MANAGED_DIR);
    FILE *fp = fopen(path, "w");
    if (!fp) return;
    fprintf(fp, "%d\n", (int)g_language);
    fclose(fp);
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

static void toggle_selected(void) {
    if (g_file_count == 0 || g_selected < 0 || g_selected >= g_file_count) return;
    char old_path[NAME_LEN * 2], new_path[NAME_LEN * 2];
    const char *name = g_files[g_selected].name;
    size_t len = strlen(name);
    snprintf(old_path, sizeof(old_path), "%s/%s", MANAGED_DIR, name);
    if (g_files[g_selected].enabled) {
        snprintf(new_path, sizeof(new_path), "%s/%.*s.disabled", MANAGED_DIR,
                 (int)(len - strlen(".pchtxt")), name);
    } else {
        snprintf(new_path, sizeof(new_path), "%s/%.*s", MANAGED_DIR,
                 (int)(len - strlen(".disabled")), name);
    }
    if (rename(old_path, new_path) == 0) scan_files();
}

static void draw_ui(void) {
    consoleClear();
    printf("\\x1b[1;37;41m  PCHTXT-MANAGER  \\x1b[0m  \\x1b[90mHOMEbrew utility\\x1b[0m\\n");
    printf("\\x1b[31m============================================================\\x1b[0m\\n");
    printf("\\n");
    printf("  \\x1b[1;31mFILE LIBRARY\\x1b[0m                                      \\x1b[90m01 / 03\\x1b[0m\\n");
    printf("  %s\\n", tr("SDカード内の管理フォルダー", "MANAGED SD FOLDER", "SD 卡管理目录"));
    printf("  \\x1b[36mswitch/PCHTXT-Manager/\\x1b[0m\\n\\n");
    printf("  \\x1b[90m----------------------------------------------------------\\x1b[0m\\n");
    if (g_file_count == 0) {
        printf("\\n  \\x1b[1;33m◇\\x1b[0m  %s\\n",
               tr("ファイルがまだありません", "Your library is empty", "文件列表为空"));
        printf("     %s\\n", tr("このフォルダーに .pchtxt を配置してください。",
                           "Place your .pchtxt files in this folder.",
                           "请将 .pchtxt 文件放入此文件夹。"));
        printf("\\n");
    } else {
        for (int i = 0; i < g_file_count; i++) {
            if (i == g_selected) printf("  \\x1b[1;31m>\\x1b[0m ");
            else printf("    ");
            if (g_files[i].enabled)
                printf("\\x1b[1;32m[ ON ]\\x1b[0m ");
            else
                printf("\\x1b[90m[ OFF ]\\x1b[0m ");
            printf("%s\\n", g_files[i].name);
        }
        printf("\\n  \\x1b[90m%d %s\\x1b[0m\\n", g_file_count,
               tr("件のファイル", "FILES", "个文件"));
    }
    printf("  \\x1b[90m----------------------------------------------------------\\x1b[0m\\n");
    printf("\\n  \\x1b[1;31mCONTROLS\\x1b[0m\\n");
    printf("  \\x1b[37m[↑/↓]\\x1b[0m %s    \\x1b[37m[A]\\x1b[0m %s\\n",
           tr("選択", "Navigate", "选择"),
           tr("有効 / 無効", "Toggle", "切换"));
    printf("  \\x1b[37m[X]\\x1b[0m %s    \\x1b[37m[B]\\x1b[0m %s\\n",
           tr("言語変更", "Language", "语言"),
           tr("終了", "Exit", "退出"));
    printf("\\n  \\x1b[90mLANGUAGE  \\x1b[0m\\x1b[1;37m%s\\x1b[0m\\n",
           g_language == LANG_JA ? "日本語" :
           g_language == LANG_EN ? "English" : "简体中文");
    printf("\\x1b[1;31m============================================================\\x1b[0m\\n");
    printf("  \\x1b[90mPCHTXT-Manager  |  SD card file organizer\\x1b[0m\\n");
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
    load_language();
    scan_files();
    draw_ui();

    while (appletMainLoop()) {
        padUpdate(&pad);
        u64 keys = padGetButtonsDown(&pad);
        if ((keys & HidNpadButton_B) || (keys & HidNpadButton_Plus)) break;
        if ((keys & HidNpadButton_Up) && g_file_count > 0 && g_selected > 0) g_selected--;
        if ((keys & HidNpadButton_Down) && g_file_count > 0 && g_selected < g_file_count - 1) g_selected++;
        if (keys & HidNpadButton_A) toggle_selected();
        if (keys & HidNpadButton_X) {
            g_language = (Language)((g_language + 1) % LANG_COUNT);
            save_language();
        }
        draw_ui();
        svcSleepThread(16 * 1000 * 1000);
    }
    fsdevUnmountDevice("sdmc");
    consoleExit(NULL);
    return 0;
}
