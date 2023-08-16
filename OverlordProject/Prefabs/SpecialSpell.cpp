#include "stdafx.h"

#include "./Scenes/Exam/MagickaScene.h"
#include "SpecialSpell.h"
#include "./Prefabs/Meteorite.h"

void Teleport::ExecuteSpell()  {
	auto transform{ m_Character->GetTransform() };
	auto fwd{ transform->GetForward() };
	fwd.x *= m_Distance;
	fwd.z *= m_Distance;

	auto pos{ transform->GetWorldPosition() };
	pos.x += fwd.x;
	pos.z += fwd.z;
	transform->Translate(pos);
}

void Thunderbolt::ExecuteSpell() {
	auto transform{ m_Character->GetTransform() };
	auto fwd{ transform->GetForward() };
	auto pos{ transform->GetWorldPosition() };

	
	m_ProjectileHolder->AddChild(new Meteorite(fwd, pos, m_Distance, m_ProjectileHolder, m_CollisionMaterial, L"Meshes/meteor.ovm"));

	//MagickaScene* scene{ reinterpret_cast<MagickaScene*>(SceneManager::Get()->GetActiveScene()) };

	//fwd.x *= m_Distance;
	//fwd.z *= m_Distance;

	//pos.x += fwd.x;
	//pos.y += 100;
	//pos.z += fwd.z;

	//auto go{ new SpherePrefab(25, 8, static_cast<XMFLOAT4>(Colors::Brown)) };
	//scene->AddChild(go);
	//go->GetTransform()->Translate(pos);

	//auto comp{ new RigidBodyComponent() };
	//go->AddComponent(comp);
	////comp->SetKinematic(false);
	//comp->AddCollider(PxSphereGeometry{ 25 }, *m_pColliderMaterial);
	//comp->AddCollider(PxSphereGeometry{ 25 }, *m_pColliderMaterial);
	//comp->AddForce({ 0,1000,0 });
	//comp->SetDensity(500);


	//auto colliderInfo = comp->GetCollider(0);
	//colliderInfo.SetTrigger(true);
	//go->SetOnTriggerCallBack(
	//	[&](GameObject* /*pTriggerObject*/, GameObject* pOtherObject, PxTriggerAction action) {
	//		if (action == PxTriggerAction::ENTER)
	//		{
	//			if (auto enemy{ dynamic_cast<BaseEnemy*>(pOtherObject) }) {
	//				enemy->DamageAOE(300, true);
	//			}
	//			else if (auto player{ dynamic_cast<MagickaCharacter*>(pOtherObject) }) {}
	//			else {
	//				go->GetScene()->RemoveChild(go);
	//			}
	//		}
	//	}
	//);

	//float value{ 150 };
	//ParticleEmitterSettings settings{};
	//settings.velocity = { 0.f,0.f, value };
	//settings.originalVelocity = { 0.f,0.f, value };
	//settings.modifier = { 1, 1, 1 };
	//settings.minSize = 10.f;
	//settings.maxSize = 10.f;
	//settings.minEnergy = 1.f;
	//settings.maxEnergy = 2.f;
	//settings.minScale = 1.5f;
	//settings.maxScale = 200.5f;
	//settings.minEmitterRadius = .2f;
	//settings.maxEmitterRadius = .5f;
	//settings.color = { 1.f,1.f,1.f, .6f };
	//settings.originalVelocity = { value, 0.f, 0 };
	//settings.modifier = { 1, 0.f, 0.f };
	//auto component = new ParticleEmitterComponent(L"Textures/FireBall.png", settings, 200);
	//component->SetShouldIncreaseOverTime(true);
	//go->AddComponent(component);


}
