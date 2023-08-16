#include "stdafx.h"
#include "DiffuseMaterial_Shadow_Skinned.h"

DiffuseMaterial_Shadow_Skinned::DiffuseMaterial_Shadow_Skinned():
	Material(L"Effects/Shadow/PosNormTex3D_Shadow_Skinned.fx")
{}

void DiffuseMaterial_Shadow_Skinned::SetDiffuseTexture(const std::wstring& assetFile)
{
	m_pDiffuseTexture = ContentManager::Load<TextureData>(assetFile);
	SetVariable_Texture(L"gDiffuseMap", m_pDiffuseTexture);
}

void DiffuseMaterial_Shadow_Skinned::InitializeEffectVariables()
{
}

void DiffuseMaterial_Shadow_Skinned::OnUpdateModelVariables(const SceneContext& sceneContext, const ModelComponent* pModel) const
{
	//Update The Shader Variables
	//1. Update the LightWVP > Used to Transform a vertex into Light clipping space
	//LightWVP = model_world * light_viewprojection
	//(light_viewprojection [LightVP] can be acquired from the ShadowMapRenderer) */
	XMFLOAT4X4 lightWVP{};
	auto shadowMapRenderer{ ShadowMapRenderer::Get() };
	auto world{ XMLoadFloat4x4(&pModel->GetTransform()->GetWorld())};
	auto lightVP{ XMLoadFloat4x4(&shadowMapRenderer->GetLightVP())};

	//XMStoreFloat4x4(&lightWVP, XMMatrixMultiply(world, lightVP));
	//2. Update the ShadowMap texture
	SetVariable_Matrix(L"gWorldViewProj_Light", lightWVP);
	SetVariable_Texture(L"gShadowMap", shadowMapRenderer->GetShadowMap());
	 
	// 3. Update the Light Direction (retrieve the direction from the LightManager > sceneContext)
	auto direction = sceneContext.pLights->GetDirectionalLight().direction;
	SetVariable_Vector(L"gLightDirection", direction);

	//4. Update Bones
	//Update Shadow Variables
	//const auto pShadowMapRenderer = ShadowMapRenderer::Get();
	auto animator = pModel->GetAnimator();
	if (animator != nullptr)
	{
		auto boneTrans = animator->GetBoneTransforms();
		SetVariable_MatrixArray(L"gBones", reinterpret_cast<const float*>(boneTrans.data()), (UINT)boneTrans.size());
	}
}
