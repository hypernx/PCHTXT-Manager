#include <MainApplication.hpp>
#include <sys/stat.h>

int main() {
    Result mountResult = 0;
    if(fsdevGetDeviceFileSystem("sdmc") == nullptr) {
        mountResult = fsdevMountSdmc();
    }
    if(R_FAILED(mountResult)) {
        diagAbortWithResult(mountResult);
    }

    mkdir("sdmc:/switch", 0777);
    mkdir("sdmc:/switch/PCHTXT-Manager", 0777);

    auto rendererOptions = pu::ui::render::RendererInitOptions(
        SDL_INIT_EVERYTHING, pu::ui::render::RendererHardwareFlags);
    rendererOptions.SetPlServiceType(PlServiceType_User);
    rendererOptions.AddDefaultAllSharedFonts();
    rendererOptions.SetInputPlayerCount(1);
    rendererOptions.AddInputNpadStyleTag(HidNpadStyleSet_NpadStandard);
    rendererOptions.AddInputNpadIdType(HidNpadIdType_Handheld);
    rendererOptions.AddInputNpadIdType(HidNpadIdType_No1);

    auto renderer = pu::ui::render::Renderer::New(rendererOptions);
    auto app = MainApplication::New(renderer);
    const Result rc = app->Load();
    if(R_FAILED(rc)) {
        diagAbortWithResult(rc);
    }
    app->Show();

    fsdevUnmountDevice("sdmc");
    return 0;
}
