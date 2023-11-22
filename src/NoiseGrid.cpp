#include "NoiseGrid.hpp"

NoiseGrid::~NoiseGrid(){}

std::unique_ptr<NoiseGrid> NoiseGrid::New(int sizeScale)
{
	int vertX = 32 * sizeScale; 
	int vertY = 32 * sizeScale; 

	auto settings = star::StarTexture::TextureCreateSettings(
		vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled, 
		vk::Format::eR8G8B8A8Snorm
	); 

	auto texture = std::shared_ptr<star::Texture>(new star::Texture(vertX, vertY, settings));
	auto material = std::shared_ptr<star::TextureMaterial>(new star::TextureMaterial(texture)); 
	return std::unique_ptr<NoiseGrid>(new NoiseGrid(vertX, vertY, texture, material));
}

void NoiseGrid::initRender(int numFramesInFlight)
{
	this->star::Grid::initRender(numFramesInFlight); 

	star::ManagerDescriptorPool::request(vk::DescriptorType::eCombinedImageSampler, numFramesInFlight); 
}

 void NoiseGrid::prepDraw(int frameInFlightIndex)
 {
	 this->commandBuffer->reset(frameInFlightIndex); 

	 recordComputeCommands(*this->commandBuffer, frameInFlightIndex);

	 //call for compute
	 this->commandBuffer->submit(frameInFlightIndex);

	 //flip noiseCompValues for next draw if needed
	 int nextIndex = frameInFlightIndex + 1 % this->swapChainImages; 
	 this->noiseComputeValues = &this->compInfos.at(nextIndex);
 }

void NoiseGrid::cleanupRender(star::StarDevice& device){
	this->star::Grid::cleanupRender(device); 
	this->descriptorLayout.reset(); 
	this->commandBuffer.reset(); 
	device.getDevice().destroyPipelineLayout(this->compPipeLayout); 
	this->compPipe.reset(); 
}

void NoiseGrid::prepRender(star::StarDevice& device, vk::Extent2D swapChainExtent,
	vk::PipelineLayout pipelineLayout, vk::RenderPass renderPass, int numSwapChainImages, 
	star::StarDescriptorSetLayout& groupLayout, star::StarDescriptorPool& groupPool, std::vector<std::vector<vk::DescriptorSet>> globalSets)
{
	this->star::Grid::prepRender(device, swapChainExtent, pipelineLayout, renderPass, numSwapChainImages, groupLayout, groupPool, globalSets); 
	this->swapChainImages = numSwapChainImages; 
	this->device = &device; 
	//this will also create a compute pipeline
	createComputeDependencies(); 
}

void NoiseGrid::prepRender(star::StarDevice& device, int numSwapChainImages, 
	star::StarDescriptorSetLayout& groupLayout, star::StarDescriptorPool& groupPool, std::vector<std::vector<vk::DescriptorSet>> globalSets, star::StarPipeline& sharedPipeline)
{
	this->star::Grid::prepRender(device, numSwapChainImages, groupLayout, groupPool, globalSets, sharedPipeline);
	this->swapChainImages = numSwapChainImages; 
	this->device = &device;
	createComputeDependencies(); 
}

void NoiseGrid::recordPreRenderPassCommands(star::StarCommandBuffer& commandBuffer, int swapChainIndexNum)
{
	auto& buffer = commandBuffer.buffer(swapChainIndexNum); 

	//image will be in wrong layout when it arrives. need to transition the image
	vk::ImageMemoryBarrier barrier{};
	barrier.sType = vk::StructureType::eImageMemoryBarrier;
	barrier.oldLayout = vk::ImageLayout::eGeneral;
	barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	barrier.srcAccessMask = vk::AccessFlagBits::eNone;
	barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
	barrier.image = this->displacementTexture->getImage();
	barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	barrier.subresourceRange.baseMipLevel = 0;                          //image does not have any mipmap levels
	barrier.subresourceRange.levelCount = 1;                            //image is not an array
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	buffer.pipelineBarrier(
		vk::PipelineStageFlagBits::eTopOfPipe,
		vk::PipelineStageFlagBits::eVertexShader,
		{},
		{},
		nullptr,
		barrier
	);

	if (!toldMainRenderToWait) {
		commandBuffer.waitFor(this->commandBuffer->getCompleteSemaphores(), vk::PipelineStageFlagBits::eVertexShader);
		toldMainRenderToWait = true;
	}
}

void NoiseGrid::recordRenderPassCommands(star::StarCommandBuffer &commandBuffer, vk::PipelineLayout &pipelineLayout, int swapChainIndexNum, uint32_t vb_start, uint32_t ib_start)
{
	this->star::StarObject::recordRenderPassCommands(commandBuffer, pipelineLayout, swapChainIndexNum, vb_start, ib_start);
}

NoiseGrid::NoiseGrid(int vertX, int vertY, std::shared_ptr<star::Texture> texture, std::shared_ptr<star::TextureMaterial> textureMaterial)
	: Grid(vertX, vertY, textureMaterial), displacementTexture(texture) { }

