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
        std::shared_ptr<Resource>& irradince;
        std::shared_ptr<Resource>& prefilter;
        std::shared_ptr<Resource>& brdflut;
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
    virtual void OnModifySSSRSettings(const SSSRSettings& settings) override;

private:
    SSSRSettings m_settings;
    RenderDevice& m_device;
    Input m_input;
    int m_width;
    int m_height;

    ProgramHolder<ForwardPBRPass_PS, ForwardPBRPass_VS> m_program;
    std::shared_ptr<Resource> m_sampler;
    std::shared_ptr<Resource> m_sampler_brdflut;

    void CreateSizeDependentResources();
};
