#include "stdafx.h"
#include "MagickaScene.h"
#include <Prefabs/MagickaCharacter.h>
#include <Prefabs/EnemyMeleeCharacter.h>
#include <Prefabs/EnemyRangedCharacter.h>
#include <unordered_set>
#include <Prefabs/Projectile.h>
#include <Materials/SkyboxMaterial.h>
#include <Materials/Post/PostVignette.h>
#include <Materials/Shadow/DiffuseMaterial_Shadow_Skinned.h>


void MagickaScene::Initialize()
{
	m_SceneContext.settings.showInfoOverlay = true;
	//m_SceneContext.settings.drawPhysXDebug = true;
	m_SceneContext.settings.drawGrid = true;
	m_SceneContext.settings.enableOnGUI = true;

	m_pDefaultMaterial = PxGetPhysics().createMaterial(0.5f, 0.5f, 0.5f);

	m_Material = PxGetPhysics().createMaterial(.5f, .5f, .5f);

	m_pMaterial = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Skinned>();
	m_pMaterial->SetDiffuseTexture(L"Textures/Chair_Dark.dds");
	
	m_pProjectileMaterial = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
	m_pProjectileMaterial->SetDiffuseTexture(L"Textures/Chair_Dark.dds");

	m_pLevelMaterial = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
	m_pLevelMaterial->SetDiffuseTexture(L"Textures/level2.png");

	GameObject* skybox{ AddChild(new GameObject{}) };
	ModelComponent* skyBoxModelComp = skybox->AddComponent(new ModelComponent{ L"Meshes/Sphere.ovm" });
	skyBoxModelComp->GetTransform()->Scale(100.f, 100.f, 100.f);
	skyBoxModelComp->GetTransform()->Rotate(0.f, 0.f, 90.f);

	auto skyboxMat = MaterialManager::Get()->CreateMaterial<SkyboxMaterial>();
	skyboxMat->SetCubeMapTexture(L"Textures/Skybox.dds");
	skyBoxModelComp->SetMaterial(skyboxMat);

	auto vignette = MaterialManager::Get()->CreateMaterial<PostVignette>();
	AddPostProcessingEffect(vignette);

	
	CreateLevel();

	CreateCharacter();

	CreateDamager();

	DefineCameraSpline();

	m_pEnemyHolder = AddChild(new GameObject());
	m_pProjectileHolder = AddChild(new GameObject());
	float width{ 50 };
	int nrOfMelee{ 5 };
	int nrOfRanged{ 0 };
	auto position{ m_pCamera->GetLineOfIndex(2) };
	CreateMeleeEnemies(width, position, nrOfMelee);
	position = m_pCamera->GetLineOfIndex(4);
	//position.z -= 300;
	CreateMeleeEnemies(width, position, nrOfMelee);
	nrOfRanged = 2;
	//position.z += 50;
	CreateRangedEnemies(width, position, nrOfRanged);
	position = m_pCamera->GetLineOfIndex(4);
	//position.x += 300;
	//position.z += 300;
	CreateMeleeEnemies(width, position, nrOfMelee);
	nrOfRanged = 5;
	//position.x -= 100;
	//position.z -= 50;
	CreateRangedEnemies(width, position, nrOfRanged);

	CreateInput();

	CreateEmitters();

	SetStartPos();

	CreateUI();

	ResetCombo();

	CreateMenu();

	CreateSound();

	CreateMagic();
}

#pragma region Setup
void MagickaScene::SetStartPos()
{
	auto projectedPoint{ m_pCamera->GetProjectedLinePoint() };
	XMFLOAT3 p{ m_pCamera->GetTransform()->GetWorldPosition() };
	p.x += projectedPoint.x;
	p.y += projectedPoint.y;
	p.z += projectedPoint.z;

	m_SceneContext.pCamera->GetTransform()->Translate(p);
	m_pCameraComponent->GetTransform()->Translate(p);

	m_pCameraComponent->GetTransform()->Rotate(47, -53, 0);
	m_SceneContext.pCamera->GetTransform()->Rotate(47, -53, 0);

	m_pCameraComponent->GetTransform()->Translate(300, 400, -100);
	m_SceneContext.pLights->SetDirectionalLight({ -95.6139526f,66.1346436f,-41.1850471f }, { 0.740129888f, -0.597205281f, 0.309117377f });
}

void MagickaScene::CreateLevel()
{
	//Ground Plane
	GameSceneExt::CreatePhysXGroundPlane(*this, m_pDefaultMaterial);

	//level
	m_pLevel = new GameObject();
	AddChild(m_pLevel);
	m_pLevel->AddComponent(new ModelComponent(L"Meshes/level2.ovm"));
	m_pLevel->GetComponent<ModelComponent>()->SetMaterial(m_pLevelMaterial);

	m_pLevel->AddComponent(new RigidBodyComponent());
	m_pLevel->GetComponent<RigidBodyComponent>()->SetKinematic(true);

	m_pLevel->GetTransform()->Scale(10);
}

void MagickaScene::CreateInput()
{
	//Input	
	auto inputAction = InputAction(ElementBottom, InputState::released, 'Q', -1, XINPUT_GAMEPAD_A);
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(ElementLeft, InputState::released, 'E', -1, XINPUT_GAMEPAD_X);
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(ElementTop, InputState::released, 'Z', -1, XINPUT_GAMEPAD_Y);
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(ElementRight, InputState::released, 'F', -1, XINPUT_GAMEPAD_B);
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(SwitchElement, InputState::down, VK_LSHIFT, -1, XINPUT_GAMEPAD_LEFT_SHOULDER);
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(AoEAttack, InputState::down, VK_LCONTROL, -1, XINPUT_GAMEPAD_RIGHT_SHOULDER);
	m_SceneContext.pInput->AddInputAction(inputAction);					  
	
	inputAction = InputAction(SwordInput, InputState::down, VK_EXECUTE, -1, XINPUT_GAMEPAD_RIGHT_SHOULDER);
	m_SceneContext.pInput->AddInputAction(inputAction);
	
	inputAction = InputAction(SelfCast, InputState::down, VK_LSHIFT, -1, XINPUT_GAMEPAD_RIGHT_SHOULDER);
	m_SceneContext.pInput->AddInputAction(inputAction);

	inputAction = InputAction(InGameMenu, InputState::down, VK_ESCAPE, -1, XINPUT_GAMEPAD_START);
	m_SceneContext.pInput->AddInputAction(inputAction);
}

void MagickaScene::CreateCharacter()
{
	//Character
	CharacterDescExtended characterDesc{ m_Material };
	characterDesc.actionId_ElementBottom = ElementBottom;
	characterDesc.actionId_ElementLeft = ElementLeft;
	characterDesc.actionId_ElementTop = ElementTop;
	characterDesc.actionId_ElementRight = ElementRight;
	characterDesc.actionId_SwitchElement = SwitchElement;

	characterDesc.actionId_AoEAttack = AoEAttack;
	characterDesc.actionId_SelfCast = SelfCast;
	characterDesc.actionId_SwordInput = SwordInput;

	characterDesc.actionId_InGameMenu = InGameMenu;
	characterDesc.actionId_Move = Move;
	characterDesc.actionId_Execute = Execute;

	characterDesc.maxMoveSpeed = 35;

	auto mat = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow_Skinned>();
	mat->SetDiffuseTexture(L"Textures/T_Wizards.jpg");

	m_pCharacter = AddChild(new MagickaCharacter(characterDesc));
	m_pCharacter->GetTransform()->Translate(-50.f, 5.f, -350.f);
	m_pCharacter->AddComponent(new ModelComponent(L"Meshes/wizard.ovm", true));
	m_pCharacter->GetComponent<ModelComponent>()->SetMaterial(mat);
	m_pCharacter->GetComponent<ModelComponent>()->GetTransform()->Rotate(0,-90,0);
	m_pCharacter->GetTransform()->Scale(0.5f);
	//m_pCharacter->GetTransform()->Rotate(0, 180, 0);

	for (auto comp : m_pCharacter->GetComponents<ParticleEmitterComponent>()) {
		comp->GetTransform()->Translate(0, 20, 0);
	}

	m_pCamera = AddChild(new MagickaCamera());
	m_pCameraComponent = m_pCamera->GetComponent<CameraComponent>();
	m_pCamera->GetTransform()->Translate(0.f, characterDesc.controller.height * .2f, 0.f);

	m_SceneContext.pCamera->GetTransform()->Rotate(-45, -90, 0);
}

