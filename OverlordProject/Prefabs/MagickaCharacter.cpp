#include "stdafx.h"
#include "MagickaCharacter.h"
#include "../OverlordEngine/Prefabs/MagickaCamera.h"
#include "./Scenes/Exam/MagickaScene.h"
#include <Scenes/Exam/Menu.h>

MagickaCharacter::MagickaCharacter(const CharacterDescExtended& characterDesc) :
	m_CharacterDescExtended{ characterDesc },
	m_MoveAcceleration(characterDesc.maxMoveSpeed / characterDesc.moveAccelerationTime),
	m_FallAcceleration(characterDesc.maxFallSpeed / characterDesc.fallAccelerationTime)
{}

void MagickaCharacter::Initialize(const SceneContext& /*sceneContext*/)
{
	//Controller
	m_pControllerComponent = AddComponent(new ControllerComponent(m_CharacterDescExtended.controller));

	auto file = ContentManager::GetFullAssetPath(L"Sound/game.wav");
	SoundSystem->createStream(file.string().c_str(), FMOD_DEFAULT, nullptr, &GameMusic);

	file = ContentManager::GetFullAssetPath(L"Sound/menu.wav");
	SoundSystem->createStream(file.string().c_str(), FMOD_DEFAULT, nullptr, &MenuMusic);

	SoundSystem->playSound(GameMusic, nullptr, false, &CurrentChannel);
}

void MagickaCharacter::Update(const SceneContext& sceneContext)
{
	auto Animator = GetComponent<ModelComponent>()->GetAnimator();
	float deltaTime = sceneContext.pGameTime->GetElapsed();
	//m_CharacterDescExtended.maxMoveSpeed = 50;
	if (!CanButton) {
		ButtonTimer -= deltaTime;
		if (ButtonTimer <= 0) {
			ButtonTimer = 0.2f;
			CanButton = true;
		}
	}


	CanWalk = true;
	//## Input Gathering (move, look, rotation)
	XMFLOAT2 look{ 0.f, 0.f };
	XMFLOAT3 newRot{ 0.f, 0.f, 0.f };
	constexpr float epsilon{ 0.01f };

	bool shiftPressed{ false };
	HandleInput(sceneContext, shiftPressed);

	MagickaScene* scene{ reinterpret_cast<MagickaScene*>(SceneManager::Get()->GetActiveScene()) };
	scene->ToggleUIElements(shiftPressed);

	HandleRotation(sceneContext, look, epsilon, newRot);

	HandleMove(sceneContext, newRot, epsilon);

	if (!CanWalk) return;
	Animator->SetAnimation(0);
	Animator->Play();
}

float MapRange(float value, float inputMin, float inputMax, float outputMin, float outputMax) {
	// Calculate the ratio of the input value within the input range
	float ratio = (value - inputMin) / (inputMax - inputMin);

	// Map the ratio to the output range
	float outputValue = outputMin + ratio * (outputMax - outputMin);

	return outputValue;
}
void MagickaCharacter::HandleRotation(const SceneContext& sceneContext, DirectX::XMFLOAT2& look, const float& epsilon, DirectX::XMFLOAT3& newRot)
{
	//Only if the Left Mouse Button is Down >
	auto mousePos = sceneContext.pInput->GetMousePosition();
	auto movement = sceneContext.pInput->GetMouseMovement();

	auto originalRotation{ GetComponent<ModelComponent>()->GetTransform()->GetWorldRotation() };
	auto originalLocation{ GetComponent<ModelComponent>()->GetTransform()->GetWorldPosition() };

	float yaw{ 0 };
	if (sceneContext.pInput->IsGamepadConnected(GamepadIndex::playerOne)) {
		XMFLOAT2 look1, look2;
		look1 = sceneContext.pInput->GetThumbstickPosition(false); // right thumbstick
		look2 = sceneContext.pInput->GetThumbstickPosition(true); // left thumbstick
		// here we should change yaw to the input of the thumbstick
		if (abs(look2.x) > epsilon || abs(look2.y) > epsilon) {
			look = look2;
			yaw = -atan2(look.y, look.x);
		}
		if (abs(look1.x) > epsilon || abs(look1.y) > epsilon) {
			look = look1;
			yaw = -atan2(look.y, look.x);
		}
		newRot = XMFLOAT3{ originalRotation.x, yaw, originalRotation.z };
	}
	else {
		newRot = MathHelper::GetRotationTowardsPoint(originalLocation, sceneContext.pCamera->PickPosition(), originalRotation, false);
	}

	//apply rotation to character
	GetComponent<ModelComponent>()->GetTransform()->Rotate(newRot, false);
}

