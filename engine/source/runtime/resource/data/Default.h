#pragma once

#include "runtime/resource/asset/Asset.h"

namespace Lumen
{
	class Default : public Asset
	{
	public:
		Default() = default;
		~Default() = default;

		static AssetType GetStaticType() { return AssetType::Default; }
		virtual AssetType GetType() const override { return GetStaticType(); }

	private:

	};

}