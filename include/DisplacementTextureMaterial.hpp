#pragma once 

#include "StarMaterial.hpp"

class DisplacementTextureMaterial : star::StarMaterial {
public:
	
	void getDescriptorSetLayout(star::StarDescriptorSetLayout::Builder& newLayout) override;

	void prep(star::StarDevice& device) override;

	vk::DescriptorSet buildDescriptorSet(star::StarDevice& device, star::StarDescriptorSetLayout& groupLayout, star::StarDescriptorPool& groupPool) override;

	void cleanup(star::StarDevice& device) override;

protected:

};