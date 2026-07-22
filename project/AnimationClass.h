#pragma once
#include "Logger.h"
#include "StringUtility.h"
#include "MyMath.h"
#include "TextureManager.h"
#include "Model.h"

#include <stdint.h>
#include <map>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace MyMath;

class AnimationClass
{
public:

	template <typename tValue>
	struct Keyframe {
		float time;
		tValue value;
	};

	using KeyframeVector3 = Keyframe<Vector3>;
	using KeyframeQuaternion = Keyframe<Quaternion>;

	struct NodeAnimation
	{
		std::vector<KeyframeVector3> translate;
		std::vector<KeyframeQuaternion> rotate;
		std::vector<KeyframeVector3> scale;
	};

	struct Animation
	{
		float duration;
		std::map<std::string, NodeAnimation> nodeAnimations;
	};

	struct Joint 
	{
		QuaternionTransform transform; // ★ここを Transform から QuaternionTransform にする！
		Matrix4x4 localMatrix;
		Matrix4x4 skeletonSpaceMatrix;
		std::string name;
		std::vector<int32_t> children;
		int32_t index;
		std::optional<int32_t> parent;
	};

	struct Skeleton
	{
		int32_t root;
		std::map<std::string, int32_t> jointMap;
		std::vector<Joint> joints;
	};

public:

	static Animation LoadAnimationFile(const std::string& directoryPath, const std::string& filename);

	static Vector3 CalculateValueVector3(const std::vector<KeyframeVector3>& keyframes, float time);

	static Quaternion CalculateValueQuaternion(const std::vector<KeyframeQuaternion>& keyframes, float time);

	static Skeleton CreateSkeleton(const Model::Node& rootNode);

	static int32_t CreateJoint(const Model::Node& node, const std::optional<int32_t>& parent, std::vector<Joint>& joints);

	static void Update(Skeleton& skeleton);

	static void ApplyAnimation(Skeleton& skeleton, const Animation& animation, float animationTime);

private:

};

