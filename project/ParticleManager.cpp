#include "ParticleManager.h"
#include <cassert>
#include "MyMath.h"

using namespace MyMath;

ParticleManager* ParticleManager::GetInstance()
{
	static ParticleManager instance;
	return &instance;
}

void ParticleManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager,Camera* camera)
{
	// スライド2枚目の処理
	this->dxCommon_ = dxCommon;
	this->srvManager_ = srvManager;
	this->camera_ = camera;

	// ランダムエンジンの初期化
	randomEngine_.seed(seedGenerator_());

	// TODO: パイプライン生成 (前回の授業などの板ポリゴン描画を参考に rootSignature_ と pipelineState_ を生成)
	
	// TODO: 頂点リソース・VBV生成 (四角形の頂点4つ分などのデータを vertexResource_ に書き込む)

	// 分割した初期化関数を順番に呼び出す
	CreateRootSignature();
	CreateGraphicsPipeline();
	CreateParticleVertexData();
	CreateRingVertexData();
	CreateCylinderVertexDate();

	// 1. マテリアル (0番)
	materialResource_ = dxCommon_->CreateBufferResource(sizeof(Material));
	Material* materialData = nullptr;
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData->enableLighting = 0; // パーティクルはひとまずライティングOFF
	materialData->uvTransform = MyMath::MakeIdentity4x4();

	// 2. WVP行列 (1番)
	wvpResource_ = dxCommon_->CreateBufferResource(sizeof(TransformationMatrix) * kMaxParticles);
	TransformationMatrix* wvpData = nullptr;
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	wvpData->WVP = MyMath::MakeIdentity4x4();
	wvpData->World = MyMath::MakeIdentity4x4();

	// 3. ライト (3番)
	directionalLightResource_ = dxCommon_->CreateBufferResource(sizeof(DirectionalLight));
	DirectionalLight* lightData = nullptr;
	directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&lightData));
	lightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	lightData->direction = { 0.0f, -1.0f, 0.0f };
	lightData->intensity = 1.0f;

	// 4. カメラ (4番)
	cameraResource_ = dxCommon_->CreateBufferResource(sizeof(CameraForGPU));
	cameraResource_->Map(0, nullptr, reinterpret_cast<void**>(&cameraData));
	cameraData->worldPosition = cameraTransform.translate;
}

// ★新しくパーティクルのグループ（枠）を作る関数
void ParticleManager::CreateGroup(const std::string& groupName, uint32_t srvIndex, bool isRing,bool isCylinder)
{
	// すでに同じ名前のグループがあったらスキップ
	if (particleGroups_.find(groupName) != particleGroups_.end()) return;

	ParticleGroup newGroup;
	newGroup.name = groupName;
	newGroup.srvIndex = srvIndex; // 使うテクスチャの番号を記録
	newGroup.isRing = isRing;
	newGroup.isCylinder = isCylinder;

	particleGroups_[groupName] = newGroup;	
}

// ★スライド5枚目：パーティクル発生（Emit）の実装
void ParticleManager::EmitEffect(const std::string& groupName, const Transform& transform, uint32_t count)
{
	// 指定されたグループが存在するかチェック
	auto it = particleGroups_.find(groupName);
	assert(it != particleGroups_.end() && "存在しないグループ名が指定されました。");

	// 速度と寿命をランダムにするための分布を設定 (例: -1.0 〜 1.0 の間)
	std::uniform_real_distribution<float> distVelocity(-1.0f, 1.0f);
	std::uniform_real_distribution<float> distLifeTime(1.0f, 3.0f); // 1秒〜3秒生きる

	for (uint32_t i = 0; i < count; ++i)
	{
		Particle newParticle;

		newParticle.position = transform.translate;
		// 速度をランダムに決定
		newParticle.velocity.x = distVelocity(randomEngine_);
		newParticle.velocity.y = distVelocity(randomEngine_);
		newParticle.velocity.z = distVelocity(randomEngine_);

		newParticle.color = { 1.0f, 1.0f, 1.0f, 1.0f }; // 初期の色（白）
		newParticle.transform.scale = { transform.scale };			// 初期サイズ
		newParticle.transform.rotate = { transform.rotate };        // 初期回転
		newParticle.transform.translate = transform.translate;      // 初期位置
		newParticle.lifeTime = distLifeTime(randomEngine_); // ランダムな寿命
		newParticle.currentLifeTime = 0.0f;             // 経過時間は0からスタート

		// グループのリストに新しく作ったパーティクルを追加
		it->second.particles.push_back(newParticle);
	}
}

