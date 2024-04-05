#ifndef ANDROIDGLINVESTIGATIONS_UTILITY_H
#define ANDROIDGLINVESTIGATIONS_UTILITY_H

#include <cassert>

// 通用工具类，提供静态方法用于OpenGL相关的操作和矩阵生成
class Utility {
public:
    /**
     * 检查并记录OpenGL错误
     *
     * @param alwaysLog 是否总是记录日志，即使没有发生错误
     * @return 如果没有错误发生返回true，否则返回false
     */
    static bool checkAndLogGlError(bool alwaysLog = false);

    /**
     * 断言OpenGL错误，如果有错误发生则触发断言
     */
    static inline void assertGlError() { assert(checkAndLogGlError()); }

    /**
     * 生成一个正交投影矩阵，给定半高、宽高比、近平面和远平面
     *
     * @param outMatrix 要写入的矩阵
     * @param halfHeight 屏幕高度的一半
     * @param aspect 屏幕宽度除以高度的比值
     * @param near 近平面的距离
     * @param far 远平面的距离
     * @return 生成的矩阵，这将和@a outMatrix相同，因此如果需要可以进行调用链式操作
     */
    static float *buildOrthographicMatrix(
            float *outMatrix,
            float halfHeight,
            float aspect,
            float near,
            float far);

    /**
     * 生成一个单位矩阵
     *
     * @param outMatrix 要写入的矩阵
     * @return 生成的单位矩阵
     */
    static float *buildIdentityMatrix(float *outMatrix);

    static void buildRotationMatrix(float pDouble[16], float angle);
};

#endif //ANDROIDGLINVESTIGATIONS_UTILITY_H
