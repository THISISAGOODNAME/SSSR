struct VS_INPUT
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 pos: SV_POSITION;
    float3 worldPos : World_Pos;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

cbuffer MVP
{
    float4x4 model;
    float4x4 view;
    float4x4 proj;
};

VS_OUTPUT mainVS(VS_INPUT input)
{
    VS_OUTPUT output;
    output.pos = mul(proj, mul(view, mul(model, float4(input.pos, 1.0f))));
    output.worldPos = mul(model, float4(input.pos, 1.0f)).xyz;
    output.normal = input.normal;
    output.uv = input.uv;
    return output;
}