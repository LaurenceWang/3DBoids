#include "App.hpp"
#include "AppInit.hpp"
#include "glimac/sphere_vertices.hpp"

App::App(p6::Context& ctx)
    : _ObjectProgram{}, _skybox(returnVertices())
{
    init(ctx);
    _ViewMatrixCamera.moveFront(-12.f);
}

void App::cameraMovement(p6::Context& ctx)
{
    if (_Z)
        _ViewMatrixCamera.moveFront(0.1);
    if (_S)
        _ViewMatrixCamera.moveFront(-0.1);
    if (_Q)
        _ViewMatrixCamera.moveLeft(0.1);
    if (_D)
        _ViewMatrixCamera.moveLeft(-0.1);
    if (_UP)
        _ViewMatrixCamera.moveUp(0.1);
    if (_DOWN)
        _ViewMatrixCamera.moveUp(-0.1);

    ctx.key_pressed = [this](const p6::Key& key) {
        if (key.physical == GLFW_KEY_W)
            _Z = true;
        if (key.physical == GLFW_KEY_S)
            _S = true;
        if (key.physical == GLFW_KEY_A)
            _Q = true;
        if (key.physical == GLFW_KEY_D)
            _D = true;
        if (key.physical == GLFW_KEY_F)
            _UP = true;
        if (key.physical == GLFW_KEY_SPACE)
            _DOWN = true;
    };

    ctx.key_released = [this](const p6::Key& key) {
        if (key.physical == GLFW_KEY_W)
            _Z = false;
        if (key.physical == GLFW_KEY_S)
            _S = false;
        if (key.physical == GLFW_KEY_A)
            _Q = false;
        if (key.physical == GLFW_KEY_D)
            _D = false;
        if (key.physical == GLFW_KEY_F)
            _UP = false;
        if (key.physical == GLFW_KEY_SPACE)
            _DOWN = false;
    };

    ctx.mouse_dragged = [&](const p6::MouseDrag& button) {
        if (ImGui::GetIO().WantCaptureMouse)
        {
            return;
        }
        _ViewMatrixCamera.rotateLeft(-button.delta.x * 50);
        _ViewMatrixCamera.rotateUp(button.delta.y * 50);
    };
}

void App::switchProgram(std::string shaderPath)
{
    _ObjectProgram = Program(shaderPath);
    _ObjectProgram._shader.use();
}

void App::for_each_obstacle(ObstacleHandler const& handler)
{
    for (auto const& obs : _obstacles1.getObstacles())
    {
        handler(obs);
    }

    for (auto const& obs : _obstacles2.getObstacles())
    {
        handler(obs);
    }
}

void App::run(p6::Context& ctx)
{
    _boids1.runBoids(
        _parametres, [&](ObstacleHandler handler) { for_each_obstacle(handler); }, _food
    );
    _boids2.runBoids(
        _parametres, [&](ObstacleHandler handler) { for_each_obstacle(handler); }, _food
    );
    _obstacles1.runObstacles(ctx);
    _obstacles2.runObstacles(ctx);
    _arpenteur.updatePosition(_ViewMatrixCamera);
}

void App::sceneRender(p6::Context& ctx)
{
    run(ctx);

    glm::vec3 posArp{_arpenteur.getPos().x, _arpenteur.getPos().y - 0.5, _arpenteur.getPos().z - 2};

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    switchProgram("Shaders/normals.fs.glsl");
    _lumiere.finalDraw(ctx, _ViewMatrixCamera.getViewMatrix(), _lum, _arpenteur.getPos(), _lodChoice);
    switchProgram("Shaders/texLights.fs.glsl");

    lights.sendUniform(_ViewMatrixCamera.getViewMatrix(), posArp);

    _arpenteurRender.finalDraw(ctx, _ViewMatrixCamera.getViewMatrix(), objectParameters{glm::vec3(posArp.x, posArp.y, posArp.z + 1.5), 0, 1}, posArp, _lodChoice);

    for (auto& meal : _food)
        _foodRender.finalDraw(ctx, _ViewMatrixCamera.getViewMatrix(), objectParameters{glm::vec3(meal.getPos()), 0.f, 1}, posArp, _lodChoice);

    for (auto& fish : _boids1.getFishPack())
        _boid1Render.finalDraw(ctx, _ViewMatrixCamera.getViewMatrix(), objectParameters{fish.getPos(), 0.f, _parametres.fishSize}, posArp, _lodChoice);

    for (auto& fish : _boids2.getFishPack())
        _boid2Render.finalDraw(ctx, _ViewMatrixCamera.getViewMatrix(), objectParameters{fish.getPos(), 0.f, _parametres.fishSize}, posArp, _lodChoice);

    _boid3Render.finalDraw(ctx, _ViewMatrixCamera.getViewMatrix(), objectParameters{_boids2.getFishPack().at(0).getPos(), 0.f, _parametres.fishSize}, posArp, _lodChoice);

    for (auto& obs : _obstacles2.getObstacles())
        _obstacleRender.finalDraw(ctx, _ViewMatrixCamera.getViewMatrix(), objectParameters{glm::vec3(obs.getPos()), 0.f, obs.getRadius()}, _arpenteur.getPos(), _lodChoice);

    _portal.finalDraw(ctx, _ViewMatrixCamera.getViewMatrix(), _port, _arpenteur.getPos(), _lodChoice);

    _chest.finalDraw(ctx, _ViewMatrixCamera.getViewMatrix(), _ches, _arpenteur.getPos(), _lodChoice);

    _castle.finalDraw(ctx, _ViewMatrixCamera.getViewMatrix(), _cas, _arpenteur.getPos(), _lodChoice);

    for (auto& island : _isl)
        _island.finalDraw(ctx, _ViewMatrixCamera.getViewMatrix(), island, _arpenteur.getPos(), _lodChoice);

    for (auto& cryst : _crys)
        _crystal.finalDraw(ctx, _ViewMatrixCamera.getViewMatrix(), cryst, _arpenteur.getPos(), _lodChoice);

    for (auto& tree : _tr)
        _tree.finalDraw(ctx, _ViewMatrixCamera.getViewMatrix(), tree, _arpenteur.getPos(), _lodChoice);

    _skybox.renderSkybox(glm::mat4(glm::mat3(_ViewMatrixCamera.getViewMatrix())), ctx);
}

void App::sceneClean()
{
    _portal.deleteRessources();
    _boid1Render.deleteRessources();
    _boid2Render.deleteRessources();
    _foodRender.deleteRessources();
    _arpenteurRender.deleteRessources();
    _portal.deleteRessources();
    _island.deleteRessources();
    _tree.deleteRessources();
    _chest.deleteRessources();
    _crystal.deleteRessources();
    _castle.deleteRessources();
    _skybox.deleteRessources();
}
