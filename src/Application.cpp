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
    {
        auto plantPath = mediaDirectoryPath + "models/aloevera/aloevera.obj";

        auto plant = BasicObject::New(plantPath);
        plant->setPosition(glm::vec3(0.5f, 0.0, 0.0)); 
        this->scene.add(std::move(plant));
    }

    //load lion
    {
        auto lionPath = mediaDirectoryPath + "models/lion-statue/source/rapid.obj";
        auto materialsPath = mediaDirectoryPath + "models/lion-statue/source";

        auto lion = BasicObject::New(lionPath);
        lion->setScale(glm::vec3{ 0.04f, 0.04f, 0.04f });
        lion->setPosition(glm::vec3{-0.5f , 0.0f, 0.0f });
        lion->rotateGlobal(star::Type::Axis::x, -90);
        lion->moveRelative(glm::vec3{ 0.0, -1.0, 0.0 });
        this->scene.add(std::move(lion));
    }

    //this->scene.add(std::make_unique<Grid>(10, 10)); 
    this->scene.add(std::make_unique<star::Light>(star::Type::Light::directional, glm::vec3{ 10, 10, 10 }));

    std::cout << "App Controls" << std::endl;
    std::cout << "Press M to switch lights off and on in order" << std::endl;
    std::cout << "Right Arrow: rotate sun light clockwise" << std::endl;
    std::cout << "Left Arrow: rotate sun light counter clockwise" << std::endl;
    std::cout << "Up Arrow: increase diameter of spot light" << std::endl;
    std::cout << "Down Arrow: decrease diameter of spot light" << std::endl;
    std::cout << "U: Increase outer diam of spot light" << std::endl;
    std::cout << "Y: Decrease outer diam of spot light" << std::endl;
    std::cout << "J: Increase inner diam of spot light" << std::endl;
    std::cout << "H: Decrease inner diam of spot light" << std::endl;
    std::cout << "Z: Enable/Disable sun" << std::endl;
}

void Application::Load()
{
}

void Application::Update()
{
   /* auto now = std::chrono::steady_clock::now();
    float elapsedTime = std::chrono::duration<float>(now - timeSinceLastUpdate).count();
    rock->rotateRelative(star::Type::Axis::y, elapsedTime * 30);

    if (star::KeyStates::state(KEY::RIGHT)) {
        auto rot = glm::mat4(1.0f);
        float rotationAmt = (float)this->time.timeElapsedLastFrameSeconds() * sunSpeed;
        rot = glm::rotate(rot, rotationAmt, glm::vec3{ 0.0f, 0.0f, -1.0f });
        sun->direction = rot * sun->direction;
    }
    else if (star::KeyStates::state(KEY::LEFT)) {
        auto rot = glm::mat4(1.0f);
        float rotationAmt = (float)this->time.timeElapsedLastFrameSeconds() * sunSpeed;
        rot = glm::rotate(rot, rotationAmt, glm::vec3{ 0.0f, 0.0f, 1.0f });
        sun->direction = rot * sun->direction;
    }

    if (star::KeyStates::state(KEY::DOWN)) {
        spot->setOuterDiameter(spot->getOuterDiameter() + (spotSpeed * elapsedTime));
        std::cout << spot->getOuterDiameter() << std::endl;
    }
    else if (star::KeyStates::state(KEY::Y)) {
        spot->setOuterDiameter(spot->getOuterDiameter() - (spotSpeed * elapsedTime));
        std::cout << spot->getOuterDiameter() << std::endl;
    }

    if (star::KeyStates::state(KEY::J)) {
        spot->setInnerDiameter(spot->getInnerDiameter() + (spotSpeed * elapsedTime));
        std::cout << spot->getInnerDiameter() << std::endl;
    }
    else if (star::KeyStates::state(KEY::H)) {
        spot->setInnerDiameter(spot->getInnerDiameter() - (spotSpeed * elapsedTime));
        std::cout << spot->getInnerDiameter() << std::endl;
    }
    this->time.updateLastFrameTime();*/
}

void Application::onKeyPress(int key, int scancode, int mods)
{
    //if (key == GLFW_KEY_M) {
    //    auto& light = StarEngine::sceneBuilder.light(lightList.at(disabledLightCounter));
    //    light.setEnabled();
    //    if (!upCounter && disabledLightCounter == 0) {
    //        upCounter = true;
    //    }
    //    else if (upCounter && disabledLightCounter == lightList.size() - 1) {
    //        upCounter = false;
    //    }
    //    disabledLightCounter = upCounter ? disabledLightCounter + 1 : disabledLightCounter - 1;
    //}
   

    //if (key == GLFW_KEY_Z) {
    //    sun->setEnabled();
    //}
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