void ParticleManager::EmitHitEffect(const std::string& groupName, const Transform& transform, uint32_t count)
{
	// 指定されたグループが存在するかチェック
	auto it = particleGroups_.find(groupName);
	assert(it != particleGroups_.end() && "存在しないグループ名が指定されました。");

	std::uniform_real_distribution<float> distRotate(-std::numbers::pi_v<float>, std::numbers::pi_v<float>);

	for (uint32_t i = 0; i < count; ++i)
	{
		Particle newParticle;

		newParticle.position = transform.translate;
		// 速度をランダムに決定
		newParticle.velocity = {0.0f,0.0f,0.0f};
		newParticle.color = { 1.0f, 1.0f, 1.0f, 1.0f };				// 初期の色（白）
		newParticle.transform.scale = { transform.scale };			// 初期サイズ
		newParticle.transform.rotate = { transform.rotate.x,transform.rotate.y,distRotate(randomEngine_)};        // 初期回転
		newParticle.transform.translate = transform.translate;      // 初期位置
		newParticle.lifeTime = 1.0f;								// 寿命固定
		newParticle.currentLifeTime = 0.0f;							// 経過時間は0からスタート

		// グループのリストに新しく作ったパーティクルを追加
		it->second.particles.push_back(newParticle);
	}
}

void ParticleManager::EmitSparkEffect(const std::string& groupName, const Transform& transform, uint32_t count)
{
	auto it = particleGroups_.find(groupName);
	assert(it != particleGroups_.end() && "存在しないグループ名が指定されました。");

	// -1.0 〜 1.0 のランダム値（方向用）
	std::uniform_real_distribution<float> distDir(-1.0f, 1.0f);
	// 5.0 〜 15.0 のランダム値（初速のスピード用。最初はかなり速くする！）
	std::uniform_real_distribution<float> distSpeed(5.0f, 15.0f);

	for (uint32_t i = 0; i < count; ++i)
	{
		Particle newParticle;
		newParticle.transform = transform;
		newParticle.position = transform.translate;
		// ① ランダムな方向ベクトルを作る
		MyMath::Vector3 dir = { distDir(randomEngine_), distDir(randomEngine_), distDir(randomEngine_) };

		// ベクトルの長さを1にする（正規化）ことで、綺麗な球状に飛ぶようにする
		float length = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
		if (length > 0.0f) {
			dir.x /= length;
			dir.y /= length;
			dir.z /= length;
		}

		// ② 正規化した方向に、ランダムなスピードを掛ける
		float speed = distSpeed(randomEngine_);
		newParticle.velocity.x = dir.x * speed;
		newParticle.velocity.y = dir.y * speed;
		newParticle.velocity.z = dir.z * speed;

		// 火花らしい色（オレンジ・黄色系）
		newParticle.color = { 1.0f, 0.6f, 0.1f, 1.0f };
		newParticle.transform.scale = { 0.3f, 0.3f, 0.3f }; // 少し小さめ

		// 寿命（0.5秒〜0.8秒くらいでランダムにすると散り際が綺麗になります）
		newParticle.lifeTime = 0.5f + (distDir(randomEngine_) + 1.0f) * 0.15f;
		newParticle.currentLifeTime = 0.0f;

		newParticle.useGravity = true; // 重力もON

		it->second.particles.push_back(newParticle);
	}
}

