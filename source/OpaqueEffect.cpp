#include "pch.h"
#include "OpaqueEffect.h"
#include "Texture.h"
#include <cassert>

namespace dae
{
	OpaqueEffect::OpaqueEffect(ID3D11Device* pDevice, const std::wstring& filePath)
		: Effect(pDevice, filePath)
	{
		m_pSamplerStateVariable = m_pEffect->GetVariableByName("gSamplerState")->AsSampler();
		if (!m_pSamplerStateVariable->IsValid())
		{
			std::wcout << L"m_pSamplerStateVariable not valid!\n";
		}

		m_pWorldMatrixVariable = m_pEffect->GetVariableByName("gWorld")->AsMatrix();
		if (!m_pWorldMatrixVariable->IsValid())
		{
			std::wcout << L"m_pWorldMatrixVariable not valid!\n";
		}

		m_pViewInverseMatrixVariable = m_pEffect->GetVariableByName("gViewInverse")->AsMatrix();
		if (!m_pViewInverseMatrixVariable->IsValid())
		{
			std::wcout << L"m_pMatWorldViewProjVariable not valid!\n";
		}

		m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
		if (!m_pNormalMapVariable->IsValid())
		{
			std::wcout << L"m_pNormalMapVariable not valid!\n";
		}

		m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
		if (!m_pSpecularMapVariable->IsValid())
		{
			std::wcout << L"m_pSpecularMapVariable not valid!\n";
		}

		m_pGlossinessMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
		if (!m_pGlossinessMapVariable->IsValid())
		{
			std::wcout << L"m_pGlossinessMapVariable not valid!\n";
		}

		//Create Vertex Layout
		static constexpr uint32_t amountOfElements{ 4 };
		D3D11_INPUT_ELEMENT_DESC vertexDesc[amountOfElements]{};

		vertexDesc[0].SemanticName = "POSITION";
		vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[0].AlignedByteOffset = 0;
		vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[1].SemanticName = "TEXTCOORD";
		vertexDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		vertexDesc[1].AlignedByteOffset = 12;
		vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[2].SemanticName = "NORMAL";
		vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[2].AlignedByteOffset = 20;
		vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[3].SemanticName = "TANGENT";
		vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[3].AlignedByteOffset = 32;
		vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

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

	OpaqueEffect::~OpaqueEffect()
	{
		if (m_pSamplerStateVariable)
			m_pSamplerStateVariable->Release();

		if (m_pWorldMatrixVariable)
			m_pWorldMatrixVariable->Release();

		if (m_pViewInverseMatrixVariable)
			m_pViewInverseMatrixVariable->Release();

		if (m_pNormalMapVariable)
			m_pNormalMapVariable->Release();

		if (m_pSpecularMapVariable)
			m_pSpecularMapVariable->Release();

		if (m_pGlossinessMapVariable)
			m_pGlossinessMapVariable->Release();
	}

	void OpaqueEffect::SetSamplerState(ID3D11SamplerState* pSamplerState)
	{
		if (m_pSamplerStateVariable)
			m_pSamplerStateVariable->SetSampler(0, pSamplerState);
	}

	void OpaqueEffect::SetWorldMatrix(const dae::Matrix& worldMatrix)
	{
		if (m_pWorldMatrixVariable)
			m_pWorldMatrixVariable->SetMatrix(reinterpret_cast<const float*>(&worldMatrix));
	}

	void OpaqueEffect::SetViewInverseMatrix(const dae::Matrix& invViewMatrix)
	{
		if (m_pViewInverseMatrixVariable)
			m_pViewInverseMatrixVariable->SetMatrix(reinterpret_cast<const float*>(&invViewMatrix));
	}

	void OpaqueEffect::SetNormalMap(dae::Texture* pNormalTexture)
	{
		if (m_pNormalMapVariable)
			m_pNormalMapVariable->SetResource(pNormalTexture->GetSRV());
	}

	void OpaqueEffect::SetSpecularMap(dae::Texture* pSpecularTexture)
	{
		if (m_pSpecularMapVariable)
			m_pSpecularMapVariable->SetResource(pSpecularTexture->GetSRV());
	}

	void OpaqueEffect::SetGlossinessMap(dae::Texture* pGlossinessTexture)
	{
		if (m_pGlossinessMapVariable)
			m_pGlossinessMapVariable->SetResource(pGlossinessTexture->GetSRV());
	}
}