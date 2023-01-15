#include "pch.h"
#include "PartialCoverageMesh.h"
#include "PartialCoverageEffect.h"
#include "Texture.h"

namespace dae
{
	PartialCoverageMesh::PartialCoverageMesh(ID3D11Device* pDevice, const std::string& modelFilePath, const std::wstring& shaderFilePath, CullMode cullMode, float windowWidth, float windowHeight)
		: Mesh(pDevice, modelFilePath, cullMode, windowWidth, windowHeight)
		, m_pEffect{ new PartialCoverageEffect(pDevice, shaderFilePath) }
	{}

	PartialCoverageMesh::~PartialCoverageMesh()
	{
		delete m_pEffect;
	}

	void PartialCoverageMesh::RenderInDirectX(ID3D11DeviceContext* pDeviceContext) const
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

	void  PartialCoverageMesh::RenderInSoftwareRasterizer(const Camera& camera, float* pDepthBufferPixels, SDL_Surface* pBackBuffer, uint32_t* pBackBufferPixels)
	{
		VertexTransformationFunction(camera);

		for (Vertex_Out& vertex : m_VerticesOut)
		{
			vertex.position.x = 0.5f * (vertex.position.x + 1.f) * m_WindowWidth;
			vertex.position.y = 0.5f * (1.f - vertex.position.y) * m_WindowHeight;
		}

		for (int index{}; index < static_cast<int>(m_AmountOfIndices); index += 3)
		{
			if (m_Indices[index] == m_Indices[index + 1]
				|| m_Indices[index + 1] == m_Indices[index + 2]
				|| m_Indices[index + 2] == m_Indices[index]		)
			{
				continue;
			}

			Vertex_Out& vertex0{ m_VerticesOut[m_Indices[index]] };
			Vertex_Out& vertex1{ m_VerticesOut[m_Indices[index + 1]] };
			Vertex_Out& vertex2{ m_VerticesOut[m_Indices[index + 2]] };

			IsTriangleInFrustum(vertex0, vertex1, vertex2);

			const Vector2 v0{ vertex0.position.x, vertex0.position.y };
			const Vector2 v1{ vertex1.position.x, vertex1.position.y };
			const Vector2 v2{ vertex2.position.x, vertex2.position.y };

			Vector2 v1ToV2{ v2 - v1 };
			Vector2 v2ToV0{ v0 - v2 };
			Vector2 v0ToV1{ v1 - v0 };

			const float area{ Vector2::Cross(v0ToV1, v2 - v0) / 2.f };

			if (!ShouldRenderTriangle(m_CullMode, area))
				continue;

			float w0{};
			float w1{};
			float w2{};

			Vector2 min{};
			Vector2 max{};

			CalculateBoundingBox(v0, v1, v2, min, max);

			for (int px{ static_cast<int>(min.x) }; px < max.x; ++px)
			{
				for (int py{ static_cast<int>(min.y) }; py < max.y; ++py)
				{
					Vector2 pixelPos = { static_cast<float>(px), static_cast<float>(py) };

					if (!IsPixelInTriange(v0, v1, v2, pixelPos))
						continue;

					CalculateWeights(pixelPos, w0, w1, w2, v0, v1, v2, area);

					float depthInterpolated{ CalculateDepthInterpolated(w0, w1, w2, vertex0.position.z, vertex1.position.z, vertex2.position.z) };

					int pixelIndex{ static_cast<int>(py * m_WindowWidth + px) };

					if (depthInterpolated >= pDepthBufferPixels[pixelIndex])
							continue;

					Vertex_Out pixel{ CalculatePixel(pixelPos, w0, w1, w2, vertex0, vertex1, vertex2, depthInterpolated) };

					ColorRGBA finalColor{ ShadePixel(pixel) };

					Uint8 rValue{}, gValue{}, bValue{};
					SDL_GetRGB(pBackBufferPixels[static_cast<int>(pixel.position.x) + static_cast<int>(pixel.position.y * m_WindowWidth)], pBackBuffer->format, &rValue, &gValue, &bValue);

					finalColor.a = std::min(1.f, finalColor.a);

					finalColor =
					{
						finalColor.a * finalColor.r + (1.f - finalColor.a) * (rValue / 255.f),
						finalColor.a * finalColor.g + (1.f - finalColor.a) * (gValue / 255.f),
						finalColor.a * finalColor.b + (1.f - finalColor.a) * (bValue / 255.f)
					};

					//Update Color in Buffer
					finalColor.MaxToOne();

					MapPixelToBackBuffer(px, py, finalColor, pBackBuffer, pBackBufferPixels);
				}
			}
		}
	}

	ColorRGBA PartialCoverageMesh::ShadePixel(const Vertex_Out& vertex) const
	{
		constexpr float lightIntensity{ 7.f };
		return lightIntensity * m_pDiffuseMap->Sample(vertex.uv) / PI;
	}

	void PartialCoverageMesh::SetDiffuseMap(Texture* diffuseMap)
	{
		m_pEffect->SetDiffuseMap(diffuseMap);
		m_pDiffuseMap = diffuseMap;
	}

	void PartialCoverageMesh::SetWorldViewProjMatrix(const Matrix& worldViewProjMatrix)
	{
		m_pEffect->SetWorldViewProjMatrix(worldViewProjMatrix);
	}
}