#pragma once

#include <AppBox/InputEvents.h>
#include <AppBox/WindowEvents.h>
#include <RenderDevice/RenderDevice.h>

#include "SSSRSettings.h"
#include "SSSRCamera/Camera.h"

#include "Passes/ForwardPBRPass.h"
#include "Passes/ImGuiPass.h"
#include "Passes/Equirectangular2Cubemap.h"
#include "Passes/BackgroundPass.h"

#include <string>
#include <glm/glm.hpp>
#include <vector>
#include <map>


class Scene
    : public InputEvents
    , public WindowEvents
    , public IModifySSSRSettings
{
public:
    Scene(const Settings& settings, GLFWwindow* window, int width, int height);
    ~Scene();

    RenderDevice& GetRenderDevice()
    {
        return *m_device;
    }

    void RenderFrame();

    void OnResize(int width, int height) override;

    virtual void OnKey(int key, int action) override;
    virtual void OnMouse(bool first, double xpos, double ypos) override;
    virtual void OnMouseButton(int button, int action) override;
    virtual void OnScroll(double xoffset, double yoffset) override;
    virtual void OnInputChar(unsigned int ch) override;
    virtual void OnModifySSSRSettings(const SSSRSettings& settings) override;

private:
    void CreateRT();

    std::shared_ptr<RenderDevice> m_device;
    GLFWwindow* m_window;

    int m_width;
    int m_height;
    std::shared_ptr<RenderCommandList> m_upload_command_list;
    std::shared_ptr<Resource> m_render_target_view;
    std::shared_ptr<Resource> m_depth_stencil_view;

    Model m_model_sphere;
    Model m_model_cube;
    Model m_model_square;

    std::shared_ptr<Resource> m_equirectangular_environment;

    Equirectangular2Cubemap m_equirectangular2cubemap;
    ForwardPBRPass m_forwardPBR_pass;
    BackgroundPass m_background_pass;
    ImGuiPass m_imgui_pass;
    SSSRSettings m_settings;

    struct PassDesc
    {
        std::string name;
        std::reference_wrapper<IPass> pass;
    };
    std::vector<PassDesc> m_passes;
    std::vector<std::shared_ptr<RenderCommandList>> m_command_lists;
    size_t m_command_list_index = 0;

    Camera m_camera;
    CameraPositioner_FirstPerson m_cameraFPSPositioner;

    std::map<int, bool> m_keys;
    float m_last_frame = 0.0;
    float m_delta_time = 0.0f;
    double m_last_x = 0.0f;
    double m_last_y = 0.0f;
    float m_angle = 0.0;
};
