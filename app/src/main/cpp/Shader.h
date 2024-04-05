#ifndef ANDROIDGLINVESTIGATIONS_SHADER_H
#define ANDROIDGLINVESTIGATIONS_SHADER_H

#include <string>
#include <GLES3/gl3.h>

class Model;

/*!
 * 代表一个简单的着色器程序的类。它包含顶点和片段组件。
 * 输入属性是位置（作为Vector3）和uv（作为Vector2）。
 * 它还接受一个uniform，用作整个模型/视图/投影矩阵。
 * 着色器预期片段着色使用单个纹理，并且不进行其他光照计算（因此没有灯光或法线属性的uniform）。
 */
class Shader {
public:
    /*!
     * 给定完整的源代码以及必要的属性和uniform的名称来加载着色器。
     * 成功时返回一个有效的着色器，失败时返回null。着色器资源会在销毁时自动清理。
     *
     * @param vertexSource 顶点程序的完整源代码
     * @param fragmentSource 片段程序的完整源代码
     * @param positionAttributeName 顶点程序中位置属性的名称
     * @param uvAttributeName 顶点程序中uv坐标属性的名称
     * @param projectionMatrixUniformName 模型/视图/投影矩阵uniform的名称
     * @return 成功时返回一个有效的Shader，否则返回null。
     */
    static Shader *loadShader(
            const std::string &vertexSource,
            const std::string &fragmentSource,
            const std::string &positionAttributeName,
            const std::string &uvAttributeName,
            const std::string &projectionMatrixUniformName);

    inline ~Shader() {
        if (program_) {
            glDeleteProgram(program_);
            program_ = 0;
        }
    }

    /*!
     * 准备着色器以便使用，执行任何绘制命令前调用此函数
     */
    void activate() const;

    /*!
     * 使用完着色器后进行清理，执行任何绘制命令后调用此函数
     */
    void deactivate() const;

    /*!
     * 渲染单个模型
     * @param model 要渲染的模型
     */
    void drawModel(const Model &model) const;

    /*!
     * 在着色器中设置模型/视图/投影矩阵。
     * @param projectionMatrix 十六个浮点数，列优先，定义了一个OpenGL投影矩阵。
     */
    void setProjectionMatrix(float *projectionMatrix) const;

    void setRotationMatrix(const float *rotationMatrix);

private:
    /*!
     * 加载给定类型的着色器的辅助函数
     * @param shaderType OpenGL着色器类型。应该是GL_VERTEX_SHADER或GL_FRAGMENT_SHADER之一
     * @param shaderSource 着色器的完整源代码
     * @return 着色器的id，由glCreateShader返回，或在出错时为0
     */
    static GLuint loadShader(GLenum shaderType, const std::string &shaderSource);

    /*!
     * 构造一个新的着色器实例。使用@a loadShader
     * @param program 着色器的GL程序id
     * @param position 位置的属性位置
     * @param uv uv坐标的属性位置
     * @param projectionMatrix 投影矩阵的uniform位置
     */
    constexpr Shader(
            GLuint program,
            GLint position,
            GLint uv,
            GLint projectionMatrix)
            : program_(program),
              position_(position),
              uv_(uv),
              projectionMatrix_(projectionMatrix) {}

    GLuint program_; // 着色器程序ID
    GLint position_; // 位置属性位置
    GLint uv_; // UV属性位置
    GLint projectionMatrix_; // 投影矩阵uniform位置
};

#endif //ANDROIDGLINVESTIGATIONS_SHADER_H
