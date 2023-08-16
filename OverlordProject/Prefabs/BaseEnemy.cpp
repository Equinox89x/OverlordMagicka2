#include "stdafx.h"
#include "BaseEnemy.h"
#include "../OverlordEngine/Prefabs/MagickaCamera.h"
#include <Scenes/Exam/MagickaScene.h>
#include "MagickaCharacter.h"

BaseEnemy::BaseEnemy(const CharacterDescExtended& characterDesc, XMFLOAT3 postitionOffset) :
	m_CharacterDescExtended{ characterDesc },
	PostitionOffset{ postitionOffset }
{
}

void BaseEnemy::Initialize(const SceneContext& sceneContext)
{
	GetTransform()->Translate(PostitionOffset);

	InitializeChild(sceneContext);
}

void BaseEnemy::Update(const SceneContext& sceneContext)
{
	float deltaTime = sceneContext.pGameTime->GetElapsed();
	//constexpr float epsilon{ 0.01f };

	if (CanDamage) {
		if (DamageTimer <= 0) {
			DamageTimer = DefaultDamageTimer;
			m_Health -= DamageToTake;
			std::cout << m_Health << "\n";
		}
		else {
			DamageTimer -= deltaTime;
		}
	}


	auto originalRotation{ GetTransform()->GetWorldRotation() };
	auto originalLocation{ GetTransform()->GetWorldPosition() };
	if (!m_pCharacter->GetTransform()) return;
	auto lookPosition{ m_pCharacter->GetTransform()->GetWorldPosition() };

	XMFLOAT3 newRot = MathHelper::GetRotationTowardsPoint(originalLocation, lookPosition, originalRotation, false);
	GetComponent<ModelComponent>()->GetTransform()->Rotate(newRot, false);

	if (CanMove && CanMoveGeneral) {
		//get forward vector
		if (!MathHelper::IsPointInCircle3D(originalLocation, lookPosition, 25)) {
			XMFLOAT3 forward{ GetTransform()->GetForward() };
			originalLocation.x += forward.x * (deltaTime * m_CharacterDescExtended.maxMoveSpeed);
			originalLocation.z += forward.z * (deltaTime * m_CharacterDescExtended.maxMoveSpeed);
			GetTransform()->Translate(originalLocation);
		}
	}

	for (auto projectile : GetChildren<Projectile>()) {
		if (projectile->IsMarkedForDelete()) {
			RemoveChild(projectile, true);
		}
	}
	UpdateChild(sceneContext);

}

void BaseEnemy::DrawImGui()
{

}

void BaseEnemy::DamageBeamEnter(float damage)
{
	CanDamage = true;
	DamageTimer = 0;
	DamageToTake = damage;
}

void BaseEnemy::DamageBeamExit()
{
	CanDamage = false;
	DamageTimer = DefaultDamageTimer;
	DamageToTake = 0;
}

void BaseEnemy::DamageAOE(float damage)
{
	if (CanDamageAoE) m_Health -= damage;
	CanDamageAoE = false;
	std::cout << m_Health << "\n";
}

void BaseEnemy::DamageAOE(float damage, bool canDamage)
{
	CanDamageAoE = canDamage;
	if (CanDamageAoE) m_Health -= damage;
	CanDamageAoE = false;
	std::cout << m_Health << "\n";
}
