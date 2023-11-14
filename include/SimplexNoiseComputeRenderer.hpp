#pragma once 

#include "StarShader.hpp"
#include "SwapChainRenderer.hpp"
#include "StarRenderer.hpp"
#include "StarObject.hpp"
#include "StarTexture.hpp"
#include "ComputeTexture.hpp"

/// <summary>
/// Creates a renderer which includes a computation shader to 
/// calcaulte perlin noise. 
/// </summary>
class SimplexNoiseComputeRenderer : public star::SwapChainRenderer {
public:
	SimplexNoiseComputeRenderer(int width, int height, 
		star::StarWindow& window, std::vector<std::unique_ptr<star::Light>>& lightList,
		std::vector<std::reference_wrapper<star::StarObject>> objectList, star::StarCamera& camera, star::RenderOptions& renderOptions, star::StarDevice& device);

protected:
	
	std::vector<vk::DescriptorSet> computeDescriptorSets;

	vk::Semaphore computeDone;
};