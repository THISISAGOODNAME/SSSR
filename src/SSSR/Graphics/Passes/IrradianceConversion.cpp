#include "IrradianceConversion.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

IrradianceConversion::IrradianceConversion(RenderDevice& device, const Input& input)
    : m_device(device)
    , m_input(input)
    , m_program_irradiance_convolution(device)
{
    m_sampler = m_device.CreateSampler({
        SamplerFilter::kAnisotropic,
        SamplerTextureAddressMode::kWrap,
        SamplerComparisonFunc::kNever
    });
}

void IrradianceConversion::OnUpdate()
{
    m_program_irradiance_convolution.vs.cbuffer.ConstantBuf.projection = glm::transpose(glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f));
}

void IrradianceConversion::OnRender(RenderCommandList& command_list)
{
    if (!is || m_settings.Get<bool>("draw_IrradianceConversion_every_frame"))
    {
        command_list.BeginEvent("DrawIrradianceConvolution");
        DrawIrradianceConvolution(command_list);
        command_list.EndEvent();

        is = true;
    }
}

void IrradianceConversion::DrawIrradianceConvolution(RenderCommandList& command_list)
{
    command_list.SetViewport(0, 0, m_input.irradince.size, m_input.irradince.size);

    command_list.UseProgram(m_program_irradiance_convolution);
    command_list.Attach(m_program_irradiance_convolution.vs.cbv.ConstantBuf, m_program_irradiance_convolution.vs.cbuffer.ConstantBuf);

    command_list.Attach(m_program_irradiance_convolution.ps.sampler.g_sampler, m_sampler);

    glm::vec4 color = { 0.0f, 0.0f, 0.0f, 1.0f };
    RenderPassBeginDesc render_pass_desc = {};
    render_pass_desc.colors[m_program_irradiance_convolution.ps.om.rtv0].texture = m_input.irradince.res;
    render_pass_desc.colors[m_program_irradiance_convolution.ps.om.rtv0].load_op = RenderPassLoadOp::kLoad;
    render_pass_desc.depth_stencil.texture = m_input.irradince.dsv;
    render_pass_desc.depth_stencil.clear_depth = 1.0f;

    m_input.model.ia.indices.Bind(command_list);
    m_input.model.ia.positions.BindToSlot(command_list, m_program_irradiance_convolution.vs.ia.POSITION);

    glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 Down = glm::vec3(0.0f, -1.0f, 0.0f);
    glm::vec3 Left = glm::vec3(-1.0f, 0.0f, 0.0f);
    glm::vec3 Right = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 ForwardRH = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 ForwardLH = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 BackwardRH = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 BackwardLH = glm::vec3(0.0f, 0.0f, -1.0f);

    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);

    glm::mat4 capture_views[] =
    {
        glm::lookAt(position, position + Right, Up),
        glm::lookAt(position, position + Left, Up),
        glm::lookAt(position, position + Up, BackwardRH),
        glm::lookAt(position, position + Down, ForwardRH),
        glm::lookAt(position, position + BackwardLH, Up),
        glm::lookAt(position, position + ForwardLH, Up)
    };

    command_list.BeginRenderPass(render_pass_desc);
    for (uint32_t i = 0; i < 6; ++i)
    {
        m_program_irradiance_convolution.vs.cbuffer.ConstantBuf.face = i;
        m_program_irradiance_convolution.vs.cbuffer.ConstantBuf.view = glm::transpose(capture_views[i]);
        command_list.Attach(m_program_irradiance_convolution.ps.srv.environmentMap, m_input.environment);
        for (auto& range : m_input.model.ia.ranges)
        {
            command_list.DrawIndexed(range.index_count, 1, range.start_index_location, range.base_vertex_location, 0);
        }
    }
    command_list.EndRenderPass();
}

void IrradianceConversion::OnModifySSSRSettings(const SSSRSettings& settings)
{
    m_settings = settings;
}
