#include "AnimationClass.h"

AnimationClass::Animation AnimationClass::LoadAnimationFile(const std::string& directoryPath, const std::string& filename)
{
	Animation animation;
	Assimp::Importer importer;
	std::string filePath = directoryPath + "/" + filename;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), 0);
	assert(scene->mNumAnimations != 0);
	aiAnimation* animationAssimp = scene->mAnimations[0];
	animation.duration = float(animationAssimp->mDuration / animationAssimp->mTicksPerSecond);
	for (uint32_t channelIndex = 0; channelIndex < animationAssimp->mNumChannels; ++channelIndex) {
		aiNodeAnim* nodeAnimationAssimp = animationAssimp->mChannels[channelIndex];
		NodeAnimation& nodeAnimation = animation.nodeAnimations[nodeAnimationAssimp->mNodeName.C_Str()];

		// 位置（Translate）のキーフレーム読み込み
		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumPositionKeys; ++keyIndex) {
			aiVectorKey& keyAssimp = nodeAnimationAssimp->mPositionKeys[keyIndex];
			KeyframeVector3 keyframe;
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
			keyframe.value = { keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z };
			nodeAnimation.translate.push_back(keyframe);
		}

		// ※同様に mRotationKeys (回転), mScalingKeys (スケール) も読み込む
		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumRotationKeys; ++keyIndex) {
			// ★ポイント1：型が aiVectorKey ではなく aiQuatKey になる！
			aiQuatKey& keyAssimp = nodeAnimationAssimp->mRotationKeys[keyIndex];
			KeyframeQuaternion keyframe;
			// 時間の計算（Translateと同様に秒単位へ変換）
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
			// ★ポイント2：値が3つ(x,y,z)ではなく、wを加えた4つになる！
			// ※ご自身の MyMath::Quaternion の構造体定義（x,y,z,w の順番）に合わせて代入してください
			keyframe.value = { keyAssimp.mValue.x, -keyAssimp.mValue.y, -keyAssimp.mValue.z, keyAssimp.mValue.w };
			nodeAnimation.rotate.push_back(keyframe);
		}

		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumScalingKeys; ++keyIndex) {
			aiVectorKey& keyAssimp = nodeAnimationAssimp->mScalingKeys[keyIndex];
			KeyframeVector3 keyframe;
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
			keyframe.value = { keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z };
			nodeAnimation.scale.push_back(keyframe);
		}
	}
	return animation;
}

Vector3 AnimationClass::CalculateValueVector3(const std::vector<AnimationClass::KeyframeVector3>& keyframes, float time)
{
	assert(!keyframes.empty()); // キーフレームが空ならエラー

	// キーフレームが1つだけ、または再生時間が最初のキーより前なら最初の値を返す
	if (keyframes.size() == 1 || time <= keyframes[0].time) {
		return keyframes[0].value;
	}
	// 再生時間が最後のキーより後ろなら最後の値を返す
	if (time >= keyframes.back().time) {
		return keyframes.back().value;
	}

	// 現在の時間に対応するキーフレームの区間を探す
	for (size_t i = 0; i < keyframes.size() - 1; ++i) {
		const auto& nextKey = keyframes[i + 1];
		if (time < nextKey.time) {
			const auto& prevKey = keyframes[i];
			// 2つのキーフレーム間の割合 t (0.0 ~ 1.0) を計算
			float t = (time - prevKey.time) / (nextKey.time - prevKey.time);
			// 線形補間（Lerp）
			return Lerp(prevKey.value, nextKey.value, t);
		}
	}
	return keyframes.back().value;
}

Quaternion AnimationClass::CalculateValueQuaternion(const std::vector<AnimationClass::KeyframeQuaternion>& keyframes, float time)
{
	assert(!keyframes.empty());

	if (keyframes.size() == 1 || time <= keyframes[0].time) {
		return keyframes[0].value;
	}
	if (time >= keyframes.back().time) {
		return keyframes.back().value;
	}

	for (size_t i = 0; i < keyframes.size() - 1; ++i) {
		const auto& nextKey = keyframes[i + 1];
		if (time < nextKey.time) {
			const auto& prevKey = keyframes[i];
			float t = (time - prevKey.time) / (nextKey.time - prevKey.time);
			// 球面線形補間（Slerp）を使って回転を滑らかに補間する
			return Slerp(prevKey.value, nextKey.value, t);
		}
	}
	return keyframes.back().value;
}
