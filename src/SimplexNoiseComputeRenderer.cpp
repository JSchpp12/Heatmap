#include "SimplexNoiseComputeRenderer.hpp"

SimplexNoiseComputeRenderer::SimplexNoiseComputeRenderer(star::StarWindow& window,
	std::vector<std::unique_ptr<star::Light>>& lightList, std::vector<std::reference_wrapper<star::StarObject>> objectList, 
	star::StarCamera& camera, star::RenderOptions& renderOptions, star::StarDevice& device) 
	: star::BasicRenderer(window, lightList, objectList, camera, renderOptions, device) { }


