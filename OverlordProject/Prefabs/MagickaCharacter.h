#pragma once
struct CharacterDescExtended
{
	CharacterDescExtended(
		PxMaterial* pMaterial,
		float radius = .5f,
		float height = 2.f)
	{
		controller.setToDefault();
		controller.radius = radius;
		controller.height = height;
		controller.material = pMaterial;
	}

	float maxMoveSpeed{ 75.f };
	float maxFallSpeed{ 15.f };

	float JumpSpeed{ 15.f };

	float moveAccelerationTime{ .3f };
	float fallAccelerationTime{ .3f };

	PxCapsuleControllerDesc controller{};

	float rotationSpeed{ 360.f };

	/*int actionId_MoveLeft{ -1 };
	int actionId_MoveRight{ -1 };
	int actionId_MoveForward{ -1 };
	int actionId_MoveBackward{ -1 };
	int actionId_Jump{ -1 };*/

	int actionId_ElementBottom{ -1 };
	int actionId_ElementLeft{ -1 };
	int actionId_ElementTop{ -1 };
	int actionId_ElementRight{ -1 };
	int actionId_SwitchElement{ -1 };

	int actionId_LeftClick{ -1 };
	int actionId_RightClick{ -1 };

	int actionId_Move{ -1 };
	int actionId_Execute{ -1 };


	int actionId_AoEAttack{ -1 };
	int actionId_SelfCast{ -1 };
	int actionId_SwordInput{-1};

	int actionId_InGameMenu{ -1 };

};

class MagickaCharacter : public GameObject
{
public:
	MagickaCharacter(const CharacterDescExtended& CharacterDescExtended);
	~MagickaCharacter() override = default;

	MagickaCharacter(const MagickaCharacter& other) = delete;
	MagickaCharacter(MagickaCharacter&& other) noexcept = delete;
	MagickaCharacter& operator=(const MagickaCharacter& other) = delete;
	MagickaCharacter& operator=(MagickaCharacter&& other) noexcept = delete;

	void DrawImGui();

	void DamagePlayer(bool canTakeDamage, float damage);
	float GetHealth() { return m_Health; };
	void ResetMoveSpeed(){ m_CharacterDescExtended.maxMoveSpeed = 70; }

protected:
	void Initialize(const SceneContext&) override;
	void Update(const SceneContext&) override;

	void HandleRotation(const SceneContext& sceneContext, DirectX::XMFLOAT2& look, const float& epsilon, DirectX::XMFLOAT3& newRot);

	void HandleMove(const SceneContext& sceneContext, DirectX::XMFLOAT3& newRot, const float& epsilon);

	void CalculateForwardMovement(const float& epsilon, const SceneContext& sceneContext);

	void HandleInput(const SceneContext& sceneContext, bool& shiftPressed);

private:
	FMOD::System* SoundSystem;
	FMOD::Sound* GameMusic;
	FMOD::Sound* MenuMusic;
	FMOD::Channel* CurrentChannel;

	ControllerComponent* m_pControllerComponent{};
	CharacterDescExtended m_CharacterDescExtended;
	float m_TotalPitch{}, m_TotalYaw{};				//Total camera Pitch(X) and Yaw(Y) rotation
	float m_MoveAcceleration{},						//Acceleration required to reach maxMoveVelocity after 1 second (maxMoveVelocity / moveAccelerationTime)
		m_FallAcceleration{},						//Acceleration required to reach maxFallVelocity after 1 second (maxFallVelocity / fallAccelerationTime)
		m_MoveSpeed{};								//MoveSpeed > Horizontal Velocity = MoveDirection * MoveVelocity (= TotalVelocity.xz)

	XMFLOAT3 m_TotalVelocity{};						//TotalVelocity with X/Z for Horizontal Movement AND Y for Vertical Movement (fall/jump)
	XMFLOAT3 m_CurrentDirection{};					//Current/Last Direction based on Camera forward/right (Stored for deacceleration)
	bool WasPressed{ false }, CanTakeDamage{ false };
	float m_Health{ 3000 };
	float DamageToTake{ 0 }, DamageTimer{ 1 };
	bool CanWalk{ true };
	float ButtonTimer{ 0.2f };
	bool CanButton{ true };
};