void MagickaCharacter::HandleMove(const SceneContext& sceneContext, DirectX::XMFLOAT3& /*newRot*/, const float& epsilon)
{
	//move character to mouse pos
	if (!sceneContext.pInput->IsGamepadConnected(GamepadIndex::playerOne)) {
		if (sceneContext.pInput->IsMouseButton(InputState::down, 1))
		{
			CalculateForwardMovement(epsilon, sceneContext);
		}
	}
	else {
		auto look = sceneContext.pInput->GetThumbstickPosition(true);
		auto look3 = sceneContext.pInput->GetThumbstickPosition(false);
		if (abs(look.x) > epsilon || abs(look.y) > epsilon) {
			if (abs(look3.x) > epsilon || abs(look3.y) > epsilon) {
				//when using controller, your left thumbstick defines rotation and moving
				//your right thumbstick defines rotation alone and takes priority over the rotation of the left stick.
				//left stick still defines the move direction

				XMFLOAT3 look2{ look.x, 0, look.y };
				//get forward vector
				float deltaTime = sceneContext.pGameTime->GetElapsed();

				XMFLOAT3 forward{ GetTransform()->GetForward() };
				look2.x *= deltaTime * m_CharacterDescExtended.maxMoveSpeed;
				look2.z *= deltaTime * m_CharacterDescExtended.maxMoveSpeed;
				m_CurrentDirection = forward;

				//calculate point to move to (based on forward vector coming from mouse rotation)
				XMFLOAT3 displacement;
				XMVECTOR displacementVec = XMVectorScale(XMLoadFloat3(&look2), 1.f);
				XMStoreFloat3(&displacement, displacementVec);

				auto Animator = GetComponent<ModelComponent>()->GetAnimator();
				Animator->SetAnimation(2);
				Animator->Play();
				CanWalk = false;

				GetComponent<ControllerComponent>()->Move(displacement, epsilon);
			}
			else {
				CalculateForwardMovement(epsilon, sceneContext);
			}
		}
	}
}

void MagickaCharacter::CalculateForwardMovement(const float& epsilon, const SceneContext& sceneContext)
{
	//get forward vector
	float deltaTime = sceneContext.pGameTime->GetElapsed();

	XMFLOAT3 forward{ GetTransform()->GetForward() };
	forward.x *= deltaTime * m_CharacterDescExtended.maxMoveSpeed;
	forward.z *= deltaTime * m_CharacterDescExtended.maxMoveSpeed;
	m_CurrentDirection = forward;

	//calculate point to move to (based on forward vector coming from mouse rotation)
	XMFLOAT3 displacement;
	XMVECTOR displacementVec = XMVectorScale(XMLoadFloat3(&forward), 1.f);
	XMStoreFloat3(&displacement, displacementVec);

	auto Animator = GetComponent<ModelComponent>()->GetAnimator();
	Animator->SetAnimation(3);
	Animator->Play();
	CanWalk = false;

	GetComponent<ControllerComponent>()->Move(displacement, epsilon);
}