void MagickaScene::CreateDamager() {
	m_pPlayerDamageTakingCollider = new GameObject();
	AddChild(m_pPlayerDamageTakingCollider);
	m_pPlayerDamageTakingCollider->AddComponent(new RigidBodyComponent());
	m_pPlayerDamageTakingCollider->GetComponent<RigidBodyComponent>()->AddCollider(PxBoxGeometry{ 20,20,20 }, *m_pDefaultMaterial);
	m_pPlayerDamageTakingCollider->GetComponent<RigidBodyComponent>()->SetKinematic(true);

	auto colliderInfo = m_pPlayerDamageTakingCollider->GetComponent<RigidBodyComponent>()->GetCollider(0);
	colliderInfo.SetTrigger(true);
	
	m_pPlayerMaxEnemyRangeCollider = new GameObject();
	AddChild(m_pPlayerMaxEnemyRangeCollider);
	m_pPlayerMaxEnemyRangeCollider->AddComponent(new RigidBodyComponent());
	m_pPlayerMaxEnemyRangeCollider->GetComponent<RigidBodyComponent>()->AddCollider(PxBoxGeometry{ 200,30,200 }, *m_pDefaultMaterial);
	m_pPlayerMaxEnemyRangeCollider->GetComponent<RigidBodyComponent>()->SetKinematic(true);

	colliderInfo = m_pPlayerMaxEnemyRangeCollider->GetComponent<RigidBodyComponent>()->GetCollider(0);
	colliderInfo.SetTrigger(true);

	m_pPlayerMaxEnemyRangeCollider->SetOnTriggerCallBack([&](GameObject* /*pTriggerObject*/, GameObject* pOtherObject, PxTriggerAction action)
	{
		if (action == PxTriggerAction::ENTER)
		{
			if (auto enemy{ dynamic_cast<EnemyRangedCharacter*>(pOtherObject) }) {
				enemy->SetCanMove(false);
				enemy->SetCanShoot(true);
			}
		}
		if (action == PxTriggerAction::LEAVE)
		{
			if (auto enemy{ dynamic_cast<EnemyRangedCharacter*>(pOtherObject) }) {
				enemy->SetCanMove(true);
				enemy->SetCanShoot(false);
			}
		}
	});

	m_pCanEnemyMoveCollider = new GameObject();
	AddChild(m_pCanEnemyMoveCollider);
	m_pCanEnemyMoveCollider->AddComponent(new RigidBodyComponent());
	m_pCanEnemyMoveCollider->GetComponent<RigidBodyComponent>()->AddCollider(PxBoxGeometry{ 400,30,400 }, *m_pDefaultMaterial);
	m_pCanEnemyMoveCollider->GetComponent<RigidBodyComponent>()->SetKinematic(true);

	colliderInfo = m_pCanEnemyMoveCollider->GetComponent<RigidBodyComponent>()->GetCollider(0);
	colliderInfo.SetTrigger(true);

	m_pCanEnemyMoveCollider->SetOnTriggerCallBack([&](GameObject* /*pTriggerObject*/, GameObject* pOtherObject, PxTriggerAction action)
	{
		if (action == PxTriggerAction::ENTER)
		{
			if (auto enemy{ dynamic_cast<BaseEnemy*>(pOtherObject) }) {
				enemy->SetCanMoveGenerally(true);
			}
		}
		if (action == PxTriggerAction::LEAVE)
		{
			if (auto enemy{ dynamic_cast<BaseEnemy*>(pOtherObject) }) {
				enemy->SetCanMoveGenerally(false);
			}
		}
	});
}

void MagickaScene::CreateMeleeEnemies(float width, XMFLOAT3 position, int nrOfEnemies) {
	auto mat = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
	mat->SetDiffuseTexture(L"Textures/Orc.tga.png");

	CharacterDescExtended enemyDesc{ m_Material };
	enemyDesc.maxMoveSpeed = 35;

	for (size_t i = 0; i < nrOfEnemies; i++)
	{
		position.x += width;
		EnemyMeleeCharacter* enemy = new EnemyMeleeCharacter(enemyDesc, position);
		m_pEnemyHolder->AddChild(enemy);
		enemy->AddComponent(new ModelComponent(L"Meshes/goblin.ovm", true));
		enemy->GetComponent<ModelComponent>()->SetMaterial(mat);

		enemy->AddComponent(new RigidBodyComponent());
		enemy->GetComponent<RigidBodyComponent>()->SetKinematic(true);
		enemy->GetComponent<RigidBodyComponent>()->AddCollider(PxBoxGeometry{ 10,50,10 }, *m_pDefaultMaterial);
		enemy->GetComponent<RigidBodyComponent>()->AddCollider(PxBoxGeometry{ 10,50,10 }, *m_pDefaultMaterial);

		auto colliderInfo = enemy->GetComponent<RigidBodyComponent>()->GetCollider(0);
		colliderInfo.SetTrigger(true);

		enemy->m_pCharacter = m_pCharacter;
		enemy->GetTransform()->Scale(4.f);
		width -= 20;
	}
}

void MagickaScene::CreateRangedEnemies(float width, XMFLOAT3 position, int nrOfEnemies) {
	auto mat = MaterialManager::Get()->CreateMaterial<DiffuseMaterial_Shadow>();
	mat->SetDiffuseTexture(L"Textures/T_Wizards.jpg");

	CharacterDescExtended enemyRangedDesc{ m_Material };
	enemyRangedDesc.maxMoveSpeed = 55;
	width = 50 ;

	for (size_t i = 0; i < nrOfEnemies; i++)
	{
		EnemyRangedCharacter* enemy = new EnemyRangedCharacter(enemyRangedDesc, position);
		m_pEnemyHolder->AddChild(enemy);
		enemy->SetProjectileHolder(m_pProjectileHolder);
		enemy->AddComponent(new ModelComponent(L"Meshes/wizard.ovm", true));
		enemy->GetComponent<ModelComponent>()->SetMaterial(mat);

		enemy->AddComponent(new RigidBodyComponent());
		enemy->GetComponent<RigidBodyComponent>()->SetKinematic(true);
		enemy->GetComponent<RigidBodyComponent>()->AddCollider(PxBoxGeometry{10,50,10}, *m_pDefaultMaterial);
		enemy->GetComponent<RigidBodyComponent>()->AddCollider(PxBoxGeometry{10,50,10}, *m_pDefaultMaterial);
		
		auto colliderInfo = enemy->GetComponent<RigidBodyComponent>()->GetCollider(0);
		colliderInfo.SetTrigger(true);

		enemy->m_pCharacter = m_pCharacter;
		enemy->GetTransform()->Scale(0.5f);
		width -= 40;
	}
}

