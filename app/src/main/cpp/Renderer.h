#ifndef ANDROIDGLINVESTIGATIONS_RENDERER_H
#define ANDROIDGLINVESTIGATIONS_RENDERER_H

#include <EGL/egl.h>
#include <memory>

#include "Model.h"
#include "Shader.h"

struct android_app;

// 渲染器类定义
class Renderer {
public:
    /*!
     * 构造函数
     * @param pApp 指向这个Renderer所属的android_app的指针，用于配置GL环境
     */
    inline Renderer(android_app *pApp) :
            app_(pApp),
            display_(EGL_NO_DISPLAY),
            surface_(EGL_NO_SURFACE),
            context_(EGL_NO_CONTEXT),
            width_(0),
            height_(0),
            shaderNeedsNewProjectionMatrix_(true) {
        initRenderer();
    }

    virtual ~Renderer();

    /*!
     * 处理来自android_app的输入。
     *
     * 注意：这会清空输入队列
     */
    void handleInput();

    /*!
     * 渲染renderer中的所有模型
     */
    void render();

private:
    /*!
     * 执行必要的OpenGL初始化。如果你想改变你的EGL上下文或应用范围的设置，可以自定义这个函数。
     */
    void initRenderer();

    /*!
     * @brief 每一帧我们都需要检查帧缓冲区的大小是否发生了变化。如果发生了变化，相应地更新视口
     */
    void updateRenderArea();

    /*!
     * 为这个示例创建模型。在你的完整游戏中，你可能会从文件加载场景配置，
     * 或使用其他设置逻辑。
     */
    void createModels();

    android_app *app_; // 指向android_app的指针
    EGLDisplay display_; // EGL显示设备
    EGLSurface surface_; // EGL表面
    EGLContext context_; // EGL上下文
    EGLint width_; // 视口宽度
    EGLint height_; // 视口高度

    bool shaderNeedsNewProjectionMatrix_; // 标记是否需要新的投影矩阵

    std::unique_ptr<Shader> shader_; // 着色器
    std::vector<Model> models_; // 模型集合
};

#endif //ANDROIDGLINVESTIGATIONS_RENDERER_H
