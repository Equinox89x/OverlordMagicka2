#pragma once
#include <Materials/Shadow/DiffuseMaterial_Shadow.h>

class Projectile : public GameObject
{
public:
	Projectile(std::wstring assetFile, XMFLOAT3 forwardVector, XMFLOAT3 initialPosition, float launchSpeed, float downWardsVelocity, DiffuseMaterial_Shadow* pMaterial, PxMaterial* pColliderMaterial, GameObject* projectileHolder, bool isBomb = false);
	~Projectile() override = default;

	Projectile(const Projectile& other) = delete;
	Projectile(Projectile&& other) noexcept = delete;
	Projectile& operator=(const Projectile& other) = delete;
	Projectile& operator=(Projectile&& other) noexcept = delete;

	float GetLife() { return m_pLife; };
	void SetDamageToGive(float damageToGive) { m_DamageToGive = damageToGive; };
	bool IsMarkedForDelete() { return (m_ShouldDelete && m_pDeleteTimer <= 0) || m_pLife <= 0; };

protected:
	void Initialize(const SceneContext&) override;
	void Update(const SceneContext&) override;


private:
	std::wstring m_AssetFile;
	XMFLOAT3 m_ForwardVector;
	XMFLOAT3 m_InitialPosition;
	float m_LaunchSpeed, m_DownWardsVelocity;
	DiffuseMaterial_Shadow* m_pMaterial{};
	PxMaterial* m_pColliderMaterial{};
	GameObject* m_Collider;
	std::list<GameObject*> m_CharactersInRange;
	float m_pLife{ 1.5f }, m_pDeleteTimer{ 1 };
	bool m_IsBomb{ false }, m_ShouldDelete{ false }, m_CanExplode{ false }, m_CanMove{ true };
	float m_ExplosionRadius{ 30 };
	float m_DamageToGive{ 0 };
	GameObject* m_ProjectileHolder;

	void Explode();
};