void MagickaScene::CreateMagic()
{
	const MagicTypes earthBomb{ ElementTypes::EARTH, ProjectileTypes::PROJECTILEBOMB, 75, 18 }; //default
	const MagicTypes iceBarrage{ ElementTypes::ICE, ProjectileTypes::PROJECTILE, 60, 12 }; //default
	const MagicTypes arcaneBeam{ ElementTypes::NONE, ProjectileTypes::BEAM, 106.25f, 30 }; //default
	const MagicTypes lifeBeam{ ElementTypes::NONE, ProjectileTypes::BEAM, -89, 27 }; //default
	const MagicTypes steamSpray{ ElementTypes::NONE, ProjectileTypes::SPRAY, 70, 23 }; //default
	const MagicTypes lightningSpray{ ElementTypes::NONE, ProjectileTypes::SPRAY, 51.25, 20 }; //default
	const MagicTypes fireSpray{ ElementTypes::NONE, ProjectileTypes::SPRAY, 15, 7 }; //default
	const MagicTypes waterSpray{ ElementTypes::NONE, ProjectileTypes::SPRAY, 0, 0 }; //default
	const MagicTypes coldSpray2{ ElementTypes::NONE, ProjectileTypes::SPRAY, 0, 0 }; //default
	const MagicTypes poisonSpray{ ElementTypes::NONE, ProjectileTypes::SPRAY, 90, 26 }; //default
	const MagicTypes shieldShield{ ElementTypes::NONE, ProjectileTypes::SHIELD, 0, 0 }; //default

	const MagicTypes iceBomb{ ElementTypes::EARTH, ProjectileTypes::PROJECTILEBOMB, 137.5f, 25 };
	const MagicTypes arcaneBomb{ ElementTypes::EARTH, ProjectileTypes::PROJECTILEBOMB, 425, 80 };
	const MagicTypes lifeBomb{ ElementTypes::EARTH, ProjectileTypes::PROJECTILEBOMB, 360, 70 };
	const MagicTypes steamBomb{ ElementTypes::EARTH, ProjectileTypes::PROJECTILEBOMB, 280, 60 };
	const MagicTypes waterBomb{ ElementTypes::EARTH, ProjectileTypes::PROJECTILEBOMB, 0, 0 };
	const MagicTypes fireBomb{ ElementTypes::EARTH, ProjectileTypes::PROJECTILEBOMB, 60, 12 };
	const MagicTypes coldBomb{ ElementTypes::EARTH, ProjectileTypes::PROJECTILEBOMB, 25, 15 };

	const MagicTypes steamBeam{ ElementTypes::NONE, ProjectileTypes::BEAM, 90, 35 };
	const MagicTypes lightningBeam{ ElementTypes::NONE, ProjectileTypes::BEAM, 51.25f, 25 };
	const MagicTypes waterBeam{ ElementTypes::NONE, ProjectileTypes::BEAM, 0, 0 };
	const MagicTypes fireBeam{ ElementTypes::NONE, ProjectileTypes::BEAM, 30, 15 };
	const MagicTypes coldBeam{ ElementTypes::NONE, ProjectileTypes::BEAM, 12.5, 8 };

	const MagicTypes arcaneBarrage{ ElementTypes::ICE, ProjectileTypes::PROJECTILE, 42.5f, 20 };
	const MagicTypes lifeBarrage{ ElementTypes::ICE, ProjectileTypes::PROJECTILE, 60, 12 };
	const MagicTypes lightningBarrage{ ElementTypes::ICE, ProjectileTypes::PROJECTILE, 42.5f, 30 };
	const MagicTypes coldBarrage{ ElementTypes::ICE, ProjectileTypes::PROJECTILE, 0, 0 };
	const MagicTypes waterBarrage{ ElementTypes::ICE, ProjectileTypes::PROJECTILE, 0, 0 };

	const MagicTypes lightningSpray2{ ElementTypes::LIGHTNING, ProjectileTypes::SPRAY, 31.25f, 15 };
	const MagicTypes fireSpray2{ ElementTypes::LIGHTNING, ProjectileTypes::SPRAY, 0, 0 };

	m_MagicTypes = { earthBomb, iceBomb, arcaneBomb, lifeBomb, steamBomb, waterBomb, fireBomb, coldBomb,
		arcaneBeam, lifeBeam, steamBeam, lightningBeam, waterBeam, fireBeam, coldBeam,
		iceBarrage, arcaneBarrage, lifeBarrage, lightningBarrage, coldBarrage, waterBarrage,
		steamSpray, lightningSpray, fireSpray, waterSpray,
		lightningSpray2, fireSpray2, coldSpray2
	};

	m_MagicSpells = { 
	/*push*/ MagicSpell{ std::deque<ElementTypes>{ElementTypes::WATER, ElementTypes::SHIELD}, Spells::PUSH , new Teleport(m_pCharacter) },
	/*emergencyTeleport*/ MagicSpell{ std::deque<ElementTypes>{ElementTypes::LIGHTNING, ElementTypes::ARCANE, ElementTypes::LIGHTNING}, Spells::EMERGENCYTELEPORT , new Teleport(m_pCharacter) },
	/*dispel*/ MagicSpell{ std::deque<ElementTypes>{ElementTypes::ARCANE, ElementTypes::SHIELD}, Spells::DISPEL , new Teleport(m_pCharacter) },
	/*randomspell*/ MagicSpell{ std::deque<ElementTypes>{ElementTypes::FIRE, ElementTypes::LIGHTNING, ElementTypes::LIFE}, Spells::RANDOMSPELL , new Teleport(m_pCharacter) },
	/*teleport*/ MagicSpell{ std::deque<ElementTypes>{ElementTypes::LIGHTNING, ElementTypes::ARCANE, ElementTypes::ARCANE}, Spells::TELEPORT, new Teleport(m_pCharacter) },
	/*haste*/ MagicSpell{ std::deque<ElementTypes>{ElementTypes::LIGHTNING, ElementTypes::ARCANE, ElementTypes::FIRE}, Spells::HASTE , new Teleport(m_pCharacter) },
	/*disruptor*/ MagicSpell{ std::deque<ElementTypes>{ElementTypes::ARCANE, ElementTypes::EARTH, ElementTypes::SHIELD}, Spells::DISRUPTOR , new Teleport(m_pCharacter) },
	/*concussion*/ MagicSpell{ std::deque<ElementTypes>{ElementTypes::WATER, ElementTypes::SHIELD, ElementTypes::WATER, ElementTypes::WATER}, Spells::CONCUSSION , new Teleport(m_pCharacter) },
	/*thunderbolt*/ MagicSpell{ std::deque<ElementTypes>{ElementTypes::STEAM, ElementTypes::LIGHTNING, ElementTypes::ARCANE, ElementTypes::LIGHTNING}, Spells::THUNDERBOLT , new Thunderbolt(m_pCharacter, m_Material, m_pProjectileHolder, m_pDefaultMaterial) },
	/*spikequake*/ MagicSpell{ std::deque<ElementTypes>{ElementTypes::ARCANE, ElementTypes::ARCANE, ElementTypes::EARTH, ElementTypes::ARCANE, ElementTypes::ARCANE}, Spells::SPIKEQUAKE , new Teleport(m_pCharacter) },
	/*icetornado*/ MagicSpell{ std::deque<ElementTypes>{ElementTypes::ICE, ElementTypes::EARTH, ElementTypes::COLD, ElementTypes::COLD}, Spells::ICETORNADO , new Teleport(m_pCharacter) },
	/*breeze*/ MagicSpell{ std::deque<ElementTypes>{ElementTypes::ICE, ElementTypes::ICE, ElementTypes::ICE, ElementTypes::ICE }, Spells::BREEZE , new Teleport(m_pCharacter) },
	/*guardian*/ MagicSpell{ std::deque<ElementTypes>{ElementTypes::LIFE, ElementTypes::LIFE, ElementTypes::LIGHTNING, ElementTypes::SHIELD }, Spells::GUARDIAN , new Teleport(m_pCharacter) },
	/*dragonstrike*/ MagicSpell{ std::deque<ElementTypes>{ElementTypes::FIRE, ElementTypes::FIRE, ElementTypes::EARTH, ElementTypes::FIRE, ElementTypes::FIRE}, Spells::DRAGONSTRIKE , new Teleport(m_pCharacter) },
	/*fowl*/ MagicSpell{ std::deque<ElementTypes>{ElementTypes::ARCANE, ElementTypes::EARTH, ElementTypes::ARCANE, ElementTypes::EARTH}, Spells::FOWL , new Teleport(m_pCharacter) },
	/*thunderhead*/ MagicSpell{ std::deque<ElementTypes>{ElementTypes::FIRE, ElementTypes::FIRE, ElementTypes::ARCANE, ElementTypes::LIGHTNING, ElementTypes::LIGHTNING}, Spells::THUNDERHEAD , new Teleport(m_pCharacter) },
	/*summon*/ MagicSpell{ std::deque<ElementTypes>{ElementTypes::ICE, ElementTypes::EARTH, ElementTypes::ARCANE, ElementTypes::COLD}, Spells::SUMMON , new Teleport(m_pCharacter) },
	/*sacrifice*/ MagicSpell{ std::deque<ElementTypes>{ElementTypes::ARCANE, ElementTypes::ARCANE, ElementTypes::ARCANE, ElementTypes::ARCANE, ElementTypes::LIGHTNING}, Spells::SACRIFICE , new Teleport(m_pCharacter) },
	/*revive*/ MagicSpell{ std::deque<ElementTypes>{ElementTypes::LIFE, ElementTypes::LIGHTNING}, Spells::REVIVE , new Teleport(m_pCharacter) },
	};


	const Magic water{ ElementTypes::WATER, waterSpray, std::vector<ElementTypes>{ElementTypes::LIGHTNING}, std::vector<ElementTypes>{ElementTypes::FIRE, ElementTypes::COLD, ElementTypes::ARCANE} };
	const Magic lightning{ ElementTypes::LIGHTNING, lightningSpray, std::vector<ElementTypes>{ElementTypes::WATER, ElementTypes::EARTH} };
	const Magic life{ ElementTypes::LIFE, lifeBeam, std::vector<ElementTypes>{ElementTypes::ARCANE, ElementTypes::POISON} };
	const Magic arcane{ ElementTypes::ARCANE, arcaneBeam, std::vector<ElementTypes>{ElementTypes::LIFE}, std::vector<ElementTypes>{ElementTypes::WATER} };
	const Magic shield{ ElementTypes::SHIELD, shieldShield, std::vector<ElementTypes>{ElementTypes::SHIELD} };
	const Magic earth{ ElementTypes::EARTH, earthBomb, std::vector<ElementTypes>{ElementTypes::LIGHTNING} };
	const Magic cold{ ElementTypes::COLD, coldSpray2, std::vector<ElementTypes>{ElementTypes::FIRE, ElementTypes::STEAM}, std::vector<ElementTypes>{ElementTypes::WATER} };
	const Magic fire{ ElementTypes::FIRE, fireSpray, std::vector<ElementTypes>{ElementTypes::COLD, ElementTypes::ICE}, std::vector<ElementTypes>{ElementTypes::WATER} };

	const Magic steam{ ElementTypes::STEAM, steamSpray, std::vector<ElementTypes>{}, std::vector<ElementTypes>{ElementTypes::WATER, ElementTypes::FIRE} };
	const Magic ice{ ElementTypes::ICE, iceBarrage, std::vector<ElementTypes>{}, std::vector<ElementTypes>{ElementTypes::WATER, ElementTypes::COLD} };
	const Magic poison{ ElementTypes::POISON, poisonSpray, std::vector<ElementTypes>{}, std::vector<ElementTypes>{ElementTypes::WATER, ElementTypes::ARCANE} };
	m_CombinedMagics = { steam, ice, poison };
	m_Magics = { water, lightning, life, arcane, shield, earth, cold, fire };

	for (int i = static_cast<int>(ElementTypes::NONE); i < static_cast<int>(ElementTypes::WATER); i++) {
		m_PatternOptions += static_cast<ElementTypes>(i);
	}
	m_ComboBar.reserve(5);
}

