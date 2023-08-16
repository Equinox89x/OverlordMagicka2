#include "stdafx.h"
#include "MagickaCamera.h"

void MagickaCamera::Initialize(const SceneContext&)
{
	m_pCamera = new CameraComponent();
	AddComponent(m_pCamera);
}

void MagickaCamera::Update(const SceneContext& /*sceneContext*/)
{

}

void MagickaCamera::InitPoints(const std::vector<XMFLOAT3>& linePoints)
{
	m_LinePoints = linePoints;
}

void MagickaCamera::Draw(const SceneContext& /*sceneContext*/)
{
	//for (size_t i = 0; i < m_LinePoints.size()-1; i++)
	//{
	//	DebugRenderer::DrawLine(m_LinePoints[i], m_LinePoints[i+1], XMFLOAT4{1,0,0,1});
	//}
}
