#pragma once

#include "RenderPass.h"
#include "SSSRCamera/Camera.h"
#include <Device/Device.h>
#include <Geometry/Geometry.h>
#include <ProgramRef/Background_PS.h>
#include <ProgramRef/Background_VS.h>

class BackgroundPass : public IPass
{
public:
    struct Input
    {
        Model& model;
        const Camera& camera;
        std::shared_ptr<Resource>& environment;
        std::shared_ptr<Resource>& rtv;
        std::shared_ptr<Resource>& dsv;
    };

    struct Output
    {
        std::shared_ptr<Resource> environment;
        std::shared_ptr<Resource> irradince;
    } output;

    BackgroundPass(RenderDevice& device, const Input& input, int width, int height);

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
    std::shared_ptr<Resource> m_sampler;
    ProgramHolder<Background_VS, Background_PS> m_program;
};
