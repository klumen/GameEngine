#pragma once

#include "runtime/resource/asset/Asset.h"

namespace Lumen
{
	class Folder : public Asset
	{
	public:
		Folder() = default;
		~Folder() = default;

		static AssetType GetStaticType() { return AssetType::Folder; }
		virtual AssetType GetType() const override { return GetStaticType(); }

	private:

	};
}