#pragma once

#include "runtime/resource/asset/AssetImporter.h"

namespace Lumen
{
	struct ShaderImportSettings
	{

	};

	class Shader;

	class ShaderImporter : public AssetImporter
	{
	public:
		ShaderImporter(const std::filesystem::path& shaderPath);
		virtual ~ShaderImporter() = default;

		virtual Shared<Asset> Import() override;
		virtual bool CreateAsset() override;

	private:

	};
}