struct VS_OUTPUT
{
    float4 pos: SV_POSITION;
    float3 worldPos : World_Pos;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

cbuffer Material
{
    float3 albedo;
    float metallic;
    float roughness;
    float ao;
};

cbuffer Lights
{
    float4 lightPositions[4];
    float4 lightColors[4];
};

cbuffer PerframeData
{
    float3 camPos;
};

#define PI 3.14159265359f
// ----------------------------------------------------------------------------
float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float4 mainPS(VS_OUTPUT input) : SV_TARGET
{
//     return float4(input.uv, 0.0f, 1.0f);

    float3 N = normalize(input.normal);
    float3 V = normalize(camPos - input.worldPos);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
    float3 F0 = (0.04).xxx;
    F0 = lerp(F0, albedo, metallic);
//     F0 = smoothstep(F0, albedo, metallic);

    // reflectance equation
    float3 Lo = (0.0).xxx;
    for(int i = 0; i < 4; ++i)
    {
        // calculate per-light radiance
        float3 L = normalize(lightPositions[i].xyz - input.worldPos);
        float3 H = normalize(V + L);
        float distance = length(lightPositions[i].xyz - input.worldPos);
        float attenuation = 1.0 / (distance * distance);
        float3 radiance = lightColors[i].xyz * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        float3 F  = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

        float3 numerator = NDF * G * F;
        float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        float3 specular = numerator / denominator;

        // kS is equal to Fresnel
        float3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        float3 kD = (1.0).xxx - kS;
        // multiply kD by the inverse metalness such that only non-metals
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metallic;

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);

        // add to outgoing radiance Lo
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }

    // ambient lighting (note that the next IBL tutorial will replace
    // this ambient lighting with environment lighting).
    float3 ambient = (0.03).xxx * albedo * ao;

    float3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + (1.0).xxx);
    // gamma correct
    color = pow(color, (1.0/2.2).xxx);

    return float4(color, 1.0);
//     return float4(N, 1.0f);
//     return float4(input.normal, 1.0f);
//     return float4(input.worldPos, 1.0f);
}