void MagickaScene::CreateEmitters()
{
	//spray
	float value{ 150 };
	ParticleEmitterSettings settings{};
	settings.velocity = { 0.f,0.f, value };
	settings.originalVelocity = { 0.f,0.f, value };
	settings.modifier = { 1, 1, 1 };
	settings.minSize = 10.f;
	settings.maxSize = 10.f;
	settings.minEnergy = 1.f;
	settings.maxEnergy = 2.f;
	settings.minScale = 1.5f;
	settings.maxScale = 200.5f;
	settings.minEmitterRadius = .2f;
	settings.maxEmitterRadius = .5f;
	settings.color = { 1.f,1.f,1.f, .6f };

	m_pSprayMagicEmitter = AddChild(new GameObject());
	settings.originalVelocity = { value, 0.f, 0 };
	settings.modifier = { 1, 0.f, 0.f };
	auto component = new ParticleEmitterComponent(L"Textures/Smoke.png", settings, 200);
	component->SetShouldIncreaseOverTime(true);
	m_pSprayMagicEmitter->AddComponent(component);

	settings.originalVelocity = { value, 0.f, 0 };
	settings.modifier = { 1, 0.f, 0.f };
	component = new ParticleEmitterComponent(L"Textures/Smoke.png", settings, 200);
	component->SetShouldIncreaseOverTime(true);
	m_pSprayMagicEmitter->AddComponent(component);

	//beam
	m_pBeamMagicEmitterContainer = m_pCharacter->AddChild(new GameObject());
	m_pBeamMagicEmitter = new CubePrefab(10, 10, 600);
	m_pBeamMagicEmitterContainer->AddChild(m_pBeamMagicEmitter);
	m_pBeamMagicEmitter->GetTransform()->Translate(0,0,320);
	m_pBeamMagicEmitter->AddComponent(new RigidBodyComponent());
	m_pBeamMagicEmitter->GetComponent<RigidBodyComponent>()->SetKinematic(true);
	m_pBeamMagicEmitter->GetComponent<RigidBodyComponent>()->AddCollider(PxBoxGeometry{ 2.5f, 2.5f, 150 }, *m_pDefaultMaterial);
	auto colliderInfo = m_pBeamMagicEmitter->GetComponent<RigidBodyComponent>()->GetCollider(0);
	colliderInfo.SetTrigger(true);

	m_pBeamMagicEmitter->SetOnTriggerCallBack([&](GameObject* /*pTriggerObject*/, GameObject* pOtherObject, PxTriggerAction action)
		{
			if (action == PxTriggerAction::ENTER)
			{
				if (auto enemy{ dynamic_cast<BaseEnemy*>(pOtherObject) }) {
					enemy->DamageBeamEnter(MagicResult.Damage);
				}
			}
			if (action == PxTriggerAction::LEAVE)
			{
				if (auto enemy{ dynamic_cast<BaseEnemy*>(pOtherObject) }) {
					enemy->DamageBeamExit();
				}
			}
		});

	//aoe
	m_pAOEMagicEmitter = AddChild(new TorusPrefab(50.f, 50, 20.f, 50, XMFLOAT4{1,0,0,1}));
	m_pAOEMagicEmitter->GetTransform()->Rotate(90,0,0);
	m_pAOEMagicEmitter->AddComponent(new RigidBodyComponent());
	m_pAOEMagicEmitter->GetComponent<RigidBodyComponent>()->AddCollider(PxBoxGeometry{ 70,70,70 }, *m_pDefaultMaterial);
	m_pAOEMagicEmitter->GetComponent<RigidBodyComponent>()->SetKinematic(true);
	colliderInfo = m_pAOEMagicEmitter->GetComponent<RigidBodyComponent>()->GetCollider(0);
	colliderInfo.SetTrigger(true);

	//shield
	//const auto pPxTriangleMesh = ContentManager::Load<PxTriangleMesh>(L"Meshes/Sphere.ovpt");
	//m_pShieldMagicEmitter->GetComponent<RigidBodyComponent>()->AddCollider(PxTriangleMeshGeometry(pPxTriangleMesh, PxMeshScale({ 10.f,10.f,10.f })), *m_pDefaultMaterial);


	//spray
	m_pSprayDamageColliderContainer = m_pCharacter->AddChild(new GameObject());
	m_pSprayDamageCollider = new GameObject();
	m_pSprayDamageColliderContainer->AddChild(m_pSprayDamageCollider);
	m_pSprayDamageCollider->GetTransform()->Translate(0, 0, 100);
	m_pSprayDamageCollider->AddComponent(new RigidBodyComponent());
	m_pSprayDamageCollider->GetComponent<RigidBodyComponent>()->AddCollider(PxBoxGeometry{ 20,20,100 }, * m_pDefaultMaterial);
	m_pSprayDamageCollider->GetComponent<RigidBodyComponent>()->SetKinematic(true);

	colliderInfo = m_pSprayDamageCollider->GetComponent<RigidBodyComponent>()->GetCollider(0);
	colliderInfo.SetTrigger(true);

	m_pSprayDamageCollider->SetOnTriggerCallBack([&](GameObject* /*pTriggerObject*/, GameObject* pOtherObject, PxTriggerAction action)
	{
		if (action == PxTriggerAction::LEAVE)
		{
			if (auto enemy{ dynamic_cast<BaseEnemy*>(pOtherObject) }) {
				enemy->DamageBeamExit();
			}
		}
	});

	m_pSprayMagicEmitter->SetVisibility(false);
	m_pBeamMagicEmitter->SetVisibility(false);
	m_pAOEMagicEmitter->SetVisibility(false);
}

void MagickaScene::CreateUI()
{
	m_pUI = AddChild(new GameObject());
	m_pUI->GetTransform()->Translate(100, m_SceneContext.windowHeight - 200, 0);

	m_pUI2 = AddChild(new GameObject());
	m_pUI2->GetTransform()->Translate(112.5f, m_SceneContext.windowHeight - 140, 0);

	m_pUI3 = AddChild(new GameObject());
	m_pUI3->GetTransform()->Translate(100, m_SceneContext.windowHeight/2, 0);

	//order is important in all 3 vectors (don't change)
	auto arr{ std::vector<std::wstring>{
			L"Textures/Magicka/Q.png",
			L"Textures/Magicka/E.png",
			L"Textures/Magicka/Z.png",
			L"Textures/Magicka/F.png",
	} };

	m_Textures = std::vector<std::wstring>{
			L"Textures/Magicka/Element_water.png",
			L"Textures/Magicka/Element_shield.png",
			L"Textures/Magicka/Element_life.png",
			L"Textures/Magicka/Element_cold.png",
			L"Textures/Magicka/Element_lightning.png",
			L"Textures/Magicka/Element_earth.png",
			L"Textures/Magicka/Element_arcane.png",
			L"Textures/Magicka/Element_fire.png",
	};
	m_TransparentTextures = std::vector<std::wstring>{
			L"Textures/Magicka/Element_water_transparent.png",
			L"Textures/Magicka/Element_shield_transparent.png",
			L"Textures/Magicka/Element_life_transparent.png",
			L"Textures/Magicka/Element_cold_transparent.png",
			L"Textures/Magicka/Element_lightning_transparent.png",
			L"Textures/Magicka/Element_earth_transparent.png",
			L"Textures/Magicka/Element_arcane_transparent.png",
			L"Textures/Magicka/Element_fire_transparent.png",
	};

	float height1{ 0 };
	float width1{ 0 };
	for (size_t i = 0; i < 4; i++)
	{
		auto go{ new GameObject() };
		m_pUI2->AddChild(go);
		go->AddComponent(new SpriteComponent(arr[i]));
		go->GetTransform()->Translate(width1, height1, 0);
		go->GetTransform()->Scale(0.2f);
		width1 += 60;
	}

	float height{ 0 };
	float width{ 0 };
	int id{ 0 };
	for (size_t i = 0; i < 2; i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			auto go{ new GameObject() };
			m_pUI->AddChild(go);
			go->AddComponent(new SpriteComponent(m_Textures[id]));
			go->GetTransform()->Translate(width, height, 0);
			width += 60;
			id++;
		}
		width = 0;
		height += 100;
	}

	float width2{ 0 };
	float height2{ 0 };
	for (size_t i = 0; i < 5; i++)
	{
		auto go{ new GameObject() };
		m_pUI3->AddChild(go);
		go->AddComponent(new SpriteComponent(L"Textures/Magicka/Element_none.png"));
		go->GetTransform()->Translate(width2, height2, 0);
		go->GetTransform()->Scale(0.6f);
		width2 += 35;
	}
}

