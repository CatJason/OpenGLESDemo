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

/**
 * 生成一个透视投影矩阵
 *
 * @param outMatrix 输出矩阵
 * @param fovY Y方向上的视场角度，以度为单位
 * @param aspect 宽高比
 * @param near 近平面距离
 * @param far 远平面距离
 * @return 生成的矩阵
 */
float *
Utility::buildPerspectiveMatrix(float *outMatrix, float fovY, float aspect, float near, float far) {
    const float DEG2RAD = 3.14159265f / 180.0f;
    // 将fovY从度转换为弧度
    float fovyInRadians = fovY * DEG2RAD;
    float f = 1.0f / tan(fovyInRadians / 2.0f); // 焦距
    float rangeInv = 1.0f / (near - far);

    outMatrix[0] = f / aspect;
    outMatrix[1] = 0.0f;
    outMatrix[2] = 0.0f;
    outMatrix[3] = 0.0f;

    outMatrix[4] = 0.0f;
    outMatrix[5] = f;
    outMatrix[6] = 0.0f;
    outMatrix[7] = 0.0f;

    outMatrix[8] = 0.0f;
    outMatrix[9] = 0.0f;
    outMatrix[10] = (far + near) * rangeInv;
    outMatrix[11] = -1.0f;

    outMatrix[12] = 0.0f;
    outMatrix[13] = 0.0f;
    outMatrix[14] = 2.0f * far * near * rangeInv;
    outMatrix[15] = 0.0f;

    return outMatrix;
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

/**
     * 构建一个绕Z轴旋转的矩阵。
     *
     * @param matrix 存储结果的4x4矩阵。
     * @param angleDegrees 旋转的角度，以度为单位。
     */
void Utility::buildRotationMatrix(float *pDouble, float angle) {
    float angleRadians = angle * (M_PI / 180.0f); // 将角度转换为弧度
    float cosAngle = cos(angleRadians);
    float sinAngle = sin(angleRadians);

    // 清空矩阵
    std::fill_n(pDouble, 16, 0.0f);

    // 填充旋转矩阵
    pDouble[0] = cosAngle;
    pDouble[1] = sinAngle;
    pDouble[4] = -sinAngle;
    pDouble[5] = cosAngle;
    pDouble[10] = 1; // Z轴不变
    pDouble[15] = 1; // 齐次坐标保持不变
}

/**
 * 乘法两个4x4矩阵。
 *
 * @param a 第一个4x4矩阵。
 * @param b 第二个4x4矩阵。
 * @param result 结果4x4矩阵。
 */
void multiplyMatrices(const float* a, const float* b, float* result) {
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            result[col * 4 + row] = 0.0f;
            for (int k = 0; k < 4; k++) {
                result[col * 4 + row] += a[k * 4 + row] * b[col * 4 + k];
            }
        }
    }
}


void Utility::buildRotationMatrix3D(float *matrix, float angleXDegrees, float angleYDegrees, float angleZDegrees) {
    float angleXRadians = angleXDegrees * (M_PI / 180.0f);
    float angleYRadians = angleYDegrees * (M_PI / 180.0f);
    float angleZRadians = angleZDegrees * (M_PI / 180.0f);

    float cosX = cos(angleXRadians);
    float sinX = sin(angleXRadians);
    float cosY = cos(angleYRadians);
    float sinY = sin(angleYRadians);
    float cosZ = cos(angleZRadians);
    float sinZ = sin(angleZRadians);

    // 绕X轴旋转的矩阵
    float rotateX[16] = {
            1,    0,     0,    0,
            0,  cosX, -sinX,  0,
            0,  sinX,  cosX,  0,
            0,    0,     0,    1
    };

    // 绕Y轴旋转的矩阵
    float rotateY[16] = {
            cosY,  0,  sinY,  0,
            0,    1,    0,  0,
            -sinY,  0,  cosY,  0,
            0,    0,    0,  1
    };

    // 绕Z轴旋转的矩阵
    float rotateZ[16] = {
            cosZ, -sinZ,  0,  0,
            sinZ,  cosZ,  0,  0,
            0,     0,  1,  0,
            0,     0,  0,  1
    };

    // 计算综合的旋转矩阵，顺序为Y*X*Z
    float tempMatrix[16]; // 用于存储中间结果
    // 首先，Y * X
    multiplyMatrices(rotateY, rotateX, tempMatrix);
    // 然后，(Y * X) * Z
    multiplyMatrices(tempMatrix, rotateZ, matrix);
}