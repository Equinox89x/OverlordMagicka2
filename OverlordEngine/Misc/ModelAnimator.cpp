#include "stdafx.h"
#include "ModelAnimator.h"

ModelAnimator::ModelAnimator(MeshFilter* pMeshFilter) :
	m_pMeshFilter{ pMeshFilter }
{
	SetAnimation(0);
}

void ModelAnimator::Update(const SceneContext& sceneContext)
{
	//We only update the transforms if the animation is running and the clip is set
	if (m_IsPlaying && m_ClipSet)
	{
		//1. 
		//Calculate the passedTicks (see the lab document)
		auto passedTicks = sceneContext.pGameTime->GetElapsed() * m_CurrentClip.ticksPerSecond * GetAnimationSpeed();
		//Make sure that the passedTicks stay between the m_CurrentClip.Duration bounds (fmod)
		passedTicks = fmod(passedTicks, m_CurrentClip.duration);

		//2. 
		if (m_Reversed) {
			//	Subtract passedTicks from m_TickCount
			m_TickCount -= passedTicks;
			//	If m_TickCount is smaller than zero, add m_CurrentClip.Duration to m_TickCount
			if (m_TickCount < 0) {
				m_TickCount += m_CurrentClip.duration;
			}
		}
		else {
			//	Add passedTicks to m_TickCount
			m_TickCount += passedTicks;
			//	if m_TickCount is bigger than the clip duration, subtract the duration from m_TickCount
			if (m_TickCount > m_CurrentClip.duration) {
				m_TickCount -= m_CurrentClip.duration;
			}
		}

		//3.
		//Find the enclosing keys
		//Iterate all the keys of the clip and find the following keys:
		AnimationKey keyA, keyB;
		for (auto key : m_CurrentClip.keys) {
			//keyA > Closest Key with Tick before/smaller than m_TickCount
			//keyB > Closest Key with Tick after/bigger than m_TickCount
			key.tick < m_TickCount ? keyA = key : keyB = key;
		}

		//4.
		//Interpolate between keys
		//Figure out the BlendFactor (See lab document)
		const float blendFactor = (m_TickCount - keyA.tick) / m_CurrentClip.duration;
		m_Transforms.clear();
		//FOR every boneTransform in a key (So for every bone)
		for (size_t i = 0; i < keyA.boneTransforms.size(); i++)
		{
			//	Retrieve the transform from keyA (transformA)
			auto transformA = keyA.boneTransforms[i];
			// 	Retrieve the transform from keyB (transformB)
			auto transformB = keyB.boneTransforms[i];

			//	Decompose both transforms
			XMVECTOR scale, rot, trans;
			XMMATRIX matr{ XMLoadFloat4x4(&transformA) };
			XMMatrixDecompose(&scale, &rot, &trans, matr);

			XMVECTOR scale1, rot1, trans1;
			XMMATRIX matr1{ XMLoadFloat4x4(&transformB) };
			XMMatrixDecompose(&scale1, &rot1, &trans1, matr1);

			//	Lerp between all the transformations (Position, Scale, Rotation)
			auto scaleLerp{ XMVectorLerp(scale, scale1, blendFactor) };
			auto rotLerp{ XMQuaternionSlerp(rot, rot1, blendFactor) };
			auto transLerp{ XMVectorLerp(trans, trans1, blendFactor) };

			//	Compose a transformation matrix with the lerp-results
			XMMATRIX matrix{ XMMatrixAffineTransformation(scaleLerp, XMVECTOR{0}, rotLerp, transLerp) };

			//	Add the resulting matrix to the m_Transforms vector
			XMFLOAT4X4 val{};
			XMStoreFloat4x4(&val, matrix);
			m_Transforms.push_back(val);
		}
	}
}

void ModelAnimator::SetAnimation(const std::wstring& clipName)
{
	m_ClipSet = false;
	//Iterate the m_AnimationClips vector and search for an AnimationClip with the given name (clipName)
	for (int i{}; i < m_pMeshFilter->m_AnimationClips.size(); i++)
	{
		if (m_pMeshFilter->m_AnimationClips[i].name == clipName)
		{
			//	Call SetAnimation(Animation Clip) with the found clip
			SetAnimation(m_pMeshFilter->m_AnimationClips[i]);
		}
		else
		{
			Reset();
			Logger::LogWarning(L"No clip could be found with the given name.");
		}
	}
}

void ModelAnimator::SetAnimation(UINT clipNumber)
{
	bool isAnimating{ false };
	if (m_CurrentClip.name == GetClip(clipNumber).name) {
		isAnimating = IsPlaying();
	}
	if (isAnimating) return;

	m_ClipSet = false;
	if (clipNumber < m_pMeshFilter->m_AnimationClips.size())
	{
		//	Retrieve the AnimationClip from the m_AnimationClips vector based on the given clipNumber
		//	Call SetAnimation(AnimationClip clip)
		SetAnimation(m_pMeshFilter->m_AnimationClips[clipNumber]);
	}
	else
	{
		Reset();
		Logger::LogWarning(L"Clipnumer was not smaller than m_AnimationClips size.");
		return;
	}
}

void ModelAnimator::SetAnimation(const AnimationClip& clip)
{
	m_ClipSet = true;
	m_CurrentClip = clip;
	Reset(false);
}

void ModelAnimator::Reset(bool pause)
{
	if (pause) {
		m_IsPlaying = false;
	}

	m_TickCount = 0;
	m_AnimationSpeed = 1.0f;


	if (m_ClipSet)
	{
		//	Retrieve the BoneTransform from the first Key from the current clip (m_CurrentClip)
		auto transforms{ m_CurrentClip.keys[0].boneTransforms };
		//	Refill the m_Transforms vector with the new BoneTransforms (have a look at vector::assign)
		m_Transforms.assign(transforms.begin(), transforms.end());
	}
	else
	{
		//	Create an IdentityMatrix 
		XMFLOAT4X4 identity;
		XMMATRIX mat{ XMMatrixIdentity() };
		XMStoreFloat4x4(&identity, mat);

		//	Refill the m_Transforms vector with this IdenityMatrix (Amount = BoneCount) (have a look at vector::assign)
		std::vector<XMFLOAT4X4> transforms;
		std::fill(transforms.begin(), transforms.end(), identity);

		m_Transforms.assign(transforms.begin(), transforms.end());
	}
}
