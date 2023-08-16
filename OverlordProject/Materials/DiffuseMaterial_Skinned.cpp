#include "stdafx.h"
#include "DiffuseMaterial_Skinned.h"

DiffuseMaterial_Skinned::DiffuseMaterial_Skinned():
	Material(L"Effects/PosNormTex3D_Skinned.fx")
{
}

void DiffuseMaterial_Skinned::SetDiffuseTexture(const std::wstring& assetFile)
{
	m_pDiffuseTexture = ContentManager::Load<TextureData>(assetFile);
	SetVariable_Texture(L"gDiffuseMap", m_pDiffuseTexture);
}

void DiffuseMaterial_Skinned::InitializeEffectVariables()
{
}

void DiffuseMaterial_Skinned::OnUpdateModelVariables(const SceneContext&, const ModelComponent* pModel) const
{
	auto animator{ pModel->GetAnimator() };
	ASSERT_NULL_(animator);
	auto& transforms{ animator->GetBoneTransforms() };
	auto transformsData{ transforms.data() };
	SetVariable_MatrixArray(L"gBones", reinterpret_cast<const float*>(transformsData), static_cast<UINT>(transforms.size()));
}
