#pragma once 

#include "BasicRenderer.hpp"

/// <summary>
/// Creates a renderer which includes a computation shader to 
/// calcaulte perlin noise. 
/// </summary>
class PerlinNoiseComputeRenderer : public star::BasicRenderer {
public:
	PerlinNoiseComputeRenderer(star::StarWindow& window, std::vector<std::unique_ptr<star::Light>>& lightList,
		std::vector<std::reference_wrapper<star::StarObject>> objectList, star::StarCamera& camera, star::RenderOptions& renderOptions, star::StarDevice& device);


protected:

};