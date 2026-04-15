#pragma once
#include "ModelCommon.h"
#include "Model.h"

#include <cassert>
//ファイルやディレクトリに関する操作を行うライブラリ
#include <filesystem>
//ファイルに書いたり読んだりするライブラリ
#include<fstream>
#include<map>

class ModelManager
{
private:
	static ModelManager* instance;

	ModelManager() = default;
	~ModelManager() = default;
	ModelManager(ModelManager&) = default;
	ModelManager& operator=(ModelManager&) = delete;

	ModelCommon* modelCommon = nullptr;

public:
	// シングルトン
	static ModelManager* GetInstance();
	// 終了
	void Finalize();
	std::map<std::string, std::unique_ptr<Model>>models;
	// 初期化
	void Initialize(DirectXCommon* dxCommon);

	void LoadModel(const std::string& filePath);

	Model* FindModel(const std::string& filePath);
};