void MagickaScene::DefineCameraSpline() {
	auto pos{ m_pCharacter->GetTransform()->GetPosition() };
	auto pos1{ XMFLOAT3{ pos.x + 20, pos.y + 10, pos.z - 200 } };
	auto pos2{ XMFLOAT3{ pos.x + 100, pos.y + 10, pos.z + 559 } };
	auto pos3{ XMFLOAT3{pos.x + 100, pos.y + 10, pos.z + 560} };
	auto pos4{ XMFLOAT3{ pos2.x + 50, pos.y + 10, pos2.z + 10 } };
	auto pos5{ XMFLOAT3{ pos2.x + 50, pos.y + 10, pos2.z + 11 } };
	auto pos6{ XMFLOAT3{ pos3.x + 350, pos.y + 10, pos3.z - 20 } };
	auto pos7{ XMFLOAT3{ pos3.x + 350, pos.y + 10, pos3.z - 19 } };
	auto pos8{ XMFLOAT3{ pos4.x + 400, pos.y + 10, pos4.z - 180 } };

	std::vector<XMFLOAT3> bezier{ pos1, pos2, pos3, pos4, pos5, pos6, pos7, pos8 };

	m_GameEnder = AddChild(new GameObject());
	m_GameEnder->AddComponent(new RigidBodyComponent());
	m_GameEnder->GetComponent<RigidBodyComponent>()->SetKinematic(true);
	m_GameEnder->GetComponent<RigidBodyComponent>()->AddCollider(PxBoxGeometry{ 30,10,30 }, *m_pDefaultMaterial);
	m_GameEnder->GetTransform()->Translate(pos8);

	auto colliderInfo = m_GameEnder->GetComponent<RigidBodyComponent>()->GetCollider(0);
	colliderInfo.SetTrigger(true);

	Sphere = AddChild(new SpherePrefab{ 10,20, XMFLOAT4{1,0,0,1} });
	Sphere->GetTransform()->Translate(pos);
	Sphere->SetHidden(true);
	m_pCamera->InitPoints(bezier);
}

void MagickaScene::CreateSound() {
	SoundSystem = SoundManager::Get()->GetSystem();
	auto file{ ContentManager::GetFullAssetPath(L"Sound/fire.wav") };
	SoundSystem->createSound(file.string().c_str(), FMOD_DEFAULT, nullptr, &FireSound);
	file = ContentManager::GetFullAssetPath(L"Sound/laser.wav");
	SoundSystem->createSound(file.string().c_str(), FMOD_DEFAULT, nullptr, &LaserSound);
	file = ContentManager::GetFullAssetPath(L"Sound/aoe.mp3");
	SoundSystem->createSound(file.string().c_str(), FMOD_DEFAULT, nullptr, &AoeSound);
	file = ContentManager::GetFullAssetPath(L"Sound/projectile.wav");
	SoundSystem->createSound(file.string().c_str(), FMOD_DEFAULT, nullptr, &ProjectileSound);
}
#pragma endregion

void MagickaScene::OnGUI()
{
	m_pCharacter->DrawImGui();
}

void MagickaScene::Update()
{
	if (IsGameOver) return;

	XMFLOAT3 pos2{ m_pCharacter->GetTransform()->GetWorldPosition()};

	HandleEmitterMovement(pos2);

	HandleUIMovement(pos2);

	HandleCameraMovement();

	HandleMagicTransform();

	HandleTimers();

	HandleEnemies();

	if (m_pCharacter->GetHealth() <= 0) {
		SetGameOver(false);
	}

	if (m_pEnemyHolder->GetChildren<MagickaCharacter>().size() == 0 && m_pCamera->GetReachedEnd()) {
		SetGameOver(true);
	}

	if (auto character{ m_GameEnder->GetCharacterInRange<MagickaCharacter>() }) {
		SetGameOver(true);
	}

	const auto pCameraTransform = m_SceneContext.pCamera->GetTransform();
	m_SceneContext.pLights->SetDirectionalLight(pCameraTransform->GetPosition(), pCameraTransform->GetForward());

	SoundManager::Get()->GetSystem()->update();

	if (SceneManager::Get()->RestartTriggered) {
		ResetGame();
		SceneManager::Get()->RestartTriggered = false;
	}
}

#pragma region Update
void MagickaScene::HandleTimers() {
	float deltaTime = m_SceneContext.pGameTime->GetElapsed();

	if (AoeFired) {
		AoeTimer -= deltaTime;
		if (AoeTimer <= 0) {
			ResetCombo();
			m_pCharacter->ResetMoveSpeed();
		}
	}

	if (IsChargingProjectile) {
		if (ProjectileTimer < MaxProjectileTimer) {
			ProjectileTimer += deltaTime;
		}
	}

	if (IsShootingIceProjectile) {
		ProjectileTimer -= deltaTime;
		if (ProjectileTimer <= 0) {
			FireProjectileBarage();
			ProjectileTimer = MaxIceProjectileTimer;
		}
	}
}

void MagickaScene::HandleEmitterMovement(XMFLOAT3 pos) {
	XMFLOAT3 pos2{ m_pCharacter->GetTransform()->GetForward() };
	pos2.x *= 400 + pos.x;
	pos2.z *= 400 + pos.z;
	m_pAOEMagicEmitter->GetComponent<RigidBodyComponent>()->UpdatePosition(pos, m_pAOEMagicEmitter->GetTransform()->GetRotation());
	m_pBeamMagicEmitter->GetTransform()->Rotate(XMFLOAT3{ 0,0,0 });
	m_pSprayDamageCollider->GetComponent<RigidBodyComponent>()->UpdatePosition(XMFLOAT3{1,1,220}, m_pSprayDamageCollider->GetTransform()->GetRotation());

	m_pPlayerDamageTakingCollider->GetComponent<RigidBodyComponent>()->UpdatePosition(pos, m_pCharacter->GetTransform()->GetRotation());
	m_pPlayerMaxEnemyRangeCollider->GetComponent<RigidBodyComponent>()->UpdatePosition(pos, m_pCharacter->GetTransform()->GetRotation());
	m_pCanEnemyMoveCollider->GetComponent<RigidBodyComponent>()->UpdatePosition(pos, m_pCharacter->GetTransform()->GetRotation());
}

void MagickaScene::HandleUIMovement(const XMFLOAT3 pos) {
	XMVECTOR screenPosition = XMVector3Project(XMLoadFloat3(&pos), 0, 0, m_SceneContext.windowWidth, m_SceneContext.windowHeight, 0.0f, 1.0f, XMLoadFloat4x4(&m_SceneContext.pCamera->GetProjection()), XMLoadFloat4x4(&m_SceneContext.pCamera->GetView()), XMMatrixIdentity());
	XMFLOAT3 screenPos;
	XMStoreFloat3(&screenPos, screenPosition);
	screenPos.x -= 90;
	screenPos.y += 30;
	m_pUI3->GetTransform()->Translate(screenPos);
}

