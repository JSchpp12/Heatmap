#pragma once 

#include "Grid.hpp"
#include "TextureMaterial.hpp"
#include "StarComputePipeline.hpp"
#include "StarDescriptors.hpp"
#include "StarCommandBuffer.hpp"
#include "ConfigFile.hpp"

#include <memory>
#include <vector>

class NoiseGrid : public star::Grid {
public:
	struct ComputeInfo {
		glm::vec2 noiseImageResolution = glm::vec2(320, 320); 
	}; 

	ComputeInfo* noiseComputeValues = nullptr; 

	virtual ~NoiseGrid(); 

	static std::unique_ptr<NoiseGrid> New(int sizeScale);

	virtual void initRender(int numFramesInFlight) override;

	 virtual void prepDraw(int frameInFlightIndex);

	/// @brief Cleanup the needed structures for this object. In this case it is the compute pipeline dependencies that need deleted.
	/// @param device star device which owns these objects
	virtual void cleanupRender(star::StarDevice& device) override; 

	/// <summary>
	/// Prepare needed objects for rendering operations.
	/// </summary>
	/// <param name="device"></param>
	virtual void prepRender(star::StarDevice& device, vk::Extent2D swapChainExtent,
		vk::PipelineLayout pipelineLayout, vk::RenderPass renderPass, int numFramesInFlight, star::StarDescriptorSetLayout& groupLayout,
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
	virtual void prepRender(star::StarDevice& device, int numFramesInFlight, star::StarDescriptorSetLayout& groupLayout,
		star::StarDescriptorPool& groupPool, std::vector<std::vector<vk::DescriptorSet>> globalSets, star::StarPipeline& sharedPipeline) override; 

	virtual void recordPreRenderPassCommands(star::StarCommandBuffer& commandBuffer, int swapChainIndexNum) override; 

	virtual void recordRenderPassCommands(star::StarCommandBuffer& commandBuffer, vk::PipelineLayout& pipelineLayout, int swapChainIndexNum, uint32_t vb_start, uint32_t ib_start) override; 
protected:
	//know grid will only have 1 mesh....so only need 1 texture
	std::shared_ptr<star::Texture> displacementTexture; 
	std::unique_ptr<star::StarDescriptorSetLayout> descriptorLayout; 
	vk::PipelineLayout compPipeLayout; 
	std::unique_ptr<star::StarComputePipeline> compPipe; 
	std::unique_ptr<star::StarCommandBuffer> commandBuffer;
	std::vector<vk::DescriptorSet> descriptorSets;
	star::StarCommandBuffer* mainDrawCommands = nullptr; 
	bool toldMainRenderToWait = false; 

	star::StarDevice* device = nullptr; 

	std::vector<ComputeInfo> compInfos; 

	int swapChainImages = 0;

	NoiseGrid(int vertX, int vertY, std::shared_ptr<star::Texture> texture, std::shared_ptr<star::TextureMaterial> textureMaterial);

	void createComputeDependencies(); 

	virtual std::unordered_map<star::Shader_Stage, star::StarShader> getShaders() override;

	void prepareCompImageForMain(star::StarCommandBuffer& commandBuffer, int imageInFlight); 

	void recordComputeCommands(star::StarCommandBuffer& commandBuffer, int imageInFlight); 
};