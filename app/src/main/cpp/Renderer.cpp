#include "Renderer.h"

#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <GLES3/gl3.h>
#include <memory>
#include <vector>
#include <android/imagedecoder.h>

#include "AndroidOut.h"
#include "Shader.h"
#include "Utility.h"
#include "TextureAsset.h"

//! 执行glGetString并将结果输出到logcat
#define PRINT_GL_STRING(s) {aout << #s": "<< glGetString(s) << std::endl;}

/*!
 * @brief 如果glGetString返回一个由空格分隔的元素列表，将每个元素打印在新行上
 *
 * 通过创建输入c风格字符串的istringstream来工作。然后使用它来创建一个vector，
 * vector中的每个元素都是输入字符串中的新元素。最后使用foreach循环将其输出到logcat，
 * 使用@a aout
 */
#define PRINT_GL_STRING_AS_LIST(s) { \
std::istringstream extensionStream((const char *) glGetString(s));\
std::vector<std::string> extensionList(\
        std::istream_iterator<std::string>{extensionStream},\
        std::istream_iterator<std::string>());\
aout << #s":\n";\
for (auto& extension: extensionList) {\
    aout << extension << "\n";\
}\
aout << std::endl;\
}

//! cornflower blue的颜色。可以直接发送到glClearColor
#define CORNFLOWER_BLUE 100 / 255.f, 149 / 255.f, 237 / 255.f, 1

// 顶点着色器，通常你会从资源中加载这个
static const char *vertex = R"vertex(#version 300 es
in vec3 inPosition;
in vec2 inUV;

out vec2 fragUV;

uniform mat4 uProjection;
uniform mat4 uRotation; // 新增旋转矩阵uniform

void main() {
    fragUV = inUV;
    gl_Position = uProjection * uRotation * vec4(inPosition, 1.0); // 应用旋转
}
)vertex";

// 片段着色器，通常你也会从资源中加载这个
static const char *fragment = R"fragment(#version 300 es
precision mediump float;

in vec2 fragUV;

uniform sampler2D uTexture;

out vec4 outColor;

void main() {
    vec4 textureColor = texture(uTexture, fragUV);
    vec4 baseColor = vec4(1.0, 0.0, 0.0, 1.0); // 红色
    outColor = mix(baseColor, textureColor, textureColor.a); // 基于alpha值混合
}
)fragment";

float rotationAngle_ = 0.0f; // 旋转角度

/*!
 * 投影矩阵高度的一半。这给你提供了一个高度为4的可渲染区域，范围从-2到2
 */
static constexpr float kProjectionHalfHeight = 2.f;

/*!
 * 投影矩阵的近平面距离。由于这是一个正交投影矩阵，对于排序（以及避免在0处的z-fighting），
 * 允许负值是方便的。
 */
static constexpr float kProjectionNearPlane = -1.f;

/*!
 * 投影矩阵的远平面距离。由于这是一个正交投影矩阵，让远平面与0等距离是方便的。
 */
static constexpr float kProjectionFarPlane = 1.f;

Renderer::~Renderer() {
    aout << "执行函数 ~Renderer" << std::endl;
    if (display_ != EGL_NO_DISPLAY) {
        eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (context_ != EGL_NO_CONTEXT) {
            eglDestroyContext(display_, context_);
            context_ = EGL_NO_CONTEXT;
        }
        if (surface_ != EGL_NO_SURFACE) {
            eglDestroySurface(display_, surface_);
            surface_ = EGL_NO_SURFACE;
        }
        eglTerminate(display_);
        display_ = EGL_NO_DISPLAY;
    }
}

void Renderer::render() {
    aout << "执行函数 render" << std::endl;
    // 检查渲染区域的大小是否有变化。在使用沉浸模式时，这是每帧都必须做的，
    // 因为你不会收到其他通知来告诉你的渲染区域已经改变。
    updateRenderArea();

    // 渲染区域改变时，投影矩阵也需要更新。即使你从示例的正交投影矩阵改变，
    // 你的纵横比可能也已经改变。
    if (shaderNeedsNewProjectionMatrix_) {
        // 在栈上分配的占位符投影矩阵。列主内存布局
        float projectionMatrix[16] = {0};

        // 为2D渲染构建正交投影矩阵
        Utility::buildOrthographicMatrix(
                projectionMatrix,
                kProjectionHalfHeight,
                float(width_) / height_,
                kProjectionNearPlane,
                kProjectionFarPlane);

        // 将矩阵发送到着色器
        // 注意：着色器必须是激活的才能工作。由于我们在这个演示中只有一个着色器，
        // 我们可以假设它是激活的。
        shader_->setProjectionMatrix(projectionMatrix);

        // 确保矩阵不是每帧都生成
        shaderNeedsNewProjectionMatrix_ = false;
    }

    // 更新旋转角度
    rotationAngle_ += 1.0f; // 每帧旋转1度，你可以根据需要调整这个值
    if(rotationAngle_ >= 360.0f) rotationAngle_ -= 360.0f; // 防止溢出

    // 计算旋转矩阵
    float rotationMatrix[16];
    Utility::buildRotationMatrix(rotationMatrix, rotationAngle_);

    // 设置旋转矩阵uniform
    shader_->setRotationMatrix(rotationMatrix);

    // 清除颜色缓冲区
    glClear(GL_COLOR_BUFFER_BIT);

    // 渲染所有模型。这个示例中没有深度测试，所以模型按提供的顺序接受。
    // 但是示例EGL设置请求了一个24位深度缓冲区，所以你可以在initRenderer的最后配置它
    if (!models_.empty()) {
        for (const auto &model: models_) {
            shader_->drawModel(model);
        }
    }

    // 展示渲染的图像。这是一个隐式的glFlush。
    auto swapResult = eglSwapBuffers(display_, surface_);
    assert(swapResult == EGL_TRUE);
}

