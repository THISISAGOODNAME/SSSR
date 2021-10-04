#include "ForwardPBRPass.h"

#include <IconsForkAwesome.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Geometry/IABuffer.h>
#include <Utilities/FormatHelper.h>

ForwardPBRPass::ForwardPBRPass(RenderDevice& device, RenderCommandList& command_list, const Input& input, int width, int height)
    : m_device(device)
    , m_input(input)
    , m_width(width)
    , m_height(height)
    , m_program(device)
{
    CreateSizeDependentResources();
    m_sampler = m_device.CreateSampler({
        SamplerFilter::kMinMagMipLinear,
        SamplerTextureAddressMode::kWrap,
        SamplerComparisonFunc::kAlways });

    m_sampler_brdflut = m_device.CreateSampler({
        SamplerFilter::kMinMagMipLinear,
        SamplerTextureAddressMode::kClamp,
        SamplerComparisonFunc::kNever });

    // lights
    // ------
    glm::vec4 lightPositions[] = {
            glm::vec4(-10.0f,  10.0f, 10.0f, 1.0f),
            glm::vec4( 10.0f,  10.0f, 10.0f, 1.0f),
            glm::vec4(-10.0f, -10.0f, 10.0f, 1.0f),
            glm::vec4( 10.0f, -10.0f, 10.0f, 1.0f),
            };
    glm::vec4 lightColors[] = {
            glm::vec4(300.0f, 300.0f, 300.0f, 1.0f),
            glm::vec4(300.0f, 300.0f, 300.0f, 1.0f),
            glm::vec4(300.0f, 300.0f, 300.0f, 1.0f),
            glm::vec4(300.0f, 300.0f, 300.0f, 1.0f)
    };
    for (int i = 0; i < 4; i++)
    {
        m_program.ps.cbuffer.Lights.lightPositions[i] = lightPositions[i];
        m_program.ps.cbuffer.Lights.lightColors[i] = lightColors[i];
    }
}

ForwardPBRPass::~ForwardPBRPass()
{
}

void ForwardPBRPass::OnUpdate()
{
    m_program.vs.cbuffer.MVP.view = m_input.camera.getViewMatrix();
    m_program.vs.cbuffer.MVP.proj = m_input.camera.getProjMatrix();

    m_program.ps.cbuffer.PerframeData.camPos = m_input.camera.getPosition();

    m_program.ps.cbuffer.Settings.use_IBL_diffuse = m_settings.Get<bool>("use_IBL_diffuse");
    m_program.ps.cbuffer.Settings.use_IBL_specular = m_settings.Get<bool>("use_IBL_specular");
    m_program.ps.cbuffer.Settings.use_f0_with_roughness = m_settings.Get<bool>("use_f0_with_roughness");
    m_program.ps.cbuffer.Settings.use_spec_ao_by_ndotv_roughness = m_settings.Get<bool>("use_spec_ao_by_ndotv_roughness");
    m_program.ps.cbuffer.Settings.only_ambient = m_settings.Get<bool>("only_ambient");
}

void ForwardPBRPass::OnRender(RenderCommandList& command_list)
{
    command_list.SetViewport(0, 0, m_width, m_height);

    command_list.UseProgram(m_program);

    command_list.Attach(m_program.ps.cbv.Settings, m_program.ps.cbuffer.Settings);
    command_list.Attach(m_program.ps.sampler.g_sampler, m_sampler);
    command_list.Attach(m_program.ps.sampler.brdf_sampler, m_sampler_brdflut);

    RenderPassBeginDesc render_pass_desc = {};
    render_pass_desc.colors[m_program.ps.om.rtv0].texture = m_input.rtv;
    render_pass_desc.colors[m_program.ps.om.rtv0].clear_color = { 0.0f, 0.2f, 0.4f, 1.0f };
    render_pass_desc.depth_stencil.texture = output.dsv;
    render_pass_desc.depth_stencil.clear_depth = 1.0f;

    m_program.ps.cbuffer.Material.albedo = {0.5f, 0.0f, 0.0f};
    m_program.ps.cbuffer.Material.ao = 1.0f;

    int nrRows    = 7;
    int nrColumns = 7;
    float spacing = 2.5;
    glm::mat4 model = glm::mat4(1.0f);
    command_list.BeginRenderPass(render_pass_desc);
    for (int row = 0; row < nrRows; row++)
    {
        m_program.ps.cbuffer.Material.metallic = (float)row / (float)nrRows;
        for (int col = 0; col < nrColumns; col++)
        {
            m_program.ps.cbuffer.Material.roughness = glm::clamp((float)col / (float)nrColumns, 0.05f, 1.0f);

            model = glm::mat4(1.0f);
            //model = glm::scale(model, glm::vec3(0.05f));
            model = glm::translate(model, glm::vec3(
                    (col - (nrColumns / 2)) * spacing,
                    (row - (nrRows / 2)) * spacing,
                    0.0f
                    ));
            model = glm::scale(model, glm::vec3(0.05f));

            m_program.vs.cbuffer.MVP.model = model;

            command_list.Attach(m_program.vs.cbv.MVP, m_program.vs.cbuffer.MVP);

            command_list.Attach(m_program.ps.cbv.Lights, m_program.ps.cbuffer.Lights);
            command_list.Attach(m_program.ps.cbv.PerframeData, m_program.ps.cbuffer.PerframeData);
            command_list.Attach(m_program.ps.cbv.Material, m_program.ps.cbuffer.Material);

            command_list.Attach(m_program.ps.srv.irradianceMap, m_input.irradince);
            command_list.Attach(m_program.ps.srv.prefilterMap, m_input.prefilter);
            command_list.Attach(m_program.ps.srv.brdfLUT, m_input.brdflut);

            m_input.sphereModel.ia.indices.Bind(command_list);
            m_input.sphereModel.ia.positions.BindToSlot(command_list, m_program.vs.ia.POSITION);
            m_input.sphereModel.ia.normals.BindToSlot(command_list, m_program.vs.ia.NORMAL);
            m_input.sphereModel.ia.texcoords.BindToSlot(command_list, m_program.vs.ia.TEXCOORD);
            //m_input.sphereModel.ia.tangents.BindToSlot(command_list, m_program.vs.ia.TANGENT);

            for (auto& range : m_input.sphereModel.ia.ranges)
            {
                command_list.DrawIndexed(range.index_count, 1, range.start_index_location, range.base_vertex_location, 0);
            }
        }
    }
    command_list.EndRenderPass();
}

void ForwardPBRPass::OnResize(int width, int height)
{
    m_width = width;
    m_height = height;
    CreateSizeDependentResources();
}

void ForwardPBRPass::CreateSizeDependentResources()
{
    output.dsv = m_device.CreateTexture(BindFlag::kDepthStencil, gli::format::FORMAT_D32_SFLOAT_PACK32, 1, m_width, m_height, 1);
}

void ForwardPBRPass::OnModifySSSRSettings(const SSSRSettings &settings)
{
    m_settings = settings;
}
