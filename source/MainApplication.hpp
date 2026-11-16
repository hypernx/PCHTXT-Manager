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
    std::string message = "A: Toggle   X: Enable   Y: Disable";
    pu::ui::elm::TextBlock::Ref count_text;

    static bool EndsWith(const std::string &value, const std::string &suffix);
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
};

class MainApplication : public pu::ui::Application {
private:
    MainLayout::Ref layout;

public:
    using Application::Application;
    PU_SMART_CTOR(MainApplication)
    void OnLoad() override;
};
