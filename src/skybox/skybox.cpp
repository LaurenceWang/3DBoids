#include "skybox.hpp"
#include "glm/gtc/type_ptr.hpp"

Skybox::Skybox(const std::vector<float>& vertices)
    : _shader(p6::load_shader("Shaders/skybox.vs.glsl", "Shaders/skybox.fs.glsl"))
    , _vertices(vertices)
{
    createVBO_VAO(_vertices.data(), _vertices.size());
    getTextureID();
}

void Skybox::createVBO_VAO(const float* skyboxVertices, size_t numVertices)
{
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);
    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(float), skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

void Skybox::getTextureID()
{
    std::vector<std::filesystem::path> faces = {
        "Assets/textures/flashy-sky/px.png",
        "Assets/textures/flashy-sky/nx.png",
        "Assets/textures/flashy-sky/ny.png",
        "Assets/textures/flashy-sky/py.png",
        "Assets/textures/flashy-sky/pz.png",
        "Assets/textures/flashy-sky/nz.png",

    };
    Texture skybox(faces, 1);
    _textureID = skybox.getTextureID();
}

void Skybox::renderSkybox(glm::mat4 ViewMatrix, p6::Context& ctx)
{
    glDepthFunc(GL_LEQUAL);

    glm::mat4 ProjMatrix = glm::perspective(glm::radians(70.f), ctx.aspect_ratio(), 0.1f, 100.f);

    _shader.use();

    _shader.set("uMVMatrix", ViewMatrix);
    _shader.set("uProjection", ProjMatrix);
    _shader.set("uKd", glm::vec3(1, 1, 1));
    _shader.set("uKs", glm::vec3(1, 1, 1));
    _shader.set("uShininess", 1);
    _shader.set("uLightPos_vs", ViewMatrix * glm::vec4(1.2f, 1.0f, 2.0f, 1));
    _shader.set("uLightIntensity", glm::vec3(1, 1, 1));

    glBindVertexArray(_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, _textureID);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
}

void Skybox::deleteRessources()
{
    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_vbo);
    glDeleteTextures(1, &_textureID);
}

std::vector<float> returnVertices()
{
    std::vector<float> skyboxVertices = {
        // positions
        -1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f};

    return skyboxVertices;
}
