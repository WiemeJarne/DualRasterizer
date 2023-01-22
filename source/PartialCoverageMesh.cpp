#include "pch.h"
#include "PartialCoverageMesh.h"
#include "PartialCoverageEffect.h"
#include "Texture.h"
#include <ppl.h> //parallel_for

namespace dae
{
	PartialCoverageMesh::PartialCoverageMesh(ID3D11Device* pDevice, const std::string& modelFilePath, const std::wstring& shaderFilePath, float windowWidth, float windowHeight)
		: Mesh(pDevice, modelFilePath, windowWidth, windowHeight)
		, m_pEffect{ new PartialCoverageEffect(pDevice, shaderFilePath) }
	{}

	PartialCoverageMesh::~PartialCoverageMesh()
	{
		delete m_pEffect;
	}

	void PartialCoverageMesh::RenderHardware(ID3D11DeviceContext* pDeviceContext) const
	{
		//1. Set Primitive Topology
		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//2. Set Input Layout
		pDeviceContext->IASetInputLayout(m_pEffect->GetInputLayout());

		//3. Set VertexBuffer
		constexpr UINT stride{ sizeof(Vertex_In) };
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

	void  PartialCoverageMesh::RenderSoftware(const Camera& camera, float* pDepthBufferPixels, SDL_Surface* pBackBuffer, uint32_t* pBackBufferPixels)
	{
		VertexTransformationFunction(camera);

		for (int index{}; index < static_cast<int>(m_AmountOfIndices); index += 3)
		{
			if (m_Indices[index] == m_Indices[index + 1]
				|| m_Indices[index + 1] == m_Indices[index + 2]
				|| m_Indices[index + 2] == m_Indices[index]		)
				continue;

			Vertex_Out& vertex0{ m_VerticesOut[m_Indices[index]] };
			Vertex_Out& vertex1{ m_VerticesOut[m_Indices[index + 1]] };
			Vertex_Out& vertex2{ m_VerticesOut[m_Indices[index + 2]] };

			//frustum clipping is turned off because it doesn't work as intended for this mesh
			//if (IsTriangleInFrustum(vertex0, vertex1, vertex2))
			//	continue;

			TransformVerticesToScreenSpace(vertex0, vertex1, vertex2);

			const Vector2 v0{ vertex0.position.x, vertex0.position.y };
			const Vector2 v1{ vertex1.position.x, vertex1.position.y };
			const Vector2 v2{ vertex2.position.x, vertex2.position.y };

			const float area{ Vector2::Cross(v1 - v0, v2 - v0) / 2.f };

			RenderTriangle(index / 3, vertex0, vertex1, vertex2, area, pDepthBufferPixels, pBackBuffer, pBackBufferPixels);
		}
	}

	void PartialCoverageMesh::RenderTriangle(uint32_t triangleIndex, const Vertex_Out& vertex0, const Vertex_Out& vertex1, const Vertex_Out& vertex2, float triangleArea, float* pDepthBufferPixels, SDL_Surface* pBackBuffer, uint32_t* pBackBufferPixels) const
	{
		const Vector2 v0{ vertex0.position.x, vertex0.position.y };
		const Vector2 v1{ vertex1.position.x, vertex1.position.y };
		const Vector2 v2{ vertex2.position.x, vertex2.position.y };

		Vector2 min{};
		Vector2 max{};

		CalculateBoundingBox(v0, v1, v2, min, max);

		if (m_VisualzeBoundingBox)
		{
			VisualizeBoundingBox(min, max, pBackBuffer, pBackBufferPixels);
			return;
		}

		const uint32_t amountOfPixels{ static_cast<uint32_t>((max.x - min.x) * (max.y - min.y)) };

		for (int px{ static_cast<int>(min.x) }; px < max.x; ++px)
		{
			for (int py{ static_cast<int>(min.y) }; py < max.y; ++py)
			{
				RenderPixel(static_cast<int>(py * m_WindowWidth + px), vertex0, vertex1, vertex2, triangleArea, pDepthBufferPixels, pBackBuffer, pBackBufferPixels);
			}
		}
	}

	void PartialCoverageMesh::RenderPixel(uint32_t pixelIndex, const Vertex_Out& triangleVertex0, const Vertex_Out& triangleVertex1, const Vertex_Out& triangleVertex2, float triangleArea, float* pDepthBufferPixels, SDL_Surface* pBackBuffer, uint32_t* pBackBufferPixels) const
	{
		Vector2 pixelPos = { static_cast<float>(pixelIndex % static_cast<int>(m_WindowWidth)), static_cast<float>(pixelIndex / static_cast<int>(m_WindowWidth)) };

		const Vector2 v0{ triangleVertex0.position.x, triangleVertex0.position.y };
		const Vector2 v1{ triangleVertex1.position.x, triangleVertex1.position.y };
		const Vector2 v2{ triangleVertex2.position.x, triangleVertex2.position.y };

		if (!IsPixelInTriange(v0, v1, v2, pixelPos))
			return;

		float w0{};
		float w1{};
		float w2{};

		CalculateWeights(pixelPos, w0, w1, w2, v0, v1, v2, triangleArea);

		float depthInterpolated{ CalculateDepthInterpolated(w0, w1, w2, triangleVertex0.position.z, triangleVertex1.position.z, triangleVertex2.position.z) };

		if (depthInterpolated >= pDepthBufferPixels[pixelIndex])
			return;

		Vertex_Out pixel{ CalculatePixel(pixelPos, w0, w1, w2, triangleVertex0, triangleVertex1, triangleVertex2, depthInterpolated) };

		ColorRGBA finalColor{ ShadePixel(pixel) };

		Uint8 rValue{}, gValue{}, bValue{};
		SDL_GetRGB(pBackBufferPixels[pixelIndex], pBackBuffer->format, &rValue, &gValue, &bValue);

		finalColor.a = std::min(1.f, finalColor.a);

		finalColor =
		{
			finalColor.a * finalColor.r + (1.f - finalColor.a) * (rValue / 255.f),
			finalColor.a * finalColor.g + (1.f - finalColor.a) * (gValue / 255.f),
			finalColor.a * finalColor.b + (1.f - finalColor.a) * (bValue / 255.f)
		};

		//Update Color in Buffer
		finalColor.MaxToOne();

		MapPixelToBackBuffer(pixelIndex, finalColor, pBackBuffer, pBackBufferPixels);
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