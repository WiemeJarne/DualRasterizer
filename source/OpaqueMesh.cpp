#include "pch.h"
#include "OpaqueMesh.h"
#include "OpaqueEffect.h"
#include "Texture.h"
#include <cassert>

namespace dae
{
	OpaqueMesh::OpaqueMesh(ID3D11Device* pDevice, const std::string& modelFilePath, const std::wstring& shaderFilePath, CullMode cullMode, float windowWidth, float windowHeight)
		: Mesh(pDevice, modelFilePath, cullMode, windowWidth, windowHeight)
		, m_pEffect{ new OpaqueEffect(pDevice, shaderFilePath) }
	{}

	OpaqueMesh::~OpaqueMesh()
	{
		delete m_pEffect;
		
		if (m_pRasterizerState)
			m_pRasterizerState->Release();
	}

	void OpaqueMesh::RenderInDirectX(ID3D11DeviceContext* pDeviceContext) const
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

	void OpaqueMesh::RenderInSoftwareRasterizer(const Camera& camera, float* pDepthBufferPixels, SDL_Surface* pBackBuffer, uint32_t* pBackBufferPixels)
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
				|| m_Indices[index + 2] == m_Indices[index])
				continue;

			Vertex_Out& vertex0{ m_VerticesOut[m_Indices[index]] };
			Vertex_Out& vertex1{ m_VerticesOut[m_Indices[index + 1]] };
			Vertex_Out& vertex2{ m_VerticesOut[m_Indices[index + 2]] };

			if (!IsTriangleInFrustum(vertex0, vertex1, vertex2))
				continue;

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

			if (m_VisualzeBoundingBox)
			{
				VisualizeBoundingBox(min, max, pBackBuffer, pBackBufferPixels);
				continue;
			}

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

					if (depthInterpolated <= pDepthBufferPixels[pixelIndex])
					{
						pDepthBufferPixels[pixelIndex] = depthInterpolated;
					}
					else continue;

					ColorRGBA finalColor{ ShadePixel( CalculatePixel(pixelPos, w0, w1, w2, vertex0, vertex1, vertex2, depthInterpolated) ) };

					//Update Color in Buffer
					finalColor.MaxToOne();

					MapPixelToBackBuffer(px, py, finalColor, pBackBuffer, pBackBufferPixels);
				}
			}
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
		m_pDiffuseMap = diffuseMap;
	}

	void OpaqueMesh::SetNormalMap(Texture* normalMap)
	{
		m_pEffect->SetNormalMap(normalMap);
		m_pNormalMap = normalMap;
	}

	void OpaqueMesh::SetSpecularMap(Texture* specularMap)
	{
		m_pEffect->SetSpecularMap(specularMap);
		m_pSpecularMap = specularMap;
	}

	void OpaqueMesh::SetGlossinessMap(Texture* glossinessMap)
	{
		m_pEffect->SetGlossinessMap(glossinessMap);
		m_pGlossinessMap = glossinessMap;
	}

	void OpaqueMesh::SetWorldViewProjMatrix(const Matrix& worldViewProjMatrix)
	{
		m_pEffect->SetWorldViewProjMatrix(worldViewProjMatrix);
	}

	void OpaqueMesh::SetViewInverseMatrix(const Matrix& viewInverseMatrix)
	{
		m_pEffect->SetViewInverseMatrix(viewInverseMatrix);
	}

	ColorRGBA OpaqueMesh::ShadePixel(const Vertex_Out& vertex) const
	{
		if (m_VisualizeDepthBuffer)
		{
			const float value{ Remap(vertex.position.z, 0.995f) };
			return { value, value, value };
		}

		float observedArea{};
		const Vector3 lightDirection{ 0.577f, -0.577f, 0.577f };
		constexpr float lightIntensity{ 7.f };
		constexpr float shininess{ 25.f };
		Vector3 normal{ vertex.normal };

		if (m_UseNormalMap)
		{
			ColorRGBA sampledNormal{ m_pNormalMap->Sample(vertex.uv) };

			//remap the normal values to range [-1, 1]
			sampledNormal.r = 2.f * sampledNormal.r - 1.f;
			sampledNormal.g = 2.f * sampledNormal.g - 1.f;
			sampledNormal.b = 2.f * sampledNormal.b - 1.f;
			sampledNormal.a = 2.f * sampledNormal.a - 1.f;

			Vector3 binormal{ Vector3::Cross(vertex.normal, vertex.tangent) };
			Matrix tangentSpaceAxis{ vertex.tangent, binormal, vertex.normal, Vector3::Zero };

			//transform the sampledNormal to tangent space
			normal = { sampledNormal.r, sampledNormal.g, sampledNormal.b };
			normal = tangentSpaceAxis.TransformVector(normal).Normalized();
		}

		observedArea = std::max(0.f, Vector3::Dot(normal, -lightDirection));

		switch (m_ShadingMode)
		{
		case ShadingMode::combined:
		{
			constexpr ColorRGBA ambient{ 0.025f, 0.025f, 0.025f };

			//lambert diffuse
			ColorRGBA diffuse{};
			
			diffuse = lightIntensity * m_pDiffuseMap->Sample(vertex.uv) / PI;

			//specular phong
			ColorRGBA specular{m_pSpecularMap->Sample(vertex.uv) * powf(std::max(Vector3::Dot(Vector3::Reflect(-lightDirection, normal), vertex.viewDirection), 0.f),  m_pGlossinessMap->Sample(vertex.uv).r * shininess )}; //glossinessMap is greyscale so all channels have the same value

			return (diffuse + specular + ambient) * observedArea;
		}

		case ShadingMode::observedArea:
			return { observedArea, observedArea, observedArea };

		case ShadingMode::diffuse:
		{
			ColorRGBA diffuse{ lightIntensity * m_pDiffuseMap->Sample(vertex.uv) / PI };
			return diffuse * observedArea;
		}

		case ShadingMode::specular:
		{
			ColorRGBA specular{ m_pSpecularMap->Sample(vertex.uv) * powf(std::max(Vector3::Dot(2.f * std::max(Vector3::Dot(normal, -lightDirection), 0.f) * normal - -lightDirection, vertex.viewDirection), 0.f), shininess * m_pGlossinessMap->Sample(vertex.uv).r) }; //glossinessMap is greyscale so all channels have the same value
			return specular * observedArea;
		}
		}
	}

	void OpaqueMesh::CycleShadingMode()
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 5); //set console text color to purple

		switch (m_ShadingMode)
		{
		case dae::OpaqueMesh::ShadingMode::combined:
			m_ShadingMode = ShadingMode::observedArea;
			std::cout << "Shading mode = ObservedArea\n";
			break;

		case dae::OpaqueMesh::ShadingMode::observedArea:
			m_ShadingMode = ShadingMode::diffuse;
			std::cout << "Shading mode = Diffuse\n";
			break;

		case dae::OpaqueMesh::ShadingMode::diffuse:
			m_ShadingMode = ShadingMode::specular;
			std::cout << "Shading mode = Specular\n";
			break;

		case dae::OpaqueMesh::ShadingMode::specular:
			m_ShadingMode = ShadingMode::combined;
			std::cout << "Shading mode = Combined\n";
			break;
		}

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15); //set console text color to white
	}

	void OpaqueMesh::ToggleUseNormalMap()
	{
		m_UseNormalMap = !m_UseNormalMap;

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 5); //set console text color to purple

		if(m_UseNormalMap)
			std::cout << "NormalMap On\n";
		else
			std::cout << "NormalMap Off\n";

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15); //set console text color to white
	}

	void OpaqueMesh::ToggleDepthBufferVisualization()
	{
		m_VisualizeDepthBuffer = !m_VisualizeDepthBuffer;

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 5); //set console text color to purple

		if (m_VisualizeDepthBuffer)
			std::cout << "BoundingBox Visualization On\n";
		else
			std::cout << "BoundingBox Visualization Off\n";

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15); //set console text color to white
	}

	void OpaqueMesh::ToggleBoundingBoxVisualization()
	{
		m_VisualzeBoundingBox = !m_VisualzeBoundingBox;

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 5); //set console text color to purple

		if (m_VisualzeBoundingBox)
			std::cout << "DepthBuffer Visualization On\n";
		else
			std::cout << "DepthBuffer Visualization Off\n";

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15); //set console text color to white
	}
}