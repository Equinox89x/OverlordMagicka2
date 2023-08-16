#include "stdafx.h"
#include "Projectile.h"
#include "EnemyMeleeCharacter.h"
#include "EnemyRangedCharacter.h"
#include "TorusPrefab.h"

Projectile::Projectile(std::wstring assetFile, XMFLOAT3 forwardVector, XMFLOAT3 initialPosition, float launchSpeed, float downWardsVelocity, DiffuseMaterial_Shadow* pMaterial, PxMaterial* pColliderMaterial, GameObject* projectileHolder, bool isBomb) :
	m_AssetFile{ assetFile },
	m_ForwardVector{ forwardVector },
	m_InitialPosition{ initialPosition },
	m_LaunchSpeed{ launchSpeed },
	m_DownWardsVelocity{ downWardsVelocity },
	m_pMaterial{ pMaterial },
	m_pColliderMaterial{ pColliderMaterial },
	m_ProjectileHolder{ projectileHolder },
	m_IsBomb{ isBomb }
{}

void Projectile::Initialize(const SceneContext&)
{
	AddChild(new GameObject());
	AddComponent(new ModelComponent(m_AssetFile));
	GetComponent<ModelComponent>()->SetMaterial(m_pMaterial);
	m_InitialPosition.z += m_ForwardVector.z * 40;
	m_InitialPosition.y += 30;
	GetTransform()->Translate(m_InitialPosition);

	auto comp{ new RigidBodyComponent() };
	AddComponent(comp);
	comp->SetKinematic(false);
	comp->AddCollider(PxSphereGeometry{ 15 }, *m_pColliderMaterial);
	comp->AddForce(m_ForwardVector);

	auto colliderInfo = comp->GetCollider(0);
	colliderInfo.SetTrigger(true);
	this->SetOnTriggerCallBack([&](GameObject* /*pTriggerObject*/, GameObject* pOtherObject, PxTriggerAction action)
	{
		if (action == PxTriggerAction::ENTER)
		{
			if (auto enemy{ dynamic_cast<BaseEnemy*>(pOtherObject) }) {
				enemy->DamageAOE(m_DamageToGive, true);
			}
			if (m_IsBomb) { Explode(); }
			else{
				m_ShouldDelete = true;
				m_pDeleteTimer = 0;
			}
		}
	});

	if (m_IsBomb) {
		m_Collider = AddChild(new GameObject());
		auto comp2{ new RigidBodyComponent() };
		m_Collider->AddComponent(comp2);

		comp2->SetKinematic(false);
		comp2->AddCollider(PxSphereGeometry{ 50 }, *m_pColliderMaterial);
		auto colliderInfo2 = comp2->GetCollider(0);
		colliderInfo2.SetTrigger(true);
	}
}

void Projectile::Update(const SceneContext& sceneContext)
{

	float deltaTime = sceneContext.pGameTime->GetElapsed();
	
	if (m_CanMove) {
		auto pos{ GetTransform()->GetWorldPosition() };
		pos.x += m_ForwardVector.x * (deltaTime * m_LaunchSpeed);
		pos.y -= m_DownWardsVelocity;
		pos.z += m_ForwardVector.z * (deltaTime * m_LaunchSpeed);
		GetTransform()->Translate(pos);

		if (m_IsBomb) {
			auto pos2{ pos };
			auto colliderpos{ m_Collider->GetTransform()->GetPosition() };
			pos2.x -= colliderpos.x;
			pos2.z -= colliderpos.z;
			m_Collider->GetTransform()->Translate(colliderpos);
		}

		if (pos.y <= 0 && !m_ShouldDelete) {
			if (m_IsBomb) { Explode(); }
			else{ m_ProjectileHolder->RemoveChild(this); }
		}
	}

	m_pLife -= deltaTime;
	if (m_pLife <= 0) {
		m_ProjectileHolder->RemoveChild(this);
	}

	if (m_ShouldDelete) {
		m_pDeleteTimer -= deltaTime;
		if (m_pDeleteTimer <= 0) {
			m_ProjectileHolder->RemoveChild(this);
		}
	}

}

void Projectile::Explode()
{

	//m_CanExplode = true;
	RemoveComponent(GetComponent<ModelComponent>(), true);
	AddChild(new TorusPrefab(m_ExplosionRadius, int(m_ExplosionRadius), m_ExplosionRadius/2, int(m_ExplosionRadius), XMFLOAT4{ 1,0,0,1 }));
	GetChild<TorusPrefab>()->GetTransform()->Rotate(90, 0, 0);
	m_ShouldDelete = true;
	m_CanMove = false;

	for (auto character : m_Collider->GetEnemiesInRange())
	{
		if (auto enemy{ dynamic_cast<BaseEnemy*>(character) }) enemy->DamageAOE(m_DamageToGive, true);
	}
	if (auto character{ m_Collider->GetCharacterInRange<MagickaCharacter>() })	character->DamagePlayer(true, m_DamageToGive);

	auto pos{ GetTransform()->GetWorldPosition() };
	pos.y = 0;
	GetTransform()->Translate(pos);
}
