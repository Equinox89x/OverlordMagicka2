#pragma once
#include "MagickaCharacter.h"
#include "BaseEnemy.h"

struct ResultingMagic;
class EnemyMeleeCharacter : public BaseEnemy
{
public:
	EnemyMeleeCharacter(const CharacterDescExtended& characterDesc, XMFLOAT3 postitionOffset);
	~EnemyMeleeCharacter() override = default;

	EnemyMeleeCharacter(const EnemyMeleeCharacter& other) = delete;
	EnemyMeleeCharacter(EnemyMeleeCharacter&& other) noexcept = delete;
	EnemyMeleeCharacter& operator=(const EnemyMeleeCharacter& other) = delete;
	EnemyMeleeCharacter& operator=(EnemyMeleeCharacter&& other) noexcept = delete;

	bool GetCanAttack() { return CanAttack; };
	void SetCanAttack(bool canAttack) { CanAttack = canAttack; };

protected:
	void InitializeChild(const SceneContext&) override;
	void UpdateChild(const SceneContext& sceneContext) override;

private:
	float DamageTimerMelee{ 1 };
	bool CanAttack{ true };

	FMOD::Sound* PunchSound;
	FMOD::System* SoundSystem;
	FMOD::Channel* CurrentChannel;

};

