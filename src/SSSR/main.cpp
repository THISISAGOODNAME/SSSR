#include <AppBox/AppBox.h>
#include <AppBox/ArgsParser.h>
#include <RenderDevice/RenderDevice.h>

#include <ProgramRef/VertexColor_PS.h>
#include <ProgramRef/VertexColor_VS.h>

#include "SSSRCamera/Camera.h"

struct MouseState
{
    glm::vec2 pos = glm::vec2(0.0f);
    bool pressedLeft = false;
} mouseState;

CameraPositioner_FirstPerson positioner( glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
//positioner.setPosition(vec3(0.0f, 0.0f, 0.0f));
Camera camera(positioner);

int main(int argc, char* argv[])
{
    Settings settings = ParseArgs(argc, argv);
    AppBox app("SSSR", settings);
    AppRect rect = app.GetAppRect();

    std::shared_ptr<RenderDevice> device = CreateRenderDevice(settings, app.GetWindow());
    app.SetGpuName(device->GetGpuName());

    std::shared_ptr<RenderCommandList> upload_command_list = device->CreateRenderCommandList();
    std::vector<uint32_t> ibuf = { 0, 1, 2 };
    std::shared_ptr<Resource> index = device->CreateBuffer(BindFlag::kIndexBuffer | BindFlag::kCopyDest, sizeof(uint32_t) * ibuf.size());
    upload_command_list->UpdateSubresource(index, 0, ibuf.data(), 0, 0);
    std::vector<glm::vec3> pbuf = {
            glm::vec3(-0.5, -0.5, 0.0),
            glm::vec3(0.0,  0.5, 0.0),
            glm::vec3(0.5, -0.5, 0.0),
    };
    std::vector<glm::vec3> colorbuf = {
            glm::vec3(1.0, 0.0, 0.0),
            glm::vec3(0.0, 1.0, 0.0),
            glm::vec3(0.0, 0.0, 1.0),
    };
    std::shared_ptr<Resource> pos = device->CreateBuffer(BindFlag::kVertexBuffer | BindFlag::kCopyDest, sizeof(glm::vec3) * pbuf.size());
    upload_command_list->UpdateSubresource(pos, 0, pbuf.data(), 0, 0);
    std::shared_ptr<Resource> col = device->CreateBuffer(BindFlag::kVertexBuffer | BindFlag::kCopyDest, sizeof(glm::vec3) * colorbuf.size());
    upload_command_list->UpdateSubresource(col, 0, colorbuf.data(), 0, 0);
    upload_command_list->Close();
    device->ExecuteCommandLists({ upload_command_list });

    ProgramHolder<VertexColor_PS, VertexColor_VS> program(*device);
    //program.ps.cbuffer.Settings.color = glm::vec4(1, 0, 0, 1);



    auto window = app.GetWindow();
    glfwSetKeyCallback(
            window,
            [](GLFWwindow* window, int key, int scancode, int action, int mods)
            {
                const bool pressed = action != GLFW_RELEASE;
                if (key == GLFW_KEY_ESCAPE && pressed)
                    glfwSetWindowShouldClose(window, GLFW_TRUE);
                if (key == GLFW_KEY_W)
                    positioner.movement_.forward_ = pressed;
                if (key == GLFW_KEY_S)
                    positioner.movement_.backward_ = pressed;
                if (key == GLFW_KEY_A)
                    positioner.movement_.left_ = pressed;
                if (key == GLFW_KEY_D)
                    positioner.movement_.right_ = pressed;
                if (key == GLFW_KEY_1)
                    positioner.movement_.up_ = pressed;
                if (key == GLFW_KEY_2)
                    positioner.movement_.down_ = pressed;
                if (mods & GLFW_MOD_SHIFT)
                    positioner.movement_.fastSpeed_ = pressed;
                if (key == GLFW_KEY_SPACE)
                    positioner.setUpVector(glm::vec3(0.0f, 1.0f, 0.0f));
            }
            );

    glfwSetCursorPosCallback(
            window,
            [](auto* window, double x, double y)
            {
                int width, height;
                glfwGetFramebufferSize(window, &width, &height);
                mouseState.pos.x = static_cast<float>(x / width);
                mouseState.pos.y = static_cast<float>(y / height);
            }
            );

    glfwSetMouseButtonCallback(
            window,
            [](auto* window, int button, int action, int mods)
            {
                if (button == GLFW_MOUSE_BUTTON_LEFT)
                    mouseState.pressedLeft = action == GLFW_PRESS;
            }
            );

    const glm::mat4 projMat = glm::perspective(45.0f, 1280.0f / 720.0f, 0.1f, 1000.0f);

    const glm::mat4 modelMat = glm::mat4(1.0f);

    std::vector<std::shared_ptr<RenderCommandList>> command_lists;
    for (uint32_t i = 0; i < settings.frame_count; ++i)
    {
        decltype(auto) command_list = device->CreateRenderCommandList();
        command_lists.emplace_back(command_list);
    }

    double timeStamp = glfwGetTime();
    float deltaSeconds = 0.0f;

    while (!app.PollEvents())
    {
        positioner.update(deltaSeconds, mouseState.pos, mouseState.pressedLeft);
        //positioner.setUpVector(glm::vec3(0.0f, 1.0f, 0.0f));
//        if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL)
//        {
//
//            positioner.update(deltaSeconds, mouseState.pos, mouseState.pressedLeft);
//        }

        const double newTimeStamp = glfwGetTime();
        deltaSeconds = static_cast<float>(newTimeStamp - timeStamp);
        timeStamp = newTimeStamp;

        RenderPassBeginDesc render_pass_desc = {};
        render_pass_desc.colors[0].texture = device->GetBackBuffer(device->GetFrameIndex());
        render_pass_desc.colors[0].clear_color = { 0.0f, 0.2f, 0.4f, 1.0f };

        program.vs.cbuffer.MVP.model = modelMat;
        program.vs.cbuffer.MVP.view = camera.getViewMatrix();
        program.vs.cbuffer.MVP.proj = projMat;

        decltype(auto) command_list = command_lists[device->GetFrameIndex()];
        device->Wait(command_lists[device->GetFrameIndex()]->GetFenceValue());
        command_list->Reset();
        command_list->BeginEvent("Compute Pass");
        command_list->UseProgram(program);
        //command_list->Attach(program.ps.cbv.Settings, program.ps.cbuffer.Settings);
        command_list->SetViewport(0, 0, rect.width, rect.height);
        command_list->IASetIndexBuffer(index, gli::format::FORMAT_R32_UINT_PACK32);
        command_list->IASetVertexBuffer(program.vs.ia.POSITION, pos);
        command_list->IASetVertexBuffer(program.vs.ia.COLOR, col);
        command_list->Attach(program.vs.cbv.MVP, program.vs.cbuffer.MVP);
        command_list->BeginRenderPass(render_pass_desc);
        command_list->DrawIndexed(3, 1, 0, 0, 0);
        command_list->EndRenderPass();
        command_list->EndEvent();
        command_list->Close();

        device->ExecuteCommandLists({ command_lists[device->GetFrameIndex()] });
        device->Present();
    }
    return 0;
}
