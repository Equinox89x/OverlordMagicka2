#include "stdafx.h"
#include "CameraComponent.h"

CameraComponent::CameraComponent() :
	m_FarPlane(2500.0f),
	m_NearPlane(0.1f),
	m_FOV(XM_PIDIV4),
	m_Size(25.0f),
	m_PerspectiveProjection(true)
{
	XMStoreFloat4x4(&m_Projection, XMMatrixIdentity());
	XMStoreFloat4x4(&m_View, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ViewInverse, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ViewProjection, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ViewProjectionInverse, XMMatrixIdentity());
}

void CameraComponent::Update(const SceneContext& sceneContext)
{
	// see https://stackoverflow.com/questions/21688529/binary-directxxmvector-does-not-define-this-operator-or-a-conversion
	using namespace DirectX;

	XMMATRIX projection{};

	if (m_PerspectiveProjection)
	{
		projection = XMMatrixPerspectiveFovLH(m_FOV, sceneContext.aspectRatio, m_NearPlane, m_FarPlane);
	}
	else
	{
		const float viewWidth = (m_Size > 0) ? m_Size * sceneContext.aspectRatio : sceneContext.windowWidth;
		const float viewHeight = (m_Size > 0) ? m_Size : sceneContext.windowHeight;
		projection = XMMatrixOrthographicLH(viewWidth, viewHeight, m_NearPlane, m_FarPlane);
	}

	const XMVECTOR worldPosition = XMLoadFloat3(&GetTransform()->GetWorldPosition());
	const XMVECTOR lookAt = XMLoadFloat3(&GetTransform()->GetForward());
	const XMVECTOR upVec = XMLoadFloat3(&GetTransform()->GetUp());

	const XMMATRIX view = XMMatrixLookAtLH(worldPosition, worldPosition + lookAt, upVec);
	const XMMATRIX viewInv = XMMatrixInverse(nullptr, view);
	const XMMATRIX viewProjectionInv = XMMatrixInverse(nullptr, view * projection);

	XMStoreFloat4x4(&m_Projection, projection);
	XMStoreFloat4x4(&m_View, view);
	XMStoreFloat4x4(&m_ViewInverse, viewInv);
	XMStoreFloat4x4(&m_ViewProjection, view * projection);
	XMStoreFloat4x4(&m_ViewProjectionInverse, viewProjectionInv);
}

void CameraComponent::SetActive(bool active)
{
	if (m_IsActive == active) return;

	const auto pGameObject = GetGameObject();
	ASSERT_IF(!pGameObject, L"Failed to set active camera. Parent game object is null");

	if (!pGameObject) return; //help the compiler... (C6011)
	const auto pScene = pGameObject->GetScene();
	ASSERT_IF(!pScene, L"Failed to set active camera. Parent game scene is null");

	m_IsActive = active;
	pScene->SetActiveCamera(active?this:nullptr); //Switch to default camera if active==false
}

