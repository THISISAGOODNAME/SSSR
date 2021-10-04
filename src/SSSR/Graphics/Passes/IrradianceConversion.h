#pragma once

#include "RenderPass.h"
#include <Device/Device.h>
#include <Geometry/Geometry.h>
#include <ProgramRef/Cubemap_VS.h>
#include <ProgramRef/IrradianceConvolution_PS.h>

class IrradianceConversion : public IPass
{
public:
    struct Target
    {
        std::shared_ptr<Resource>& res;
        std::shared_ptr<Resource>& dsv;
        size_t size;
    };

    struct Input
    {
        Model& model;
        std::shared_ptr<Resource>& environment;
        Target irradince;
    };

    struct Output
    {
    } output;

    IrradianceConversion(RenderDevice& device, const Input& input);

    virtual void OnUpdate() override;
    virtual void OnRender(RenderCommandList& command_list)override;
    virtual void OnModifySSSRSettings(const SSSRSettings& settings) override;

private:
    void DrawIrradianceConvolution(RenderCommandList& command_list);

    SSSRSettings m_settings;
    RenderDevice& m_device;
    Input m_input;
    std::shared_ptr<Resource> m_sampler;
    ProgramHolder<Cubemap_VS, IrradianceConvolution_PS> m_program_irradiance_convolution;
    bool is = false;
};
