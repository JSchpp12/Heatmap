#include "Application.hpp"

using namespace star; 

int Application::disabledLightCounter = int(0);
bool Application::upCounter = true;
bool Application::rotatingCounterClock = true;

#include "Grid.hpp"

std::chrono::steady_clock::time_point Application::timeSinceLastUpdate = std::chrono::steady_clock::now();

Application::Application(star::StarScene& scene) : StarApplication(scene)
{
    this->camera.setPosition(glm::vec3{ 0.0f, 0.3f, 3.0f });
    auto camPosition = this->camera.getPosition();
    this->camera.setLookDirection(-camPosition);

    auto mediaDirectoryPath = star::ConfigFile::getSetting(star::Config_Settings::mediadirectory);

    //load plant
    //{
    //    auto plantPath = mediaDirectoryPath + "models/aloevera/aloevera.obj";

    //    auto plant = BasicObject::New(plantPath);
    //    plant->setPosition(glm::vec3(0.5f, 0.0, 0.0));
    //    this->scene.add(std::move(plant));
    //}

    ////load lion
    //{
    //    auto lionPath = mediaDirectoryPath + "models/lion-statue/source/rapid.obj";
    //    auto materialsPath = mediaDirectoryPath + "models/lion-statue/source";

    //    auto lion = BasicObject::New(lionPath);
    //    lion->setScale(glm::vec3{ 0.04f, 0.04f, 0.04f });
    //    lion->setPosition(glm::vec3{ -0.5f , 0.0f, 0.0f });
    //    lion->rotateGlobal(star::Type::Axis::x, -90);
    //    lion->moveRelative(glm::vec3{ 0.0, -1.0, 0.0 });
    //    this->scene.add(std::move(lion));
    //}

    this->scene.add(NoiseGrid::New(4)); 
    
    this->scene.add(std::unique_ptr<star::Light>(new Light(star::Type::Light::directional, glm::vec3{ 10,10,10 })));
}

void Application::Load()
{
}

void Application::Update()
{

}

void Application::onKeyPress(int key, int scancode, int mods)
{

}

std::unique_ptr<star::SwapChainRenderer> Application::getMainRenderer(StarDevice& device, StarWindow& window, RenderOptions& options)
{
    std::vector<std::unique_ptr<Light>>& lightList = scene.getLights();
    std::vector<std::reference_wrapper<StarObject>> prepObjects;
    for (auto& obj : scene.getObjects()) {
        prepObjects.push_back(*obj.second);
    }

    std::string computePath = std::string(star::ConfigFile::getSetting(star::Config_Settings::mediadirectory) + "/shaders/grid.comp");
    return std::unique_ptr<SimplexNoiseComputeRenderer>(new SimplexNoiseComputeRenderer(this->gridBaseHeight, 
        this->gridBaseWidth, window, lightList, prepObjects, camera, options, device));
}

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
}
