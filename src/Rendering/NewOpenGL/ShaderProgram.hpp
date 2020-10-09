#pragma once
#include <fstream>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <iostream>
#include <sstream>
#include <string>

namespace voxer {

using vec2 = std::array<float, 2>;
using vec3 = std::array<float, 3>;
using vec4 = std::array<float, 4>;
using ivec2 = std::array<int32_t, 2>;
using ivec3 = std::array<int32_t, 3>;
using ivec4 = std::array<int32_t, 4>;

class ShaderProgram {
public:
  unsigned int ID;
  ShaderProgram(const char *vertexShader, const char *fragmentShader,
                const char *geometryShader = nullptr);
  void use() const;
  void setBool(const std::string &name, bool value) const;
  void setInt(const std::string &name, int value) const;
  void setFloat(const std::string &name, float value) const;
  void setVec2(const std::string &name, const vec2 &value) const;
  void setVec2(const std::string &name, float x, float y) const;
  void setVec3(const std::string &name, const vec3 &value) const;
  void setVec3(const std::string &name, float x, float y, float z) const;
  void setIVec3(const std::string &name, const ivec3 &value) const;
  void setVec4(const std::string &name, const vec4 &value) const;
  void setVec4(const std::string &name, float x, float y, float z,
               float w) const;
  void setMat2(const std::string &name, const glm::mat2 &mat) const;
  void setMat3(const std::string &name, const glm::mat3 &mat) const;
  void setMat4(const std::string &name, const glm::mat4 &mat) const;
  static void checkCompileErrors(GLuint id, const std::string &type);
};

inline ShaderProgram::ShaderProgram(const char *vertexShader,
                                    const char *fragmentShader,
                                    const char *geometryShader) {
  // 1.load glsl file
  std::string vertexCode;
  std::string fragmentCode;
  std::string geometryCode;
  std::ifstream vShaderFile, fShaderFile, gShaderFile;

  vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  // 2.compile shaders
  const char *vShaderCode = vertexShader;
  const char *fShaderCode = fragmentShader;

  unsigned int vShader, fShader, gShader;

  vShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vShader, 1, &vShaderCode, nullptr);
  glCompileShader(vShader);
  checkCompileErrors(vShader, "VERTEX");

  fShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fShader, 1, &fShaderCode, nullptr);
  glCompileShader(fShader);
  checkCompileErrors(fShader, "FRAGMENT");

  if (geometryShader != nullptr) {
    const char *gShaderCode = geometryCode.c_str();

    gShader = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(gShader, 1, &gShaderCode, nullptr);
    glCompileShader(gShader);
    checkCompileErrors(gShader, "GEOMETRY");
  }

  ID = glCreateProgram();
  glAttachShader(ID, vShader);
  glAttachShader(ID, fShader);
  if (geometryShader != nullptr)
    glAttachShader(ID, gShader);
  glLinkProgram(ID);
  checkCompileErrors(ID, "PROGRAM");

  glDeleteShader(vShader);
  glDeleteShader(fShader);
  if (geometryShader != nullptr)
    glDeleteShader(gShader);
}

inline void ShaderProgram::use() const { glUseProgram(ID); }

inline void ShaderProgram::setBool(const std::string &name, bool value) const {
  glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

inline void ShaderProgram::setInt(const std::string &name, int value) const {
  glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

inline void ShaderProgram::setFloat(const std::string &name,
                                    float value) const {
  glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

inline void ShaderProgram::setVec2(const std::string &name,
                                   const vec2 &value) const {
  glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

inline void ShaderProgram::setVec2(const std::string &name, float x,
                                   float y) const {
  glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
}

inline void ShaderProgram::setVec3(const std::string &name,
                                   const vec3 &value) const {
  glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

inline void ShaderProgram::setVec3(const std::string &name, float x, float y,
                                   float z) const {
  glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}

inline void ShaderProgram::setIVec3(const std::string &name,
                                    const ivec3 &value) const {
  glUniform3iv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

inline void ShaderProgram::setVec4(const std::string &name,
                                   const vec4 &value) const {
  glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

inline void ShaderProgram::setVec4(const std::string &name, float x, float y,
                                   float z, float w) const {
  glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
}

inline void ShaderProgram::setMat2(const std::string &name,
                                   const glm::mat2 &mat) const {
  glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE,
                     &mat[0][0]);
}

inline void ShaderProgram::setMat3(const std::string &name,
                                   const glm::mat3 &mat) const {
  glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE,
                     &mat[0][0]);
}

inline void ShaderProgram::setMat4(const std::string &name,
                                   const glm::mat4 &mat) const {
  glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE,
                     &mat[0][0]);
}

inline void ShaderProgram::checkCompileErrors(GLuint id,
                                              const std::string &type) {
  GLint success;
  GLchar infoLog[1024];
  if (type != "PROGRAM") {
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(id, 1024, nullptr, infoLog);
      std::cout
          << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
          << infoLog
          << "\n -- --------------------------------------------------- -- "
          << std::endl;
    }
  } else {
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(id, 1024, nullptr, infoLog);
      std::cout
          << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
          << infoLog
          << "\n -- --------------------------------------------------- -- "
          << std::endl;
    }
  }
}

} // namespace voxer