void ParticleManager::EmitCylinderEffect(const std::string& groupName, const Transform& transform, uint32_t count)
{
	// 指定されたグループが存在するかチェック
	auto it = particleGroups_.find(groupName);
	assert(it != particleGroups_.end() && "存在しないグループ名が指定されました。");
	// すでにシリンダーが存在する場合は、新しく作らずに位置だけを更新（追従）させる
	if (!it->second.particles.empty())
	{
		for (auto& particle : it->second.particles)
		{
			// 位置を最新のエミッターの場所に合わせる
			particle.position = transform.translate;
			particle.transform.translate = transform.translate;

			// 寿命をリセットしてフェードアウトしないようにする
			particle.currentLifeTime = 0.0f;
		}
		// 新規作成はせずにここで処理を終わる！
		return;
	}
	for (uint32_t i = 0; i < 1; ++i)
	{
		Particle newParticle;

		newParticle.position = transform.translate;
		// 速度をランダムに決定
		newParticle.velocity = { 0.0f,0.0f,0.0f };
		newParticle.color = { 1.0f, 1.0f, 1.0f, 1.0f };				// 初期の色（白）
		newParticle.transform.scale = { transform.scale };			// 初期サイズ
		newParticle.transform.rotate = { transform.rotate.x,transform.rotate.y,transform.rotate.z };        // 初期回転
		newParticle.transform.translate = transform.translate;      // 初期位置
		newParticle.lifeTime = 1.0f;								// 寿命固定
		newParticle.currentLifeTime = 0.0f;							// 経過時間は0からスタート
		newParticle.useGravity = false;

		// グループのリストに新しく作ったパーティクルを追加
		it->second.particles.push_back(newParticle);
	}
}

// ★スライド6枚目：更新処理（移動と寿命チェック）
void ParticleManager::Update()
{
	// 本来は毎フレームの経過時間（DeltaTime）を取得しますが、ここでは仮に 1/60秒 とします
	const float kDeltaTime = 1.0f / 60.0f;
	// すべてのパーティクルグループをループ
	for (auto& pair : particleGroups_)
	{
		ParticleGroup& group = pair.second;

		// リスト内のパーティクルをループ（消去を行うためイテレータを使用します）
		for (auto it = group.particles.begin(); it != group.particles.end(); )
		{
			// 経過時間を進める
			it->currentLifeTime += kDeltaTime;

			// 寿命が尽きたらリストから削除
			if (it->currentLifeTime >= it->lifeTime)
			{
				// eraseすると、自動的に「次の要素」を指すイテレータが返ってきます
				it = group.particles.erase(it);
			}
			else
			{
				it->velocity.x *= 0.92f; // 1.0より少し小さい値を掛ける
				it->velocity.y *= 0.92f;
				it->velocity.z *= 0.92f;
				if (it->useGravity)
				{
					it->velocity.y -= 9.8f * kDeltaTime; // 9.8fは重力加速度。好みに応じて調整してください
				}
				// まだ生きている場合は移動させる (座標 = 速度 * 時間)
				it->position.x += it->velocity.x * kDeltaTime;
				it->position.y += it->velocity.y * kDeltaTime;
				it->position.z += it->velocity.z * kDeltaTime;

				it->transform.translate = it->position;
				
				if (group.isCylinder)
				{
					// 回転スピード（例: std::numbers::pi_v<float> で1秒間に半回転）
					// 好みに合わせて掛け率を変えてスピード調整してください
					const float kRotationSpeed = std::numbers::pi_v<float> * 0.5f; // これだと1秒間に1回転
					it->transform.rotate.y += kRotationSpeed * kDeltaTime;
				}

				// ★追加：フェードアウト処理 (0.0 〜 1.0 の範囲)
				float alpha = 1.0f - (it->currentLifeTime / it->lifeTime);
				it->color.w = alpha;

				// 次のパーティクルへ進む
				++it;
			}
		}
	}
}

