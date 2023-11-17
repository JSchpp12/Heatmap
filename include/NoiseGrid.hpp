#pragma once 

#include "Grid.hpp"
#include "TextureMaterial.hpp"
#include "StarComputePipeline.hpp"
#include "StarDescriptors.hpp"
#include "StarCommandBuffer.hpp"
#include "ConfigFile.hpp"

#include <memory>

class NoiseGrid : public star::Grid {
public:
	struct ComputeInfo {
		uint32_t deltaTime; 
	}; 

	virtual ~NoiseGrid(); 

	static std::unique_ptr<NoiseGrid> New(int vertX, int vertY); 

	virtual void prepDraw(); 

	/// @brief Cleanup the needed structures for this object. In this case it is the compute pipeline dependencies that need deleted.
	/// @param device star device which owns these objects
	virtual void cleanupRender(star::StarDevice& device) override; 

	/// <summary>
	/// Prepare needed objects for rendering operations.
	/// </summary>
	/// <param name="device"></param>
	virtual void prepRender(star::StarDevice& device, vk::Extent2D swapChainExtent,
		vk::PipelineLayout pipelineLayout, vk::RenderPass renderPass, int numSwapChainImages, star::StarDescriptorSetLayout& groupLayout,
		star::StarDescriptorPool& groupPool, std::vector<std::vector<vk::DescriptorSet>> globalSets) override; 
	
	/// <summary>
	/// Initalize this object. This object wil not have its own pipeline. It will expect to share one.
	/// </summary>
	/// <param name="device"></param>
	/// <param name="numSwapChainImages"></param>
	/// <param name="groupLayout"></param>
	/// <param name="groupPool"></param>
	/// <param name="globalSets"></param>
	/// <param name="sharedPipeline"></param>
	virtual void prepRender(star::StarDevice& device, int numSwapChainImages, star::StarDescriptorSetLayout& groupLayout,
		star::StarDescriptorPool& groupPool, std::vector<std::vector<vk::DescriptorSet>> globalSets, star::StarPipeline& sharedPipeline) override; 

protected:
	//know grid will only have 1 mesh....so only need 1 texture
	std::shared_ptr<star::Texture> displacementTexture; 
	std::shared_ptr<star::TextureMaterial> textureMaterial; 
	vk::PipelineLayout compPipeLayout; 
	std::unique_ptr<star::StarComputePipeline> compPipe; 
	std::unique_ptr<star::StarCommandBuffer> commandBuffer; 

	int swapChainImages = 0;

	NoiseGrid(int vertX, int vertY, std::shared_ptr<star::TextureMaterial> textureMaterial);

	void createComputeDependencies(star::StarDevice& device); 
};