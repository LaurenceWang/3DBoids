#include "Object.hpp"
#include "glm/gtc/type_ptr.hpp"

Object::Object(Object& obj)
    : _program(obj._program) {}

Object::Object(Program& program, Model vertices)
    : _program(program), _vertices(vertices.getVertices())
{
    createVBO();
    createVAO();
}

Object::Object(Program& program, Model vertices, Texture texture)
    : Object(program, vertices)
{
    _texture = texture;
}

Object::Object(Program& program, std::vector<glimac::ShapeVertex> vertices)
    : _program(program), _vertices(vertices), _texture(Texture("Assets/textures/default_tex.png", 0))
{
    createVBO();
    createVAO();
}

Object::Object(Program& program, std::vector<glimac::ShapeVertex> vertices, Texture texture)
    : Object(program, vertices)
{
    _texture = texture;
}

Object::Object(Program& program, Model vertices, Model lowVertices, Texture texture)
    : _program(program), _texture(texture)
{
    _lodVertices.push_back(vertices.getVertices());
    _lodVertices.push_back(lowVertices.getVertices());
    createVBO2();
    createVAO2();
}

void Object::createVBO()
{
    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(glimac::ShapeVertex), _vertices.data(), GL_STATIC_DRAW);
}

void Object::createVAO()
{
    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    const GLuint VERTEX_ATTR_POSITION  = 0;
    const GLuint VERTEX_ATTR_NORMAL    = 1;
    const GLuint VERTEX_ATTR_TEXCOORDS = 2;

    glEnableVertexAttribArray(VERTEX_ATTR_POSITION);
    glEnableVertexAttribArray(VERTEX_ATTR_NORMAL);
    glEnableVertexAttribArray(VERTEX_ATTR_TEXCOORDS);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);

    glVertexAttribPointer(VERTEX_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(glimac::ShapeVertex), (const GLvoid*)(offsetof(glimac::ShapeVertex, position)));
    glVertexAttribPointer(VERTEX_ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(glimac::ShapeVertex), (const GLvoid*)(offsetof(glimac::ShapeVertex, normal)));
    glVertexAttribPointer(VERTEX_ATTR_TEXCOORDS, 2, GL_FLOAT, GL_FALSE, sizeof(glimac::ShapeVertex), (const GLvoid*)(offsetof(glimac::ShapeVertex, texCoords)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}

void Object::createVBO2()
{
    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, (_lodVertices[_lod]).size() * sizeof(glimac::ShapeVertex), (_lodVertices[_lod]).data(), GL_STATIC_DRAW);
}

void Object::createVAO2()
{
    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    const GLuint VERTEX_ATTR_POSITION  = 0;
    const GLuint VERTEX_ATTR_NORMAL    = 1;
    const GLuint VERTEX_ATTR_TEXCOORDS = 2;

    glEnableVertexAttribArray(VERTEX_ATTR_POSITION);
    glEnableVertexAttribArray(VERTEX_ATTR_NORMAL);
    glEnableVertexAttribArray(VERTEX_ATTR_TEXCOORDS);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);

    glVertexAttribPointer(VERTEX_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(glimac::ShapeVertex), (const GLvoid*)(offsetof(glimac::ShapeVertex, position)));
    glVertexAttribPointer(VERTEX_ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(glimac::ShapeVertex), (const GLvoid*)(offsetof(glimac::ShapeVertex, normal)));
    glVertexAttribPointer(VERTEX_ATTR_TEXCOORDS, 2, GL_FLOAT, GL_FALSE, sizeof(glimac::ShapeVertex), (const GLvoid*)(offsetof(glimac::ShapeVertex, texCoords)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}

void Object::createDrawEnvironment(p6::Context& ctx) // TODO if needed vector of Texture, passing entire texture of just GLuint bc might need to
{
    _program._Program.use(); // TODO change confusing names
    glBindVertexArray(_vao);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _texture.getTextureID());
    _ProjMatrix = glm::perspective(glm::radians(70.f), ctx.aspect_ratio(), 0.1f, 100.f);
}

