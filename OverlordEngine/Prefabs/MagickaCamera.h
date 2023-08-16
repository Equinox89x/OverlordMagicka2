#pragma once

class MagickaCamera final : public GameObject
{
public:
	MagickaCamera() = default;
	~MagickaCamera() override = default;
	MagickaCamera(const MagickaCamera& other) = delete;
	MagickaCamera(MagickaCamera&& other) noexcept = delete;
	MagickaCamera& operator=(const MagickaCamera& other) = delete;
	MagickaCamera& operator=(MagickaCamera&& other) noexcept = delete;

	void SetFollowPoint(XMFLOAT3 followPoint) { PointToFollow = followPoint; };
	XMFLOAT3 GetPointToFollow() { return PointToFollow; };
	XMFLOAT3 GetProjectedLinePoint() { return ProjectedLinePoint; };
	XMFLOAT3 GetProjectedPoint() { return ProjectedPoint; };

	std::vector<XMFLOAT3> m_LinePoints;
	void InitPoints(const std::vector<XMFLOAT3>& linePoints);
	XMFLOAT3 GetLineOfIndex(int id) { return m_LinePoints[id]; };
	std::vector<XMFLOAT3>& GetControlPoints() { return m_LinePoints; };

	void Draw(const SceneContext& sceneContext) override;

	bool GetReachedEnd() { return HasReachedEnd; };
	void SetReachedEnd(bool hasReachedEnd) { HasReachedEnd = hasReachedEnd; };
	bool GetIsInStart() { return IsInStart; };

protected:
	void Initialize(const SceneContext&) override;
	void Update(const SceneContext&) override;

private:

	float m_TotalPitch{}, m_TotalYaw{};
	float m_MoveSpeed{ 10.f }, m_RotationSpeed{ 10.f }, m_SpeedMultiplier{ 10.f };
	CameraComponent* m_pCamera{};
	int CurrentLineIndex{ 0 };
	bool CanIncrease{ false }, HasReachedEnd{ false }, IsInStart{ false };
	XMFLOAT3 PointToFollow{};
	XMFLOAT3 ProjectedLinePoint{};
	XMFLOAT3 ProjectedPoint{};

	float m_Time;
};
