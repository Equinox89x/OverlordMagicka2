float4x4 gWorld : WORLD;
float4x4 gWorldViewProj : WORLDVIEWPROJECTION; 
float4x4 gWorldViewProj_Light;
float3 gLightDirection = float3(-0.577f, -0.577f, 0.577f);
float gShadowMapBias = 0.01f;

Texture2D gDiffuseMap;
Texture2D gShadowMap;

SamplerComparisonState cmpSampler
{
	// sampler state
    Filter = COMPARISON_MIN_MAG_MIP_LINEAR;
    AddressU = MIRROR;
    AddressV = MIRROR;

	// sampler comparison state
    ComparisonFunc = LESS_EQUAL;
};

SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;// or Mirror or Clamp or Border
    AddressV = Wrap;// or Mirror or Clamp or Border
};

SamplerState samPoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Wrap;// or Mirror or Clamp or Border
	AddressV = Wrap;// or Mirror or Clamp or Border
};

RasterizerState Solid
{
	FillMode = SOLID;
	CullMode = FRONT;
};

struct VS_INPUT
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
	float4 lPos : TEXCOORD1;
};

DepthStencilState EnableDepth
{
	DepthEnable = TRUE;
	DepthWriteMask = ALL;
};

RasterizerState NoCulling
{
	CullMode = NONE;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	
    output.pos = mul(float4(input.pos, 1.f), gWorldViewProj);
    output.normal = input.normal;
    output.texCoord = input.texCoord;
	//Hint: Don't forget to project our position to light clip space and store it in lPos
    output.lPos = mul(float4(input.pos, 1.f), gWorldViewProj_Light);

	return output;
}

float2 texOffset(int u, int v)
{
	//TODO: return offseted value (our shadow map has the following dimensions: 1280 * 720)
    float x = float(u) / 1280.f;
    float y = float(v) / 720.f;
    return float2(x, y);

}

float EvaluateShadowMap(float4 lpos)
{
 //   //re-homogenize position after interpolation
 //   lpos.xyz /= lpos.w;
	
	////if position is not visible to the light - dont illuminate it
 //   //results in hard light frustum
 //   if (lpos.x < -1.0f || lpos.x > 1.0f ||
 //       lpos.y < -1.0f || lpos.y > 1.0f ||
 //       lpos.z < 0.0f || lpos.z > 1.0f)
 //       return 1.f;
	
 //   lpos.x = lpos.x / 2 + 0.5;
 //   lpos.y = lpos.y / -2 + 0.5;
 //   lpos.z -= gShadowMapBias;
	
	////if clip space z value greater than shadow map value then pixel is in shadow
 //   float sum = 0;
 //   float margin = 1.5f;
 //   for (float y = -margin; y <= margin; y++)
 //   {
 //       for (float x = -margin; x <= margin; x++)
 //       {
 //           sum += gShadowMap.SampleCmpLevelZero(cmpSampler, lpos.xy + texOffset(x, y), lpos.z);
 //       }
 //   }

 //   float pcf = 16.f;
 //   return (sum / pcf) * 0.5 + 0.5;
	
	
	//TODO: complete
    lpos.xyz /= lpos.w;
    if (lpos.x < -1.0f || lpos.x > 1.0f ||
	   lpos.y < -1.0f || lpos.y > 1.0f ||
	   lpos.z < 0.0f || lpos.z > 1.0f)
    {
        return 1.0f;
    }

	//transform clip space coords to texture space coords (-1:1 to 0:1)
    lpos.x = lpos.x / 2 + 0.5;
    lpos.y = lpos.y / -2 + 0.5;
	// float shadowMapDepth = gShadowMap.Sample(samPoint,input.lpos.xy).r;
	//     //if clip space z value greater than shadow map value then pixel is in shadow
	// if(shadowMapDepth < input.lpos.z) return 1.0f;

    lpos.z -= gShadowMapBias;
	    //PCF sampling for shadow map
    float sum = 0;
    float x, y;
 
    //perform PCF filtering on a 4 x 4 texel neighborhood
    for (y = -1.5; y <= 1.5; y += 1.0)
    {
        for (x = -1.5; x <= 1.5; x += 1.0)
        {
            sum += gShadowMap.SampleCmpLevelZero(cmpSampler, lpos.xy + texOffset(x, y), lpos.z);
        }
    }
 
    float shadowFactor = sum / 16.0;

    return shadowFactor;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_TARGET
{
	float shadowValue = EvaluateShadowMap(input.lPos);

	float4 diffuseColor = gDiffuseMap.Sample( samLinear,input.texCoord );
	float3 color_rgb= diffuseColor.rgb;
	float color_a = diffuseColor.a;
	
	//HalfLambert Diffuse :)
	float diffuseStrength = dot(input.normal, -gLightDirection);
	diffuseStrength = diffuseStrength * 0.5 + 0.5;
	diffuseStrength = saturate(diffuseStrength);
	color_rgb = color_rgb * diffuseStrength;

	return float4( color_rgb * shadowValue , color_a );
}

//--------------------------------------------------------------------------------------
// Technique
//--------------------------------------------------------------------------------------
technique11 Default
{
    pass P0
    {
		SetRasterizerState(NoCulling);
		SetDepthStencilState(EnableDepth, 0);

		SetVertexShader( CompileShader( vs_4_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}

