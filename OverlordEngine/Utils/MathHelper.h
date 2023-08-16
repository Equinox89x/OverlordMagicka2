#pragma once

namespace MathHelper
{
	inline XMFLOAT3 QuaternionToEuler(XMFLOAT4 q)
	{
		XMFLOAT3 euler{};

		euler.y = atan2f(2.f * q.x * q.w + 2.f * q.y * q.z, 1.f - 2.f * (q.z * q.z + q.w * q.w));     // Yaw 
		euler.x = asinf(2.f * (q.x * q.z - q.w * q.y));                             // Pitch 
		euler.z = atan2f(2.f * q.x * q.y + 2.f * q.z * q.w, 1.f - 2.f * (q.y * q.y + q.z * q.z));      // Roll 

		return euler;
	}

	inline bool XMFloat4Equals(const XMFLOAT4& a, const XMFLOAT4& b)
	{
		return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
	}

	inline bool XMFloat3Equals(const XMFLOAT3& a, const XMFLOAT3& b)
	{
		return a.x == b.x && a.y == b.y && a.z == b.z;
	}

	inline bool XMFloat2Equals(const XMFLOAT2& a, const XMFLOAT2& b)
	{
		return a.x == b.x && a.y == b.y;
	}

	inline float randF(float min, float max)
	{
		const float random = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
		const float diff = max - min;
		const float r = random * diff;
		return min + r;
	}

	inline LONG BinaryClamp(LONG x)
	{
		if (x > 0) return 1;
		if (x < 0) return -1;
		return 0;
	}

	template<typename T>
	void Clamp(T& value, T hi, T lo)
	{
		if (value > hi)
			value = hi;

		if (value < lo)
			value = lo;
	}

	inline double Distance3D(XMFLOAT3 p1, XMFLOAT3 p2) {
		double dx = p2.x - p1.x;
		double dy = p2.y - p1.y;
		double dz = p2.z - p1.z;
		return std::sqrt(dx * dx + dy * dy + dz * dz);
	}

	inline bool IsPointInCircle3D(XMFLOAT3 point, XMFLOAT3 circleCenter, double circleRadius) {
		double dist = Distance3D(point, circleCenter);
		return (dist <= circleRadius);
	}

	inline bool IsPointOnLine(const XMFLOAT3& point, const XMFLOAT3& linePoint1, const XMFLOAT3& linePoint2, float epsilon) {
		float slope = (linePoint2.z - linePoint1.z) / (linePoint2.x - linePoint1.x);
		float expectedY = linePoint1.z + slope * (point.x - linePoint1.x);

		return std::abs(point.z - expectedY) <= epsilon;
	}

	inline bool IsPointNearViewport(const XMFLOAT3& point, float threshold, float windowHeight, float windowWidth)
	{
		// Calculate the normalized device coordinates (NDC) for the point
		float ndcX = (point.x + 1.0f) / 2.0f; // Assuming viewport range: -1 to 1
		float ndcY = (point.z + 1.0f) / 2.0f; // Assuming viewport range: -1 to 1

		// Calculate the screen coordinates for the point
		int screenX = static_cast<int>(ndcX * windowHeight);
		int screenY = static_cast<int>(ndcY * windowWidth);

		// Check if the point is within the threshold distance from any border
		if (screenX < threshold || screenX >(windowHeight - threshold) ||
			screenY < threshold || screenY >(windowWidth - threshold)) {
			return true; // Point is near the viewport borders
		}
		else {
			return false; // Point is not near the viewport borders
		}
	}

	inline bool CheckRange(double value, double lowerBound, double upperBound) {
		return value > lowerBound && value < upperBound;
	}

	//make object face direction of the forward vector, or face the direction towards the target
	inline XMFLOAT3 GetRotationTowardsPoint(XMFLOAT3 originPosition, XMFLOAT3 targetPosition, XMFLOAT4 originalRotation, bool useForwardVector) {
		XMVECTOR direction;
		if (useForwardVector) {
			auto origin{ originPosition };
			originPosition.x += targetPosition.x * 10;
			originPosition.z += targetPosition.z * 10;
			direction = XMVectorSubtract(XMLoadFloat3(&originPosition), XMLoadFloat3(&origin));
		}
		else {
			direction = XMVectorSubtract(XMLoadFloat3(&targetPosition), XMLoadFloat3(&originPosition));
		}

		float yaw = std::atan2(XMVectorGetX(direction), XMVectorGetZ(direction));
		return XMFLOAT3{ originalRotation.x, yaw, originalRotation.z };
	}

	inline int BinomialCoeff(int n, int i) {
		if (i == 0 || i == n)
			return 1;

		int numerator = 1;
		int denominator = 1;

		for (int j = 1; j <= i; j++) {
			numerator *= (n - j + 1);
			denominator *= j;
		}

		return numerator / denominator;
	}

	inline XMFLOAT3 CalculateBezierPoint(float t, const std::vector<XMFLOAT3>& controlPoints) {
		int n{ static_cast<int>(controlPoints.size()) - 1 };
		float x{ 0.0 };
		float z{ 0.0 };

		//Apply the quadtratic bezier formula as many times as there are controlPoint values
		for (int i = 0; i <= n; i++) {
			float blend{ static_cast<float>(BinomialCoeff(n, i) * pow(1 - t, n - i) * pow(t, i)) };
			x += controlPoints[i].x * blend;
			z += controlPoints[i].z * blend;
		}

		return XMFLOAT3{ x, controlPoints[0].y, z };
	}

	inline XMFLOAT3 CalculateProjectedPointOnBezier(int nrOfSteps, const XMFLOAT3& point,  const std::vector<XMFLOAT3>& controlPoints) {
		float minDistanceSq = std::numeric_limits<float>::max();
		float tAtMinDistance = 0.0f;

		for (int i = 0; i <= nrOfSteps; i++) {
			float t = static_cast<float>(i) / nrOfSteps;
			DirectX::XMFLOAT3 bezierPoint = MathHelper::CalculateBezierPoint(t, controlPoints);

			float distanceSq = (point.x - bezierPoint.x) * (point.x - bezierPoint.x) +
				(point.y - bezierPoint.y) * (point.y - bezierPoint.y) +
				(point.z - bezierPoint.z) * (point.z - bezierPoint.z);

			if (distanceSq < minDistanceSq) {
				minDistanceSq = distanceSq;
				tAtMinDistance = t;
			}
		}

		// Calculate the point on the Bezier curve closest to the object
		return MathHelper::CalculateBezierPoint(tAtMinDistance, controlPoints);
	}
}