#include "NoiseGrid.hpp"

NoiseGrid::~NoiseGrid(){}

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

// void NoiseGrid::prepDraw()
// {
// 	this->mainDrawCommands->waitFor(this->commandBuffer->getCompleteSemaphores(), vk::PipelineStageFlagBits::eVertexShader); 

// 	this->commandBuffer->submit(swapChainTarget); 
// }

void NoiseGrid::cleanupRender(star::StarDevice& device){
	this->star::Grid::cleanupRender(device); 

	device.getDevice().destroyPipelineLayout(this->compPipeLayout); 
	this->compPipe.release(); 
}

void NoiseGrid::prepRender(star::StarDevice& device, vk::Extent2D swapChainExtent,
	vk::PipelineLayout pipelineLayout, vk::RenderPass renderPass, int numSwapChainImages, 
	star::StarDescriptorSetLayout& groupLayout, star::StarDescriptorPool& groupPool, std::vector<std::vector<vk::DescriptorSet>> globalSets)
{
	this->star::Grid::prepRender(device, swapChainExtent, pipelineLayout, renderPass, numSwapChainImages, groupLayout, groupPool, globalSets); 

	this->swapChainImages = numSwapChainImages; 
	//this will also create a compute pipeline
	createComputeDependencies(device); 
}

void NoiseGrid::prepRender(star::StarDevice& device, int numSwapChainImages, 
	star::StarDescriptorSetLayout& groupLayout, star::StarDescriptorPool& groupPool, std::vector<std::vector<vk::DescriptorSet>> globalSets, star::StarPipeline& sharedPipeline)
{
	this->star::Grid::prepRender(device, numSwapChainImages, groupLayout, groupPool, globalSets, sharedPipeline);
	
	this->swapChainImages = numSwapChainImages; 
	createComputeDependencies(device); 
}

void NoiseGrid::recordCommands(star::StarCommandBuffer &commandBuffer, vk::PipelineLayout &pipelineLayout, int swapChainIndexNum, uint32_t vb_start, uint32_t ib_start)
{
	this->star::StarObject::recordCommands(commandBuffer, pipelineLayout, swapChainIndexNum, vb_start, ib_start); 

	this->mainDrawCommands = &commandBuffer; 
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

	auto setLayout = descriptorSetLayout->getDescriptorSetLayout();
	// compute pipe needs to know what kind of descriptors it will be recieving
	vk::PipelineLayoutCreateInfo compLayoutInfo{}; 
	compLayoutInfo.sType = vk::StructureType::ePipelineLayoutCreateInfo; 
	compLayoutInfo.pSetLayouts = &setLayout;
	compLayoutInfo.pPushConstantRanges = &pushConstant; 
	compLayoutInfo.pushConstantRangeCount = 1; 

	this->compPipeLayout = device.getDevice().createPipelineLayout(compLayoutInfo);

	auto compPath = star::ConfigFile::getSetting(star::Config_Settings::mediadirectory) + "/shaders/noise.comp"; 
	//create pipeline
	this->compPipe = std::make_unique<star::StarComputePipeline>(device, this->compPipeLayout, star::StarShader(compPath, star::Shader_Stage::compute)); 

	//record command buffer
	this->commandBuffer = std::make_unique<star::StarCommandBuffer>(device, this->swapChainImages, star::Command_Buffer_Type::Tcompute); 
	for (int i = 0; i < this->swapChainImages; i++){
		this->commandBuffer->begin(i); 
		vk::CommandBuffer buffer = this->commandBuffer->buffer(i);
		this->compPipe->bind(buffer);
		
		buffer.end(); 
	}
}