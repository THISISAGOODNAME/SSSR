#pragma once

#include "../SSSRSettings.h"
#include <imgui.h>
#include <string>
#include <vector>

class ImGuiSettings
{
public:
    ImGuiSettings(IModifySSSRSettings& listener, SSSRSettings& settings)
        : listener(listener)
        , settings(settings)
    {
    }

    void NewFrame()
    {
        ImGui::NewFrame();
        ImGui::Begin("SSSR Settings");

        if (settings.Has("gpu_name"))
        {
            std::string gpu_name = settings.Get<std::string>("gpu_name");
            if (!gpu_name.empty())
                ImGui::Text("%s", gpu_name.c_str());
        }

        bool has_changed = settings.OnDraw();

        ImGui::End();
        if (has_changed)
            listener.OnModifySSSRSettings(settings);
    }

    void OnKey(int key, int action)
    {
        if (settings.OnKey(key, action))
            listener.OnModifySSSRSettings(settings);
    }

private:
    IModifySSSRSettings& listener;
    SSSRSettings& settings;
};
