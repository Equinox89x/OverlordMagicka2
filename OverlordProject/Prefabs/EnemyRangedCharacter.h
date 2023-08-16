#pragma once
#include "MagickaCharacter.h"
#include "BaseEnemy.h"
class Projectile;
class DiffuseMaterial_Shadow;

class EnemyRangedCharacter : public BaseEnemy
{
public:
	EnemyRangedCharacter(const CharacterDescExtended& characterDesc, XMFLOAT3 postitionOffset);
	~EnemyRangedCharacter() override = default;

	EnemyRangedCharacter(const EnemyRangedCharacter& other) = delete;
	EnemyRangedCharacter(EnemyRangedCharacter&& other) noexcept = delete;
	EnemyRangedCharacter& operator=(const EnemyRangedCharacter& other) = delete;
	EnemyRangedCharacter& operator=(EnemyRangedCharacter&& other) noexcept = delete;

	void SetCanShoot(bool canShoot) { CanShoot = canShoot; };
	void FireProjectile();
	void SetProjectileHolder(GameObject* projectileHolder) { m_ProjectileHolder = projectileHolder; };

protected:
	void InitializeChild(const SceneContext&) override;
	void UpdateChild(const SceneContext&) override;

private:
	float ShootTimer, DefaultShootTimer{ 3.f };
	bool CanShoot{ false };

	DiffuseMaterial_Shadow* m_pMaterial{};
	PxMaterial* m_pDefaultMaterial{};
	std::vector<Projectile*> m_Projectiles;
	GameObject* m_ProjectileHolder;

	FMOD::System* SoundSystem;
	FMOD::Sound* ProjectileSound;
	FMOD::Channel* CurrentChannel;


};

