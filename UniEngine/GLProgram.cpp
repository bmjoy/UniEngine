#include "pch.h"
#include "GLProgram.h"
#include "Debug.h"
#include "GLShader.h"
using namespace UniEngine;

UniEngine::GLProgram::GLProgram()
{
    _ID = glCreateProgram();
    _VertexShader = nullptr;
    _FragmentShader = nullptr;
}

UniEngine::GLProgram::GLProgram(GLShader* vertexShader, GLShader* fragmentShader, GLShader* geometryShader)
{
    _ID = glCreateProgram();
    _VertexShader = nullptr;
    _FragmentShader = nullptr;
    Attach(ShaderType::Vertex, vertexShader);
    Attach(ShaderType::Fragment, fragmentShader);
    _VertexShader = vertexShader;
    _FragmentShader = fragmentShader;
    if(geometryShader) Attach(ShaderType::Geometry, geometryShader);
    Link();
}

UniEngine::GLProgram::~GLProgram()
{
    glDeleteProgram(_ID);
}

void UniEngine::GLProgram::Use()
{
    glUseProgram(_ID);
}

void UniEngine::GLProgram::Link()
{
    glLinkProgram(_ID);
    GLint status = 0;
    glGetProgramiv(_ID, GL_LINK_STATUS, &status);
    if (!status) {
        GLchar infoLog[1024];
        std::string type = "PROGRAM";
        glGetProgramInfoLog(_ID, 1024, NULL, infoLog);
        Debug::Error("ERROR::PROGRAM_LINKING_ERROR of type: " + type + "\n" + infoLog + "\n -- --------------------------------------------------- -- ");
    }
}

void UniEngine::GLProgram::Attach(ShaderType type, GLShader* shader)
{
    switch (type)
    {
    case UniEngine::ShaderType::Vertex:
        if (_VertexShader != nullptr) Detach(type);
        _VertexShader = shader;
        _VertexShader->Attach(_ID);
        break;
    case UniEngine::ShaderType::Geometry:
        if (_GeometryShader != nullptr) Detach(type);
        _GeometryShader = shader;
        _GeometryShader->Attach(_ID);
        break;
    case UniEngine::ShaderType::Fragment:
        if (_FragmentShader != nullptr) Detach(type);
        _FragmentShader = shader;
        _FragmentShader->Attach(_ID);
        break;
    default:
        break;
    }
}

void UniEngine::GLProgram::Detach(ShaderType type)
{
    switch (type)
    {
    case UniEngine::ShaderType::Vertex:
        _VertexShader->Detach(_ID);
        _VertexShader = nullptr;
        break;
    case UniEngine::ShaderType::Geometry:
        _GeometryShader->Detach(_ID);
        _GeometryShader = nullptr;
        break;
    case UniEngine::ShaderType::Fragment:
        _FragmentShader->Detach(_ID);
        _FragmentShader = nullptr;
        break;
    }
}

void GLProgram::SetBool(const std::string& name, bool value) const
{
    glUseProgram(_ID);
    glUniform1i(glGetUniformLocation(_ID, name.c_str()), (int)value);
}
void GLProgram::SetInt(const std::string& name, int value) const
{
    glUseProgram(_ID);
    glUniform1i(glGetUniformLocation(_ID, name.c_str()), value);
}
void GLProgram::SetFloat(const std::string& name, float value) const
{
    glUseProgram(_ID);
    glUniform1f(glGetUniformLocation(_ID, name.c_str()), value);
}
void GLProgram::SetFloat2(const std::string& name, const glm::vec2& value) const
{
    glUseProgram(_ID);
    glUniform2fv(glGetUniformLocation(_ID, name.c_str()), 1, &value[0]);
}
void GLProgram::SetFloat2(const std::string& name, float x, float y) const
{
    glUseProgram(_ID);
    glUniform2f(glGetUniformLocation(_ID, name.c_str()), x, y);
}
void GLProgram::SetFloat3(const std::string& name, const glm::vec3& value) const
{
    glUseProgram(_ID);
    glUniform3fv(glGetUniformLocation(_ID, name.c_str()), 1, &value[0]);
}
void GLProgram::SetFloat3(const std::string& name, float x, float y, float z) const
{
    glUseProgram(_ID);
    glUniform3f(glGetUniformLocation(_ID, name.c_str()), x, y, z);
}
void GLProgram::SetFloat4(const std::string& name, const glm::vec4& value) const
{
    glUseProgram(_ID);
    glUniform4fv(glGetUniformLocation(_ID, name.c_str()), 1, &value[0]);
}
void GLProgram::SetFloat4(const std::string& name, float x, float y, float z, float w)
{
    glUseProgram(_ID);
    glUniform4f(glGetUniformLocation(_ID, name.c_str()), x, y, z, w);
}
void GLProgram::SetFloat2x2(const std::string& name, const glm::mat2& mat) const
{
    glUseProgram(_ID);
    glUniformMatrix2fv(glGetUniformLocation(_ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
void GLProgram::SetFloat3x3(const std::string& name, const glm::mat3& mat) const
{
    glUseProgram(_ID);
    glUniformMatrix3fv(glGetUniformLocation(_ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
void GLProgram::SetFloat4x4(const std::string& name, const glm::mat4& mat) const
{
    glUseProgram(_ID);
    glUniformMatrix4fv(glGetUniformLocation(_ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}