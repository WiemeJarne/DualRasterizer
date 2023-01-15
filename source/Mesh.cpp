#include "pch.h"
#include "Mesh.h"
#include "Utils.h"
#include "Texture.h"
#include "OpaqueEffect.h"
#include "PartialCoverageEffect.h"
#include <cassert>

namespace dae
{
	Mesh::Mesh(ID3D11Device* pDevice, const std::string& modelFilePath)
		: m_RotationAngle{}
		, m_RotationSpeed{ 0.785398163f }
	{
		std::vector<Vertex_PosTex> vertices;
		std::vector<uint32_t> indices;
		Utils::ParseOBJ(modelFilePath, vertices, indices);

		HRESULT result{};
		//Create vertex buffer
		D3D11_BUFFER_DESC bd{};
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(Vertex_PosTex) * static_cast<uint32_t>(vertices.size());
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
	
		D3D11_SUBRESOURCE_DATA initData{};
		initData.pSysMem = vertices.data();
	
		result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
	
		if (FAILED(result))
			assert("Failed to create vertex buffer");
	
		//Create index buffer
		m_AmountOfIndices = static_cast<uint32_t>(indices.size());
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(uint32_t) * m_AmountOfIndices;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
	
		initData.pSysMem = indices.data();
	
		result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
	
		if (FAILED(result))
			assert("Failed to create index buffer");
	}

	Mesh::~Mesh()
	{
		if (m_pVertexBuffer)
		{
			m_pVertexBuffer->Release();
		}

		if (m_pIndexBuffer)
		{
			m_pIndexBuffer->Release();
		}
	}

	void Mesh::RotateYCW(float angle)
	{
		m_RotationAngle = angle;
		m_WorldMatrix = Matrix::CreateRotationY(m_RotationAngle) * Matrix::CreateTranslation(m_WorldMatrix.GetTranslation());
	}
}