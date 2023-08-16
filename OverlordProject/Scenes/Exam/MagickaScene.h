#pragma once
#include <queue>
#include <Materials/DiffuseMaterial.h>
#include <Prefabs/MagickaCamera.h>
#include "Prefabs/SpherePrefab.h"
#include "Prefabs/CubePrefab.h"
#include "Prefabs/TorusPrefab.h"
#include "Prefabs/Projectile.h"
#include "Prefabs/SpecialSpell.h"
#include <list>
#include <set>
#include <Materials/Shadow/DiffuseMaterial_Shadow.h>
#include <Materials/DiffuseMaterial_Skinned.h>

class MagickaCharacter;
class EnemyRangedCharacter;
class EnemyMeleeCharacter;

#pragma region enums & structs
//enum class MenuItems {
//	ToMenu = 0,
//	Restart = 1,
//	Exit = 2
//};

enum ElementTypes : char
{
	WATER = 'W',
	LIGHTNING = 'L',
	LIFE = 'Y',
	ARCANE = 'A',
	SHIELD = 'S',
	EARTH = 'E',
	COLD = 'C',
	FIRE = 'F',

	STEAM = 'M',
	ICE = 'I',
	POISON = 'P',
	NONE = 'N'
};

enum class ProjectileTypes {
	SPRAY,
	BEAM,
	PROJECTILE,
	PROJECTILEBOMB,
	SHIELD,
	AOE,
};

enum class Spells {
	PUSH,
	EMERGENCYTELEPORT,
	DISPEL,
	RANDOMSPELL,
	TELEPORT,
	HASTE,
	DISRUPTOR,
	CONCUSSION,
	THUNDERBOLT,
	SPIKEQUAKE,
	ICETORNADO,
	BREEZE,
	GUARDIAN,
	DRAGONSTRIKE,
	FOWL,
	THUNDERHEAD,
	SUMMON,
	SACRIFICE,
	REVIVE
};

enum class SolidType {
	SOLID,
	NOTSOLID
};

struct ResultingMagic {
	ElementTypes BaseElementType;
	ProjectileTypes ProjectileType;
	SolidType SolidType;
	std::set<ElementTypes> Modifiers;
	std::set<std::wstring> TextureName;
	std::set<std::wstring> AOETextureName;
	float Damage;
};

struct MagicTypes {
	MagicTypes(ElementTypes baseType, ProjectileTypes magicType, float damage, float addedDamage) :
		BaseType{ baseType },
		ProjectileType{ magicType },
		Damage{ damage },
		AddedDamage{ addedDamage }
	{
		AoEDamage = Damage * 2;
	}

	ProjectileTypes ProjectileType{};
	ElementTypes BaseType{};
	float Damage{ 0 };
	float AoEDamage{ 0 };
	float AddedDamage{ 0 };
};

struct MagicSpell {

	MagicSpell(std::deque<ElementTypes> magics, Spells spell, SpecialSpell* specialSpell = nullptr) :
		Magics{ magics },
		Spell{ spell },
		m_SpecialSpell{ specialSpell }
	{
		CreatePatternString();
	}

	void ReleaseMagic() {
		if (m_SpecialSpell) {
			delete m_SpecialSpell;
			m_SpecialSpell = nullptr;
		}
	}

	std::deque<ElementTypes> Magics{};
	Spells Spell{};
	SpecialSpell* m_SpecialSpell;
	std::string Pattern;

	void CreatePatternString() {
		for (auto magic : Magics) {
			Pattern += magic;
		}
	}

	void ExecuteSpell() {
		if (!m_SpecialSpell) return;
		m_SpecialSpell->ExecuteSpell();
	}
};

struct Magic {
	Magic() :
		ElementType{ ElementTypes::ARCANE },
		CancellingMagicTypes{}
	{}

	Magic(ElementTypes elementType, MagicTypes defaultMagicType, std::vector<ElementTypes> cancellingMagicTypes, std::vector<ElementTypes> combiningMagicTypes = std::vector<ElementTypes>{}) :
		ElementType{ elementType },
		CancellingMagicTypes{ cancellingMagicTypes },
		CombiningMagicTypes{ combiningMagicTypes },
		DefaultMagicType{ defaultMagicType }
	{
		AssignName();
	}

