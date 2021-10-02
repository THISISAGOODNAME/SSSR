#include <AppBox/AppBox.h>
#include <AppBox/ArgsParser.h>
#include <RenderDevice/RenderDevice.h>

#include "Graphics/Scene.h"

int main(int argc, char* argv[])
{
    Settings settings = ParseArgs(argc, argv);
    AppBox app("SSSR", settings);
    AppRect rect = app.GetAppRect();
    Scene scene(settings, app.GetWindow(), rect.width, rect.height);
    app.SubscribeEvents(&scene, &scene);
    app.SetGpuName(scene.GetRenderDevice().GetGpuName());
    while (!app.PollEvents())
    {
        scene.RenderFrame();
    }
    return 0;
}
