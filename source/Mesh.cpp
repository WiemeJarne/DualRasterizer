#include "pch.h"
#include "Mesh.h"
#include "Utils.h"
#include "Texture.h"
#include "OpaqueEffect.h"
#include "PartialCoverageEffect.h"
#include "Camera.h"
#include <cassert>

namespace dae
{
	Mesh::Mesh(ID3D11Device* pDevice, const std::string& modelFilePath, CullMode cullMode, float windowWidth, float windowHeight)
		: m_RotationAngle{}
		, m_RotationSpeed{ 0.785398163f }
		, m_CullMode{ cullMode }
		, m_WindowWidth{ windowWidth }
		, m_WindowHeight{ windowHeight }
	{
		Utils::ParseOBJ(modelFilePath, m_Vertices, m_Indices);

		HRESULT result{};
		//Create vertex buffer
		D3D11_BUFFER_DESC bd{};
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(Vertex_PosTex) * static_cast<uint32_t>(m_Vertices.size());
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
	
		D3D11_SUBRESOURCE_DATA initData{};
		initData.pSysMem = m_Vertices.data();
	
		result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
	
		if (FAILED(result))
			assert("Failed to create vertex buffer");
	
		//Create index buffer
		m_AmountOfIndices = static_cast<uint32_t>(m_Indices.size());
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(uint32_t) * m_AmountOfIndices;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
	
		initData.pSysMem = m_Indices.data();
	
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

	void Mesh::VertexTransformationFunction(const Camera& camera)
	{
		m_VerticesOut.clear();
		Matrix worldViewProjectionMatrix{ m_WorldMatrix * camera.viewMatrix * camera.projectionMatrix };

		for (const Vertex_PosTex& vertex : m_Vertices)
		{
			Vertex_Out vertexOut{};

			vertexOut.position = worldViewProjectionMatrix.TransformPoint({ vertex.position.x, vertex.position.y, vertex.position.z, 0 });

			//perspective divide
			const float wInversed{ 1.f / vertexOut.position.w };
			vertexOut.position.x *= wInversed;
			vertexOut.position.y *= wInversed;
			vertexOut.position.z *= wInversed;
			vertexOut.position.w = wInversed;

			//set the normal of the vertex
			vertexOut.normal = m_WorldMatrix.TransformVector(vertex.normal);

			//set the tangent of the vertex
			vertexOut.tangent = m_WorldMatrix.TransformVector(vertex.tangent);

			//set the viewDirection of the vertex
			vertexOut.viewDirection = camera.origin - m_WorldMatrix.TransformPoint(vertex.position);

			//set uv of the vertex
			vertexOut.uv = vertex.uv;

			m_VerticesOut.emplace_back(vertexOut);
		}
	}

	bool Mesh::IsPixelInTriange(const Vector2& v0, const Vector2& v1, const Vector2& v2, const Vector2& pixelPos) const
	{
		float cross1{ Vector2::Cross(v2 - v1, pixelPos - v1) };
		bool cross1IsPositive{};

		if (cross1 > 0)
		{
			cross1IsPositive = true;
		}

		float cross2{ Vector2::Cross(v0 - v2, pixelPos - v2) };
		bool cross2IsPositive{};

		if (cross2 > 0)
		{
			cross2IsPositive = true;
		}

		float cross3{ Vector2::Cross(v1 - v0, pixelPos - v0) };
		bool cross3IsPositive{};

		if (cross3 > 0)
		{
			cross3IsPositive = true;
		}

		if (cross1IsPositive && cross2IsPositive && cross3IsPositive
			|| !cross1IsPositive && !cross2IsPositive && !cross3IsPositive)
			return true;

		return false;
	}

	void Mesh::CalculateBoundingBox(const Vector2& v0, const Vector2& v1, const Vector2& v2, Vector2& min, Vector2& max)
	{
		min = { std::min(v0.x, v1.x), std::min(v0.y, v1.y) };
		min.x = std::min(min.x, v2.x);
		min.x = std::max(min.x, 0.f);
		min.y = std::min(min.y, v2.y);
		min.y = std::max(min.y, 0.f);

		max = { std::max(v0.x, v1.x), std::max(v0.y, v1.y) };
		max.x = std::max(max.x, v2.x);
		max.x = std::min(max.x, m_WindowWidth);
		max.y = std::max(max.y, v2.y);
		max.y = std::min(max.y, m_WindowHeight);
	}

	void Mesh::VisualizeBoundingBox(const Vector2& min, const Vector2& max, SDL_Surface* pBackBuffer, uint32_t* pBackBufferPixels) const
	{
		ColorRGBA color{ 1.f, 1.f, 1.f };

		for (int px{ static_cast<int>(min.x) }; px < max.x; ++px)
		{
			for (int py{ static_cast<int>(min.y) }; py < max.y; ++py)
			{
				MapPixelToBackBuffer(px, py, color, pBackBuffer, pBackBufferPixels);
			}
		}
	}

	bool Mesh::IsTriangleInFrustum(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2)
	{
		if (v0.position.x < -1.f || v0.position.x > 1.f
			|| v1.position.x < -1.f || v1.position.x > 1.f
			|| v2.position.x < -1.f || v2.position.x > 1.f)
			return false;

		if (v0.position.y < -1.f || v0.position.y > 1.f
			|| v1.position.y < -1.f || v1.position.y > 1.f
			|| v2.position.y < -1.f || v2.position.y > 1.f)
			return false;

		if (v0.position.z < 0.f || v0.position.z > 1.f
			|| v1.position.z < 0.f || v1.position.z > 1.f
			|| v2.position.z < 0.f || v2.position.z > 1.f)
			return false;

		return true;
	}

	bool Mesh::ShouldRenderTriangle(CullMode cullMode, float area) const
	{
		switch (cullMode)
		{
		case dae::Mesh::CullMode::BackFace:
			if (area > 0) return true;
			break;

		case dae::Mesh::CullMode::FrontFace:
			if (area < 0) return true;
			break;

		case dae::Mesh::CullMode::None:
			return true;
		}

		return false;
	}

	void Mesh::CalculateWeights(const Vector2& pixelPos, float& w0, float& w1, float& w2, const Vector2& v0, const Vector2& v1, const Vector2& v2, float area) const
	{
		w0 = (Vector2::Cross(v2 - v1, pixelPos - v1) / 2.f) / area;
		w1 = (Vector2::Cross(v0 - v2, pixelPos - v2) / 2.f) / area;
		w2 = (Vector2::Cross(v1 - v0, pixelPos - v0) / 2.f) / area;
	}

	float Mesh::CalculateDepthInterpolated(float w0, float w1, float w2, float v0Depth, float v1Depth, float v2Depth) const
	{
		return
		{
			1.f / (   (1.f * w0) / v0Depth
				    + (1.f * w1) / v1Depth
				    + (1.f * w2) / v2Depth )
		};
	}

	Mesh::Vertex_Out Mesh::CalculatePixel( const Vector2& pixelPos, float w0, float w1, float w2, const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, float depthInterpolated) const
	{
		Vertex_Out pixel{};

		float interpolatedCameraSpaceZ =
		{
			1.f / (w0 * v0.position.w
				   + w1 * v1.position.w
				   + w2 * v2.position.w)
		};

		pixel.uv =
		{
			interpolatedCameraSpaceZ *
			(v0.uv * w0 * v0.position.w
			+ v1.uv * w1 * v1.position.w
			+ v2.uv * w2 * v2.position.w)
		};

		pixel.position =
		{
			pixelPos.x,
			pixelPos.y,
			depthInterpolated,
			interpolatedCameraSpaceZ
		};

		pixel.normal =
		{
			Vector3{v0.normal * w0 * v0.position.w
					+ v1.normal * w1 * v1.position.w
					+ v2.normal * w2 * v2.position.w}.Normalized()
		};

		pixel.tangent =
		{
			Vector3{v0.tangent * w0 * v0.position.w
					+ v1.tangent * w1 * v1.position.w
					+ v2.tangent * w2 * v2.position.w}.Normalized()
		};

		pixel.viewDirection =
		{
			Vector3{v0.viewDirection * w0 * v0.position.w
					+ v1.viewDirection * w1 * v1.position.w
					+ v2.viewDirection * w2 * v2.position.w}.Normalized()
		};

		return pixel;
	}

	void Mesh::MapPixelToBackBuffer(int px, int py, ColorRGBA color, SDL_Surface* pBackBuffer, uint32_t* pBackBufferPixels) const
	{
		pBackBufferPixels[px + (py * static_cast<int>(m_WindowWidth))] = SDL_MapRGB(pBackBuffer->format,
			static_cast<uint8_t>(color.r * 255),
			static_cast<uint8_t>(color.g * 255),
			static_cast<uint8_t>(color.b * 255));
	}

	void Mesh::CycleCullMode()
	{
		switch (m_CullMode)
		{
		case dae::Mesh::CullMode::BackFace:
			m_CullMode = CullMode::FrontFace;
			break;

		case dae::Mesh::CullMode::FrontFace:
			m_CullMode = CullMode::None;
			break;

		case dae::Mesh::CullMode::None:
			m_CullMode = CullMode::BackFace;
			break;
		}
	}
}