// ★スライド7・8枚目：描画処理
void ParticleManager::Draw()
{
	assert(rootSignature_ != nullptr && "ルートシグネチャが作られていません！");
	assert(pipelineState_ != nullptr && "パイプラインが作られていません！");

	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	commandList->SetGraphicsRootSignature(rootSignature_.Get());
	commandList->SetPipelineState(pipelineState_.Get());
	
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 共通データのセット
	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress()); // [0] マテリアル
	commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress()); // [3] ライト
	commandList->SetGraphicsRootConstantBufferView(4, cameraResource_->GetGPUVirtualAddress());   // [4] カメラ

	// 配列バッファに書き込むためのポインタを取得
	TransformationMatrix* wvpData = nullptr;
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));

	size_t particleIndex = 0; // 何個目のパーティクルか数えるカウンター

	cameraData->worldPosition = camera_->GetTranslate();
	for (const auto& pair : particleGroups_)
	{
		const ParticleGroup& group = pair.second;
		if (group.particles.empty()) continue;

		// テクスチャのセット [2]
		srvManager_->SetGraphicsRootDescriptorTable(2, group.srvIndex);

		for (const auto& particle : group.particles)
		{
			// 制限個数を超えたら安全のために描画を終了
			if (particleIndex >= kMaxParticles) break;
			// ① パーティクルの現在位置からワールド行列を作る
			MyMath::Matrix4x4 worldMatrix = MakeAffineMatrix(particle.transform.scale, particle.transform.rotate, particle.transform.translate);
			Matrix4x4 worldViewProjectionMatrix;
			if (camera_)
			{
				const Matrix4x4& viewProjectionMatrix = camera_->GetViewProjectionMatrix();
				worldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);
			}
			else
			{
				worldViewProjectionMatrix = worldMatrix;
			}
			// ② 行列を書き込む（本来はここにカメラのViewProjection行列を掛け算します）
			wvpData[particleIndex].World = worldMatrix;
			wvpData[particleIndex].WVP = worldViewProjectionMatrix;
			wvpData[particleIndex].color = particle.color;

			// ③ GPU側のアドレスを、このパーティクルの番号（インデックス）の分だけズラして指定する
			D3D12_GPU_VIRTUAL_ADDRESS wvpAddress = wvpResource_->GetGPUVirtualAddress() + (sizeof(TransformationMatrix) * particleIndex);
			commandList->SetGraphicsRootConstantBufferView(1, wvpAddress); // [1] WVP行列

			// 描画
			if (group.isRing)
			{
				commandList->IASetVertexBuffers(0, 1, &ringVbv_);
				commandList->DrawInstanced(192, 1, 0, 0);
			}
			else if (group.isCylinder)
			{
				commandList->IASetVertexBuffers(0, 1, &cylinderVbv_);
				commandList->DrawInstanced(192, 1, 0, 0);
			}
			else
			{
				commandList->IASetVertexBuffers(0, 1, &vbv_);
				commandList->DrawInstanced(6, 1, 0, 0);
			}
			

			particleIndex++; // 次のパーティクルへ
		}
	}

	wvpResource_->Unmap(0, nullptr);
}

// -------------------------------------------------------------------------
// 1. ルートシグネチャ作成関数
// -------------------------------------------------------------------------
void ParticleManager::CreateRootSignature()
{
	ID3D12Device* device = dxCommon_->GetDevice();

	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// ★ここが重要！シェーダーに合わせて5つのパラメータを用意する
	D3D12_ROOT_PARAMETER rootParameters[5] = {};

	// [0] マテリアル用 (PixelShader: b0)
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].Descriptor.ShaderRegister = 0;

	// [1] WVP行列用 (VertexShader: b0)
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[1].Descriptor.ShaderRegister = 0;

	// [2] テクスチャ用 (PixelShader: t0)
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;

	// [3] 平行光源(ライト)用 (PixelShader: b1)
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[3].Descriptor.ShaderRegister = 1;

	// [4] カメラ用 (PixelShader: b2)
	rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[4].Descriptor.ShaderRegister = 2;

	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);

	// サンプラーの設定 (PixelShader: s0)
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	// シリアライズして生成
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));
}

// -------------------------------------------------------------------------
// 2. パイプライン(PSO)作成関数
// -------------------------------------------------------------------------
void ParticleManager::CreateGraphicsPipeline()
{
	ID3D12Device* device = dxCommon_->GetDevice();

	// シェーダーのコンパイル (※お使いの環境のコンパイル関数名に合わせてください)
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = dxCommon_->CompileShader(L"Particle.VS.hlsl", L"vs_6_0");
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = dxCommon_->CompileShader(L"Particle.PS.hlsl", L"ps_6_0");

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.pRootSignature = rootSignature_.Get();
	// 【変更後】Particle.VS.hlsl の VertexShaderInput に合わせたレイアウト設定
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].InputSlot = 0;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;

	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].InputSlot = 0;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;

	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].InputSlot = 0;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[2].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;

	psoDesc.InputLayout.pInputElementDescs = inputElementDescs;
	psoDesc.InputLayout.NumElements = _countof(inputElementDescs);
	psoDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
	psoDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };

	// 加算合成（光るエフェクト用）のブレンド設定
	psoDesc.BlendState.RenderTarget[0].BlendEnable = TRUE;
	psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	psoDesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	psoDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	psoDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// ラスタライザ・デプスの設定
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	psoDesc.DepthStencilState.DepthEnable = TRUE;
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	psoDesc.SampleDesc.Count = 1;

	HRESULT hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState_));
	assert(SUCCEEDED(hr));
}