GameObject* CameraComponent::Pick(CollisionGroup ignoreGroups) const
{
	SceneContext sceneContext{ m_pScene->GetSceneContext() };
	POINT mousePos{ sceneContext.pInput->GetMousePosition() };

	D3D11_VIEWPORT viewport{};
	UINT count{ 1 };
	sceneContext.d3dContext.pDeviceContext->RSGetViewports(&count, &viewport);
	float halfWidth{ viewport.Width * .5f };
	float halfHeight{ viewport.Height * .5f };

	XMFLOAT2 ndcPos{ XMFLOAT2(
		(float(mousePos.x) - halfWidth) / halfWidth, 
		(halfHeight - float(mousePos.y)) / halfHeight) 
	};

	XMFLOAT4 nearPlaneValue{ ndcPos.x, ndcPos.y, 0.f, 0.f };
	XMFLOAT4 farPlaneValue{ ndcPos.x, ndcPos.y, 1.f, 0.f };
	XMVECTOR nearPlane{ XMLoadFloat4(&nearPlaneValue) };
	XMVECTOR farPlane{ XMLoadFloat4(&farPlaneValue) };

	XMMATRIX viewProjectionInverse{ XMLoadFloat4x4(&m_ViewProjectionInverse) };
	XMFLOAT4 nearPointValue{};
	XMFLOAT4 farPointValue{};

	XMVECTOR nearPoint{ XMVector3TransformCoord(nearPlane, viewProjectionInverse) };
	XMVECTOR farPoint{ XMVector3TransformCoord(farPlane, viewProjectionInverse) };
	XMStoreFloat4(&nearPointValue, nearPoint);
	XMStoreFloat4(&farPointValue, farPoint);

	PxVec3 start{ PxVec3{ nearPointValue.x, nearPointValue.y, nearPointValue.z } };
	PxVec3 end{ PxVec3{ farPointValue.x, farPointValue.y, farPointValue.z } };

	PxQueryFilterData filterData{};
	filterData.data.word0 = ~UINT(ignoreGroups);

	PxRaycastBuffer hit{};
	GameScene* scene{ SceneManager::Get()->GetActiveScene() };
	auto direction{ end - start };
	if (scene->GetPhysxProxy()->Raycast(start, direction.getNormalized(), direction.magnitude(), hit, PxHitFlag::eDEFAULT, filterData))
	{
		auto const comp{ hit.block.actor->userData };
		GameObject* gameObject{ reinterpret_cast<BaseComponent*>(comp)->GetGameObject() };
		return gameObject;
	}

	return nullptr;
}

XMFLOAT3 CameraComponent::PickPosition(CollisionGroup ignoreGroups) const
{
	SceneContext sceneContext{ m_pScene->GetSceneContext() };
	POINT mousePos{ sceneContext.pInput->GetMousePosition() };

	D3D11_VIEWPORT viewport{};
	UINT count{ 1 };
	sceneContext.d3dContext.pDeviceContext->RSGetViewports(&count, &viewport);
	float halfWidth{ viewport.Width * .5f };
	float halfHeight{ viewport.Height * .5f };

	XMFLOAT2 ndcPos{ XMFLOAT2(
		(float(mousePos.x) - halfWidth) / halfWidth,
		(halfHeight - float(mousePos.y)) / halfHeight)
	};

	XMFLOAT4 nearPlaneValue{ ndcPos.x, ndcPos.y, 0.f, 0.f };
	XMFLOAT4 farPlaneValue{ ndcPos.x, ndcPos.y, 1.f, 0.f };
	XMVECTOR nearPlane{ XMLoadFloat4(&nearPlaneValue) };
	XMVECTOR farPlane{ XMLoadFloat4(&farPlaneValue) };

	XMMATRIX viewProjectionInverse{ XMLoadFloat4x4(&m_ViewProjectionInverse) };
	XMFLOAT4 nearPointValue{};
	XMFLOAT4 farPointValue{};

	XMVECTOR nearPoint{ XMVector3TransformCoord(nearPlane, viewProjectionInverse) };
	XMVECTOR farPoint{ XMVector3TransformCoord(farPlane, viewProjectionInverse) };
	XMStoreFloat4(&nearPointValue, nearPoint);
	XMStoreFloat4(&farPointValue, farPoint);

	PxVec3 start{ PxVec3{ nearPointValue.x, nearPointValue.y, nearPointValue.z } };
	PxVec3 end{ PxVec3{ farPointValue.x, farPointValue.y, farPointValue.z } };

	PxQueryFilterData filterData{};
	filterData.data.word0 = ~UINT(ignoreGroups);

	PxRaycastBuffer hit{};
	GameScene* scene{ SceneManager::Get()->GetActiveScene() };
	auto direction{ end - start };
	if (scene->GetPhysxProxy()->Raycast(start, direction.getNormalized(), direction.magnitude(), hit, PxHitFlag::eDEFAULT, filterData))
	{
		return XMFLOAT3{ hit.block.position.x, hit.block.position.y, hit.block.position.z };
	}

	return XMFLOAT3{1,1,1};
}
