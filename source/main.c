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
    printf("\x1b[1;31m========================================\x1b[0m\n");
    printf("             PCHTXT-Manager\n");
    printf("\x1b[1;31m========================================\x1b[0m\n\n");
    printf("%s\n\n", tr("管理フォルダー: switch/PCHTXT-Manager",
                       "Managed folder: switch/PCHTXT-Manager",
                       "管理目录: switch/PCHTXT-Manager"));
    if (g_file_count == 0) {
        printf("%s\n", tr("ファイルがありません。", "No .pchtxt files found.",
                          "没有找到 .pchtxt 文件。"));
        printf("%s\n", tr("SDカード上のフォルダーにファイルを手動で配置してください。",
                          "Manually place files in the folder on the SD card.",
                          "请手动将文件放入 SD 卡上的文件夹。"));
    } else {
        for (int i = 0; i < g_file_count; i++) {
            printf("%s %s %s\n", i == g_selected ? ">" : " ",
                   g_files[i].enabled ? tr("[有効]", "[ON]", "[启用]")
                                      : tr("[無効]", "[OFF]", "[停用]"),
                   g_files[i].name);
        }
    }
    printf("\n\x1b[1;31m----------------------------------------\x1b[0m\n");
    printf("%s\n", tr("↑↓ 選択   A 切替   X 言語   B 終了",
                       "↑↓ Select   A Toggle   X Language   B Exit",
                       "↑↓ 选择   A 切换   X 语言   B 退出"));
    printf("%s: %s\n", tr("言語", "Language", "语言"),
           g_language == LANG_JA ? "日本語" :
           g_language == LANG_EN ? "English" : "简体中文");
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
    Result rc = fsdevMountSdmc();
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
