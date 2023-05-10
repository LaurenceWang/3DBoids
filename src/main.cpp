#include <stdlib.h>
#include <cstddef>
#include "doctest/parts/doctest_fwd.h"
#include "p6/p6.h"
#define DOCTEST_CONFIG_IMPLEMENT
#include <random>
#include "Boids.hpp"
#include "Food.hpp"
#include "Model.hpp"
#include "Object.hpp"
#include "ObstacleCollection.hpp"
#include "Params.hpp"
#include "Program.hpp"
#include "doctest/doctest.h"
#include "glimac/FreeflyCamera.hpp"
#include "glimac/cone_vertices.hpp"
#include "glimac/sphere_vertices.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/geometric.hpp"
#include "glm/gtc/random.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.hpp"

using ObstacleHandler = std::function<void(Obstacle const&)>;

int main(int argc, char* argv[])
{
    { // Run the tests
        if (doctest::Context{}.run() != 0)
            return EXIT_FAILURE;
        // The CI does not have a GPU so it cannot run the rest of the code.
        const bool no_gpu_available = argc >= 2 && strcmp(argv[1], "-nogpu") == 0; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (no_gpu_available)
            return EXIT_SUCCESS;
    }

    // Actual app
    auto ctx = p6::Context{{.title = "Swimming with boids"}};

    const std::vector<glimac::ShapeVertex> vertices  = glimac::sphere_vertices(0.2f, 32, 16);
    const std::vector<glimac::ShapeVertex> vertices2 = glimac::cone_vertices(0.2f, 0.2f, 32, 16);

    /********************PROGRAMS***********************/

    Program lightAndText{};
    Program Objects("Shaders/multiTex3D.fs.glsl");
    Program light("Shaders/pointLight.fs.glsl");

    GLint uFishTexture = glGetUniformLocation(Objects._Program.id(), "uFishTexture");
    GLint uMoonTexture = glGetUniformLocation(Objects._Program.id(), "uMoonTexture");

    /********************TEXTURE LOADING***********************/
    glEnable(GL_DEPTH_TEST);

    ///////////////fish
    GLuint     fishTextureID = 0;
    const auto textureFish   = p6::load_image_buffer("Assets/textures/fish.jpg");

    glGenTextures(1, &fishTextureID);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fishTextureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureFish.width(), textureFish.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, textureFish.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Unbind the texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    ///////////////////moon
    GLuint moonTextureID = 0;

    const auto textureMoon = p6::load_image_buffer("Assets/textures/MoonMap.jpg");

    glGenTextures(1, &moonTextureID);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, moonTextureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureMoon.width(), textureMoon.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, textureMoon.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);

    //////////////////earth

    GLuint earthTextureID = 0;

    const auto textureEarth = p6::load_image_buffer("Assets/textures/EarthMap.jpg");

    glGenTextures(1, &earthTextureID);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, earthTextureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureEarth.width(), textureEarth.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, textureEarth.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    /*****************************MODEL LOADING****************************/

    Model fishV("Assets/models/fish/model_371254902470.obj");

    /*****OBJECT CREATION******/

    Object test(Objects, fishV);

    /***********************************VBO & VAOS*************************************/

    //////////////VBO
    GLuint vbos[3];
    glGenBuffers(3, vbos);

    // sphere
    glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glimac::ShapeVertex), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // fish
    glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
    glBufferData(GL_ARRAY_BUFFER, fishV.getVertextSize() * sizeof(glimac::ShapeVertex), fishV.getVertextData(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // cone
    glBindBuffer(GL_ARRAY_BUFFER, vbos[2]);
    glBufferData(GL_ARRAY_BUFFER, vertices2.size() * sizeof(glimac::ShapeVertex), vertices2.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    ///////////////VAO
    GLuint vaos[3];
    glGenVertexArrays(3, vaos);

    // sphere
    glBindVertexArray(vaos[0]);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glimac::ShapeVertex), (const GLvoid*)(offsetof(glimac::ShapeVertex, position)));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glimac::ShapeVertex), (const GLvoid*)(offsetof(glimac::ShapeVertex, normal)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glimac::ShapeVertex), (const GLvoid*)(offsetof(glimac::ShapeVertex, texCoords)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // fish
    glBindVertexArray(vaos[1]);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glimac::ShapeVertex), (const GLvoid*)(offsetof(glimac::ShapeVertex, position)));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glimac::ShapeVertex), (const GLvoid*)(offsetof(glimac::ShapeVertex, normal)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glimac::ShapeVertex), (const GLvoid*)(offsetof(glimac::ShapeVertex, texCoords)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // cone

    glBindVertexArray(vaos[2]);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, vbos[2]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glimac::ShapeVertex), (const GLvoid*)(offsetof(glimac::ShapeVertex, position)));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glimac::ShapeVertex), (const GLvoid*)(offsetof(glimac::ShapeVertex, normal)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glimac::ShapeVertex), (const GLvoid*)(offsetof(glimac::ShapeVertex, texCoords)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    /********************BOIDS & CO INIT********************/

    // ctx.maximize_window();
    int    fishNb = 100;
    Params p{.separation = 0.070f, .alignment = 0.040f, .steer = 6.f, .neighRadius = 0.5f, .fishSize = 0.02f};
    // bool   nbChanged   = false;
    // bool   sizeChanged = false;

    Boids boids(fishNb, 0);
    // boids.generateFish(fishNb, 0);

    int   fishNb2 = 20;
    Boids boids2(fishNb2, 1);
    // boids.generateFish(fishNb2, 1);

    imGuiInit(&ctx, p, fishNb, boids);

    imGuiInit(&ctx, p, fishNb, boids);

    ObstacleCollection obstacle;

    obstacle.generateObstacles(3);
    obstacle.generateBorders(ctx);

    ObstacleCollection obstacle2;

    obstacle2.generateObstacles(3);

    Food              seaweed(glm::vec3(3, 1, 4), 0.5);
    Food              seaweed2(glm::vec3(-3, 1, 0), 0.5);
    std::vector<Food> meals;
    meals.push_back(seaweed);
    meals.push_back(seaweed2);

    FreeflyCamera ViewMatrixCamera = FreeflyCamera();

    bool Z = false;
    bool Q = false;
    bool S = false;
    bool D = false;

    /********kd & ks tests*******/
    // glm::vec3 Kd2 = glm::vec3(glm::linearRand(0.f, 0.5f), glm::linearRand(0.f, 0.5f), glm::linearRand(0.f, 0.5f));
    // glm::vec3 Ks2 = glm::vec3(glm::linearRand(0.f, 1.f), glm::linearRand(0.f, 1.f), glm::linearRand(0.f, 1.f));
    //   Declare your infinite update loop

    ctx.update = [&]() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /***********************CAMERA MOUSE************************/

        ctx.background({0.33, 0.8, 0.98});

        if (Z)
        {
            ViewMatrixCamera.moveFront(0.1);
        }
        if (Q)
        {
            ViewMatrixCamera.moveLeft(0.1);
            // ViewMatrixCamera.rotateLeft(0.1);
        }
        if (S)
        {
            ViewMatrixCamera.moveFront(-0.1);
        }
        if (D)
        {
            // ViewMatrixCamera.rotateLeft(-0.1);
            ViewMatrixCamera.moveLeft(-0.1);
        }

        ctx.key_pressed = [&Z, &Q, &S, &D](const p6::Key& key) {
            if (key.physical == GLFW_KEY_W)
            {
                Z = true;
            }
            if (key.physical == GLFW_KEY_A)
            {
                Q = true;
            }
            if (key.physical == GLFW_KEY_S)
            {
                S = true;
            }
            if (key.physical == GLFW_KEY_D)
            {
                D = true;
            }
        };

        ctx.key_released = [&Z, &Q, &S, &D](const p6::Key& key) {
            if (key.physical == GLFW_KEY_W)
            {
                Z = false;
            }
            if (key.physical == GLFW_KEY_A)
            {
                Q = false;
            }
            if (key.physical == GLFW_KEY_S)
            {
                S = false;
            }
            if (key.physical == GLFW_KEY_D)
            {
                D = false;
            }
        };

        ctx.mouse_dragged = [&ViewMatrixCamera](const p6::MouseDrag& button) {
            ViewMatrixCamera.rotateLeft(-button.delta.x * 50);
            ViewMatrixCamera.rotateUp(button.delta.y * 50);
        };

        /*********************BOIDS & CO MATRIX*********************/

        // seaweed.draw(ctx);
        // seaweed2.draw(ctx);

        const auto for_each_obstacle = [&](ObstacleHandler const& handler) {
            for (auto const& obs : obstacle.getObstacles())
            {
                handler(obs);
            }

            for (auto const& obs : obstacle2.getObstacles())
            {
                handler(obs);
            }
        };
        ;
        boids.runBoids(p, ctx, for_each_obstacle, meals);
        obstacle.runObstacles(ctx);
        obstacle2.runObstacles(ctx);

        Objects._Program.use();

        glUniform1i(uFishTexture, 0);
        glUniform1i(uMoonTexture, 1);

        glBindVertexArray(vaos[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, earthTextureID);

        glm::mat4 ProjMatrix = glm::perspective(glm::radians(70.f), ctx.aspect_ratio(), 0.1f, 100.f);

        /*************************** FOOD *******************/

        glm::mat4 Food1MVMatrix = ViewMatrixCamera.getViewMatrix();
        Food1MVMatrix           = glm::translate(ViewMatrixCamera.getViewMatrix(), glm::vec3(seaweed.getPos().x, seaweed.getPos().y, -5));
        Food1MVMatrix           = glm::scale(
                      Food1MVMatrix,
                      glm::vec3(2, 2, 2)
                  );
        glm::mat4 Food1NormalMatrix = glm::transpose(glm::inverse(Food1MVMatrix));
        glUniformMatrix4fv(Objects.uMVMatrix, 1, GL_FALSE, glm::value_ptr(Food1MVMatrix));
        glUniformMatrix4fv(Objects.uMVPMatrix, 1, GL_FALSE, glm::value_ptr(ProjMatrix * Food1MVMatrix));
        glUniformMatrix4fv(Objects.uNormalMatrix, 1, GL_FALSE, glm::value_ptr(Food1NormalMatrix));
        glDrawArrays(GL_TRIANGLES, 0, vertices.size());

        glm::mat4 Food2MVMatrix = ViewMatrixCamera.getViewMatrix();
        Food2MVMatrix           = glm::translate(ViewMatrixCamera.getViewMatrix(), glm::vec3(seaweed2.getPos().x, seaweed2.getPos().y, -5));
        Food2MVMatrix           = glm::scale(
                      Food2MVMatrix,
                      glm::vec3(2, 2, 2)
                  );
        glm::mat4 Food2NormalMatrix = glm::transpose(glm::inverse(Food2MVMatrix));
        glUniformMatrix4fv(Objects.uMVMatrix, 1, GL_FALSE, glm::value_ptr(Food2MVMatrix));
        glUniformMatrix4fv(Objects.uMVPMatrix, 1, GL_FALSE, glm::value_ptr(ProjMatrix * Food2MVMatrix));
        glUniformMatrix4fv(Objects.uNormalMatrix, 1, GL_FALSE, glm::value_ptr(Food2NormalMatrix));
        glDrawArrays(GL_TRIANGLES, 0, vertices.size());
        glBindVertexArray(0);

        /*********************** BOIDS **************************/

        glBindVertexArray(vaos[1]);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fishTextureID);

        glm::mat4         boidMVMatrix = ViewMatrixCamera.getViewMatrix();
        std::vector<Fish> cur          = boids.getFishPack();
        for (int i = 0; i < cur.size(); ++i)
        {
            glm::vec3 pos = cur[i].getPos();

            boidMVMatrix = glm::translate(ViewMatrixCamera.getViewMatrix(), glm::vec3(pos.x, pos.y, pos.z));
            boidMVMatrix = glm::scale(
                boidMVMatrix,
                glm::vec3(p.fishSize * 25, p.fishSize * 25, p.fishSize * 25)
            );

            glm::mat4 boidNormalMatrix = glm::transpose(glm::inverse(boidMVMatrix));

            glUniformMatrix4fv(Objects.uMVMatrix, 1, GL_FALSE, glm::value_ptr(boidMVMatrix));
            glUniformMatrix4fv(Objects.uMVPMatrix, 1, GL_FALSE, glm::value_ptr(ProjMatrix * boidMVMatrix));
            glUniformMatrix4fv(Objects.uNormalMatrix, 1, GL_FALSE, glm::value_ptr(boidNormalMatrix));
            // glDrawArrays(GL_TRIANGLES, 0, fishVertices.size());
            glDrawArrays(GL_TRIANGLES, 0, fishV.getVertextSize());
        }
        glBindVertexArray(0);

        // test.draw(ViewMatrixCamera, fishTextureID, ctx, glm::vec3 position, p.fishSize * 25, boids.sizeFishpack())

        /*************************** OBSTACLES *************************/

        glBindVertexArray(vaos[0]);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, moonTextureID);

        glm::mat4 obs1MVMatrix = ViewMatrixCamera.getViewMatrix();
        glm::mat4 obs2MVMatrix = ViewMatrixCamera.getViewMatrix();

        for (int i = 0; i < 3; ++i)
        {
            glm::vec3 pos = obstacle.getObstacles()[i].getPos();
            float     siz = obstacle.getObstacles()[i].getRadius();

            obs1MVMatrix = glm::translate(ViewMatrixCamera.getViewMatrix(), glm::vec3(pos.x, pos.y, -5));
            obs1MVMatrix = glm::scale(
                obs1MVMatrix,
                glm::vec3(5 * siz, 5 * siz, 5 * siz)
            );
            glUniformMatrix4fv(Objects.uMVMatrix, 1, GL_FALSE, glm::value_ptr(obs1MVMatrix));
            glUniformMatrix4fv(Objects.uMVPMatrix, 1, GL_FALSE, glm::value_ptr(ProjMatrix * obs1MVMatrix));
            glDrawArrays(GL_TRIANGLES, 0, vertices.size());

            pos          = obstacle2.getObstacles()[i].getPos();
            siz          = obstacle2.getObstacles()[i].getRadius();
            obs2MVMatrix = glm::translate(ViewMatrixCamera.getViewMatrix(), glm::vec3(pos.x, pos.y, -5));
            obs2MVMatrix = glm::scale(
                obs2MVMatrix,
                glm::vec3(1 + siz, 1 + siz, 1 + siz)
            );
            glm::mat4 obsNormalMatrix = glm::transpose(glm::inverse(obs2MVMatrix));
            glUniformMatrix4fv(Objects.uMVMatrix, 1, GL_FALSE, glm::value_ptr(obs2MVMatrix));
            glUniformMatrix4fv(Objects.uMVPMatrix, 1, GL_FALSE, glm::value_ptr(ProjMatrix * obs2MVMatrix));
            glUniformMatrix4fv(Objects.uNormalMatrix, 1, GL_FALSE, glm::value_ptr(obsNormalMatrix));
            glDrawArrays(GL_TRIANGLES, 0, vertices.size());
        };

        /*************LIGHT*****************/

        light._Program.use();

        glm::mat4 lightMVMatrix = ViewMatrixCamera.getViewMatrix();
        lightMVMatrix           = glm::translate(ViewMatrixCamera.getViewMatrix(), glm::vec3(0, 0, -5));
        lightMVMatrix           = glm::scale(
                      lightMVMatrix,
                      glm::vec3(2, 2, 2)
                  );

        glm::mat4 lightNormalMatrix = glm::transpose(glm::inverse(lightMVMatrix));

        glUniformMatrix4fv(light.uMVMatrix, 1, GL_FALSE, glm::value_ptr(lightMVMatrix));
        glUniformMatrix4fv(light.uMVPMatrix, 1, GL_FALSE, glm::value_ptr(ProjMatrix * lightMVMatrix));
        glUniformMatrix4fv(light.uNormalMatrix, 1, GL_FALSE, glm::value_ptr(lightNormalMatrix));

        glm::vec3 Kd = glm::vec3(1, 1, 1);
        glm::vec3 Ks = glm::vec3(1, 1, 1);
        //  glm::vec4 lightDir = ViewMatrixCamera.getViewMatrix() * glm::vec4(1, 1, -5, 1);
        glm::vec4 lightDir = lightMVMatrix * glm::vec4(1, 0, 1, 1);

        glUniform3fv(light.uKd, 1, glm::value_ptr(Kd));
        glUniform3fv(light.uKs, 1, glm::value_ptr(Ks));
        glUniform1f(light.uShininess, 0.5);
        glUniform3fv(light.uLightPos_vs, 1, glm::value_ptr(lightDir));

        // glUniform3fv(uniformLightDir2, 1, glm::value_ptr(lightDir));
        // glUniform3fv(uniformLightPos2, 1, glm::value_ptr(ViewMatrixCamera.getViewMatrix() * glm::vec4(1, 1, 0, 1)));
        //  glUniform3fv(uniformLightDir2, 1, glm::value_ptr(glm::vec3(glm::rotate(ViewMatrixCamera.getViewMatrix(), ctx.time(), glm::vec3(0, 1, 0)) * glm::vec4(1, 1, 0, 1))));
        glUniform3fv(light.uLightIntensity, 1, glm::value_ptr(glm::vec3(8, 8, 8)));

        glDrawArrays(GL_TRIANGLES, 0, vertices.size());
        glBindVertexArray(0);

        /*******TEST TEXTURE + LIGHT *****/
        glBindVertexArray(vaos[2]);
        lightAndText._Program.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, moonTextureID);
        glm::mat4 testMVMatrix = ViewMatrixCamera.getViewMatrix();
        testMVMatrix           = glm::translate(ViewMatrixCamera.getViewMatrix(), glm::vec3(-4, -2, -5));
        testMVMatrix           = glm::scale(
                      testMVMatrix,
                      glm::vec3(2, 2, 2)
                  );

        glm::mat4 testNormalMatrix = glm::transpose(glm::inverse(testMVMatrix));
        glUniformMatrix4fv(lightAndText.uMVMatrix, 1, GL_FALSE, glm::value_ptr(testMVMatrix));
        glUniformMatrix4fv(lightAndText.uMVPMatrix, 1, GL_FALSE, glm::value_ptr(ProjMatrix * testMVMatrix));
        glUniformMatrix4fv(lightAndText.uNormalMatrix, 1, GL_FALSE, glm::value_ptr(testNormalMatrix));

        glUniform3fv(lightAndText.uKd, 1, glm::value_ptr(Kd));
        glUniform3fv(lightAndText.uKs, 1, glm::value_ptr(Ks));
        glUniform1f(lightAndText.uShininess, 1);
        glUniform3fv(lightAndText.uLightDir_vs, 1, glm::value_ptr(glm::vec3(glm::rotate(ViewMatrixCamera.getViewMatrix(), ctx.time(), glm::vec3(0, 1, 0)) * glm::vec4(1, 1, 0, 1))));

        glUniform3fv(lightAndText.uLightIntensity, 1, glm::value_ptr(glm::vec3(1, 1, 1)));

        glDrawArrays(GL_TRIANGLES, 0, vertices2.size());
        glBindVertexArray(0);

        test.draw(ViewMatrixCamera, fishTextureID, ctx, glm::vec3(2, 2, -5), p.fishSize * 25, 1);
    };

    // Should be done last. It starts the infinite loop.
    ctx.start();
    test.deleteVBO_VAO();
    glDeleteVertexArrays(1, vaos);
    glDeleteBuffers(1, vbos);
    glDeleteTextures(1, &earthTextureID);
    glDeleteTextures(1, &moonTextureID);
    glDeleteTextures(1, &fishTextureID);
}