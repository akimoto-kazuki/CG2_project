#include "ParticleManager.h"
#include <cassert>

ParticleManager* ParticleManager::GetInstance()
{
	static ParticleManager instance;
	return &instance;
}

void ParticleManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager)
{
	// スライド2枚目の処理
	dxCommon_ = dxCommon;
	srvManager_ = srvManager;

	// ランダムエンジンの初期化
	randomEngine_.seed(seedGenerator_());

	// TODO: パイプライン生成 (前回の授業などの板ポリゴン描画を参考に rootSignature_ と pipelineState_ を生成)
	// TODO: 頂点リソース・VBV生成 (四角形の頂点4つ分などのデータを vertexResource_ に書き込む)

	// 分割した初期化関数を順番に呼び出す
	CreateRootSignature();
	CreateGraphicsPipeline();
	CreateParticleVertexData();

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
	CameraForGPU* cameraData = nullptr;
	cameraResource_->Map(0, nullptr, reinterpret_cast<void**>(&cameraData));
	cameraData->worldPosition = { 0.0f, 0.0f, 0.0f };
}

// ★新しくパーティクルのグループ（枠）を作る関数
void ParticleManager::CreateGroup(const std::string& groupName, uint32_t srvIndex)
{
	// すでに同じ名前のグループがあったらスキップ
	if (particleGroups_.find(groupName) != particleGroups_.end()) return;

	ParticleGroup newGroup;
	newGroup.name = groupName;
	newGroup.srvIndex = srvIndex; // 使うテクスチャの番号を記録

	particleGroups_[groupName] = newGroup;
}

// ★スライド5枚目：パーティクル発生（Emit）の実装
void ParticleManager::Emit(const std::string& groupName, const Vector3& position, uint32_t count)
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
		newParticle.position = position; // 発生座標を設定

		// 速度をランダムに決定
		newParticle.velocity.x = distVelocity(randomEngine_);
		newParticle.velocity.y = distVelocity(randomEngine_);
		newParticle.velocity.z = distVelocity(randomEngine_);

		newParticle.color = { 1.0f, 1.0f, 1.0f, 1.0f }; // 初期の色（白）
		newParticle.scale = 1.0f;                       // 初期サイズ
		newParticle.lifeTime = distLifeTime(randomEngine_); // ランダムな寿命
		newParticle.currentLifeTime = 0.0f;             // 経過時間は0からスタート

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
				// まだ生きている場合は移動させる (座標 = 速度 * 時間)
				it->position.x += it->velocity.x * kDeltaTime;
				it->position.y += it->velocity.y * kDeltaTime;
				it->position.z += it->velocity.z * kDeltaTime;

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
	commandList->IASetVertexBuffers(0, 1, &vbv_);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 共通データのセット
	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress()); // [0] マテリアル
	commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress()); // [3] ライト
	commandList->SetGraphicsRootConstantBufferView(4, cameraResource_->GetGPUVirtualAddress());   // [4] カメラ

	// 配列バッファに書き込むためのポインタを取得
	TransformationMatrix* wvpData = nullptr;
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));

	size_t particleIndex = 0; // 何個目のパーティクルか数えるカウンター

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
			MyMath::Matrix4x4 worldMatrix = MyMath::MakeTranslateMatrix(particle.position);

			// ② 行列を書き込む（本来はここにカメラのViewProjection行列を掛け算します）
			// ※ もしカメラの行列を掛け算する関数（例: Multiply等）があれば、ここで適用してください。
			// 一旦動くか確認するため、今回はそのままワールド行列を入れます。
			wvpData[particleIndex].World = worldMatrix;
			wvpData[particleIndex].WVP = worldMatrix;

			// ③ GPU側のアドレスを、このパーティクルの番号（インデックス）の分だけズラして指定する
			D3D12_GPU_VIRTUAL_ADDRESS wvpAddress = wvpResource_->GetGPUVirtualAddress() + (sizeof(TransformationMatrix) * particleIndex);
			commandList->SetGraphicsRootConstantBufferView(1, wvpAddress); // [1] WVP行列

			// 描画
			commandList->DrawInstanced(6, 1, 0, 0);

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
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
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
	vertexData[0] = { {-0.5f, -0.5f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f} }; // 左下
	vertexData[1] = { {-0.5f,  0.5f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f} }; // 左上
	vertexData[2] = { { 0.5f, -0.5f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f} }; // 右下
	vertexData[3] = { {-0.5f,  0.5f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f} }; // 左上
	vertexData[4] = { { 0.5f,  0.5f, 0.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f} }; // 右上
	vertexData[5] = { { 0.5f, -0.5f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f} }; // 右下

	// 頂点バッファビュー（VBV）の設定
	vbv_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vbv_.SizeInBytes = static_cast<UINT>(vertexBufferSize);
	vbv_.StrideInBytes = sizeof(VertexData);
}
