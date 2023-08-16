#include "stdafx.h"
#include "DeferredLightRenderer.h"

DeferredLightRenderer::~DeferredLightRenderer()
{
	//Release Resources
}

void DeferredLightRenderer::Initialize(const D3D11Context& d3dContext)
{
	//Directional LightPass
	//...
	m_pDirectionalLightMaterial = MaterialManager::Get()->CreateMaterial<DirectionalLightMaterial>();

	//Volumetric LightPass
	//...
	m_pVolumetricLightMaterial = MaterialManager::Get()->CreateMaterial<VolumetricLightMaterial>();
	const auto inputLayoutID{ m_pVolumetricLightMaterial->GetTechniqueContext().inputLayoutID };

	//Sphere Light Mesh
	//...
	m_pSphereMesh = ContentManager::Load<MeshFilter>(L"Meshes/UnitSphere.ovm");
	m_pSphereMesh->BuildVertexBuffer(d3dContext, m_pVolumetricLightMaterial);
	m_pSphereVB = m_pSphereMesh->GetVertexBufferData(inputLayoutID).pVertexBuffer;
	m_pSphereMesh->BuildIndexBuffer(d3dContext);
	m_pSphereIB = m_pSphereMesh->GetIndexBuffer();

	//Cone Light Mesh
	//...
	m_pConeMesh = ContentManager::Load<MeshFilter>(L"Meshes/UnitCone.ovm");
	m_pConeMesh->BuildVertexBuffer(d3dContext, m_pVolumetricLightMaterial);
	auto& data{ m_pConeMesh->GetVertexBufferData(inputLayoutID) };
	m_pConeVB = data.pVertexBuffer;
	m_VertexStride = data.VertexStride;

	m_pConeMesh->BuildIndexBuffer(d3dContext);
	m_pConeIB = m_pConeMesh->GetIndexBuffer();
}

void DeferredLightRenderer::DirectionalLightPass(const SceneContext& sceneContext, ID3D11ShaderResourceView* const gbufferSRVs[]) const
{
	//Retrieve Directional light
	const auto& light = sceneContext.pLights->GetDirectionalLight();

	if(light.isEnabled)
	{
		//Prepare Effect

		//Ambient SRV > Already on Main RenderTarget
		//...
		m_pDirectionalLightMaterial->SetVariable_Texture(L"gTextureDiffuse", gbufferSRVs[int(DeferredRenderer::eGBufferId::Diffuse)]);
		m_pDirectionalLightMaterial->SetVariable_Texture(L"gTextureSpecular", gbufferSRVs[int(DeferredRenderer::eGBufferId::Specular)]);
		m_pDirectionalLightMaterial->SetVariable_Texture(L"gTextureNormal", gbufferSRVs[int(DeferredRenderer::eGBufferId::Normal)]);
		m_pDirectionalLightMaterial->SetVariable_Texture(L"gTextureDepth", gbufferSRVs[int(DeferredRenderer::eGBufferId::Depth)]);

		m_pDirectionalLightMaterial->SetVariable_Matrix(L"gMatrixViewProjInv", sceneContext.pCamera->GetViewProjectionInverse());
		m_pDirectionalLightMaterial->SetVariable_Vector(L"gEyePos", sceneContext.pCamera->GetTransform()->GetWorldPosition());
		m_pDirectionalLightMaterial->SetVariable(L"gDirectionalLight", &light, 0, sizeof(Light)-4);
		//Draw Effect (Full Screen Quad)
		//...
		QuadRenderer::Get()->Draw(m_pDirectionalLightMaterial);
	}
}

void DeferredLightRenderer::VolumetricLightPass(const SceneContext& sceneContext, ID3D11ShaderResourceView* const /*gbufferSRVs*/[], ID3D11RenderTargetView* pDefaultRTV) const
{
	//Set DefaultRTV WITH Read-Only DSV (no write access to DepthBuffer, but still able to write to StencilBuffer)
	//...
	sceneContext.d3dContext.pDeviceContext->OMSetRenderTargets(1, &pDefaultRTV, m_pReadOnlyDepthStencilView);

	//Prepare Effect

	//Ambient SRV > Already on Main RenderTarget
	//...

	//Iterate Lights & Render Volumes
	//...
}

void DeferredLightRenderer::DrawVolumetricLight(const SceneContext& /*sceneContext*/, const Light& /*light*/) const
{
	//Draw Light Volume & Apply Shading
}

void DeferredLightRenderer::CreateReadOnlyDSV(const D3D11Context& d3dContext, ID3D11Resource* pDepthResource, DXGI_FORMAT format)
{
	//Create DSV with Read-Only Depth (m_pReadOnlyDepthStencilView)
	SafeRelease(m_pReadOnlyDepthStencilView);

	D3D11_DEPTH_STENCIL_VIEW_DESC desc{};
	desc.Format = format;
	desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MipSlice = 0;
	desc.Flags = D3D11_DSV_READ_ONLY_DEPTH;

	HANDLE_ERROR(d3dContext.pDevice->CreateDepthStencilView(pDepthResource, &desc, &m_pReadOnlyDepthStencilView));
}