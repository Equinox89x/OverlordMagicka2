#include "stdafx.h"
#include "SkyboxMaterial.h"

SkyboxMaterial::SkyboxMaterial() : Material(L"Effects/Post/Skybox.fx")
{
}


void SkyboxMaterial::SetCubeMapTexture(const std::wstring& assetFile)
{
	m_pSkyboxTexture = ContentManager::Load<TextureData>(assetFile);
	InitializeEffectVariables();
}


void SkyboxMaterial::InitializeEffectVariables()
{
	if (m_pSkyboxTexture)
	{
		SetVariable_Texture(L"m_CubeMap", m_pSkyboxTexture->GetShaderResourceView());
	}
}
