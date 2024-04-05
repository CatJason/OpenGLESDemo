#include "Shader.h"

#include "AndroidOut.h"
#include "Model.h"
#include "Utility.h"

// 加载着色器的静态函数
Shader *Shader::loadShader(
        const std::string &vertexSource,
        const std::string &fragmentSource,
        const std::string &positionAttributeName,
        const std::string &uvAttributeName,
        const std::string &projectionMatrixUniformName) {
    aout << "执行函数 loadShader" << std::endl;
    Shader *shader = nullptr;

    // 加载顶点着色器
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vertexSource);
    if (!vertexShader) {
        return nullptr;
    }

    // 加载片段着色器
    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (!fragmentShader) {
        glDeleteShader(vertexShader);
        return nullptr;
    }

    // 创建着色器程序
    GLuint program = glCreateProgram();
    if (program) {
        // 将顶点和片段着色器附加到程序
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);

        // 链接程序
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            // 如果链接失败，记录错误信息
            GLint logLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

            if (logLength) {
                GLchar *log = new GLchar[logLength];
                glGetProgramInfoLog(program, logLength, nullptr, log);
                aout << "程序链接失败:\n" << log << std::endl;
                delete[] log;
            }

            glDeleteProgram(program);
        } else {
            // 获取属性和统一变量的位置
            GLint positionAttribute = glGetAttribLocation(program, positionAttributeName.c_str());
            GLint uvAttribute = glGetAttribLocation(program, uvAttributeName.c_str());
            GLint projectionMatrixUniform = glGetUniformLocation(
                    program,
                    projectionMatrixUniformName.c_str());

            // 如果所有属性都找到了，创建新的着色器
            if (positionAttribute != -1
                && uvAttribute != -1
                && projectionMatrixUniform != -1) {

                shader = new Shader(
                        program,
                        positionAttribute,
                        uvAttribute,
                        projectionMatrixUniform);
            } else {
                glDeleteProgram(program);
            }
        }
    }

    // 程序链接后，不再需要单独的着色器，释放它们的内存
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shader;
}

// 加载单个着色器的函数
GLuint Shader::loadShader(GLenum shaderType, const std::string &shaderSource) {
    aout << "执行函数 loadShader" << std::endl;
    Utility::assertGlError();
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        auto *shaderRawString = (GLchar *) shaderSource.c_str();
        GLint shaderLength = shaderSource.length();
        glShaderSource(shader, 1, &shaderRawString, &shaderLength);
        glCompileShader(shader);

        GLint shaderCompiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderCompiled);

        // 如果着色器编译失败，记录错误信息
        if (!shaderCompiled) {
            GLint infoLength = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLength);

            if (infoLength) {
                auto *infoLog = new GLchar[infoLength];
                glGetShaderInfoLog(shader, infoLength, nullptr, infoLog);
                aout << "编译失败:\n" << infoLog << std::endl;
                delete[] infoLog;
            }

            glDeleteShader(shader);
            shader = 0;
        }
    }
    return shader;
}

// 激活着色器程序
void Shader::activate() const {
    aout << "执行函数 activate" << std::endl;
    glUseProgram(program_);
}

// 取消激活着色器程序
void Shader::deactivate() const {
    aout << "执行函数 deactivate" << std::endl;
    glUseProgram(0);
}

void Shader::setRotationMatrix(const float* rotationMatrix) {
    GLint rotationMatrixUniform = glGetUniformLocation(program_, "uRotation");
    glUniformMatrix4fv(rotationMatrixUniform, 1, GL_FALSE, rotationMatrix);
}
void Shader::drawModel(const Model &model) const {
    aout << "执行函数 drawModel" << std::endl;

    // 设置顶点属性
    glVertexAttribPointer(position_, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), model.getVertexData());
    glEnableVertexAttribArray(position_);

    // 设置UV属性
    glVertexAttribPointer(uv_, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), ((uint8_t *)model.getVertexData()) + sizeof(Vector3));
    glEnableVertexAttribArray(uv_);

    // 设置纹理
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, model.getTexture().getTextureID());

    // 使用模型指定的绘制模式绘制
    glDrawElements(model.getMode(), model.getIndexCount(), GL_UNSIGNED_SHORT, model.getIndexData());

    // 禁用属性
    glDisableVertexAttribArray(uv_);
    glDisableVertexAttribArray(position_);
}

// 设置投影矩阵
void Shader::setProjectionMatrix(float *projectionMatrix) const {
    aout << "执行函数 setProjectionMatrix" << std::endl;
    glUniformMatrix4fv(projectionMatrix_, 1, false, projectionMatrix);
}