void MagickaScene::HandleMagicTransform()
{
	auto originalRotation{ m_pCharacter->GetComponent<ModelComponent>()->GetTransform()->GetWorldRotation() };
	auto originalLocation{ m_pCharacter->GetComponent<ModelComponent>()->GetTransform()->GetWorldPosition() };

	m_pSprayMagicEmitter->GetTransform()->Translate(originalLocation);
	//m_pBeamMagicEmitter->GetTransform()->Translate(originalLocation);
	//m_pBeamMagicEmitter->GetTransform()->Rotate(XMFLOAT3{ originalRotation.x, originalRotation.y, originalRotation.z });

	XMVECTOR rotationQuaternion = XMLoadFloat4(&originalRotation);
	rotationQuaternion = XMQuaternionMultiply(rotationQuaternion, XMQuaternionRotationRollPitchYaw(0.0f, -XM_PIDIV2, 0.0f));
	rotationQuaternion = XMQuaternionNormalize(rotationQuaternion);

	//apply rotation to the particles
	auto comps{ m_pSprayMagicEmitter->GetComponents<ParticleEmitterComponent>() };
	for (auto comp : comps) {
		auto velo{ comp->GetSettings().originalVelocity };
		XMVECTOR vectorToRotate = XMLoadFloat3(&velo);
		XMVECTOR rotatedVector = XMVector3Rotate(vectorToRotate, rotationQuaternion);
		XMFLOAT3 rot;
		XMStoreFloat3(&rot, rotatedVector);
		comp->GetSettings().velocity = rot;
	}

}

void MagickaScene::HandleCameraMovement()
{
	// The number of steps to take along the Bezier curve to find the closest point
	const auto objectPosition{ m_pCharacter->GetTransform()->GetWorldPosition() };

	const auto closestPoint{ MathHelper::CalculateProjectedPointOnBezier(4000, objectPosition, m_pCamera->GetControlPoints()) };

	Sphere->GetTransform()->Translate(closestPoint);

	auto pos{ closestPoint };
	pos.x += 250;
	pos.y += 300;
	pos.z -= 80;
	m_SceneContext.pCamera->GetTransform()->Translate(pos);
}

void MagickaScene::HandleEnemies()
{
	for (auto projectile : m_pProjectileHolder->GetChildren<Projectile>()) {
		if (projectile->IsMarkedForDelete()) {
			m_pProjectileHolder->RemoveChild(projectile, true);
		}
	}
	
	for (auto enemy : m_pEnemyHolder->GetChildren<EnemyRangedCharacter>()) {
		if (enemy->GetHealth() <= 0) {
			enemy->SetMarkedForDestroy(true);
			m_pEnemyHolder->RemoveChild(enemy, true);
		}
	}
	for (auto enemy : m_pEnemyHolder->GetChildren<EnemyMeleeCharacter>()) {
		if (enemy->GetHealth() <= 0) {
			enemy->SetMarkedForDestroy(true);
			m_pEnemyHolder->RemoveChild(enemy, true);
		}
	}

	for (auto enemy : m_pPlayerDamageTakingCollider->GetEnemiesInRange()) {
		if (enemy && !enemy->GetMarkedForDestroy() && enemy != nullptr) {
			if (auto hittingEnemy{ dynamic_cast<EnemyMeleeCharacter*>(enemy) }) {
				if (hittingEnemy->GetCanAttack()) {
					hittingEnemy->SetCanAttack(false);
					m_pCharacter->DamagePlayer(true, hittingEnemy->GetAttackDamage());
				}
			}
		}
		else {
			break;
		}
	}
}
#pragma endregion

void MagickaScene::Draw(){/* m_pCamera->Draw(m_SceneContext);*/ }

#pragma region Execution
void MagickaScene::ExecuteMagicCombo()
{
	if (!IsExecutingMagic) {
		IsExecutingMagic = true;
		
		FillMagicResultData();

		if (m_ComboBar.size() > 0) {
			bool isPlaying;
			CurrentChannel->isPlaying(&isPlaying);

			const auto spell{ std::find_if(m_MagicSpells.begin(), m_MagicSpells.end(), [&](const MagicSpell& spell) {
				return spell.Pattern == m_ComboPattern;
			}) };
			const auto& selectedMagicShield{ std::find_if(m_ComboBar.begin(), m_ComboBar.end(), [&](const Magic& magic) {
				return magic.DefaultMagicType.ProjectileType == ProjectileTypes::SHIELD;
			}) };
			const auto& selectedMagicProjectile{ std::find_if(m_ComboBar.begin(), m_ComboBar.end(), [&](const Magic& magic) {
				return magic.DefaultMagicType.ProjectileType == ProjectileTypes::PROJECTILE;
			}) };			
			const auto& selectedMagicProjectileBomb{ std::find_if(m_ComboBar.begin(), m_ComboBar.end(), [&](const Magic& magic) {
				return magic.DefaultMagicType.ProjectileType == ProjectileTypes::PROJECTILEBOMB;
			}) };
			const auto& selectedMagicBeam{ std::find_if(m_ComboBar.begin(), m_ComboBar.end(), [&](const Magic& magic) {
				return magic.DefaultMagicType.ProjectileType == ProjectileTypes::BEAM;
			}) };			
			const auto& selectedMagicSpray{ std::find_if(m_ComboBar.begin(), m_ComboBar.end(), [&](const Magic& magic) {
				return magic.DefaultMagicType.ProjectileType == ProjectileTypes::SPRAY;
			}) };
			if (spell != m_MagicSpells.end()) {
				//cast the selected spell
				HandlePrint2(spell->Spell);
				spell->ExecuteSpell();
				ResetCombo();
			}
			else if (selectedMagicShield != m_ComboBar.end()) {
				MagicResult.ProjectileType = ProjectileTypes::SHIELD;
				MagicResult.BaseElementType = selectedMagicShield->ElementType;
				return;
			}
			else if (selectedMagicProjectileBomb != m_ComboBar.end()) {
				MagicResult.ProjectileType = ProjectileTypes::PROJECTILEBOMB;
				MagicResult.BaseElementType = selectedMagicProjectileBomb->ElementType;

				ChargeProjectile(true);
				return;
				//FireProjectile(true);
			}
			else if (selectedMagicProjectile != m_ComboBar.end()) {
				MagicResult.ProjectileType = ProjectileTypes::PROJECTILE;
				MagicResult.BaseElementType = selectedMagicProjectile->ElementType;

				if (MagicResult.BaseElementType == ElementTypes::ICE) {
					IsShootingIceProjectile = true;
					ProjectileTimer = MaxIceProjectileTimer;
				}
				else {
					ChargeProjectile(true);
				}
				return;
				//FireProjectile();
			}
			else if (selectedMagicBeam != m_ComboBar.end()) {
				MagicResult.ProjectileType = ProjectileTypes::BEAM;
				MagicResult.BaseElementType = selectedMagicBeam->ElementType;
				m_pBeamMagicEmitter->SetVisibility(true);

				if (!isPlaying) SoundSystem->playSound(LaserSound, nullptr, false, &CurrentChannel);
				return;
			}
			else {
				MagicResult.ProjectileType = ProjectileTypes::SPRAY;
				MagicResult.BaseElementType = selectedMagicSpray->ElementType;
				m_pSprayMagicEmitter->SetVisibility(true);
				for (auto comp : m_pSprayMagicEmitter->GetComponents<ParticleEmitterComponent>()) {
					auto it = MagicResult.TextureName.begin(); // Iterator to the beginning of the set
					std::advance(it, 0);
					comp->SetTexture(m_SceneContext, *it);
				}

				if (!isPlaying) SoundSystem->playSound(FireSound, nullptr, false, &CurrentChannel);
			}
		}
	}


	if (MagicResult.Modifiers.size() > 0 && m_ComboBar.size() > 0 && !IsChargingProjectile && MagicResult.ProjectileType == ProjectileTypes::SPRAY) {
		m_pSprayDamageCollider->GetComponent<RigidBodyComponent>()->SetEnableCollision(true);

		for (auto enemy : m_pSprayDamageCollider->GetEnemiesInRange()) {
			if (enemy->GetMarkedForDestroy()) {
				m_pSprayDamageCollider->GetEnemiesInRange().remove(enemy);
				break;
			}

			if (auto melee{ dynamic_cast<BaseEnemy*>(enemy) }) {
				if (!melee->GetCanDamage()) {
					melee->DamageBeamEnter(MagicResult.Damage);
				}
			}
		}
	}

}

