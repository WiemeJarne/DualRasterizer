#include "pch.h"
#include "PartialCoverageEffect.h"
#include "Texture.h"
#include <cassert>

namespace dae
{
	PartialCoverageEffect::PartialCoverageEffect(ID3D11Device* pDevice, const std::wstring& filePath)
		: Effect(pDevice, filePath)
	{
		//Create Vertex Layout
		static constexpr uint32_t amountOfElements{ 2 };
		D3D11_INPUT_ELEMENT_DESC vertexDesc[amountOfElements]{};

		vertexDesc[0].SemanticName = "POSITION";
		vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[0].AlignedByteOffset = 0;
		vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[1].SemanticName = "TEXTCOORD";
		vertexDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		vertexDesc[1].AlignedByteOffset = 12;
		vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		//Create Input Layout
		D3DX11_PASS_DESC passDesc{};
		m_pTechnique->GetPassByIndex(0)->GetDesc(&passDesc);

		HRESULT result
		{
			pDevice->CreateInputLayout
			(
				vertexDesc,
				amountOfElements,
				passDesc.pIAInputSignature,
				passDesc.IAInputSignatureSize,
				&m_pInputLayout
			)
		};

		if (FAILED(result))
			assert("Failed to create input layout\n");
	}
}