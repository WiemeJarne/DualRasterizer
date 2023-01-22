#include "pch.h"
#include "Effect.h"

namespace dae
{
	class PartialCoverageEffect final : public Effect
	{
	public:
		PartialCoverageEffect(ID3D11Device* pDevice, const std::wstring& filePath);
		~PartialCoverageEffect() = default;

		PartialCoverageEffect(const PartialCoverageEffect& other) = delete;
		PartialCoverageEffect(PartialCoverageEffect&& other) = delete;
		PartialCoverageEffect& operator=(const PartialCoverageEffect& other) = delete;
		PartialCoverageEffect& operator=(PartialCoverageEffect&& other) = delete;
	};
}