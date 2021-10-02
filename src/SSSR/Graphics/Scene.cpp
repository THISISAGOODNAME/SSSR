#include "Scene.h"
#include <Utilities/FileUtility.h>
#include <GLFW/glfw3.h>
#include <chrono>
#include <glm/gtx/transform.hpp>

Scene::Scene(const Settings& settings, GLFWwindow* window, int width, int height)
    : m_device(CreateRenderDevice(settings, window))
    , m_window(window)
    , m_width(width)
    , m_height(height)
    , m_upload_command_list(m_device->CreateRenderCommandList())
    , m_imgui_pass(*m_device, *m_upload_command_list, { m_render_target_view, *this, m_settings }, width, height, window)
    , m_cameraFPSPositioner(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f))
    , m_camera(m_cameraFPSPositioner, 45.0f, 1280.0f, 720.0f, 0.1f, 1000.0f)
{
    CreateRT();

    m_passes.push_back({ "ImGui Pass", m_imgui_pass });

    for (uint32_t i = 0; i < settings.frame_count * m_passes.size(); ++i)
        m_command_lists.emplace_back(m_device->CreateRenderCommandList());

    m_upload_command_list->Close();
    m_device->ExecuteCommandLists({ m_upload_command_list });

    m_settings.Set("gpu_name", m_device->GetGpuName());
    OnModifySSSRSettings(m_settings);
}

Scene::~Scene()
{
    m_device->WaitForIdle();
}

void Scene::RenderFrame()
{
    for (auto& desc : m_passes)
    {
        desc.pass.get().OnUpdate();
    }

    m_render_target_view = m_device->GetBackBuffer(m_device->GetFrameIndex());

    std::vector<std::shared_ptr<RenderCommandList>> command_lists;

    for (auto& desc : m_passes)
    {
        decltype(auto) command_list = m_command_lists[m_command_list_index];
        m_command_list_index = (m_command_list_index + 1) % m_command_lists.size();
        m_device->Wait(command_list->GetFenceValue());
        command_list->Reset();
        command_list->BeginEvent(desc.name);
        desc.pass.get().OnRender(*command_list);
        command_list->EndEvent();
        command_list->Close();

        command_lists.emplace_back(command_list);
    }

    m_device->ExecuteCommandLists(command_lists);
    m_device->Present();
}

void Scene::OnResize(int width, int height)
{
    if (width == m_width && height == m_height)
        return;

    m_width = width;
    m_height = height;

    m_render_target_view.reset();
    m_device->WaitForIdle();

    for (auto& cmd : m_command_lists)
        cmd = m_device->CreateRenderCommandList();

    m_device->Resize(m_width, m_height);
    m_camera.OnResize(m_width, m_height);

    CreateRT();

    for (auto& desc : m_passes)
    {
        desc.pass.get().OnResize(width, height);
    }
}

void Scene::OnKey(int key, int action)
{
    m_imgui_pass.OnKey(key, action);
    if (glfwGetInputMode(m_window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL)
        return;

    if (action == GLFW_PRESS)
        m_keys[key] = true;
    else if (action == GLFW_RELEASE)
        m_keys[key] = false;

    m_cameraFPSPositioner.OnKey(key, action);
}

void Scene::OnMouse(bool first_event, double xpos, double ypos)
{
    m_cameraFPSPositioner.OnMouse(first_event, xpos, ypos);

    if (glfwGetInputMode(m_window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL)
    {
        m_imgui_pass.OnMouse(first_event, xpos, ypos);
        return;
    }

    if (first_event)
    {
        m_last_x = xpos;
        m_last_y = ypos;
    }

    double xoffset = xpos - m_last_x;
    double yoffset = m_last_y - ypos;

    m_last_x = xpos;
    m_last_y = ypos;
}

void Scene::OnMouseButton(int button, int action)
{
    m_cameraFPSPositioner.OnMouseButton(button, action);
    if (glfwGetInputMode(m_window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL)
    {
        m_imgui_pass.OnMouseButton(button, action);
    }
}

void Scene::OnScroll(double xoffset, double yoffset)
{
    if (glfwGetInputMode(m_window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL)
    {
        m_imgui_pass.OnScroll(xoffset, yoffset);
    }
}

void Scene::OnInputChar(unsigned int ch)
{
    if (glfwGetInputMode(m_window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL)
    {
        m_imgui_pass.OnInputChar(ch);
    }
}

void Scene::OnModifySSSRSettings(const SSSRSettings& settings)
{
    m_settings = settings;
    for (auto& desc : m_passes)
    {
        desc.pass.get().OnModifySSSRSettings(m_settings);
    }
}

void Scene::CreateRT()
{
    m_depth_stencil_view = m_device->CreateTexture(BindFlag::kDepthStencil, gli::format::FORMAT_D32_SFLOAT_PACK32, 1, m_width, m_height, 1);
}