// -------------------------------------------------------------------------
// 3. 頂点データ(四角形ポリゴン)作成関数
// -------------------------------------------------------------------------
void ParticleManager::CreateParticleVertexData()
{
	// 頂点データの構造体
	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal; // ← 追加
	};

	size_t vertexBufferSize = sizeof(VertexData) * 6;
	vertexResource_ = dxCommon_->CreateBufferResource(vertexBufferSize);

	VertexData* vertexData = nullptr;
	vertexResource_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	// 四角形を形作る三角形2枚分の頂点データ (position, texcoord, normal)
	vertexData[0] = { {-1.0f, -1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f} }; // 左下
	vertexData[1] = { {-1.0f,  1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f} }; // 左上
	vertexData[2] = { { 1.0f, -1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f} }; // 右下
	vertexData[3] = { {-1.0f,  1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f} }; // 左上
	vertexData[4] = { { 1.0f,  1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f} }; // 右上
	vertexData[5] = { { 1.0f, -1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f} }; // 右下

	// 頂点バッファビュー（VBV）の設定
	vbv_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vbv_.SizeInBytes = static_cast<UINT>(vertexBufferSize);
	vbv_.StrideInBytes = sizeof(VertexData);
}

void ParticleManager::CreateRingVertexData()
{
	// 頂点データの構造体
	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};

	// --- 資料に基づいたリングの設定 ---
	const uint32_t kRingDivide = 32;
	const float kOuterRadius = 1.0f;
	const float kInnerRadius = 0.2f;
	const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / float(kRingDivide);

	// 1分割につき四角形1つ(2ポリゴン=6頂点)なので、全体で kRingDivide * 6 の頂点が必要
	const uint32_t vertexCount = kRingDivide * 6;
	size_t vertexBufferSize = sizeof(VertexData) * vertexCount;
	vertexResource_ = dxCommon_->CreateBufferResource(vertexBufferSize);

	VertexData* vertexData = nullptr;
	vertexResource_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	// --- 資料のforループを使って頂点を計算 ---
	for (uint32_t index = 0; index < kRingDivide; ++index) {
		float sin = std::sin(index * radianPerDivide);
		float cos = std::cos(index * radianPerDivide);
		float sinNext = std::sin((index + 1) * radianPerDivide);
		float cosNext = std::cos((index + 1) * radianPerDivide);
		float u = float(index) / float(kRingDivide);
		float uNext = float(index + 1) / float(kRingDivide);

		// 配列のどこに書き込むかのインデックス（1ループにつき6個進む）
		uint32_t vIndex = index * 6;

		// ① 外側の頂点
		VertexData v1 = { {-sin * kOuterRadius, cos * kOuterRadius, 0.0f, 1.0f}, {u, 0.0f}, {0.0f, 0.0f, -1.0f} };
		// ② 外側の次の頂点
		VertexData v2 = { {-sinNext * kOuterRadius, cosNext * kOuterRadius, 0.0f, 1.0f}, {uNext, 0.0f}, {0.0f, 0.0f, -1.0f} };
		// ③ 内側の頂点
		VertexData v3 = { {-sin * kInnerRadius, cos * kInnerRadius, 0.0f, 1.0f}, {u, 1.0f}, {0.0f, 0.0f, -1.0f} };
		// ④ 内側の次の頂点
		VertexData v4 = { {-sinNext * kInnerRadius, cosNext * kInnerRadius, 0.0f, 1.0f}, {uNext, 1.0f}, {0.0f, 0.0f, -1.0f} };

		// 時計回りに2つの三角形を構築して四角形を作る
		// 1つ目の三角形 (③ -> ① -> ②)
		vertexData[vIndex + 0] = v3;
		vertexData[vIndex + 1] = v1;
		vertexData[vIndex + 2] = v2;

		// 2つ目の三角形 (③ -> ② -> ④)
		vertexData[vIndex + 3] = v3;
		vertexData[vIndex + 4] = v2;
		vertexData[vIndex + 5] = v4;
	}

	// 頂点バッファビュー（VBV）の設定
	ringVbv_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	ringVbv_.SizeInBytes = static_cast<UINT>(vertexBufferSize);
	ringVbv_.StrideInBytes = sizeof(VertexData);
}

