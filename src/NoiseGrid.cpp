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
 	this->commandBuffer->submit(frameInFlightIndex);
 }

void NoiseGrid::cleanupRender(star::StarDevice& device){
	this->star::Grid::cleanupRender(device); 

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

void NoiseGrid::createComputeDependencies(star::StarDevice& device)
{
	auto workGrpSquare = this->star::Grid::getSizeX() / 32; 

	this->commandBuffer = std::make_unique<star::StarCommandBuffer>(device, this->swapChainImages, star::Command_Buffer_Type::Tcompute); 

	//create compute pipeline layout
	auto descriptorSetLayout = star::StarDescriptorSetLayout::Builder(device)
		.addBinding(0, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute)
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
	compLayoutInfo.setLayoutCount = 1; 
	compLayoutInfo.pPushConstantRanges = &pushConstant; 
	compLayoutInfo.pushConstantRangeCount = 1; 

	this->compPipeLayout = device.getDevice().createPipelineLayout(compLayoutInfo);

	auto compPath = star::ConfigFile::getSetting(star::Config_Settings::mediadirectory) + "/shaders/noise.comp"; 
	//create pipeline
	this->compPipe = std::make_unique<star::StarComputePipeline>(device, this->compPipeLayout, star::StarShader(compPath, star::Shader_Stage::compute)); 
	this->compPipe->init(); 

	//record command buffer
	for (int i = 0; i < this->swapChainImages; i++){
		this->commandBuffer->begin(i); 
		vk::CommandBuffer buffer = this->commandBuffer->buffer(i);
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

		auto format = vk::Format::eR8G8B8A8Snorm;
		auto texInfo = vk::DescriptorImageInfo{
			VK_NULL_HANDLE,
			this->displacementTexture->getImageView(), 
			vk::ImageLayout::eGeneral
		};

		//bind image
		vk::DescriptorSet set;

		auto builder = star::StarDescriptorWriter(device, *descriptorSetLayout, star::ManagerDescriptorPool::getPool())
			.writeImage(0, texInfo)
			.build(set);
		this->descriptorSets.push_back(set);

		auto sets = std::vector{ this->descriptorSets.at(i) };

		buffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, this->compPipeLayout, 0, sets.size(), sets.data(), 0, nullptr);

		buffer.dispatch(workGrpSquare, workGrpSquare, 1);

		buffer.end(); 
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
