#pragma once

#include <pu/Plutonium>
#include <dirent.h>
#include <string>
#include <vector>

struct PatchFile {
    std::string name;
    bool enabled;
};

class MainLayout : public pu::ui::Layout {
private:
    std::vector<PatchFile> files;
    int selected = 0;
    bool japanese = true;
    std::string message;
    static constexpr const char *LanguagePath = "sdmc:/switch/PCHTXT-Manager/language.txt";

    static bool EndsWith(const std::string &value, const std::string &suffix);
    void LoadLanguage();
    void SaveLanguage();
    const char *Text(const char *ja, const char *en) const;
    void ScanFiles();
    bool SetSelectedEnabled(bool enable);

public:
    MainLayout();
    PU_SMART_CTOR(MainLayout)

    void Refresh();
    void MoveSelection(int delta);
    void EnableSelected();
    void DisableSelected();
    void ToggleSelected();
    void ToggleLanguage();
};

class MainApplication : public pu::ui::Application {
private:
    MainLayout::Ref layout;

public:
    using Application::Application;
    PU_SMART_CTOR(MainApplication)
    void OnLoad() override;
};