void MagickaCharacter::HandleInput(const SceneContext& sceneContext, bool& shiftPressed)
{
	//menu
	MagickaScene* scene{ reinterpret_cast<MagickaScene*>(SceneManager::Get()->GetActiveScene()) };

	if (sceneContext.pInput->IsActionTriggered(m_CharacterDescExtended.actionId_InGameMenu)) {
		if (CanButton) {
			CurrentChannel->stop();
			SoundSystem->playSound(MenuMusic, nullptr, false, &CurrentChannel);
			SceneManager::Get()->SetActiveGameScene(L"GameMenu");
			CanButton = false;
		}
	}
	//game
	auto Animator = GetComponent<ModelComponent>()->GetAnimator();
	if (sceneContext.pInput->IsActionTriggered(m_CharacterDescExtended.actionId_SwitchElement)) {
		shiftPressed = true;
	}
	if (sceneContext.pInput->IsActionTriggered(m_CharacterDescExtended.actionId_ElementLeft)) {
		scene->HandleCombobarFilling(shiftPressed ? ElementTypes::EARTH : ElementTypes::SHIELD);
	}
	if (sceneContext.pInput->IsActionTriggered(m_CharacterDescExtended.actionId_ElementBottom)) {
		scene->HandleCombobarFilling(shiftPressed ? ElementTypes::LIGHTNING : ElementTypes::WATER);
	}
	if (sceneContext.pInput->IsActionTriggered(m_CharacterDescExtended.actionId_ElementTop)) {
		scene->HandleCombobarFilling(shiftPressed ? ElementTypes::ARCANE : ElementTypes::LIFE);
	}
	if (sceneContext.pInput->IsActionTriggered(m_CharacterDescExtended.actionId_ElementRight)) {
		scene->HandleCombobarFilling(shiftPressed ? ElementTypes::FIRE : ElementTypes::COLD);
	}
	if (sceneContext.pInput->IsActionTriggered(m_CharacterDescExtended.actionId_AoEAttack)) {
		scene->ExecuteAOE();
		Animator->SetAnimation(1);
		Animator->Play();
		m_CharacterDescExtended.maxMoveSpeed = 0;
		CanWalk = false;
	}
	if (sceneContext.pInput->IsActionTriggered(m_CharacterDescExtended.actionId_SelfCast)) {
		scene->ExecuteSelfCast();
	}
	if (sceneContext.pInput->IsActionTriggered(m_CharacterDescExtended.actionId_SwordInput)) {
		scene->ExecuteSword();
	}

	if (!sceneContext.pInput->IsGamepadConnected(GamepadIndex::playerOne)) {
		if (sceneContext.pInput->IsMouseButton(InputState::down, 2)) {
			scene->ExecuteMagicCombo();
			Animator->SetAnimation(2);
			Animator->Play();
			CanWalk = false;
			m_CharacterDescExtended.maxMoveSpeed = 25;
		}
		if (sceneContext.pInput->IsMouseButton(InputState::released, 2)) {
			scene->FireProjectile();
			scene->ResetCombo();
			m_CharacterDescExtended.maxMoveSpeed = 70;
		}
	}
	else {
		auto look = sceneContext.pInput->GetThumbstickPosition(false);

		if (abs(look.x) > 0.01f || abs(look.y) > 0.01f) {
			scene->ExecuteMagicCombo();
			Animator->SetAnimation(2);
			Animator->Play(); 
			m_CharacterDescExtended.maxMoveSpeed = 25;
			WasPressed = true;
		}
		else {
			if (WasPressed) {
				m_CharacterDescExtended.maxMoveSpeed = 70;
				scene->ResetCombo();
				WasPressed = false;

			}
		}

	}
}

void MagickaCharacter::DrawImGui()
{
	if (ImGui::CollapsingHeader("MagickaCharacter"))
	{
		ImGui::Text(std::format("Move Speed: {:0.1f} m/s", m_MoveSpeed).c_str());
		ImGui::Text(std::format("Fall Speed: {:0.1f} m/s", m_TotalVelocity.y).c_str());

		ImGui::Text(std::format("Move Acceleration: {:0.1f} m/s2", m_MoveAcceleration).c_str());
		ImGui::Text(std::format("Fall Acceleration: {:0.1f} m/s2", m_FallAcceleration).c_str());

		const float jumpMaxTime = m_CharacterDescExtended.JumpSpeed / m_FallAcceleration;
		const float jumpMaxHeight = (m_CharacterDescExtended.JumpSpeed * jumpMaxTime) - (0.5f * (m_FallAcceleration * powf(jumpMaxTime, 2)));
		ImGui::Text(std::format("Jump Height: {:0.1f} m", jumpMaxHeight).c_str());

		ImGui::Dummy({ 0.f,5.f });
		if (ImGui::DragFloat("Max Move Speed (m/s)", &m_CharacterDescExtended.maxMoveSpeed, 0.1f, 0.f, 0.f, "%.1f") ||
			ImGui::DragFloat("Move Acceleration Time (s)", &m_CharacterDescExtended.moveAccelerationTime, 0.1f, 0.f, 0.f, "%.1f"))
		{
			m_MoveAcceleration = m_CharacterDescExtended.maxMoveSpeed / m_CharacterDescExtended.moveAccelerationTime;
		}

		ImGui::Dummy({ 0.f,5.f });
		if (ImGui::DragFloat("Max Fall Speed (m/s)", &m_CharacterDescExtended.maxFallSpeed, 0.1f, 0.f, 0.f, "%.1f") ||
			ImGui::DragFloat("Fall Acceleration Time (s)", &m_CharacterDescExtended.fallAccelerationTime, 0.1f, 0.f, 0.f, "%.1f"))
		{
			m_FallAcceleration = m_CharacterDescExtended.maxFallSpeed / m_CharacterDescExtended.fallAccelerationTime;
		}

		ImGui::Dummy({ 0.f,5.f });
		ImGui::DragFloat("Jump Speed", &m_CharacterDescExtended.JumpSpeed, 0.1f, 0.f, 0.f, "%.1f");
		ImGui::DragFloat("Rotation Speed (deg/s)", &m_CharacterDescExtended.rotationSpeed, 0.1f, 0.f, 0.f, "%.1f");
	}
}

void MagickaCharacter::DamagePlayer(bool /*canTakeDamage*/, float damage)
{
	m_Health -= damage;
	std::cout << "Damage Taken: " << damage << ", Remaining Health: " << m_Health << "\n";
}