void MagickaScene::ExecuteAOE()
{
	if (!IsExecutingMagic && m_ComboBar.size() > 0) {
		IsExecutingMagic = true;

		auto components{ m_pAOEMagicEmitter->GetComponents<ParticleEmitterComponent>() };
		MagicResult.ProjectileType = ProjectileTypes::AOE;

		FillMagicResultData();

		//find most occuring character in the pattern
		const auto count = std::unordered_multiset<char>{ m_ComboPattern.begin(), m_ComboPattern.end() };
		const auto comparator = [count](auto a, auto b) { return count.count(a) < count.count(b); };
		if (!count.empty()) {
			auto character{ *std::max_element(count.begin(), count.end(), comparator) };
			const auto selectedMagic{ std::find_if(m_Magics.begin(), m_Magics.end(), [&](const Magic& magic) {
				return magic.ElementType == static_cast<ElementTypes>(character);
			}) };
			if (selectedMagic != m_Magics.end()) {
				MagicResult.Damage = selectedMagic->DefaultMagicType.AoEDamage;
				MagicResult.BaseElementType = selectedMagic->ElementType;
			}
		}

		if (MagicResult.Modifiers.size() > 0) {
			m_pAOEMagicEmitter->GetComponent<RigidBodyComponent>()->SetEnableCollision(true);
			m_pAOEMagicEmitter->SetVisibility(true);
			m_pSprayMagicEmitter->SetVisibility(false);
			AoeFired = true;

			bool isPlaying;
			CurrentChannel->isPlaying(&isPlaying);
			if (!isPlaying) SoundSystem->playSound(AoeSound, nullptr, false, &CurrentChannel);

			for (auto enemy : m_pAOEMagicEmitter->GetEnemiesInRange()) {
				if (enemy->GetMarkedForDestroy()) {
					m_pAOEMagicEmitter->GetEnemiesInRange().remove(enemy);
					break;
				}

				if (auto melee{ dynamic_cast<BaseEnemy*>(enemy) }) {
					melee->SetCanDamageAoE(true);
					melee->DamageAOE(MagicResult.Damage);
				}
				/*if (auto ranged{ dynamic_cast<EnemyRangedCharacter*>(enemy) }) {
					ranged->SetCanDamageAoE(true);
					ranged->DamageAOE(MagicResult.Damage);
				}*/
			}
		}
	}
}

void MagickaScene::ExecuteSelfCast()
{

}

void MagickaScene::ExecuteSword()
{
	const auto spell{ std::find_if(m_MagicSpells.begin(), m_MagicSpells.end(), [&](const MagicSpell& spell) {
		return spell.Pattern == m_ComboPattern;
	}) };

	if (spell != m_MagicSpells.end()) {
		//cast the selected spell
		HandlePrint2(spell->Spell);

		ResetCombo();
	}
	else {
		//sword cast
		//sword attack
		
	}
}

void MagickaScene::FireProjectile(bool /*isBomb*/)
{
	if (IsChargingProjectile) {
		IsChargingProjectile = false;
		//add damage the more the projectile is charged
		MagicResult.Damage *= ProjectileTimer;
		auto trans{ m_pCharacter->GetTransform() };
		float force{ ProjectileTimer * 3.5f };
		float downForce{ 1 };
		
		//increase downwards velocity based on how slow the projectile is
		int timerToInt{ (int)ProjectileTimer };
		if (timerToInt == 3) {
			downForce = 1.4f;
		}
		else if (timerToInt == 4) {
			downForce = 1.1f;
		}
		else if (timerToInt == 5) {
			downForce = 0.8f;
		}
		else {
			downForce += ProjectileTimer;
		}

		float launchSpeed{ 100 };
		launchSpeed += launchSpeed * force;
		auto go{ new Projectile(L"Meshes/Rock.ovm", trans->GetForward(), trans->GetWorldPosition(), launchSpeed, downForce, m_pProjectileMaterial, m_pDefaultMaterial, m_pProjectileHolder, IsBombProjectile) };
		m_pProjectileHolder->AddChild(go);
		go->SetDamageToGive(MagicResult.Damage);
	}
}

void MagickaScene::FireProjectileBarage() {
	auto trans{ m_pCharacter->GetTransform() };
	auto forward{ trans->GetForward() };
	auto originalPosition{ trans->GetWorldPosition() };
	auto rotation{ trans->GetWorldRotation() };
	auto go{ new Projectile(L"Meshes/IceCone.ovm", forward, originalPosition, 1000, 1, m_pProjectileMaterial, m_pDefaultMaterial, m_pProjectileHolder, false) };
	m_pProjectileHolder->AddChild(go);
	go->SetDamageToGive(MagicResult.Damage);
	go->GetTransform()->Scale(2);

	//apply rotation to character
	rotation.x += 90;
	auto newRot = MathHelper::GetRotationTowardsPoint(originalPosition, forward, rotation, true);
	go->GetTransform()->Rotate(newRot, false);
	SoundSystem->playSound(ProjectileSound, nullptr, false, &CurrentChannel);
}

void MagickaScene::ResetCombo()
{
	m_ComboPattern = "";
	m_ComboBar.clear();
	m_ComboBar.reserve(5);

	m_pSprayMagicEmitter->SetVisibility(false);
	m_pBeamMagicEmitter->SetVisibility(false);
	m_pAOEMagicEmitter->SetVisibility(false);
	MagicResult.Modifiers.clear();
	MagicResult.TextureName.clear();
	MagicResult.AOETextureName.clear();
	MagicResult.Damage = 0;
	MagicResult.BaseElementType = ElementTypes::NONE;
	IsExecutingMagic = false;
	ProjectileTimer = 0;

	AoeFired = false;
	AoeTimer = 2;

	for (auto go : m_pUI3->GetChildren<GameObject>()) {
		go->GetComponent<SpriteComponent>()->SetTexture(L"Textures/Magicka/Element_none.png");
	}

	m_pAOEMagicEmitter->GetComponent<RigidBodyComponent>()->SetEnableCollision(false);
	m_pSprayDamageCollider->GetComponent<RigidBodyComponent>()->SetEnableCollision(false);
	IsShootingIceProjectile = false;

	if (CurrentChannel) {
		bool isPlaying;
		CurrentChannel->isPlaying(&isPlaying);
		if (isPlaying) CurrentChannel->stop();
	}
}
#pragma endregion

#pragma region UI
void MagickaScene::ToggleUIElements(bool shiftPressed)
{
	auto arr{ m_pUI->GetChildren<GameObject>() };
	for (size_t i = 0; i < arr.size(); i++)
	{
		auto comp{ arr[i]->GetComponent<SpriteComponent>() };
		if (i < 4) {
			comp->SetTexture(shiftPressed ? m_TransparentTextures[i] : m_Textures[i]);
		}
		else {
			comp->SetTexture(shiftPressed ? m_Textures[i] : m_TransparentTextures[i]);
		}
	}
}
#pragma endregion

#pragma region Helpers
Magic* MagickaScene::GetCombinedMagicType(ElementTypes elementType1, ElementTypes elementType2)
{
	const auto it{ std::find_if(m_CombinedMagics.begin(), m_CombinedMagics.end(), [&elementType1, &elementType2](const Magic& magic) {
		//safe to check like this because [0] will always be the water magic
		return magic.CombiningMagicTypes[0] == elementType1 && magic.CombiningMagicTypes[1] == elementType2;
	}) };

	if (it != m_CombinedMagics.end()) {
		return it._Ptr;
	}
	else {
		return nullptr;
	}
}

bool MagickaScene::IsMagicCancelled(int currentIterator, const Magic* currentMagic, bool& foundCanceller)
{
	for (ElementTypes cancellingMagicType : m_ComboBar[currentIterator].CancellingMagicTypes) {
		if (cancellingMagicType == currentMagic->ElementType) {
			foundCanceller = true;
			std::cout << "CANCELLED" << ", Pattern: " << m_ComboPattern << std::endl;
			return true;
		}
	}
	return false;
}

void MagickaScene::FillMagicResultData()
{
	//set up result magic
	for (const auto& type : m_ComboBar) {
		//add modifiers to the result magic
		const auto magic{ std::find_if(MagicResult.Modifiers.begin(), MagicResult.Modifiers.end(), [&](const ElementTypes& magic) {
			return magic == type.ElementType;
		}) };

		//push back only 1 of the elements
		MagicResult.Modifiers.insert(type.ElementType);
		MagicResult.AOETextureName.insert(type.AOETextureName);
		MagicResult.TextureName.insert(type.TextureName);

		////add the damage from the extra element once
		//if (magic == MagicResult.Modifiers.end()) {
		//}
		//adds damage from extra elements
		MagicResult.Damage += type.DefaultMagicType.AddedDamage;
	}
}

void MagickaScene::ChargeProjectile(bool isBomb)
{
	IsChargingProjectile = true;
	IsBombProjectile = isBomb;
}
#pragma endregion

