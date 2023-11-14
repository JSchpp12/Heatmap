#include "DisplacementTextureMaterial.hpp"

void DisplacementTextureMaterial::getDescriptorSetLayout(star::StarDescriptorSetLayout::Builder& newLayout)
{
}

void DisplacementTextureMaterial::prep(star::StarDevice& device)
{
}

vk::DescriptorSet DisplacementTextureMaterial::buildDescriptorSet(star::StarDevice& device, star::StarDescriptorSetLayout& groupLayout, star::StarDescriptorPool& groupPool)
{
	return vk::DescriptorSet();
}

void DisplacementTextureMaterial::cleanup(star::StarDevice& device)
{
}