#pragma once
#include "MagickaCharacter.h"
class Projectile;
class DiffuseMaterial;

class BaseEnemy : public GameObject
{
public:
	BaseEnemy(const CharacterDescExtended& characterDesc, XMFLOAT3 postitionOffset);
	~BaseEnemy() override = default;

	BaseEnemy(const BaseEnemy& other) = delete;
	BaseEnemy(BaseEnemy&& other) noexcept = delete;
	BaseEnemy& operator=(const BaseEnemy& other) = delete;
	BaseEnemy& operator=(BaseEnemy&& other) noexcept = delete;

	void DrawImGui();
	void DamageBeamEnter(float damage);
	void DamageBeamExit();
	void DamageAOE(float damage);
	void DamageAOE(float damage, bool canDamage);

	void SetCanDamageAoE(bool canDamage) { CanDamageAoE = canDamage; };
	bool GetCanDamage() { return CanDamage; };

	float GetHealth() { return m_Health; };
	void SetCanDamage(bool canDamage) { CanDamageAoE = canDamage; };
	void SetCanMove(bool canMove) { CanMove = canMove; };
	float GetAttackDamage() { return m_AttackDamage; };
	//void SetCanShoot(bool canShoot) { CanShoot = canShoot; };
	//void FireProjectile();

	bool GetMarkedForDestroy() { return m_MarkedForDestroy; };
	void SetMarkedForDestroy(bool isMarked) { m_MarkedForDestroy = isMarked; };

	MagickaCharacter* m_pCharacter{ nullptr };

	void SetCanMoveGenerally(bool canMove) { CanMoveGeneral = canMove; };

protected:
	void Initialize(const SceneContext&) override;
	void Update(const SceneContext&) override;
	virtual void UpdateChild(const SceneContext& sceneContext) = 0;
	virtual void InitializeChild(const SceneContext& sceneContext) = 0;

	//DiffuseMaterial* m_pMaterial{};
	//PxMaterial* m_pDefaultMaterial{};

private:
	CharacterDescExtended m_CharacterDescExtended;
	//ControllerComponent* m_pControllerComponent{};
	bool WasPressed{ false };
	float m_Health{ 1500 };

	float DamageTimer, DefaultDamageTimer{ 0.5f }/*, ShootTimer, DefaultShootTimer{ 3.f }*/;
	bool CanDamage{ false }, CanMove{ true }, CanDamageAoE{ false }/*, CanShoot{ false }*/;
	float DamageToTake, m_AttackDamage{ 100 };
	bool m_MarkedForDestroy{ false }, CanMoveGeneral{ false };

	XMFLOAT3 PostitionOffset;

	//std::vector<Projectile*> m_Projectiles;

};