void ParticleManager::CreateCylinderVertexDate()
{
	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};

	// --- 円柱の設定 ---
	const uint32_t kSubdivision = 32; // 分割数
	const float kRadius = 1.0f;       // 半径
	const float kHeight = 2.0f;       // 高さ
	const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / float(kSubdivision);

	// 1分割につき四角形1つ(2ポリゴン=6頂点)
	const uint32_t vertexCount = kSubdivision * 6;
	size_t vertexBufferSize = sizeof(VertexData) * vertexCount;

	// ※別途 cylinderVertexResource_ と cylinderVbv_ をヘッダで定義しておく必要があります
	cylinderVertexResource_ = dxCommon_->CreateBufferResource(vertexBufferSize);

	VertexData* vertexData = nullptr;
	cylinderVertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	// --- 頂点の計算 ---
	for (uint32_t index = 0; index < kSubdivision; ++index) {
		// 現在の角度と次の角度
		float theta = index * radianPerDivide;
		float nextTheta = (index + 1) * radianPerDivide;

		// XZ平面の円の計算 (標準的なcosがX, sinがZの配置)
		float cos = std::cos(theta);
		float sin = std::sin(theta);
		float nextCos = std::cos(nextTheta);
		float nextSin = std::sin(nextTheta);

		// UV座標のU (横方向)
		float u = float(index) / float(kSubdivision);
		float uNext = float(index + 1) / float(kSubdivision);

		uint32_t vIndex = index * 6;

		// 高さの半分 (原点を中心にするため)
		float topY = kHeight; // 上端
		float bottomY = 0.0f; // 下端

		// ① 左上 (現在角度・上側)
		// 【Flip】 0.0f だった部分を 1.0f に変更
		VertexData v1 = { {cos * kRadius, topY, sin * kRadius, 1.0f}, {u, 1.0f}, {cos, 0.0f, sin} };

		// ② 右上 (次角度・上側)
		// 【Flip】 0.0f だった部分を 1.0f に変更
		VertexData v2 = { {nextCos * kRadius, topY, nextSin * kRadius, 1.0f}, {uNext, 1.0f}, {nextCos, 0.0f, nextSin} };

		// ③ 左下 (現在角度・下側)
		// 【Flip】 1.0f だった部分を 0.0f に変更
		VertexData v3 = { {cos * kRadius, bottomY, sin * kRadius, 1.0f}, {u, 0.0f}, {cos, 0.0f, sin} };

		// ④ 右下 (次角度・下側)
		// 【Flip】 1.0f だった部分を 0.0f に変更
		VertexData v4 = { {nextCos * kRadius, bottomY, nextSin * kRadius, 1.0f}, {uNext, 0.0f}, {nextCos, 0.0f, nextSin} };

		// ポリゴンの構築 (時計回りで表面になるように結ぶ)
		// 1つ目の三角形 (左下 -> 左上 -> 右上)
		vertexData[vIndex + 0] = v3;
		vertexData[vIndex + 1] = v1;
		vertexData[vIndex + 2] = v2;

		// 2つ目の三角形 (左下 -> 右上 -> 右下)
		vertexData[vIndex + 3] = v3;
		vertexData[vIndex + 4] = v2;
		vertexData[vIndex + 5] = v4;
	}

	// 頂点バッファビュー（VBV）の設定
	cylinderVbv_.BufferLocation = cylinderVertexResource_->GetGPUVirtualAddress();
	cylinderVbv_.SizeInBytes = static_cast<UINT>(vertexBufferSize);
	cylinderVbv_.StrideInBytes = sizeof(VertexData);
}
