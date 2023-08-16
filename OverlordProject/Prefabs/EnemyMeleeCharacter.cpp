#include "stdafx.h"
#include "EnemyMeleeCharacter.h"
#include "../OverlordEngine/Prefabs/MagickaCamera.h"
#include <Scenes/Exam/MagickaScene.h>

EnemyMeleeCharacter::EnemyMeleeCharacter(const CharacterDescExtended& characterDesc, XMFLOAT3 postitionOffset):
	BaseEnemy(characterDesc, postitionOffset)
{
}

void EnemyMeleeCharacter::InitializeChild(const SceneContext& /*sceneContext*/)
{
	SoundSystem = SoundManager::Get()->GetSystem();
	auto file = ContentManager::GetFullAssetPath(L"Sound/punch.mp3");
	SoundSystem->createSound(file.string().c_str(), FMOD_DEFAULT, nullptr, &PunchSound);
}

void EnemyMeleeCharacter::UpdateChild(const SceneContext& sceneContext)
{
	float deltaTime = sceneContext.pGameTime->GetElapsed();
	if (!CanAttack) {
		DamageTimerMelee -= deltaTime;
		if (DamageTimerMelee <= 0) {
			CanAttack = true;
			DamageTimerMelee = 1;
			SoundSystem->playSound(PunchSound, nullptr, false, &CurrentChannel);
		}
	}
}
