#pragma once

#include <imgui.h>
#include <Device/Device.h>
#include <Geometry/Geometry.h>

#include "RenderPass.h"
#include "SSSRCamera/Camera.h"

#include <ProgramRef/ForwardPBRPass_PS.h>
#include <ProgramRef/ForwardPBRPass_VS.h>

class ForwardPBRPass : public IPass
                     , public InputEvents
{
public:
    struct Input
    {
        Model& sphereModel;
        std::shared_ptr<Resource>& rtv;
        const Camera& camera;
    };

    struct Output
    {
        std::shared_ptr<Resource> dsv;
    } output;

    ForwardPBRPass(RenderDevice& device, RenderCommandList& command_list, const Input& input, int width, int height);
    ~ForwardPBRPass();

    virtual void OnUpdate() override;
    virtual void OnRender(RenderCommandList& command_list)override;
    virtual void OnResize(int width, int height) override;

private:
    RenderDevice& m_device;
    Input m_input;
    int m_width;
    int m_height;

    ProgramHolder<ForwardPBRPass_PS, ForwardPBRPass_VS> m_program;
    std::shared_ptr<Resource> m_sampler;

    void CreateSizeDependentResources();
};