	std::wstring TextureName{};
	std::wstring UITextureName{};
	std::wstring AOETextureName{};
	ElementTypes ElementType{ ElementTypes::ARCANE };
	std::vector<ElementTypes> CombiningMagicTypes{ };
	std::vector<ElementTypes> CancellingMagicTypes{ };
	std::vector<MagicTypes> MagicType{ };
	MagicTypes DefaultMagicType{ MagicTypes{ElementTypes::NONE, ProjectileTypes::BEAM, 0, 0} };

	void AssignName() {
		TextureName = L"Textures/";
		UITextureName = L"Textures/Magicka/Element_";
		AOETextureName = L"Textures/Magicka/Element_aoe_";
		switch (ElementType)
		{
		case WATER:
			TextureName += L"Water.png";
			UITextureName += L"Water.png";
			AOETextureName += L"Water.png";
			break;
		case LIGHTNING:
			TextureName += L"Lightning.png";
			UITextureName += L"lightning.png";
			AOETextureName += L"lightning.png";
			break;
		case LIFE:
			UITextureName += L"life.png";
			AOETextureName += L"life.png";
			break;
		case ARCANE:
			UITextureName += L"arcane.png";
			AOETextureName += L"arcane.png";
			break;
		case SHIELD:
			UITextureName += L"shield.png";
			AOETextureName += L"shield.png";
			break;
		case EARTH:
			UITextureName += L"earth.png";
			AOETextureName += L"earth.png";
			break;
		case COLD:
			TextureName += L"Cold.png";
			UITextureName += L"cold.png";
			AOETextureName += L"cold.png";
			break;
		case FIRE:
			TextureName += L"FireBall.png";
			UITextureName += L"fire.png";
			AOETextureName += L"fire.png";
			break;
		case STEAM:
			TextureName += L"Smoke.png";
			UITextureName += L"steam.png";
			AOETextureName += L"steam.png";
			break;
		case ICE:
			UITextureName += L"ice.png";
			AOETextureName += L"ice.png";
			break;
		case POISON:
			TextureName += L"Poison.png";
			UITextureName += L"poison.png";
			AOETextureName += L"poison.png";
			break;
		case NONE:
			break;
		default:
			break;
		}
	}
};
#pragma endregion

class MagickaScene : public GameScene
{
public:
	MagickaScene() : GameScene(L"MagickaScene") {};
	~MagickaScene() override {
		for (size_t i = 0; i < m_MagicSpells.size(); i++)
		{
			m_MagicSpells[i].ReleaseMagic();
		}
	}
	MagickaScene(const MagickaScene& other) = delete;
	MagickaScene(MagickaScene&& other) noexcept = delete;
	MagickaScene& operator=(const MagickaScene& other) = delete;
	MagickaScene& operator=(MagickaScene&& other) noexcept = delete;

	void HandleCombobarFilling(ElementTypes elementType);
	void ResetCombo();
	void ToggleUIElements(bool shiftPressed);

	void ExecuteMagicCombo();
	void ExecuteAOE();
	void ExecuteSelfCast();
	void ExecuteSword();
	void FireProjectile(bool isBomb = false);
	void FireProjectileBarage();
	void ResetGame();

protected:
	void Initialize() override;
	void OnGUI() override;
	void Update() override;
	void Draw() override;

private:
	//setup
	PxMaterial* m_Material{};
	DiffuseMaterial_Shadow* m_pProjectileMaterial{};
	PxMaterial* m_pDefaultMaterial{};
	XMFLOAT3 m_Size{ 10,10, 10 };
	MagickaCharacter* m_pCharacter{};
	GameObject* m_pSprayMagicEmitter{};
	GameObject* m_pBeamMagicEmitter{ nullptr };
	GameObject* m_pBeamMagicEmitterContainer{ nullptr };
	GameObject* m_pAOEMagicEmitter{ nullptr };
	GameObject* m_pShieldMagicEmitter{ nullptr };
	GameObject* m_pLevel{ nullptr };
	GameObject* m_pUI{ nullptr };
	GameObject* m_pUI2{ nullptr };
	GameObject* m_pUI3{ nullptr };
	GameObject* m_pPlayerDamageTakingCollider{ nullptr };
	GameObject* m_pPlayerMaxEnemyRangeCollider{ nullptr };
	GameObject* m_pCanEnemyMoveCollider{ nullptr };
	GameObject* m_pSprayDamageCollider{ nullptr };
	GameObject* m_pSprayDamageColliderContainer{ nullptr };
	GameObject* m_pProjectileHolder{ nullptr };
	GameObject* m_pEnemyHolder{ nullptr };
	GameObject* m_GameEnder{ nullptr };

