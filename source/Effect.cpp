#include "pch.h"
#include "Effect.h"
#include "Texture.h"
#include <cassert>

Effect::Effect(ID3D11Device* pDevice, const std::wstring& filePath)
{
	m_pEffect = LoadEffect(pDevice, filePath);

	m_pTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");
	if (!m_pTechnique->IsValid())
	{
		std::wcout << L"Technique not valid\n";
	}

	m_pWorldViewProjMatrixVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
	if (!m_pWorldViewProjMatrixVariable->IsValid())
	{
		std::wcout << L"m_pMatWorldViewProjVariable not valid!\n";
	}
	
	//Create Vertex Layout
	static constexpr uint32_t amountOfElements{ 4 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[amountOfElements]{};

	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[1].SemanticName = "NORMAL";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[1].AlignedByteOffset = 12;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[2].SemanticName = "TANGENT";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[2].AlignedByteOffset = 24;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[3].SemanticName = "TEXTCOORD";
	vertexDesc[3].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexDesc[3].AlignedByteOffset = 36;
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

Effect::~Effect()
{
	if(m_pEffect)
		m_pEffect->Release();

	if (m_pInputLayout)
		m_pInputLayout->Release();
}

ID3DX11Effect* Effect::LoadEffect(ID3D11Device* pDevice, const std::wstring& filePath)
{
	ID3D10Blob* pErrorBlob{};
	ID3DX11Effect* pEffect;

	DWORD shaderFlags{ 0 };

#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT result
	{
		D3DX11CompileEffectFromFile
		(
			filePath.c_str(),
			nullptr,
			nullptr,
			shaderFlags,
			0,
			pDevice,
			&pEffect,
			&pErrorBlob
		)
	};

	if (FAILED(result))
	{
		std::wstringstream stringStream;

		if (pErrorBlob != nullptr)
		{
			const char* pErrors{ static_cast<char*>(pErrorBlob->GetBufferPointer()) };

			for (unsigned int index{}; index < pErrorBlob->GetBufferSize(); ++index)
				stringStream << pErrors[index];

			OutputDebugStringW(stringStream.str().c_str());

			pErrorBlob->Release();
			pErrorBlob = nullptr;

			std::wcout << stringStream.str() << '\n';
		}
		else
		{
			stringStream << "EffectLoader: Failed to CreateEffectFromFile!\nPath: " << filePath << '\n';
			std::wcout << stringStream.str();
			return nullptr;
		}
	}

	return pEffect;
}

void Effect::SetWorldViewProjMatrix(const dae::Matrix& worldViewProjMatrix)
{
	if (m_pWorldViewProjMatrixVariable)
	{
		m_pWorldViewProjMatrixVariable->SetMatrix(reinterpret_cast<const float*>(&worldViewProjMatrix));
	}
}