void Object::draw(const glm::mat4& ViewMatrixCamera, glm::vec3 position, float degRotate, float scaleSize, glm::vec3 arpPos)
{
    glm::mat4 MVMatrix = ViewMatrixCamera;
    MVMatrix           = glm::translate(MVMatrix, position);
    MVMatrix           = glm::rotate(MVMatrix, glm::radians(degRotate), glm::vec3(0.0f, 1.0f, 0.0f));
    MVMatrix           = glm::scale(
        MVMatrix,
        glm::vec3(scaleSize, scaleSize, scaleSize)
    );

    glm::mat4 NormalMatrix = glm::transpose(glm::inverse(MVMatrix));

    glUniformMatrix4fv(_program.uMVMatrix, 1, GL_FALSE, glm::value_ptr(MVMatrix));
    glUniformMatrix4fv(_program.uMVPMatrix, 1, GL_FALSE, glm::value_ptr(_ProjMatrix * MVMatrix));
    glUniformMatrix4fv(_program.uNormalMatrix, 1, GL_FALSE, glm::value_ptr(NormalMatrix));

    // only for lights
    glUniform3fv(_program.uKd, 1, glm::value_ptr(glm::vec3(1, 0, 1)));
    glUniform3fv(_program.uKs, 1, glm::value_ptr(glm::vec3(1, 0, 1)));
    glUniform1f(_program.uShininess, 1);
    // glUniform3fv(_program.uLightPos_vs, 1, glm::value_ptr(MVMatrix * glm::vec4(0.f, -1.5f, -15.f, 0)));

    glUniform3fv(_program.uLightPos_vs, 1, glm::value_ptr(ViewMatrixCamera * glm::vec4(0.f, 0.f, 0.f, 1)));

    glUniform3fv(_program.uLightPosArp_vs, 1, glm::value_ptr(ViewMatrixCamera * glm::vec4(arpPos, 1)));
    // glUniform3fv(_program.uLightDir_vs, 1, glm::value_ptr(MVMatrix * glm::vec4(10.f, 2.f, -13.f, 1)));
    //  glUniform3fv(_program.uLightDir_vs, 1, glm::value_ptr(MVMatrix * glm::vec4(10.f, 2.f, -13.f, 1)));
    //   glUniform3fv(_program.uLightDir_vs, 1, glm::value_ptr(ViewMatrixCamera * glm::vec4(3, 1, -3, 1)));

    // glUniform3fv(_program.uLightDir_vs, 1, glm::value_ptr(ViewMatrixCamera * glm::vec4(3, 1, -3, 1)));
    glUniform3fv(_program.uLightDir_vs, 1, glm::value_ptr(ViewMatrixCamera * glm::vec4(1, 1, 1, 0)));
    // glUniform3fv(_program.uLightIntensity, 1, glm::value_ptr(glm::vec3(0.05, 0.05, 0.05))); // change to 1 to see effects on light & tex object
    glUniform3fv(_program.uLightIntensity, 1, glm::value_ptr(glm::vec3(0.5, 0.5, 0.5))); // change to 1 to see effects on light & tex object

    glDrawArrays(GL_TRIANGLES, 0, _vertices.size());

    // glBindVertexArray(0);
}

