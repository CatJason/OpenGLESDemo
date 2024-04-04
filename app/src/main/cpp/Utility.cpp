#include "Utility.h"
#include "AndroidOut.h"

#include <GLES3/gl3.h>

// 宏定义，用于检查OpenGL错误并打印
#define CHECK_ERROR(e) case e: aout << "GL错误: "#e << std::endl; break;

// 检查并记录OpenGL错误的函数，如果alwaysLog为真，即使没有错误也会记录
bool Utility::checkAndLogGlError(bool alwaysLog) {
    aout << "执行函数 checkAndLogGlError" << std::endl;
    GLenum error = glGetError();
    if (error == GL_NO_ERROR) {
        if (alwaysLog) {
            aout << "无GL错误" << std::endl;
        }
        return true;
    } else {
        switch (error) {
            CHECK_ERROR(GL_INVALID_ENUM); // 无效的枚举值错误
            CHECK_ERROR(GL_INVALID_VALUE); // 无效的值错误
            CHECK_ERROR(GL_INVALID_OPERATION); // 无效的操作错误
            CHECK_ERROR(GL_INVALID_FRAMEBUFFER_OPERATION); // 无效的帧缓冲操作错误
            CHECK_ERROR(GL_OUT_OF_MEMORY); // 内存不足错误
            default:
                aout << "未知GL错误: " << error << std::endl;
        }
        return false;
    }
}

// 构建正交投影矩阵的函数
float *
Utility::buildOrthographicMatrix(float *outMatrix, float halfHeight, float aspect, float near,
                                 float far) {
    aout << "执行函数 buildOrthographicMatrix" << std::endl;
    float halfWidth = halfHeight * aspect; // 通过纵横比和半高计算半宽

    // 第1列
    outMatrix[0] = 1.f / halfWidth;
    outMatrix[1] = 0.f;
    outMatrix[2] = 0.f;
    outMatrix[3] = 0.f;

    // 第2列
    outMatrix[4] = 0.f;
    outMatrix[5] = 1.f / halfHeight;
    outMatrix[6] = 0.f;
    outMatrix[7] = 0.f;

    // 第3列
    outMatrix[8] = 0.f;
    outMatrix[9] = 0.f;
    outMatrix[10] = -2.f / (far - near);
    outMatrix[11] = -(far + near) / (far - near);

    // 第4列
    outMatrix[12] = 0.f;
    outMatrix[13] = 0.f;
    outMatrix[14] = 0.f;
    outMatrix[15] = 1.f;

    return outMatrix;
}

// 构建单位矩阵的函数
float *Utility::buildIdentityMatrix(float *outMatrix) {
    aout << "执行函数 buildIdentityMatrix" << std::endl;
    // 第1列
    outMatrix[0] = 1.f;
    outMatrix[1] = 0.f;
    outMatrix[2] = 0.f;
    outMatrix[3] = 0.f;

    // 第2列
    outMatrix[4] = 0.f;
    outMatrix[5] = 1.f;
    outMatrix[6] = 0.f;
    outMatrix[7] = 0.f;

    // 第3列
    outMatrix[8] = 0.f;
    outMatrix[9] = 0.f;
    outMatrix[10] = 1.f;
    outMatrix[11] = 0.f;

    // 第4列
    outMatrix[12] = 0.f;
    outMatrix[13] = 0.f;
    outMatrix[14] = 0.f;
    outMatrix[15] = 1.f;

    return outMatrix;
}
