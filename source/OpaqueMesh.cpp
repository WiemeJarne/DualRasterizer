#include "pch.h"
#include "OpaqueMesh.h"
#include "OpaqueEffect.h"
#include <cassert>
namespace dae
{
	OpaqueMesh::OpaqueMesh(ID3D11Device* pDevice, const std::string& modelFilePath, const std::wstring& shaderFilePath)
		: Mesh(pDevice, modelFilePath)
		, m_pEffect{ new OpaqueEffect(pDevice, shaderFilePath) }
	{}

	OpaqueMesh::~OpaqueMesh()
	{
		delete m_pEffect;
		
		if (m_pRasterizerState)
			m_pRasterizerState->Release();
	}

	void OpaqueMesh::Render(ID3D11DeviceContext* pDeviceContext) const
	{
		m_pEffect->SetWorldMatrix(m_WorldMatrix);

		//1. Set Primitive Topology and the rasterizerState
		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pDeviceContext->RSSetState(m_pRasterizerState);
		
		//2. Set Input Layout
		pDeviceContext->IASetInputLayout(m_pEffect->GetInputLayout());

		//3. Set VertexBuffer
		constexpr UINT stride{ sizeof(Vertex_PosTex) };
		constexpr UINT offset{ 0 };
		pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

		//4. Set IndexBuffer
		pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		//5. Draw
		D3DX11_TECHNIQUE_DESC techniqueDesc{};
		m_pEffect->GetTechnique()->GetDesc(&techniqueDesc);

		for (UINT index{}; index < techniqueDesc.Passes; ++index)
		{
			m_pEffect->GetTechnique()->GetPassByIndex(index)->Apply(0, pDeviceContext);
			pDeviceContext->DrawIndexed(m_AmountOfIndices, 0, 0);
		}
	}
	
	void OpaqueMesh::ChangeSamplerState(ID3D11Device* pDevice, Sampler* pSampler)
	{
		m_SamplerState = pSampler->GetSamplerStateKind();
		m_pEffect->SetSamplerState(pSampler->GetSamplerState());
	}

	void OpaqueMesh::SetRasterizerState(ID3D11RasterizerState* pRasterizerState)
	{
		if (m_pRasterizerState)
		{
			m_pRasterizerState->Release();
			m_pRasterizerState = nullptr;
		}
		
		m_pRasterizerState = pRasterizerState;
	}

	void OpaqueMesh::SetDiffuseMap(Texture* diffuseMap)
	{
		m_pEffect->SetDiffuseMap(diffuseMap);
	}

	void OpaqueMesh::SetNormalMap(Texture* normalMap)
	{
		m_pEffect->SetNormalMap(normalMap);
	}

	void OpaqueMesh::SetSpecularMap(Texture* specularMap)
	{
		m_pEffect->SetSpecularMap(specularMap);
	}

	void OpaqueMesh::SetGlossinessMap(Texture* glossinessMap)
	{
		m_pEffect->SetGlossinessMap(glossinessMap);
	}

	void OpaqueMesh::SetWorldViewProjMatrix(const Matrix& worldViewProjMatrix)
	{
		m_pEffect->SetWorldViewProjMatrix(worldViewProjMatrix);
	}

	void OpaqueMesh::SetViewInverseMatrix(const Matrix& viewInverseMatrix)
	{
		m_pEffect->SetViewInverseMatrix(viewInverseMatrix);
	}
}