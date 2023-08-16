#pragma once
#include "MagickaCharacter.h"
#include "BaseEnemy.h"


class SpecialSpell
{
public:
	SpecialSpell() = default;
	virtual ~SpecialSpell() = default;
	SpecialSpell(const SpecialSpell& other) = delete;
	SpecialSpell(SpecialSpell&& other) = delete;
	SpecialSpell& operator=(const SpecialSpell& other) = delete;
	SpecialSpell& operator=(SpecialSpell&& other) = delete;


	virtual void ExecuteSpell() {};

};

class Teleport final : public SpecialSpell {
public:
	Teleport(MagickaCharacter* character) : m_Character{ character } {};

	virtual void ExecuteSpell() override;

private:
	float m_Distance{ 100 };
	MagickaCharacter* m_Character;
};

class Thunderbolt final : public SpecialSpell {
public:
	Thunderbolt(MagickaCharacter* character, PxMaterial* material, GameObject* projectileHolder, PxMaterial* collisionMaterial) : m_Character{ character }, m_pColliderMaterial{ material }, m_ProjectileHolder{ projectileHolder }, m_CollisionMaterial{ collisionMaterial } {};

	virtual void ExecuteSpell() override;

private:
	float m_Distance{ 100 };
	MagickaCharacter* m_Character;
	PxMaterial* m_pColliderMaterial;
	GameObject* m_ProjectileHolder;
	PxMaterial* m_CollisionMaterial;
};