#pragma once

#include "RenderPass.h"
#include <Device/Device.h>
#include <Geometry/Geometry.h>
#include <ProgramRef/BRDFlut_PS.h>
#include <ProgramRef/BRDFlut_VS.h>

class BRDFGen : public IPass
{
public:
    struct Input
    {
        Model& square_model;
    };

    struct Output
    {
        std::shared_ptr<Resource> brdflut;
    } output;

    BRDFGen(RenderDevice& device, const Input& input);

    virtual void OnUpdate() override;
    virtual void OnRender(RenderCommandList& command_list)override;
    virtual void OnResize(int width, int height) override;
    virtual void OnModifySSSRSettings(const SSSRSettings& settings) override;

private:
    void DrawBRDF(RenderCommandList& command_list);

    SSSRSettings m_settings;
    RenderDevice& m_device;
    Input m_input;
    std::shared_ptr<Resource> m_dsv;
    ProgramHolder<BRDFlut_PS, BRDFlut_VS> m_program;
    size_t m_size = 512;
    bool is = false;
};
