#pragma once
#include "pch.h"
#include "Effect.h"

namespace dae
{
	class Texture;
	struct Camera;

	class Mesh
	{
	public:
		struct Vertex_PosTex
		{
			Vector3 position;
			Vector3 normal;
			Vector3 tangent;
			Vector2 uv;
		};

		struct Vertex_Out
		{
			Vector4 position;
			Vector3 normal;
			Vector3 tangent;
			Vector3 viewDirection;
			Vector2 uv;
		};

		enum class CullMode
		{
			BackFace,
			FrontFace,
			None
		};

		Mesh(ID3D11Device* pDevice, const std::string& modelFilePath, CullMode cullMode, float windowWidth, float windowHeight);
		virtual ~Mesh();

		Mesh(const Mesh& other) = delete;
		Mesh(Mesh&& other) = delete;
		Mesh& operator=(const Mesh& other) = delete;
		Mesh& operator=(Mesh&& other) = delete;

		virtual void RenderInDirectX(ID3D11DeviceContext* pDeviceContext) const = 0;
		virtual void RenderInSoftwareRasterizer(const Camera& camera, float* pDepthBufferPixels, SDL_Surface* pBackBuffer, uint32_t* pBackBufferPixels) = 0;
		void RotateYCW(float angle); //CW = clockwise
		const Matrix& GetWorldMatrix() const { return m_WorldMatrix; }
		float GetRotationSpeed() const { return m_RotationSpeed; }
		float GetRotationAngle() const { return m_RotationAngle; }
		void SetCullMode(CullMode cullMode) { m_CullMode = cullMode; }

	protected:
		Matrix m_WorldMatrix
		{
			{1, 0, 0, 0},
			{0, 1, 0, 0},
			{0, 0, 1, 0},
			{0, 0, 0, 1}
		};

		ID3D11Buffer* m_pVertexBuffer{};
		ID3D11Buffer* m_pIndexBuffer{};

		uint32_t m_AmountOfIndices{};

		CullMode m_CullMode{};

		std::vector<Vertex_PosTex> m_Vertices;
		std::vector<Vertex_Out> m_VerticesOut;
		std::vector<uint32_t> m_Indices;

		float m_WindowWidth;
		float m_WindowHeight;

		void VertexTransformationFunction(const Camera& camera);
		bool IsPixelInTriange(const Vector2& v0, const Vector2& v1, const Vector2& v2, const Vector2& pixelPos) const;
		void CalculateBoundingBox(const Vector2& v0, const Vector2& v1, const Vector2& v2, Vector2& min, Vector2& max);
		void VisualizeBoundingBox(const Vector2& min, const Vector2& max, SDL_Surface* pBackBuffer, uint32_t* pBackBufferPixels) const;
		virtual ColorRGBA ShadePixel(const Vertex_Out& vertex) const = 0;
		bool IsTriangleInFrustum(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2);
		bool ShouldRenderTriangle(CullMode cullMode, float area) const;
		void CalculateWeights(const Vector2& pixelPos, float& w0, float& w1, float& w2, const Vector2& v0, const Vector2& v1, const Vector2& v2, float area) const;
		float CalculateDepthInterpolated(float w0, float w1, float w2, float v0Depth, float v1Depth, float v2Depth) const;
		Vertex_Out CalculatePixel(const Vector2& pixelPos, float w0, float w1, float w2, const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, float depthInterpolated) const;
		void MapPixelToBackBuffer(int px, int py, ColorRGBA color, SDL_Surface* pBackBuffer, uint32_t* pBackBufferPixels) const;

	private:
		float m_RotationAngle{};
		float m_RotationSpeed{};
	};
}