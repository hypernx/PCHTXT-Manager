#include <MainApplication.hpp>

int main() {
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
    if(R_FAILED(rc)) diagAbortWithResult(rc);
    app->Show();
    return 0;
}
