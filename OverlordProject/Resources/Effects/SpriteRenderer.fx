float4x4 gTransform : WorldViewProjection;
Texture2D gSpriteTexture;
float2 gTextureSize;

SamplerState samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = WRAP;
    AddressV = WRAP;
};

BlendState EnableBlending
{
    BlendEnable[0] = TRUE;
    SrcBlend = SRC_ALPHA;
    DestBlend = INV_SRC_ALPHA;
};

DepthStencilState NoDepth
{
    DepthEnable = FALSE;
};

RasterizerState BackCulling
{
    CullMode = BACK;
};

//SHADER STRUCTS
//**************
struct VS_DATA
{
    uint TextureId : TEXCOORD0;
    float4 TransformData : POSITION; //PosX, PosY, Depth (PosZ), Rotation
    float4 TransformData2 : POSITION1; //PivotX, PivotY, ScaleX, ScaleY
    float4 Color : COLOR;
};

struct GS_DATA
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD0;
};

//VERTEX SHADER
//*************
VS_DATA MainVS(VS_DATA input)
{
    return input;
}

//GEOMETRY SHADER
//***************
void CreateVertex(inout TriangleStream<GS_DATA> triStream, float3 pos, float4 color, float2 texCoord, float rotation, float2 rotCosSin, float2 offset, float2 pivotOffset)
{
    if (rotation != 0)
    {
		//Step 3.
		//Do rotation calculations
		//Transform to origin
        pos.x = pos.x - pivotOffset.x - offset.x;
        pos.y = pos.y - pivotOffset.y - offset.y;
        float x = pos.x;
        float y = pos.y;
		//Rotate
        pos.x = x * rotCosSin.x - y * rotCosSin.y;
        pos.y = y * rotCosSin.x + x * rotCosSin.y;
		//Retransform to initial position
        pos.x = pos.x + offset.x;
        pos.y = pos.y + offset.y;
    }
    else
    {
		//Step 2.
		//No rotation calculations (no need to do the rotation calculations if there is no rotation applied > redundant operations)
		//Just apply the pivot offset
        pos.x = pos.x - pivotOffset.x;
        pos.y = pos.y - pivotOffset.y;
    }

    //Geometry Vertex Output
    GS_DATA geomData = (GS_DATA) 0;
    geomData.Position = mul(float4(pos, 1.0f), gTransform);
    geomData.Color = color;
    geomData.TexCoord = texCoord;
    triStream.Append(geomData);
}

[maxvertexcount(4)]
void MainGS(point VS_DATA vertex[1], inout TriangleStream<GS_DATA> triStream)
{
	//Given Data (Vertex Data)
    float2 position = float2(vertex[0].TransformData.x, vertex[0].TransformData.y); //Extract the position data from the VS_DATA vertex struct
    float depth = vertex[0].TransformData.z; //Extract the position data from the VS_DATA vertex struct
    float2 offset = float2(vertex[0].TransformData.x, vertex[0].TransformData.y); //Extract the offset data from the VS_DATA vertex struct (initial X and Y position)
    float rotation = vertex[0].TransformData.w; //Extract the rotation data from the VS_DATA vertex struct
    float2 pivot = float2(vertex[0].TransformData2.x, vertex[0].TransformData2.y); //Extract the pivot data from the VS_DATA vertex struct
    float2 scale = float2(vertex[0].TransformData2.z, vertex[0].TransformData2.w); //Extract the scale data from the VS_DATA vertex struct  

	
    pivot.xy *= gTextureSize.xy * scale.xy;
    
    float2 rotCosSin;
    if (rotation == 0)
    {
        rotCosSin = float2(0, 0);
    }
    else
    {
        rotCosSin = float2(cos(rotation), sin(rotation));
    }
    
    
    //...

	// LT----------RT //TringleStrip (LT > RT > LB, LB > RB > RT)
	// |          / |
	// |       /    |
	// |    /       |
	// | /          |
	// LB----------RB

    
	//VERTEX 1 [LT]
    float3 pos = float3(position + float2(0, 0) * scale, depth);
    float2 texCoord = float2(0, 0);
    CreateVertex(triStream, pos, float4(1, 1, 1, 1), texCoord, rotation, rotCosSin, offset, pivot);
    
    //VERTEX 2 [RT]
    pos = float3(position + float2(gTextureSize.x, 0) * scale, depth);
    texCoord = float2(clamp(gTextureSize.x, 0, 1), 0);
    CreateVertex(triStream, pos, float4(1, 1, 1, 1), texCoord, rotation, rotCosSin, offset, pivot);

    //VERTEX 3 [LB]
    pos = float3(position + float2(0, gTextureSize.y) * scale, depth);
    texCoord = float2(0, clamp(gTextureSize.y, 0, 1));
    CreateVertex(triStream, pos, float4(1, 1, 1, 1), texCoord, rotation, rotCosSin, offset, pivot);

    //VERTEX 4 [RB]
    pos = float3(position + float2(gTextureSize.x, gTextureSize.y) * scale, depth);
    texCoord = float2(clamp(gTextureSize.x, 0, 1), clamp(gTextureSize.y, 0, 1));
    CreateVertex(triStream, pos, float4(1, 1, 1, 1), texCoord, rotation, rotCosSin, offset, pivot);

}

//PIXEL SHADER
//************
float4 MainPS(GS_DATA input) : SV_TARGET
{

    return gSpriteTexture.Sample(samPoint, input.TexCoord) * input.Color;
}

// Default Technique
technique11 Default
{
    pass p0
    {
        SetRasterizerState(BackCulling);
        SetBlendState(EnableBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		//SetDepthStencilState(NoDepth,0);
        SetVertexShader(CompileShader(vs_4_0, MainVS()));
        SetGeometryShader(CompileShader(gs_4_0, MainGS()));
        SetPixelShader(CompileShader(ps_4_0, MainPS()));
    }
}