void Renderer::initRenderer() {
    aout << "执行函数 initRenderer" << std::endl;
    // 选择你的渲染属性
    constexpr
    EGLint attribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_DEPTH_SIZE, 24,
            EGL_NONE
    };

    // 默认显示设备可能是你在Android上想要的
    auto display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, nullptr, nullptr);

    // 弄清楚有多少配置
    EGLint numConfigs;
    eglChooseConfig(display, attribs, nullptr, 0, &numConfigs);

    // 获取配置列表
    std::unique_ptr<EGLConfig[]> supportedConfigs(new EGLConfig[numConfigs]);
    eglChooseConfig(display, attribs, supportedConfigs.get(), numConfigs, &numConfigs);

    // 找到我们喜欢的配置。
    // 如果我们不关心配置中的其他内容，可能直接抓取第一个就行。
    // 否则，根据你自己的启发式方法挂钩
    auto config = *std::find_if(
            supportedConfigs.get(),
            supportedConfigs.get() + numConfigs,
            [&display](const EGLConfig &config) {
                EGLint red, green, blue, depth;
                if (eglGetConfigAttrib(display, config, EGL_RED_SIZE, &red)
                    && eglGetConfigAttrib(display, config, EGL_GREEN_SIZE, &green)
                    && eglGetConfigAttrib(display, config, EGL_BLUE_SIZE, &blue)
                    && eglGetConfigAttrib(display, config, EGL_DEPTH_SIZE, &depth)) {

                    aout << "找到配置 " << red << ", " << green << ", " << blue << ", "
                         << depth << std::endl;
                    return red == 8 && green == 8 && blue == 8 && depth == 24;
                }
                return false;
            });

    aout << "找到 " << numConfigs << " 个配置" << std::endl;
    aout << "选择了 " << config << std::endl;

    // 创建合适的窗口表面
    EGLint format;
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
    EGLSurface surface = eglCreateWindowSurface(display, config, app_->window, nullptr);

    // 创建一个GLES 3上下文
    EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    EGLContext context = eglCreateContext(display, config, nullptr, contextAttribs);

    // 获取一些窗口尺寸信息
    auto madeCurrent = eglMakeCurrent(display, surface, surface, context);
    assert(madeCurrent);

    display_ = display;
    surface_ = surface;
    context_ = context;

    // 让宽度和高度无效，以便在@a updateRenderArea()的第一帧中更新
    width_ = -1;
    height_ = -1;

    PRINT_GL_STRING(GL_VENDOR);
    PRINT_GL_STRING(GL_RENDERER);
    PRINT_GL_STRING(GL_VERSION);
    PRINT_GL_STRING_AS_LIST(GL_EXTENSIONS);

    shader_ = std::unique_ptr<Shader>(
            Shader::loadShader(vertex, fragment, "inPosition", "inUV", "uProjection"));
    assert(shader_);

    // 注意：这个演示中只有一个着色器，所以我将在这里激活它。对于更复杂的游戏
    // 你要跟踪激活的着色器，并根据需要激活/停用它
    shader_->activate();

    // 设置其他任何与gl相关的全局状态
    glClearColor(CORNFLOWER_BLUE);

    // 现在为了演示先全局启用alpha，你可能不希望在游戏中这样做
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 把一些演示模型放入内存
    createModels();
}

void Renderer::updateRenderArea() {
    aout << "执行函数 updateRenderArea" << std::endl;
    EGLint width;
    eglQuerySurface(display_, surface_, EGL_WIDTH, &width);

    EGLint height;
    eglQuerySurface(display_, surface_, EGL_HEIGHT, &height);

    if (width != width_ || height != height_) {
        width_ = width;
        height_ = height;
        glViewport(0, 0, width, height);

        // 确保在我们渲染之前懒惰地重新创建投影矩阵
        shaderNeedsNewProjectionMatrix_ = true;
    }
}

/**
 * @brief 创建并初始化模型
 *
 * 本函数用于在渲染器中创建和初始化一个正方形模型。它首先定义了正方形的四个顶点及其纹理坐标，
 * 然后通过索引定义了构成正方形的两个三角形。此外，函数加载了一个名为"android_robot.png"的纹理图像，
 * 并将这个纹理应用到正方形上。最后，创建的模型会被添加到模型列表中，以便后续渲染。
 *
 * 注意：此示例没有实现纹理管理，如果重用图像，应注意避免重复加载。
 *
 * @param 无
 * @return 无
 */
