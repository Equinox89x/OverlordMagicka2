#pragma once
#include <Materials/Shadow/DiffuseMaterial_Shadow.h>

class Meteorite : public GameObject
{
public:
	Meteorite(XMFLOAT3 forwardVector, XMFLOAT3 initialPosition, float launchSpeed, GameObject* projectileHolder, PxMaterial* pColliderMaterial, std::wstring assetFile = L"", DiffuseMaterial_Shadow* pMaterial = nullptr);
	~Meteorite() override = default;

	Meteorite(const Meteorite& other) = delete;
	Meteorite(Meteorite&& other) noexcept = delete;
	Meteorite& operator=(const Meteorite& other) = delete;
	Meteorite& operator=(Meteorite&& other) noexcept = delete;

	float GetLife() { return m_pLife; };
	void SetDamageToGive(float damageToGive) { m_DamageToGive = damageToGive; };
	bool IsMarkedForDelete() { return (m_ShouldDelete && m_pDeleteTimer <= 0) || m_pLife <= 0; };

protected:
	void Initialize(const SceneContext&) override;
	void Update(const SceneContext&) override;


private:
	XMFLOAT3 m_ForwardVector;
	XMFLOAT3 m_InitialPosition;
	float m_Distance;

	std::wstring m_AssetFile;
	DiffuseMaterial_Shadow* m_pMaterial{};
	PxMaterial* m_pColliderMaterial{};
	GameObject* m_Collider;

	std::list<GameObject*> m_CharactersInRange;
	float m_pLife{ 2.f }, m_pDeleteTimer{ 1 };
	bool m_ShouldDelete{ false }, m_CanMove{ true };
	float m_DamageToGive{ 0 };
	GameObject* m_ProjectileHolder;
};

