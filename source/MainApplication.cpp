#include <MainApplication.hpp>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>

namespace {
constexpr const char *ManagedDir = "sdmc:/switch/PCHTXT-Manager";
constexpr int MaxFiles = 64;

const pu::ui::Color Navy{18, 24, 38, 255};
const pu::ui::Color Panel{31, 40, 58, 255};
const pu::ui::Color Selected{74, 34, 43, 255};
const pu::ui::Color Red{225, 55, 75, 255};
const pu::ui::Color White{245, 247, 250, 255};
const pu::ui::Color Muted{160, 173, 190, 255};
const pu::ui::Color Green{65, 190, 125, 255};
const pu::ui::Color Gray{115, 128, 145, 255};

pu::ui::elm::TextBlock::Ref AddText(
    pu::ui::Layout *layout, int x, int y, const std::string &value,
    pu::ui::Color color, pu::ui::DefaultFontSize size = pu::ui::DefaultFontSize::Medium) {
    auto text = pu::ui::elm::TextBlock::New(x, y, value);
    text->SetColor(color);
    text->SetFont(pu::ui::GetDefaultFont(size));
    layout->Add(text);
    return text;
}

void AddPanel(pu::ui::Layout *layout, int x, int y, int w, int h, pu::ui::Color color, int radius = 12) {
    layout->Add(pu::ui::elm::Rectangle::New(x, y, w, h, color, radius));
}
}

bool MainLayout::EndsWith(const std::string &value, const std::string &suffix) {
    return value.size() >= suffix.size() &&
        value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
}

const char *MainLayout::Text(const char *ja, const char *en) const {
    return this->japanese ? ja : en;
}

void MainLayout::LoadLanguage() {
    FILE *file = std::fopen(LanguagePath, "rb");
    if(file == nullptr) {
        this->japanese = true;
        return;
    }
    char value[8] = {};
    std::fgets(value, sizeof(value), file);
    std::fclose(file);
    this->japanese = (value[0] != 'e' && value[0] != 'E');
}

void MainLayout::SaveLanguage() {
    FILE *file = std::fopen(LanguagePath, "wb");
    if(file == nullptr) return;
    std::fputs(this->japanese ? "ja\n" : "en\n", file);
    std::fclose(file);
}

MainLayout::MainLayout() : Layout::Layout() {
    this->SetBackgroundColor(Navy);
    this->LoadLanguage();
    this->message = this->Text("準備完了", "Ready");
    this->ScanFiles();
    this->Refresh();
}

void MainLayout::ScanFiles() {
    this->files.clear();
    DIR *dir = opendir(ManagedDir);
    if(dir == nullptr) {
        this->message = this->Text("フォルダーを開けません。SDカードを確認してください。", "Folder unavailable. Check SD card.");
        return;
    }

    dirent *entry = nullptr;
    while((entry = readdir(dir)) != nullptr && this->files.size() < MaxFiles) {
        const std::string name(entry->d_name);
        const bool enabled = EndsWith(name, ".pchtxt");
        const bool disabled = EndsWith(name, ".pchtxt.disabled");
        if(enabled || disabled) this->files.push_back({name, enabled});
    }
    closedir(dir);

    if(this->selected >= static_cast<int>(this->files.size())) {
        this->selected = this->files.empty() ? 0 : static_cast<int>(this->files.size()) - 1;
    }
}

bool MainLayout::SetSelectedEnabled(bool enable) {
    if(this->files.empty() || this->selected < 0 ||
       this->selected >= static_cast<int>(this->files.size())) {
        this->message = this->Text("ファイルが選択されていません。", "No file selected.");
        return false;
    }

    auto &file = this->files[this->selected];
    if(file.enabled == enable) {
        this->message = enable ? this->Text("このファイルは既にONです。", "This file is already ON.")
                               : this->Text("このファイルは既にOFFです。", "This file is already OFF.");
        return false;
    }

    const std::string oldPath = std::string(ManagedDir) + "/" + file.name;
    std::string newName = file.name;
    if(enable) {
        if(!EndsWith(newName, ".pchtxt.disabled")) return false;
        newName.resize(newName.size() - std::strlen(".disabled"));
    } else {
        if(!EndsWith(newName, ".pchtxt")) return false;
        newName += ".disabled";
    }

    const std::string newPath = std::string(ManagedDir) + "/" + newName;
    struct stat st{};
    if(stat(newPath.c_str(), &st) == 0) {
        this->message = this->Text("変更先に同名ファイルがあります。", "Destination file already exists.");
        return false;
    }

    if(std::rename(oldPath.c_str(), newPath.c_str()) != 0) {
        this->message = this->Text("名前の変更に失敗しました。SDカードを確認してください。", "Rename failed. Check SD card.");
        return false;
    }

    this->message = enable ? this->Text("ファイルを有効にしました。", "File enabled.")
                           : this->Text("ファイルを無効にしました。", "File disabled.");
    this->ScanFiles();
    return true;
}

