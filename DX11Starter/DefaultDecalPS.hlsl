#include "Lighting.hlsli"

#define MAX_LIGHTS 32
#define MAX_DECALS 32

struct Decal {
	matrix localTransform;
	int type;
};

// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 position		: SV_POSITION;
	float3 normal       : NORMAL;
	float2 uv           : TEXCOORD;
	float3 worldPos		: POSITION;
	float3 tangent		: TANGENT;
	float4 posForShadow : SHADOW;
};

cbuffer lightCBuffer : register(b0)
{
	Light lights[MAX_LIGHTS];
	int lightCount;
};

cbuffer uvRepeatCBuffer : register(b1) {
	float2 uvMult;
};

cbuffer externalData : register(b2) {

	int specularValue;
	float3 cameraPosition;
	int illumination;
	float3 manualColor;
};

cbuffer decalCBuffer : register(b3) {
	Decal decals[MAX_DECALS];
	int decalCount;
	matrix worldMatrix;
};

Texture2D DiffuseTexture  :  register(t0);

Texture2D ShadowMap		  : register(t1);

Texture2D Decals		: register(t2);
//Texture2D Decals[8]		: register(t2);

SamplerState BasicSampler               : register(s0);
SamplerComparisonState ShadowSampler	: register(s1);

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	input.uv = float2(input.uv.x * uvMult.x, input.uv.y * uvMult.y);

	float4 surfaceColor = DiffuseTexture.Sample(BasicSampler, input.uv);
	// Just return the input color
	// - This color (like most values passing through the rasterizer) is 
	//   interpolated for each pixel between the corresponding vertices 
	//   of the triangle we're rendering

	if (illumination == 11) {
		surfaceColor = surfaceColor.xyzw - (1 - float4(manualColor.xyz, 0));
	}

	// Shadow calculations
	float2 shadowUV = input.posForShadow.xy / input.posForShadow.w * 0.5f + 0.5f;
	shadowUV.y = 1.0f - shadowUV.y;

	// This pixel's actual depth from the light
	float depthFromLight = input.posForShadow.z / input.posForShadow.w;

	// Sample the shadow map in the same location to get
	// the closest depth along that "ray" from the light
	// (Samples the shadow map with comparison built in)
	float shadowAmount = ShadowMap.SampleCmpLevelZero(ShadowSampler, shadowUV, depthFromLight);

	//return ShadowMap.Sample(BasicSampler, shadowUV);

	float3 toCameraVector = normalize(cameraPosition - input.worldPos);

	//float3 min(-0.5f, -0.5f, -0.5f);
	//float3 max(0.5f, 0.5f, 0.5f);
	//float3 right(0.5f, -0.5f, -0.5f);
	//float3 up(-0.5f, 0.5f, -0.5f);
	float3 minPoint;
	float3 maxPoint;
	//float3 rightPoint;
	//float3 upPoint;
	float3 decalUVec;
	float3 decalVVec;
	float3 relativePixelPos;
	float decalUVecLength;
	float decalVVecLength;
	float2 decalUV;
	for (int i = 0; i < decalCount; i++)
	{
		Decal d = decals[i];
		minPoint = mul(mul(float4(-0.5f, -0.5f, -0.5f, 1.0f), d.localTransform), worldMatrix);
		maxPoint = mul(mul(float4(0.5f, 0.5f, 0.5f, 1.0f), d.localTransform), worldMatrix);
		if (input.worldPos.x >= minPoint.x && input.worldPos.x <= maxPoint.x &&
			input.worldPos.y >= minPoint.y && input.worldPos.y <= maxPoint.y &&
			input.worldPos.z >= minPoint.z && input.worldPos.z <= maxPoint.z) {
			decalUVec = mul(mul(float4(0.5f, -0.5f, -0.5f, 1.0f), d.localTransform), worldMatrix).xyz - minPoint;
			decalVVec = mul(mul(float4(-0.5f, 0.5f, -0.5f, 1.0f), d.localTransform), worldMatrix).xyz - minPoint;
			decalUVecLength = length(decalUVec);
			decalVVecLength = length(decalVVec);
			relativePixelPos = input.worldPos - minPoint;
			decalUV.x = length(dot(relativePixelPos, decalUVec) / pow(decalUVecLength, 2)) / decalUVecLength;
			decalUV.y = 1 - length(dot(relativePixelPos, decalVVec) / pow(decalVVecLength, 2)) / decalVVecLength;
			surfaceColor = Decals.Sample(BasicSampler, decalUV);
			if (surfaceColor.a < 1.0f) continue;
			//surfaceColor = Decals[d.type].Sample(BasicSampler, decalUV);
		}
	}

	float3 finalColor = float3(0.f,0.f,0.f);
	for (int i = 0; i < lightCount; i++)
	{
		switch (lights[i].Type) {
		case LIGHT_TYPE_DIR:
			finalColor += (CalcDirectionalLight(surfaceColor, input.normal, lights[i], toCameraVector, specularValue, shadowAmount));
			break;
		case LIGHT_TYPE_POINT:
			finalColor += (CalcPointLight(surfaceColor, input.normal, lights[i], toCameraVector, specularValue, input.worldPos));
			break;
		case LIGHT_TYPE_SPOT:
			finalColor += (CalcSpotLight(surfaceColor, input.normal, lights[i], toCameraVector, specularValue, input.worldPos));
			break;
		}
	}

	float3 gammaCorrect = pow(abs(finalColor), 1.0f / 2.2f);

	return float4(gammaCorrect, 1.f);
}