void NoiseGrid::createComputeDependencies()
{
	this->compInfos.resize(this->swapChainImages); 
	this->noiseComputeValues = &this->compInfos.at(0); 

	this->commandBuffer = std::make_unique<star::StarCommandBuffer>(*this->device, this->swapChainImages, star::Command_Buffer_Type::Tcompute); 

	//create compute pipeline layout
	this->descriptorLayout = star::StarDescriptorSetLayout::Builder(*this->device)
		.addBinding(0, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute)
		.build(); 
	
	//using push constants 
	vk::PushConstantRange pushConstant{}; 
	pushConstant.offset = 0; 
	pushConstant.size = sizeof(ComputeInfo); 
	pushConstant.stageFlags = vk::ShaderStageFlagBits::eCompute; 

	auto setLayout = this->descriptorLayout->getDescriptorSetLayout();
	// compute pipe needs to know what kind of descriptors it will be recieving
	vk::PipelineLayoutCreateInfo compLayoutInfo{}; 
	compLayoutInfo.sType = vk::StructureType::ePipelineLayoutCreateInfo; 
	compLayoutInfo.pSetLayouts = &setLayout;
	compLayoutInfo.setLayoutCount = 1; 
	compLayoutInfo.pPushConstantRanges = &pushConstant; 
	compLayoutInfo.pushConstantRangeCount = 1; 

	this->compPipeLayout = this->device->getDevice().createPipelineLayout(compLayoutInfo);

	auto compPath = star::ConfigFile::getSetting(star::Config_Settings::mediadirectory) + "/shaders/noise.comp"; 
	//create pipeline
	this->compPipe = std::make_unique<star::StarComputePipeline>(*this->device, this->compPipeLayout, star::StarShader(compPath, star::Shader_Stage::compute)); 
	this->compPipe->init(); 

	//record command buffer
	for (int i = 0; i < this->swapChainImages; i++){
		auto texInfo = vk::DescriptorImageInfo{
			VK_NULL_HANDLE,
			this->displacementTexture->getImageView(),
			vk::ImageLayout::eGeneral
		};

		vk::DescriptorSet set;
		//bind image
		auto builder = star::StarDescriptorWriter(*this->device, *this->descriptorLayout, star::ManagerDescriptorPool::getPool())
			.writeImage(0, texInfo)
			.build(set);
		this->descriptorSets.push_back(set);

		recordComputeCommands(*this->commandBuffer, i); 
	}
}

std::unordered_map<star::Shader_Stage, star::StarShader> NoiseGrid::getShaders()
{
	//default shader is 
	auto shaders = std::unordered_map<star::Shader_Stage, star::StarShader>();

	//load vertex shader
	std::string vertShaderPath = star::ConfigFile::getSetting(star::Config_Settings::mediadirectory) + "/shaders/default.vert";
	shaders.insert(std::pair<star::Shader_Stage, star::StarShader>(star::Shader_Stage::vertex, star::StarShader(vertShaderPath, star::Shader_Stage::vertex)));

	//load fragment shader
	std::string fragShaderPath = star::ConfigFile::getSetting(star::Config_Settings::mediadirectory) + "/shaders/textureColor.frag";
	shaders.insert(std::pair<star::Shader_Stage, star::StarShader>(star::Shader_Stage::fragment, star::StarShader(fragShaderPath, star::Shader_Stage::fragment)));

	return shaders;
}

void NoiseGrid::prepareCompImageForMain(star::StarCommandBuffer& commandBuffer, int imageInFlight)
{
	//only need to transition image if we have to
	if (this->displacementTexture->getCurrentLayout() != vk::ImageLayout::eShaderReadOnlyOptimal) {
		commandBuffer.transitionImageLayout(imageInFlight, *this->displacementTexture, vk::ImageLayout::eShaderReadOnlyOptimal,
			vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead, 
			vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eVertexShader); 
	}
}

void NoiseGrid::recordComputeCommands(star::StarCommandBuffer& commandBuffer, int imageInFlight)
{
	auto workGrpSquare = this->star::Grid::getSizeX() / 32;

	this->commandBuffer->begin(imageInFlight);
	vk::CommandBuffer buffer = this->commandBuffer->buffer(imageInFlight);
	this->compPipe->bind(buffer);

	//prepare image for compute shader
	{
		vk::ImageMemoryBarrier barrier{};
		barrier.sType = vk::StructureType::eImageMemoryBarrier;
		barrier.oldLayout = vk::ImageLayout::eUndefined;
		barrier.newLayout = vk::ImageLayout::eGeneral;
		barrier.srcAccessMask = vk::AccessFlagBits::eNone;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderWrite;
		barrier.image = this->displacementTexture->getImage();
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		barrier.subresourceRange.baseMipLevel = 0;                          //image does not have any mipmap levels
		barrier.subresourceRange.levelCount = 1;                            //image is not an array
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		buffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eTopOfPipe,
			vk::PipelineStageFlagBits::eComputeShader,
			{},
			{},
			nullptr,
			barrier
		);
	}

	buffer.pushConstants(compPipeLayout, vk::ShaderStageFlagBits::eCompute, 0, sizeof(ComputeInfo), &compInfos.at(imageInFlight));

	auto sets = std::vector{ this->descriptorSets.at(imageInFlight) };

	buffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, this->compPipeLayout, 0, sets.size(), sets.data(), 0, nullptr);

	buffer.dispatch(workGrpSquare, workGrpSquare, 1);

	buffer.end();
}
