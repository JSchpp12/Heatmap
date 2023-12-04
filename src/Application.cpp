#include "Application.hpp"

using namespace star; 

int Application::disabledLightCounter = int(0);
bool Application::upCounter = true;
bool Application::rotatingCounterClock = true;

#include "Grid.hpp"

std::chrono::steady_clock::time_point Application::timeSinceLastUpdate = std::chrono::steady_clock::now();

Application::Application(star::StarScene& scene) : StarApplication(scene)
{

    std::cout << "Controls:" << std::endl; 
    std::cout << "P - Set modification to permute value of the noise function" << std::endl; 
    std::cout << "X - Set modification to X dimension on the noise function" << std::endl; 
    std::cout << "Y - Set modification to Y dimension on the noise function" << std::endl; 
    std::cout << "UP Arrow - Increase targeted modification value" << std::endl; 
    std::cout << "DOWN Arrow - Decrease targeted modification value" << std::endl; 

    this->camera.setPosition(glm::vec3{ 2.0f, 1.0f, 1.0f });
    auto camPosition = this->camera.getPosition();
    this->camera.setLookDirection(-camPosition);

    auto mediaDirectoryPath = star::ConfigFile::getSetting(star::Config_Settings::mediadirectory);

    auto gridRef = this->scene.add(NoiseGrid::New(100)); 
    this->grid = static_cast<NoiseGrid*>(&this->scene.getObject(gridRef)); 
    this->grid->setScale(glm::vec3{2.0, 2.0, 2.0});
    this->grid->moveRelative(glm::vec3{ -1.0f, 0.0, -1.0f });
    
    this->scene.add(std::unique_ptr<star::Light>(new Light(star::Type::Light::directional, glm::vec3{ 10,10,10 })));
}

void Application::Load()
{
}

void Application::onKeyPress(int key, int scancode, int mods)
{

}

//std::unique_ptr<star::SwapChainRenderer> Application::getMainRenderer(StarDevice& device, StarWindow& window, RenderOptions& options)
//{
//    std::vector<std::unique_ptr<Light>>& lightList = scene.getLights();
//    std::vector<std::reference_wrapper<StarObject>> prepObjects;
//    for (auto& obj : scene.getObjects()) {
//        prepObjects.push_back(*obj.second);
//    }
//
//    std::string computePath = std::string(star::ConfigFile::getSetting(star::Config_Settings::mediadirectory) + "/shaders/grid.comp");
//    return std::unique_ptr<SimplexNoiseComputeRenderer>(new SimplexNoiseComputeRenderer(this->gridBaseHeight, 
//        this->gridBaseWidth, window, lightList, prepObjects, camera, options, device));
//}

void Application::onKeyRelease(int key, int scancode, int mods)
{
}

void Application::onMouseMovement(double xpos, double ypos)
{
}

void Application::onMouseButtonAction(int button, int action, int mods)
{
}

void Application::onScroll(double xoffset, double yoffset)
{
}

void Application::onWorldUpdate()
{
    double timeChange = time.timeElapsedLastFrameSeconds(); 
    int valueChange = 0;

    if (star::KeyStates::state(star::KEY::X)) {
        std::cout << "Updating X of noise function resolution" << std::endl;
        this->targetX = true;
        this->targetY = false;
        this->targetP = false; 
    }
    else if (star::KeyStates::state(star::KEY::Y)) {
        std::cout << "Updating Y of noise function resolution" << std::endl;
        this->targetX = false;
        this->targetY = true;
        this->targetP = false; 
    }
    else if (star::KeyStates::state(star::KEY::P)) {
        std::cout << "Permute" << std::endl; 
        this->targetX = false; 
        this->targetY = false; 
        this->targetP = true; 
    }

    if (star::KeyStates::state(star::KEY::UP)) {
        valueChange = 1;
    }
    else if (star::KeyStates::state(star::KEY::DOWN)) {
        valueChange = -1;
    }

    if (this->targetX) {
        if (valueChange != 0)
            std::cout << this->grid->noiseComputeValues->noiseImageResolution.x << std::endl;

        this->grid->noiseComputeValues->noiseImageResolution.x += (valueChange) ;
 
    }
    else if (this->targetY) {
        if (valueChange != 0)
            std::cout << this->grid->noiseComputeValues->noiseImageResolution.y << std::endl;

        this->grid->noiseComputeValues->noiseImageResolution.y += (valueChange);
    }
    else if (this->targetP) {
        if (valueChange != 0)
            std::cout << this->grid->noiseComputeValues->permute << std::endl;

        this->grid->noiseComputeValues->permute += (valueChange) * (acceleration * timeChange); 
    }

    time.updateLastFrameTime(); 
}
