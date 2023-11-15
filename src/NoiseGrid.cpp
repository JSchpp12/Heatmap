#include "NoiseGrid.hpp"


std::unique_ptr<NoiseGrid> NoiseGrid::New(int vertX, int vertY)
{
	auto settings = star::StarTexture::TextureCreateSettings(
		vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled, 
		vk::Format::eR8G8B8A8Snorm
	); 
	auto texture = std::shared_ptr<star::Texture>(new star::Texture(vertX, vertY, settings));
	auto material = std::shared_ptr<star::TextureMaterial>(new star::TextureMaterial(texture)); 
	return std::unique_ptr<NoiseGrid>(new NoiseGrid(vertX, vertY, material));
}

void NoiseGrid::prepRender(star::StarDevice& device, vk::Extent2D swapChainExtent,
	vk::PipelineLayout pipelineLayout, vk::RenderPass renderPass, int numSwapChainImages, 
	star::StarDescriptorSetLayout& groupLayout, star::StarDescriptorPool& groupPool, std::vector<std::vector<vk::DescriptorSet>> globalSets)
{
	this->star::Grid::prepRender(device, swapChainExtent, pipelineLayout, renderPass, numSwapChainImages, groupLayout, groupPool, globalSets); 

	//this will also create a compute pipeline
	createComputeDependencies(device); 
}

void NoiseGrid::prepRender(star::StarDevice& device, int numSwapChainImages, 
	star::StarDescriptorSetLayout& groupLayout, star::StarDescriptorPool& groupPool, std::vector<std::vector<vk::DescriptorSet>> globalSets, star::StarPipeline& sharedPipeline)
{
	this->star::Grid::prepRender(device, numSwapChainImages, groupLayout, groupPool, globalSets, sharedPipeline);

	createComputeDependencies(device); 
}


NoiseGrid::NoiseGrid(int vertX, int vertY, std::shared_ptr<star::TextureMaterial> textureMaterial)
	: Grid(vertX, vertY, textureMaterial){ }

void NoiseGrid::createComputeDependencies(star::StarDevice& device)
{
	//create compute pipeline layout
	auto descriptorSetLayout = star::StarDescriptorSetLayout::Builder(device)
		.addBinding(0, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute)
		.build(); 
	
	//using push constants 
	vk::PushConstantRange pushConstant{}; 
	pushConstant.offset = 0; 
	pushConstant.size = sizeof(ComputeInfo); 
	pushConstant.stageFlags = vk::ShaderStageFlagBits::eCompute; 

	// auto descriptorBuilder = star::

	//compute pipe needs to know what kind of descriptors it will be recieving
	// vk::PipelineLayoutCreateInfo compLayoutInfo{}; 
	// compLayoutInfo.sType = vk::StructureType::ePipelineLayoutCreateInfo; 
	// compLayoutInfo.pSetLayouts = &
	// compLayoutInfo.pPushConstantRanges = &pushConstant; 
	// compLayoutInfo.pushConstantRangeCount = 1; 

	// this->compPipeLayout = device.getDevice().createPipelineLayout(compLayoutInfo);

	// auto map = star::ConfigFile::
	// //create pipeline
	// this->computePipe = std::make_unique<star::StarComputePipeline>(device, this->compPipeLayout, ); 
}