	DiffuseMaterial_Skinned* m_pMaterial{};
	DiffuseMaterial_Shadow* m_pLevelMaterial{};
	std::vector<std::wstring> m_Textures{};
	std::vector<std::wstring> m_TransparentTextures{};
	ResultingMagic MagicResult{};
	std::vector<Projectile*> m_Projectiles;

	FMOD::System* SoundSystem;
	FMOD::Sound* FireSound;
	FMOD::Sound* Music;
	FMOD::Sound* AoeSound;
	FMOD::Sound* ProjectileSound;
	FMOD::Sound* LaserSound;
	FMOD::Channel* CurrentChannel;

	//Magic information
	//Water must always be FIRST in the vector of combined magics
	std::vector<Magic> m_CombinedMagics{ };
	std::vector<Magic> m_Magics{ };
	std::vector<MagicSpell> m_MagicSpells{ };
	std::vector<MagicTypes> m_MagicTypes{ };

	//combo information
	std::vector<Magic> m_ComboBar{};
	std::string m_ComboPattern;
	std::string m_PatternOptions;

	SpherePrefab* Sphere;
	CameraComponent* m_pCameraComponent{};
	MagickaCamera* m_pCamera{};
	int currentLineIndex{ 0 };
	bool CanIncrease{ true };
	bool IsExecutingMagic{ false }, IsChargingProjectile{ false }, IsShootingIceProjectile, IsBombProjectile;
	bool AoeFired{ false };
	float AoeTimer{ 2 }, ProjectileTimer{ 0 }, MaxProjectileTimer{ 5 }, MaxIceProjectileTimer{ 0.2f };
	bool IsGameOver{ false };

	//input
	enum InputIds
	{
		ElementBottom = 0,
		ElementLeft = 1,
		ElementTop = 2,
		ElementRight = 3,
		Elementcombo5 = 4,
		SwitchElement = 5,
		CharacterMoveLeft = 6,
		CharacterMoveRight = 7,
		CharacterMoveForward = 8,
		CharacterMoveBackward = 9,
		CharacterJump = 10,
		Move = 11,
		Execute = 12,
		SwordInput = 13,
		SelfCast = 14,
		AoEAttack = 15,
		InGameMenu = 16,
	};

	//setup helpers
	void CreateLevel();
	void CreateInput();
	void CreateCharacter();
	void CreateMagic();
	void CreateEmitters();
	void CreateUI();
	void CreateDamager();
	void CreateMeleeEnemies(float width, XMFLOAT3 position, int nrOfEnemies);
	void CreateRangedEnemies(float width, XMFLOAT3 position, int nrOfEnemies);
	void CreateSound();
	void DefineCameraSpline();

	//magic helper functions
	Magic* GetCombinedMagicType(ElementTypes elementType1, ElementTypes elementType2);
	bool IsMagicCancelled(int currentIterator, const Magic* currentMagic, bool& foundCanceller);
	void FillMagicResultData();

	void ChargeProjectile(bool isBomb = false);
	void SetGameOver(bool hasPlayerWon);

	void SetStartPos();
	void HandleEnemies();
	void HandleTimers();
	void HandleMagicTransform();
	void HandleCameraMovement();
	void HandleUIMovement(const XMFLOAT3 pos);
	void HandleEmitterMovement(XMFLOAT3 pos);


	//debug
	void HandlePrint(const ElementTypes currentMagic) const;
	void HandlePrint(const char currentMagic) const;
	void HandlePrint2(const Spells spell) const;
};
