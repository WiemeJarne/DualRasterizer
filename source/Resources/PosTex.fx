//// Input/Output Structs
struct VS_INPUT
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float2 TextCoord : TEXTCOORD;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float4 WorldPosition : WORLD_POSITION;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float2 TextCoord : TEXTCOORD;
};

BlendState gBlendState
{
	BlendEnable[0] = false;
	SrcBlend = one;
	DestBlend = zero;
	BlendOp = add;
	SrcBlendAlpha = one;
	DestBlendAlpha = zero;
	BlendOpAlpha = add;
	RenderTargetWriteMask[0] = 0x0F;
};

DepthStencilState gDepthStencilState
{
	DepthEnable = true;
	DepthWriteMask = all;
	DepthFunc = less;
	StencilEnable = false;

	StencilReadMask = 0xFF;
	StencilWriteMask = 0xFF;

	FrontFaceStencilFunc = always;
	BackFaceStencilFunc = always;

	FrontFaceStencilDepthFail = keep;
	BackFaceStencilDepthFail = keep;

	FrontFaceStencilPass = keep;
	BackfaceStencilPass = keep;

	FrontFaceStencilFail = keep;
	BackFaceStencilFail = keep;
};

SamplerState gSamplerState : Sampler;
float4x4 gWorld : World;
float4x4 gWorldViewProj : WorldViewProjection;
float4x4 gViewInverse : ViewInverse;
Texture2D gDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossinessMap : GlossinessMap;
float3 gLightDirection = float3(0.577f, -0.577f, 0.577f);
float gPI = 3.14159265358979f;
float gLightIntensity = 7.f;
float gShininess = 25.f;
float4 gAmbient = float4(0.025f, 0.025f, 0.025f, 1.f);


// Vertex Shader
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
	output.WorldPosition = mul(float4(input.Position, 1.f), gWorld);
	output.Normal = mul(normalize(input.Normal), (float3x3)gWorld);
	output.Tangent = mul(normalize(input.Tangent), (float3x3)gWorld); 
	output.TextCoord = input.TextCoord;
	return output;
}

float4 Diffuse(float kd, float4 cd)
{
	return cd * kd / gPI;
}

float Phong(float ks, float exp, float3 l, float3 v, float3 n)
{
	return ks * pow(saturate(dot(reflect(l, n), v)), exp);
}

// Pixel Shader
float4 PS(VS_OUTPUT input) : SV_TARGET
{
	float3 binormal = cross(input.Normal, input.Tangent);

	float3x3 tangentSpaceAxis = float3x3(input.Tangent, binormal, input.Normal);

	float3 sampledNormal = mul(normalize(2.f * gNormalMap.Sample(gSamplerState, input.TextCoord).xyz - 1.f), tangentSpaceAxis).xyz;
	
	float observedArea = saturate(dot(sampledNormal, -gLightDirection));

	float3 viewDirection = normalize(input.WorldPosition.xyz - gViewInverse[3].xyz);
	
	float4 diffuse = gLightIntensity * Diffuse(1.f, gDiffuseMap.Sample(gSamplerState, input.TextCoord));
	float4 specular = gSpecularMap.Sample(gSamplerState, input.TextCoord) * Phong(1.f, gShininess * gGlossinessMap.Sample(gSamplerState, input.TextCoord).r, -gLightDirection, viewDirection, sampledNormal);
	
	return observedArea * (diffuse + specular + gAmbient);
}

// Technique
technique11 DefaultTechnique
{
	pass P0
	{
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PS() ) );
	}
}