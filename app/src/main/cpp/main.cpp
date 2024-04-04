#include <jni.h>

#include "AndroidOut.h"
#include "Renderer.h"

#include <game-activity/GameActivity.cpp>
#include <game-text-input/gametextinput.cpp>

extern "C" {

#include <game-activity/native_app_glue/android_native_app_glue.c>

/*!
 * 处理发送到此Android应用的命令
 * @param pApp 发送命令的应用
 * @param cmd 要处理的命令
 */
void handle_cmd(android_app *pApp, int32_t cmd) {
    aout << "执行函数 handle_cmd" << std::endl;
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            // 创建了一个新窗口，与之关联一个渲染器。你可以根据需要用“游戏”类替换这个渲染器。
            // 如果你改变了这里的类，请记得在android_main函数和APP_CMD_TERM_WINDOW处理器中
            // 更改所有userData的实例，因为reinterpret_cast在这里使用是危险的。
            pApp->userData = new Renderer(pApp);
            break;
        case APP_CMD_TERM_WINDOW:
            // 窗口正在被销毁。使用此机会清理你的userData以避免资源泄露。
            //
            // 我们需要检查userData是否已分配，以防这个命令来得特别快
            if (pApp->userData) {
                //
                auto *pRenderer = reinterpret_cast<Renderer *>(pApp->userData);
                pApp->userData = nullptr;
                delete pRenderer;
            }
            break;
        default:
            break;
    }
}

/*!
 * 启用你想要处理的移动事件；未处理的事件会被返回给OS以供进一步处理。对于这个示例，
 * 只启用了指针和操纵杆设备的事件。
 *
 * @param motionEvent 新到达的GameActivityMotionEvent。
 * @return 如果事件来自指针或操纵杆设备，则为true，
 *         对于所有其他输入设备，为false。
 */
bool motion_event_filter_func(const GameActivityMotionEvent *motionEvent) {
    aout << "欢迎来到 motion_event_filter_func" << std::endl;
    auto sourceClass = motionEvent->source & AINPUT_SOURCE_CLASS_MASK;
    return (sourceClass == AINPUT_SOURCE_CLASS_POINTER ||
            sourceClass == AINPUT_SOURCE_CLASS_JOYSTICK);
}

/*!
 * 这是原生活动的主入口点
 */
void android_main(struct android_app *pApp) {
    // 可以移除，用于确保你的代码正在运行
    aout << "欢迎来到 android_main" << std::endl;

    // 为Android事件注册一个事件处理器
    pApp->onAppCmd = handle_cmd;

    // 设置输入事件过滤器（如果应用想要处理所有输入，则将其设置为NULL）。
    // 注意，对于按键输入，这个示例使用在android_native_app_glue.c中实现的默认default_key_filter()。
    android_app_set_motion_event_filter(pApp, motion_event_filter_func);

    // 这设置了一个典型的游戏/事件循环。它将运行直到应用被销毁。
    int events;
    android_poll_source *pSource;
    do {
        // 在运行游戏逻辑之前处理所有待处理的事件。
        if (ALooper_pollAll(0, nullptr, &events, (void **) &pSource) >= 0) {
            if (pSource) {
                pSource->process(pApp, pSource);
            }
        }

        // 检查是否有任何用户数据关联。这是在handle_cmd中分配的
        if (pApp->userData) {

            // 我们知道我们的用户数据是一个Renderer，因此重新解释转换它。如果你更改了你的
            // 用户数据，请记得在这里更改它
            auto *pRenderer = reinterpret_cast<Renderer *>(pApp->userData);

            // 处理游戏输入
            pRenderer->handleInput();

            // 渲染一帧
            pRenderer->render();
        }
    } while (!pApp->destroyRequested);
}
}
