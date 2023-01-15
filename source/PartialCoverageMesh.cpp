#include "pch.h"
#include "PartialCoverageMesh.h"
#include "PartialCoverageEffect.h"

namespace dae
{
	PartialCoverageMesh::PartialCoverageMesh(ID3D11Device* pDevice, const std::string& modelFilePath, const std::wstring& shaderFilePath)
		: Mesh(pDevice, modelFilePath)
		, m_pEffect{ new PartialCoverageEffect(pDevice, shaderFilePath) }
	{}

	PartialCoverageMesh::~PartialCoverageMesh()
	{
		delete m_pEffect;
	}

	void PartialCoverageMesh::Render(ID3D11DeviceContext* pDeviceContext) const
	{
		//1. Set Primitive Topology
		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

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

	void PartialCoverageMesh::SetDiffuseMap(Texture* diffuseMap)
	{
		m_pEffect->SetDiffuseMap(diffuseMap);
	}

	void PartialCoverageMesh::SetWorldViewProjMatrix(const Matrix& worldViewProjMatrix)
	{
		m_pEffect->SetWorldViewProjMatrix(worldViewProjMatrix);
	}
}