#pragma once

#include "RenderPass.h"

#include <Device/Device.h>
#include <Geometry/Geometry.h>

#include <ProgramRef/Cubemap_VS.h>
#include <ProgramRef/Equirectangular2Cubemap_PS.h>
#include <ProgramRef/DownSample_CS.h>

class Equirectangular2Cubemap : public IPass
{
public:
    struct Input
    {
        Model& model;
        std::shared_ptr<Resource>& hdr;
    };

    struct Output
    {
        std::shared_ptr<Resource> environment;
    } output;

    Equirectangular2Cubemap(RenderDevice& device, const Input& input);

    virtual void OnUpdate() override;
    virtual void OnRender(RenderCommandList& command_list)override;
    virtual void OnModifySSSRSettings(const SSSRSettings& settings) override;

private:
    void DrawEquirectangular2Cubemap(RenderCommandList& command_list);
    void CreateSizeDependentResources();

    SSSRSettings m_settings;
    RenderDevice& m_device;
    Input m_input;
    std::shared_ptr<Resource> m_sampler;
    std::shared_ptr<Resource> m_dsv;
    ProgramHolder<Cubemap_VS, Equirectangular2Cubemap_PS> m_program_equirectangular2cubemap;
    ProgramHolder<DownSample_CS> m_program_downsample;
    size_t m_texture_size = 512;
    size_t m_texture_mips = 0;
    bool is = false;
};
