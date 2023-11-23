#pragma once 

#include "BasicObject.hpp"
#include "StarEngine.hpp"
#include "StarApplication.hpp"
#include "ConfigFile.hpp"
#include "Time.hpp"
#include "Interactivity.hpp"
#include "DebugHelpers.hpp"
#include "ShaderManager.hpp"
#include "TextureManager.hpp"
#include "LightManager.hpp"
#include "NoiseGrid.hpp"
#include "KeyStates.hpp"
#include "Key.hpp"
#include "BasicObject.hpp"
#include "Square.hpp"
#include "SimplexNoiseComputeRenderer.hpp"

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string> 
#include <memory> 


class Application :
    public star::StarApplication
{
public:
    Application(star::StarScene& scene);

    void Load();

    virtual std::string getApplicationName() { return "Compute Displacement Grid"; }

    void onKeyPress(int key, int scancode, int mods) override;

    std::unique_ptr<star::SwapChainRenderer> getMainRenderer(star::StarDevice& device, star::StarWindow& window, star::RenderOptions& options) override; 

protected:

private:
    //const double 
    const float acceleration = 0.00015f; 
    const int gridBaseHeight = 100, gridBaseWidth = 100;
    const int sunSpeed = 50;
    const float spotSpeed = 2;
    double scaleAmt = 0.1;

    star::Light* sun = nullptr;
    star::Light* spot = nullptr;

    NoiseGrid* grid = nullptr; 
    star::Time time; 

    bool targetX = false; 
    bool targetY = false; 
    bool targetP = false; 

    static int disabledLightCounter;
    static bool upCounter;
    static bool rotatingCounterClock;
    static std::chrono::steady_clock::time_point timeSinceLastUpdate;

    // Inherited via StarApplication
    void onKeyRelease(int key, int scancode, int mods) override;
    void onMouseMovement(double xpos, double ypos) override;
    void onMouseButtonAction(int button, int action, int mods) override;
    void onScroll(double xoffset, double yoffset) override;
    void onWorldUpdate() override;
};