void Object::draw(const glm::mat4& ViewMatrixCamera, objectParameters parameters)
{
    glm::mat4 MVMatrix = ViewMatrixCamera;
    MVMatrix           = glm::translate(MVMatrix, parameters.position);
    MVMatrix           = glm::rotate(MVMatrix, glm::radians(parameters.degRotate), glm::vec3(0.0f, 1.0f, 0.0f));
    MVMatrix           = glm::scale(
        MVMatrix,
        glm::vec3(parameters.scaleSize)
    );

    glm::mat4 NormalMatrix = glm::transpose(glm::inverse(MVMatrix));

    glUniformMatrix4fv(_program.uMVMatrix, 1, GL_FALSE, glm::value_ptr(MVMatrix));
    glUniformMatrix4fv(_program.uMVPMatrix, 1, GL_FALSE, glm::value_ptr(_ProjMatrix * MVMatrix));
    glUniformMatrix4fv(_program.uNormalMatrix, 1, GL_FALSE, glm::value_ptr(NormalMatrix));

    // only for lights
    glUniform3fv(_program.uKd, 1, glm::value_ptr(glm::vec3(1, 0, 1)));
    glUniform3fv(_program.uKs, 1, glm::value_ptr(glm::vec3(1, 0, 1)));
    glUniform1f(_program.uShininess, 1);

    glUniform3fv(_program.uLightPos_vs, 1, glm::value_ptr(ViewMatrixCamera * glm::vec4(0.f, 0.f, 0.f, 1)));
    glUniform3fv(_program.uLightDir_vs, 1, glm::value_ptr(ViewMatrixCamera * glm::vec4(1, 1, 1, 0)));

    glUniform3fv(_program.uLightIntensity, 1, glm::value_ptr(glm::vec3(0.5, 0.5, 0.5))); // change to 1 to see effects on light & tex object

    glDrawArrays(GL_TRIANGLES, 0, _vertices.size());

    // glBindVertexArray(0);
}

void Object::draw3(const glm::mat4& ViewMatrixCamera, glm::vec3 position, float degRotate, float scaleSize, glm::vec3 arpPos)
{
    glm::mat4 MVMatrix = ViewMatrixCamera;
    MVMatrix           = glm::translate(MVMatrix, position);
    MVMatrix           = glm::rotate(MVMatrix, glm::radians(degRotate), glm::vec3(0.0f, 1.0f, 0.0f));
    MVMatrix           = glm::scale(
        MVMatrix,
        glm::vec3(scaleSize, scaleSize, scaleSize)
    );

    glm::mat4 NormalMatrix = glm::transpose(glm::inverse(MVMatrix));

    glUniformMatrix4fv(_program.uMVMatrix, 1, GL_FALSE, glm::value_ptr(MVMatrix));
    glUniformMatrix4fv(_program.uMVPMatrix, 1, GL_FALSE, glm::value_ptr(_ProjMatrix * MVMatrix));
    glUniformMatrix4fv(_program.uNormalMatrix, 1, GL_FALSE, glm::value_ptr(NormalMatrix));

    // only for lights
    glUniform3fv(_program.uKd, 1, glm::value_ptr(glm::vec3(1, 0, 1)));
    glUniform3fv(_program.uKs, 1, glm::value_ptr(glm::vec3(1, 0, 1)));
    glUniform1f(_program.uShininess, 1);
    // glUniform3fv(_program.uLightPos_vs, 1, glm::value_ptr(MVMatrix * glm::vec4(0.f, -1.5f, -15.f, 0)));

    glUniform3fv(_program.uLightPos_vs, 1, glm::value_ptr(ViewMatrixCamera * glm::vec4(0.f, 0.f, 0.f, 1)));

    glUniform3fv(_program.uLightPosArp_vs, 1, glm::value_ptr(ViewMatrixCamera * glm::vec4(arpPos, 1)));
    glUniform3fv(_program.uLightDir_vs, 1, glm::value_ptr(ViewMatrixCamera * glm::vec4(1, 1, 1, 0)));
    glUniform3fv(_program.uLightIntensity, 1, glm::value_ptr(glm::vec3(0.5, 0.5, 0.5))); // change to 1 to see effects on light & tex object

    glDrawArrays(GL_TRIANGLES, 0, (_lodVertices[_lod]).size());
}

void Object::distanceLOD(glm::vec3 arpenteurPos, glm::vec3 objPos)
{
    double distance = glm::distance(arpenteurPos, objPos);
    _lod            = (distance > 7) ? 1 : 0;
}

void Object::adjustLOD(glm::vec3 arpenteurPos, glm::vec3 objPos, int choice)
{
    if (choice < 2)
    {
        _lod = choice;
    }
    else
    {
        distanceLOD(arpenteurPos, objPos);
    }
}

void Object::debindVAO()
{
    glBindVertexArray(0);
}

void Object::deleteVBO_VAO()
{
    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_vbo);
    _texture.deleteTex();
}