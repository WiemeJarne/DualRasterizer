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
		struct Vertex_In
		{
			Vector3 position;
			Vector2 uv;
			Vector3 normal;
			Vector3 tangent;
		};

		struct Vertex_Out
		{
			Vector4 position;
			Vector2 uv;
			Vector3 normal;
			Vector3 tangent;
			Vector3 viewDirection;
		};

		Mesh(ID3D11Device* pDevice, const std::string& modelFilePath, float windowWidth, float windowHeight);
		virtual ~Mesh();

		Mesh(const Mesh& other) = delete;
		Mesh(Mesh&& other) = delete;
		Mesh& operator=(const Mesh& other) = delete;
		Mesh& operator=(Mesh&& other) = delete;

		virtual void RenderHardware(ID3D11DeviceContext* pDeviceContext) const = 0;
		virtual void RenderSoftware(const Camera& camera, float* pDepthBufferPixels, SDL_Surface* pBackBuffer, uint32_t* pBackBufferPixels) = 0;
		void ToggleBoundingBoxVisualization();
		void RotateYCW(float angle); //CW = clockwise
		const Matrix& GetWorldMatrix() const { return m_WorldMatrix; }
		float GetRotationSpeed() const { return m_RotationSpeed; }
		float GetRotationAngle() const { return m_RotationAngle; }

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

		std::vector<Vertex_In> m_Vertices;
		std::vector<Vertex_Out> m_VerticesOut;
		std::vector<uint32_t> m_Indices;

		float m_WindowWidth;
		float m_WindowHeight;

		bool m_VisualzeBoundingBox{};

		void VertexTransformationFunction(const Camera& camera);
		//vertices have to be in NDC space
		bool IsTriangleInFrustum(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2);
		void TransformVerticesToScreenSpace(Vertex_Out& v0, Vertex_Out& v1, Vertex_Out& v2);
		void CalculateBoundingBox(const Vector2& v0, const Vector2& v1, const Vector2& v2, Vector2& min, Vector2& max) const;
		void VisualizeBoundingBox(const Vector2& min, const Vector2& max, SDL_Surface* pBackBuffer, uint32_t* pBackBufferPixels) const;
		bool IsPixelInTriange(const Vector2& v0, const Vector2& v1, const Vector2& v2, const Vector2& pixelPos) const;
		void CalculateWeights(const Vector2& pixelPos, float& w0, float& w1, float& w2, const Vector2& v0, const Vector2& v1, const Vector2& v2, float area) const;
		float CalculateDepthInterpolated(float w0, float w1, float w2, float v0Depth, float v1Depth, float v2Depth) const;
		Vertex_Out CalculatePixel(const Vector2& pixelPos, float w0, float w1, float w2, const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, float depthInterpolated) const;
		virtual void RenderTriangle(uint32_t triangleIndex, const Vertex_Out& vertex0, const Vertex_Out& vertex1, const Vertex_Out& vertex2, float triangleArea, float* pDepthBufferPixels, SDL_Surface* pBackBuffer, uint32_t* pBackBufferPixels) const = 0;
		virtual void RenderPixel(uint32_t pixelIndex, const Vertex_Out& triangleVertex0, const Vertex_Out& triangleVertex1, const Vertex_Out& triangleVertex2, float triangleArea, float* pDepthBufferPixels, SDL_Surface* pBackBuffer, uint32_t* pBackBufferPixels) const = 0;
		virtual ColorRGBA ShadePixel(const Vertex_Out& vertex) const = 0;
		void MapPixelToBackBuffer(int pixelIndex, const ColorRGBA& color, SDL_Surface* pBackBuffer, uint32_t* pBackBufferPixels) const;

	private:
		float m_RotationAngle{};
		float m_RotationSpeed{};
	};
}