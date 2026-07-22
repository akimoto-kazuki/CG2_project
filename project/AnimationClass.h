#pragma once
#include "Logger.h"
#include "StringUtility.h"
#include "MyMath.h"
#include "TextureManager.h"

#include <stdint.h>
#include <map>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace MyMath;

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

	static Animation LoadAnimationFile(const std::string& directoryPath, const std::string& filename);

	static Vector3 CalculateValueVector3(const std::vector<KeyframeVector3>& keyframes, float time);

	static Quaternion CalculateValueQuaternion(const std::vector<KeyframeQuaternion>& keyframes, float time);

private:

};