void MainLayout::Refresh() {
    this->Clear();
    this->SetBackgroundColor(Navy);

    AddPanel(this, 0, 0, 1280, 112, Red, 0);
    AddText(this, 54, 24, "PCHTXT-MANAGER", White, pu::ui::DefaultFontSize::Large);
    AddText(this, 56, 76, this->Text("パッチファイル管理", "PATCH FILE LIBRARY"), White, pu::ui::DefaultFontSize::Small);
    AddText(this, 1010, 44, "SWITCH HOMEBREW", White, pu::ui::DefaultFontSize::Small);

    AddText(this, 56, 145, this->Text("ファイル一覧", "Your files"), White, pu::ui::DefaultFontSize::Large);
    AddText(this, 56, 195, "switch/PCHTXT-Manager/", Muted, pu::ui::DefaultFontSize::Small);

    AddPanel(this, 40, 238, 1200, 360, Panel, 14);
    AddText(this, 72, 258, this->Text("ファイル名", "FILE NAME"), Muted, pu::ui::DefaultFontSize::Small);
    AddText(this, 1050, 258, this->Text("状態", "STATUS"), Muted, pu::ui::DefaultFontSize::Small);

    if(this->files.empty()) {
        AddText(this, 82, 340, this->Text("pchtxtファイルが見つかりません。", "No .pchtxt files found."), White, pu::ui::DefaultFontSize::Large);
        AddText(this, 82, 395, this->Text("SDカードの switch/PCHTXT-Manager にファイルを入れてください。", "Copy files into switch/PCHTXT-Manager on your SD card."), Muted);
    } else {
        const int visibleRows = 6;
        int first = this->selected - visibleRows / 2;
        if(first < 0) first = 0;
        const int maxFirst = std::max(0, static_cast<int>(this->files.size()) - visibleRows);
        if(first > maxFirst) first = maxFirst;
        const int last = std::min(static_cast<int>(this->files.size()), first + visibleRows);
        int y = 294;
        for(int i = first; i < last; ++i, y += 48) {
            const bool isSelected = i == this->selected;
            AddPanel(this, 60, y, 1160, 42, isSelected ? Selected : Navy, 8);
            if(isSelected) AddPanel(this, 60, y, 6, 42, Red, 3);
            std::string displayName = this->files[i].name;
            if(displayName.size() > 66) displayName = displayName.substr(0, 63) + "...";
            AddText(this, 82, y + 7, displayName, isSelected ? White : Muted);
            const auto statusColor = this->files[i].enabled ? Green : Gray;
            const auto status = this->files[i].enabled ? "ON" : "OFF";
            AddPanel(this, 1080, y + 5, 92, 32, statusColor, 8);
            AddText(this, 1108, y + 8, status, White, pu::ui::DefaultFontSize::Small);
        }
    }

    AddText(this, 62, 615, std::to_string(this->files.size()) + this->Text(" 個のファイル", " FILE(S)"), Muted, pu::ui::DefaultFontSize::Small);
    AddText(this, 62, 650, this->message, White, pu::ui::DefaultFontSize::Small);
    AddPanel(this, 0, 690, 1280, 30, Panel, 0);
    AddText(this, 42, 691, this->Text("↑/↓ 選択   A 切替   X 有効   Y 無効   ＋ 言語   B 戻る", "UP/DOWN Select   A Toggle   X Enable   Y Disable   + Language   B Exit"), White, pu::ui::DefaultFontSize::Small);
    AddText(this, 870, 615, this->Text("ファイル整理のみ・パッチ適用はしません", "FILE ORGANIZER ONLY - DOES NOT APPLY PATCHES"), Muted, pu::ui::DefaultFontSize::Small);
}

void MainLayout::MoveSelection(int delta) {
    if(this->files.empty()) return;
    const int count = static_cast<int>(this->files.size());
    this->selected = (this->selected + delta + count) % count;
    this->message = this->Text("選択中：", "Selected: ") + this->files[this->selected].name;
    this->Refresh();
}

void MainLayout::EnableSelected() {
    this->SetSelectedEnabled(true);
    this->Refresh();
}

void MainLayout::DisableSelected() {
    this->SetSelectedEnabled(false);
    this->Refresh();
}

void MainLayout::ToggleSelected() {
    if(this->files.empty()) return;
    this->SetSelectedEnabled(!this->files[this->selected].enabled);
    this->Refresh();
}

void MainLayout::ToggleLanguage() {
    this->japanese = !this->japanese;
    this->SaveLanguage();
    this->message = this->Text("言語を日本語に変更しました。", "Language changed to English.");
    this->Refresh();
}

void MainApplication::OnLoad() {
    this->layout = MainLayout::New();
    this->LoadLayout(this->layout);
    this->SetOnInput([&](const u64 keysDown, const u64, const u64, const pu::ui::TouchPoint) {
        if(keysDown & HidNpadButton_B) this->Close();
        else if(keysDown & HidNpadButton_Up) this->layout->MoveSelection(-1);
        else if(keysDown & HidNpadButton_Down) this->layout->MoveSelection(1);
        else if(keysDown & HidNpadButton_X) this->layout->EnableSelected();
        else if(keysDown & HidNpadButton_Y) this->layout->DisableSelected();
        else if(keysDown & HidNpadButton_A) this->layout->ToggleSelected();
        else if(keysDown & HidNpadButton_Plus) this->layout->ToggleLanguage();
    });
}
