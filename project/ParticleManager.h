#pragma once
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "Logger.h"
#include "StringUtility.h"
#include "Camera.h"
//
#include "MyMath.h"

#include <random>
#include <list>
#include <unordered_map>
#include <string>
#include <wrl.h>
#include <d3d12.h>
#include <numbers>

using namespace MyMath;
class Camera;

class ParticleManager
{
public:
	// ★追加：シェーダーに送るデータ（定数バッファ）の構造体
	struct Material {
		MyMath::Vector4 color;
		int32_t enableLighting;
		float padding[3];
		MyMath::Matrix4x4 uvTransform;
	};
	struct TransformationMatrix {
		MyMath::Matrix4x4 WVP;
		MyMath::Matrix4x4 World;
		MyMath::Vector4 color;      // ★追加：16バイト（色と透明度）
		char padding[112]; // ★256バイトアライメントにするためのゴミデータ（余白）
	};

	// ★追加：同時に画面に出せるパーティクルの最大数
	static const size_t kMaxParticles = 1024;

	struct DirectionalLight {
		MyMath::Vector4 color;
		MyMath::Vector3 direction;
		float intensity;
	};
	struct CameraForGPU {
		MyMath::Vector3 worldPosition;
	};
	// ★スライド4枚目：パーティクル単体の構造体
	struct Particle
	{
		Vector3 position;       // 座標
		Vector3 velocity;       // 速度
		Vector4 color;          // 色
		Transform transform;
		float lifeTime;         // 生存可能時間（寿命）
		float currentLifeTime;  // 現在の経過時間

		// ★追加：このパーティクルが重力の影響を受けるかどうか
		bool useGravity = false;
		bool isExploded = false; // ★追加：爆発済みかどうかのフラグ
	};

	// ★スライド3枚目：パーティクルグループの構造体
	struct ParticleGroup
	{
		std::string name;               // グループ名
		std::list<Particle> particles;  // 所属するパーティクル一覧
		uint32_t srvIndex;              // 使用するテクスチャのSRVインデックス

		bool isRing = false; // 使うエフェクトがリングかリングじゃないのか
		bool isCylinder = false;
	};

private:
	// シングルトンのための設定
	ParticleManager() = default;
	~ParticleManager() = default;
	ParticleManager(const ParticleManager&) = delete;
	ParticleManager& operator=(const ParticleManager&) = delete;

public:
	// インスタンス取得
	static ParticleManager* GetInstance();

	// 初期化（引数にSrvManagerを追加）
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager, Camera* camera);

	// 更新処理
	void Update();

	// 描画処理
	void Draw();

	// ★スライド5枚目：パーティクル発生関数
	// どこから、何個のパーティクルを出すかを指定して発生させます
	void EmitEffect(const std::string& groupName, const Transform& transform, uint32_t count);

	void EmitHitEffect(const std::string& groupName, const Transform& transform, uint32_t count);

	void EmitSparkEffect(const std::string& groupName, const Transform& transform, uint32_t count);

	void EmitCylinderEffect(const std::string& groupName, const Transform& transform, uint32_t count);

	// ★新しくグループを作って登録する関数
	void CreateGroup(const std::string& groupName, uint32_t srvIndex,bool isRing = false,bool isCylinder = false);

	// set
	void SetDefaultCamera(Camera* camera) { this->defaultCamera = camera; }
	// get
	Camera* GetDefaultCamera()const { return defaultCamera; }

	void SetCamera(Camera* camera) { this->camera_ = camera; }

private:

	// ★追加：長くなる初期化処理を分割した関数
	void CreateRootSignature();
	void CreateGraphicsPipeline();
	void CreateParticleVertexData();
	void CreateRingVertexData();
	void CreateCylinderVertexDate();

	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;

	// ランダム生成用
	std::random_device seedGenerator_;
	std::mt19937 randomEngine_;

	// 全てのパーティクルグループを「名前」で連想配列（マップ）管理する
	std::unordered_map<std::string, ParticleGroup> particleGroups_;

	// パイプライン（PSO）まわり
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;

	// 頂点バッファ（VBV）まわり
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vbv_{};

	// リング用
	Microsoft::WRL::ComPtr<ID3D12Resource> ringVertexResource_;
	D3D12_VERTEX_BUFFER_VIEW ringVbv_{};

	// 円柱(サークル)用
	Microsoft::WRL::ComPtr<ID3D12Resource> cylinderVertexResource_;
	D3D12_VERTEX_BUFFER_VIEW cylinderVbv_{};

	// --- クラスの下の方に追加：定数バッファ用のリソース変数 ---
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource_;

	Camera* defaultCamera = nullptr;
	Camera* camera_ = nullptr;
	CameraForGPU* cameraData = nullptr;

	Transform cameraTransform;
};

