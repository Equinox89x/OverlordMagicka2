#include "stdafx.h"
#include "Meteorite.h"
#include "EnemyMeleeCharacter.h"
#include "EnemyRangedCharacter.h"
#include "TorusPrefab.h"
#include <Scenes/Exam/MagickaScene.h>

Meteorite::Meteorite(XMFLOAT3 forwardVector, XMFLOAT3 initialPosition, float distance, GameObject* projectileHolder, PxMaterial* pColliderMaterial, std::wstring assetFile, DiffuseMaterial_Shadow* pMaterial) :
	m_AssetFile{ assetFile },
	m_ForwardVector{ forwardVector },
	m_InitialPosition{ initialPosition },
	m_Distance{ distance },
	m_pMaterial{ pMaterial },
	m_pColliderMaterial{ pColliderMaterial },
	m_ProjectileHolder{ projectileHolder }
{
	m_pMaterial = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
	m_pMaterial->SetDiffuseTexture(L"Textures/meteor.png");
}

void Meteorite::Initialize(const SceneContext&)
{
	//AddChild(new GameObject());
	//AddComponent(new ModelComponent(m_AssetFile));
	//GetComponent<ModelComponent>()->SetMaterial(m_pMaterial);
	//m_InitialPosition.z += m_ForwardVector.z * 40;
	//m_InitialPosition.y += 30;
	//GetTransform()->Translate(m_InitialPosition);

	//auto comp{ new RigidBodyComponent() };
	//AddComponent(comp);
	//comp->SetKinematic(false);
	//comp->AddCollider(PxSphereGeometry{ 15 }, *m_pColliderMaterial);
	//comp->AddForce(m_ForwardVector);

	//auto colliderInfo = comp->GetCollider(0);
	//colliderInfo.SetTrigger(true);
	//this->SetOnTriggerCallBack([&](GameObject* /*pTriggerObject*/, GameObject* pOtherObject, PxTriggerAction action)
	//	{
	//		if (action == PxTriggerAction::ENTER)
	//		{
	//			if (auto enemy{ dynamic_cast<BaseEnemy*>(pOtherObject) }) {
	//				enemy->DamageAOE(m_DamageToGive, true);
	//			}
	//			if (m_IsBomb) { Explode(); }
	//			else {
	//				m_ShouldDelete = true;
	//				m_pDeleteTimer = 0;
	//			}
	//		}
	//	});


	m_ForwardVector.x *= m_Distance;
	m_ForwardVector.z *= m_Distance;

	m_InitialPosition.x += m_ForwardVector.x;
	m_InitialPosition.y += 100;
	m_InitialPosition.z += m_ForwardVector.z;

	auto go{ new GameObject() };
	AddChild(go);
	go->AddComponent(new ModelComponent(m_AssetFile));
	go->GetComponent<ModelComponent>()->SetMaterial(m_pMaterial);
	go->GetComponent<ModelComponent>()->GetTransform()->Scale(10);
	go->GetTransform()->Translate(m_InitialPosition);

	auto comp{ new RigidBodyComponent() };
	go->AddComponent(comp);
	comp->SetKinematic(false);
	comp->AddCollider(PxSphereGeometry{ 25 }, *m_pColliderMaterial);
	comp->AddCollider(PxSphereGeometry{ 25 }, *m_pColliderMaterial);
	comp->AddForce({ 0,-6000,0 });

	auto colliderInfo = comp->GetCollider(0);
	colliderInfo.SetTrigger(true);
	go->SetOnTriggerCallBack(
		[&](GameObject* /*pTriggerObject*/, GameObject* pOtherObject, PxTriggerAction action) {
			if (action == PxTriggerAction::ENTER)
			{
				if (auto enemy{ dynamic_cast<BaseEnemy*>(pOtherObject) }) {
					enemy->DamageAOE(300, true);
				}
				else if (auto player{ dynamic_cast<MagickaCharacter*>(pOtherObject) }) {}
				else {
					//go->GetScene()->RemoveChild(go);
					m_ProjectileHolder->RemoveChild(this);
				}
			}
		}
	);

	float value{ 150 };
	ParticleEmitterSettings settings{};
	settings.velocity = { 0.f,value, 0.f };
	settings.originalVelocity = { 0.f,value, 0.f };
	settings.modifier = { 1, 1, 1 };
	settings.minSize = 10.f;
	settings.maxSize = 10.f;
	settings.minEnergy = 1.f;
	settings.maxEnergy = 2.f;
	settings.minScale = -200.f;
	settings.maxScale = 1.5f;
	settings.minEmitterRadius = .2f;
	settings.maxEmitterRadius = .5f;
	settings.color = { 1.f,1.f,1.f, .6f };
	settings.originalVelocity = { value, 0.f, 0 };
	settings.modifier = { 1, 0.f, 0.f };
	auto component = new ParticleEmitterComponent(L"Textures/FireBall.png", settings, 200);
	component->SetShouldIncreaseOverTime(true);
	go->AddComponent(component);
}

void Meteorite::Update(const SceneContext& sceneContext)
{

	float deltaTime = sceneContext.pGameTime->GetElapsed();

	//if (m_CanMove) {
		auto pos{ GetTransform()->GetWorldPosition() };
	//	//pos.x += m_ForwardVector.x * (deltaTime * m_LaunchSpeed);
		pos.y -= (deltaTime * 100);
	//	//pos.z += m_ForwardVector.z * (deltaTime * m_LaunchSpeed);
	//	//GetTransform()->Translate(pos);

	//	//if (m_IsBomb) {
	//	//	auto pos2{ pos };
	//	//	auto colliderpos{ m_Collider->GetTransform()->GetPosition() };
	//	//	pos2.x -= colliderpos.x;
	//	//	pos2.z -= colliderpos.z;
	//	//	m_Collider->GetTransform()->Translate(colliderpos);
	//	//}

	//	//if (pos.y <= 0 && !m_ShouldDelete) {
	//	//	m_ProjectileHolder->RemoveChild(this);
	//	//}
	//}

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
