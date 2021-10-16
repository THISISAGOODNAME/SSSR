set(target SSSR)

set(include_path "${CMAKE_CURRENT_SOURCE_DIR}")
set(source_path "${CMAKE_CURRENT_SOURCE_DIR}")
set(shaders_path "${assets_path}/shaders/SSSR")

set(headers
    ${source_path}/SSSRCamera/Camera.h
    ${source_path}/Misc/Trackball.h
    ${source_path}/Misc/UtilsMath.h
    ${source_path}/Graphics/Scene.h
    ${source_path}/Graphics/SSSRSettings.h
    ${source_path}/Graphics/Passes/RenderPass.h
    ${source_path}/Graphics/Passes/ImGuiPass.h
    ${source_path}/Graphics/Passes/ImGuiSettings.h
    ${source_path}/Graphics/Passes/ForwardPBRPass.h
    ${source_path}/Graphics/Passes/Equirectangular2Cubemap.h
    ${source_path}/Graphics/Passes/BackgroundPass.h
    ${source_path}/Graphics/Passes/IrradianceConversion.h
    ${source_path}/Graphics/Passes/BRDFGen.h
)

set(sources
    ${source_path}/main.cpp
    ${source_path}/Graphics/Scene.cpp
    ${source_path}/Graphics/SSSRSettings.cpp
    ${source_path}/Graphics/Passes/ImGuiPass.cpp
    ${source_path}/Graphics/Passes/ForwardPBRPass.cpp
    ${source_path}/Graphics/Passes/Equirectangular2Cubemap.cpp
    ${source_path}/Graphics/Passes/BackgroundPass.cpp
    ${source_path}/Graphics/Passes/IrradianceConversion.cpp
    ${source_path}/Graphics/Passes/BRDFGen.cpp
)

set(shader_headers
    ${shaders_path}/common.h
)

set(pixel_shaders
    ${shaders_path}/VertexColor_PS.hlsl
    ${shaders_path}/ImGuiPass_PS.hlsl
    ${shaders_path}/ForwardPBRPass_PS.hlsl
    ${shaders_path}/Equirectangular2Cubemap_PS.hlsl
    ${shaders_path}/Background_PS.hlsl
    ${shaders_path}/IrradianceConvolution_PS.hlsl
    ${shaders_path}/Prefilter_PS.hlsl
    ${shaders_path}/BRDFlut_PS.hlsl
)

set(vertex_shaders
    ${shaders_path}/VertexColor_VS.hlsl
    ${shaders_path}/ImGuiPass_VS.hlsl
    ${shaders_path}/ForwardPBRPass_VS.hlsl
    ${shaders_path}/Cubemap_VS.hlsl
    ${shaders_path}/Background_VS.hlsl
    ${shaders_path}/BRDFlut_VS.hlsl
)

set(compute_shaders
    ${shaders_path}/DownSample_CS.hlsl
)

set_property(SOURCE ${pixel_shaders} PROPERTY VS_SHADER_ENTRYPOINT mainPS)
set_property(SOURCE ${pixel_shaders} PROPERTY VS_SHADER_TYPE Pixel)
set_property(SOURCE ${pixel_shaders} PROPERTY VS_SHADER_MODEL 6.0)
set_property(SOURCE ${pixel_shaders} PROPERTY VS_SHADER_FLAGS "/Zi /auto-binding-space 0")

set_property(SOURCE ${vertex_shaders} PROPERTY VS_SHADER_ENTRYPOINT mainVS)
set_property(SOURCE ${vertex_shaders} PROPERTY VS_SHADER_TYPE Vertex)
set_property(SOURCE ${vertex_shaders} PROPERTY VS_SHADER_MODEL 6.0)
set_property(SOURCE ${vertex_shaders} PROPERTY VS_SHADER_FLAGS "/Zi /auto-binding-space 0")

set_property(SOURCE ${compute_shaders} PROPERTY VS_SHADER_ENTRYPOINT mainCS)
set_property(SOURCE ${compute_shaders} PROPERTY VS_SHADER_TYPE Compute)
set_property(SOURCE ${compute_shaders} PROPERTY VS_SHADER_MODEL 6.0)
set_property(SOURCE ${compute_shaders} PROPERTY VS_SHADER_FLAGS "/Zi /auto-binding-space 0")

set(shaders_files ${pixel_shaders} ${vertex_shaders} ${compute_shaders})

include(${project_root}/3rdparty/FlyCube/cmake/shader_parser.cmake)

source_group("Shader Files" FILES ${shaders_files})
source_group("Shader Ref Files" FILES ${shaders_ref})

add_executable(${target} WIN32 ${headers} ${sources} ${shader_headers} ${shaders_files} ${shaders_ref})
gen_shaders_ref(${target} "${shaders_files}" shaders_ref)

if (WIN32)
    set_target_properties(${target} PROPERTIES
                            LINK_FLAGS "/ENTRY:mainCRTStartup")
endif()

target_include_directories(${target}
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/.."
        ${CMAKE_BINARY_DIR}/gen/${target}
)

target_link_libraries(${target}
    AppBox
    RenderDevice
    assimp
    imgui
    IconFontCppHeaders
    Geometry
    Texture
)

add_custom_command(TARGET ${target} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${project_root}/3rdparty/FlyCube/3rdparty/winpixeventruntime/bin/x64/WinPixEventRuntime.dll"
    "${CMAKE_CURRENT_BINARY_DIR}"
)

set_target_properties(${target} PROPERTIES FOLDER "Apps")

#set_property(TARGET ${target}  PROPERTY UNITY_BUILD ON)
#set_target_properties(${target} PROPERTIES
#    UNITY_BUILD ON
#    UNITY_BUILD_MODE BATCH
#    UNITY_BUILD_BATCH_SIZE 10
#)

install(TARGETS ${target})