void Renderer::createModels() {
    aout << "执行函数 createModels" << std::endl;
    /*
     * 这是一个正方形：
     * 0 --- 1
     * | \   |
     * |  \  |
     * |   \ |
     * 3 --- 2
     */
    std::vector<Vertex> vertices = {
            Vertex(Vector3{1, 1, 0}, Vector2{0, 0}), // 0
            Vertex(Vector3{-1, 1, 0}, Vector2{1, 0}), // 1
            Vertex(Vector3{-1, -1, 0}, Vector2{1, 1}), // 2
            Vertex(Vector3{1, -1, 0}, Vector2{0, 1}) // 3
    };
    std::vector<Index> indices = {
            0, 1, 2, 0, 2, 3
    };

    // 加载一个图像并将其分配给正方形。
    //
    // 注意：这个示例中没有纹理管理，所以如果你重用一个图像，请小心不要重复加载它。
    // 由于你得到了一个shared_ptr，你可以安全地在许多模型中重用它。
    auto assetManager = app_->activity->assetManager;
    auto spAndroidRobotTexture = TextureAsset::loadAsset(assetManager, "android_robot.png");

    // 创建一个模型并将其放在渲染列表的后面。
    models_.emplace_back(vertices, indices, spAndroidRobotTexture);
}

void Renderer::handleInput() {
    aout << "执行函数 handleInput" << std::endl;
    // 处理所有排队的输入
    auto *inputBuffer = android_app_swap_input_buffers(app_);
    if (!inputBuffer) {
        // 还没有输入。
        return;
    }

    // 处理运动事件（motionEventsCounts可以是0）。
    for (auto i = 0; i < inputBuffer->motionEventsCount; i++) {
        auto &motionEvent = inputBuffer->motionEvents[i];
        auto action = motionEvent.action;

        // 查找指针索引，掩码和位移使其变成一个可读值。
        auto pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)
                >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
        aout << "指针: ";

        // 获取这个事件的x和y位置，如果它不是ACTION_MOVE。
        auto &pointer = motionEvent.pointers[pointerIndex];
        auto x = GameActivityPointerAxes_getX(&pointer);
        auto y = GameActivityPointerAxes_getY(&pointer);

        // 确定动作类型并相应处理事件。
        switch (action & AMOTION_EVENT_ACTION_MASK) {
            case AMOTION_EVENT_ACTION_DOWN:
            case AMOTION_EVENT_ACTION_POINTER_DOWN:
                aout << "(" << pointer.id << ", " << x << ", " << y << ") "
                     << "指针按下";
                break;

            case AMOTION_EVENT_ACTION_CANCEL:
                // 将CANCEL视为UP事件：在应用程序中不执行任何操作，除了
                // 如果本地保存了指针，则从缓存中删除指针。
                // 代码故意穿透。
            case AMOTION_EVENT_ACTION_UP:
            case AMOTION_EVENT_ACTION_POINTER_UP:
                aout << "(" << pointer.id << ", " << x << ", " << y << ") "
                     << "指针抬起";
                break;

            case AMOTION_EVENT_ACTION_MOVE:
                // ACTION_MOVE没有指针索引，只有所有活动指针的快照；
                // 应用需要缓存先前的活动指针
                // 来确定哪些实际上已经移动了。
                for (auto index = 0; index < motionEvent.pointerCount; index++) {
                    pointer = motionEvent.pointers[index];
                    x = GameActivityPointerAxes_getX(&pointer);
                    y = GameActivityPointerAxes_getY(&pointer);
                    aout << "(" << pointer.id << ", " << x << ", " << y << ")";

                    if (index != (motionEvent.pointerCount - 1)) aout << ",";
                    aout << " ";
                }
                aout << "指针移动";
                break;
            default:
                aout << "未知的MotionEvent动作: " << action;
        }
        aout << std::endl;
    }
    // 清除此缓冲区中的运动输入计数，以便主线程重新使用。
    android_app_clear_motion_events(inputBuffer);

    // 处理输入键事件。
    for (auto i = 0; i < inputBuffer->keyEventsCount; i++) {
        auto &keyEvent = inputBuffer->keyEvents[i];
        aout << "键: " << keyEvent.keyCode << " ";
        switch (keyEvent.action) {
            case AKEY_EVENT_ACTION_DOWN:
                aout << "键按下";
                break;
            case AKEY_EVENT_ACTION_UP:
                aout << "键抬起";
                break;
            case AKEY_EVENT_ACTION_MULTIPLE:
                // 从Android API级别29开始已弃用。
                aout << "多个键动作";
                break;
            default:
                aout << "未知的KeyEvent动作: " << keyEvent.action;
        }
        aout << std::endl;
    }
    // 同样清除键输入计数。
    android_app_clear_key_events(inputBuffer);
}