#pragma region Combo-ing
void MagickaScene::HandleCombobarFilling(ElementTypes elementType)
{	
	//find the spell of the current element type
	const auto comparator{ [&elementType](const Magic& magic) { return magic.ElementType == elementType; } };
	Magic* currentMagic{ std::find_if(m_Magics.begin(), m_Magics.end(), comparator)._Ptr };

	//get index of previous element
	const int currentIterator{ static_cast<int>(m_ComboBar.size()) - 1 };
	bool foundCanceller{ false };
	bool isCombined{ false };

	if (m_ComboBar.empty()) {
		//push the magic
		m_ComboBar.push_back(*currentMagic);
		m_ComboPattern += currentMagic->ElementType;
		HandlePrint(currentMagic->ElementType);
		m_pUI3->GetChildren<GameObject>()[m_ComboBar.size() - 1]->GetComponent<SpriteComponent>()->SetTexture(currentMagic->UITextureName);
	}
	else if (!m_ComboBar.empty() && m_ComboBar.size() <= 5) {
		//remove element from pattern if opposite and return
		if (IsMagicCancelled(currentIterator, currentMagic, foundCanceller))
		{
			m_pUI3->GetChildren<GameObject>()[m_ComboBar.size() - 1]->GetComponent<SpriteComponent>()->SetTexture(L"Textures/Magicka/Element_none.png");
			m_ComboBar.pop_back();
			m_ComboPattern.pop_back();
			return;
		}

		//get the previous elemet to check against (for element combination)
		const Magic prevMagic{ m_ComboBar[currentIterator] };

		if (!foundCanceller) {
			//check for combined magic
			if (!prevMagic.CombiningMagicTypes.empty() && !currentMagic->CombiningMagicTypes.empty()) {

				//Water should be the first element to check
				if (auto magic{ 
					prevMagic.ElementType == ElementTypes::WATER ?
						GetCombinedMagicType(prevMagic.ElementType, currentMagic->ElementType) : 
						GetCombinedMagicType(currentMagic->ElementType, prevMagic.ElementType) 
				}) {
					m_ComboBar.pop_back();
					m_ComboPattern.pop_back();
					currentMagic = magic;
					isCombined = true;
				}
			}

			if (m_ComboBar.size() < 5) {
				//push the magic
				m_ComboBar.push_back(*currentMagic);
				m_ComboPattern += currentMagic->ElementType;
				HandlePrint(currentMagic->ElementType);
				m_pUI3->GetChildren<GameObject>()[m_ComboBar.size() - 1]->GetComponent<SpriteComponent>()->SetTexture(currentMagic->UITextureName);
			}
			else if (isCombined)
			{
				m_pUI3->GetChildren<GameObject>()[m_ComboBar.size() - 1]->GetComponent<SpriteComponent>()->SetTexture(currentMagic->UITextureName);
			}
		}
	}
}
#pragma endregion

#pragma region Debug
void MagickaScene::HandlePrint(const char currentMagic) const {
	std::string name;
	switch (static_cast<ElementTypes>(currentMagic))
	{
	case ElementTypes::WATER: name = "WATER"; break;
	case ElementTypes::LIGHTNING: name = "LIGHTNING"; break;
	case ElementTypes::LIFE: name = "LIFE"; break;
	case ElementTypes::ARCANE: name = "ARCANE"; break;
	case ElementTypes::SHIELD: name = "SHIELD"; break;
	case ElementTypes::EARTH: name = "EARTH"; break;
	case ElementTypes::COLD: name = "COLD"; break;
	case ElementTypes::FIRE: name = "FIRE"; break;
	case ElementTypes::STEAM: name = "STEAM"; break;
	case ElementTypes::ICE: name = "ICE"; break;
	case ElementTypes::POISON: name = "POISON"; break;
	default: break;
	}
	std::cout << "Most occuring Element: " << name << std::endl;

}

void MagickaScene::HandlePrint(const ElementTypes currentMagic) const {
	std::string name;
	switch (currentMagic)
	{
	case ElementTypes::WATER: name = "WATER"; break;
	case ElementTypes::LIGHTNING: name = "LIGHTNING"; break;
	case ElementTypes::LIFE: name = "LIFE"; break;
	case ElementTypes::ARCANE: name = "ARCANE"; break;
	case ElementTypes::SHIELD: name = "SHIELD"; break;
	case ElementTypes::EARTH: name = "EARTH"; break;
	case ElementTypes::COLD: name = "COLD"; break;
	case ElementTypes::FIRE: name = "FIRE"; break;
	case ElementTypes::STEAM: name = "STEAM"; break;
	case ElementTypes::ICE: name = "ICE"; break;
	case ElementTypes::POISON: name = "POISON"; break;
	default: break;
	}

	std::cout << "Type: " << name << ", Size: " << m_ComboBar.size() << ", Pattern: " << m_ComboPattern << std::endl;
}

void MagickaScene::HandlePrint2(Spells spell) const {
	std::string name;
	switch (spell)
	{
	case Spells::PUSH: name = "PUSH"; break;
	case Spells::EMERGENCYTELEPORT: name = "EMERGENCYTELEPORT"; break;
	case Spells::DISPEL: name = "DISPEL"; break;
	case Spells::RANDOMSPELL: name = "RANDOMSPELL"; break;
	case Spells::TELEPORT: name = "TELEPORT"; break;
	case Spells::HASTE: name = "HASTE"; break;
	case Spells::DISRUPTOR: name = "DISRUPTOR"; break;
	case Spells::CONCUSSION: name = "CONCUSSION"; break;
	case Spells::THUNDERBOLT: name = "THUNDERBOLT"; break;
	case Spells::SPIKEQUAKE: name = "SPIKEQUAKE"; break;
	case Spells::ICETORNADO: name = "ICETORNADO"; break;
	case Spells::BREEZE: name = "BREEZE"; break;
	case Spells::GUARDIAN: name = "GUARDIAN"; break;
	case Spells::DRAGONSTRIKE: name = "DRAGONSTRIKE"; break;
	case Spells::FOWL: name = "FOWL"; break;
	case Spells::THUNDERHEAD: name = "THUNDERHEAD"; break;
	case Spells::SUMMON: name = "SUMMON"; break;
	case Spells::SACRIFICE: name = "SACRIFICE"; break;
	case Spells::REVIVE: name = "REVIVE"; break;
	default: break;
	}

	std::cout << "Casted spell: " << name << std::endl;
}
#pragma endregion

#pragma region Menu and start/end
void MagickaScene::SetGameOver(bool hasWon) {
	IsGameOver = true;
	SceneManager::Get()->HasWon = hasWon;
	ResetGame();
	SceneManager::Get()->SetActiveGameScene(L"EndGameMenu");
}

void MagickaScene::ResetGame() {

	if (!m_pProjectileHolder->GetMarkedForDestroy()) {
		for (auto projectile : m_pProjectileHolder->GetChildren<Projectile>()) {
			if (!projectile->GetMarkedForDestroy())
				m_pProjectileHolder->RemoveChild(projectile, true);

		}
	}

	if (!m_pCharacter->GetMarkedForDestroy()) {
		RemoveChild(m_pCharacter, true);
	}

	if (!m_pEnemyHolder->GetMarkedForDestroy()) {
		for (auto enemy : m_pEnemyHolder->GetChildren<EnemyRangedCharacter>()) {
			if (!enemy->GetMarkedForDestroy())
				m_pEnemyHolder->RemoveChild(enemy, true);

		}
		for (auto enemy : m_pEnemyHolder->GetChildren<EnemyMeleeCharacter>()) {
			if (!enemy->GetMarkedForDestroy())
				m_pEnemyHolder->RemoveChild(enemy, true);
		
		}
	}

	if (!m_pBeamMagicEmitterContainer->GetMarkedForDestroy()) {
		RemoveChild(m_pBeamMagicEmitterContainer, true);
		m_pBeamMagicEmitter = nullptr;
	}
	if (!m_pSprayDamageColliderContainer->GetMarkedForDestroy()) {
		RemoveChild(m_pSprayDamageColliderContainer, true);
		m_pSprayDamageCollider = nullptr;
	}
	if(!m_pAOEMagicEmitter->GetMarkedForDestroy())RemoveChild(m_pAOEMagicEmitter, true);

	CreateCharacter();

	DefineCameraSpline();

	m_pEnemyHolder = AddChild(new GameObject());
	m_pProjectileHolder = AddChild(new GameObject());
	float width{ 50 };
	int nrOfMelee{ 5 };
	int nrOfRanged{ 0 };
	auto position{ m_pCamera->GetLineOfIndex(0) };
	CreateMeleeEnemies(width, position, nrOfMelee);
	width = 50;
	position = m_pCamera->GetLineOfIndex(2);
	position.z -= 300;
	nrOfRanged = 2;
	CreateMeleeEnemies(width, position, nrOfMelee);
	position.z += 50;
	CreateRangedEnemies(width, position, nrOfRanged);
	width = 50;
	position = m_pCamera->GetLineOfIndex(2);
	position.x += 300;
	position.z += 300;
	nrOfRanged = 5;
	CreateMeleeEnemies(width, position, nrOfMelee);
	position.x -= 100;
	position.z -= 50;
	CreateRangedEnemies(width, position, nrOfRanged);

	CreateInput();

	CreateEmitters();

	SetStartPos();

	ResetCombo();

	IsGameOver = false;
}
#pragma endregion
