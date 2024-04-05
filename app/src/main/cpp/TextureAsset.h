#ifndef ANDROIDGLINVESTIGATIONS_TEXTUREASSET_H
#define ANDROIDGLINVESTIGATIONS_TEXTUREASSET_H

#include <memory>
#include <android/asset_manager.h>
#include <GLES3/gl3.h>
#include <string>
#include <vector>

// 纹理资源类
class TextureAsset {
public:
    /*!
     * 从assets/目录加载一个纹理资源
     * @param assetManager 用于加载资源的AssetManager
     * @param assetPath 资源的路径
     * @return 返回一个纹理资源的共享指针，资源会在清理时被回收
     */
    static std::shared_ptr<TextureAsset>
    loadAsset(AAssetManager *assetManager, const std::string &assetPath);

    ~TextureAsset(); // 析构函数，用于资源清理

    /*!
     * @return 返回用于OpenGL的纹理ID
     */
    constexpr GLuint getTextureID() const { return textureID_; }

    // 创建一个单色的纹理
    static std::shared_ptr<TextureAsset> createSolidColorTexture(GLubyte r, GLubyte g, GLubyte b, GLubyte a) {
        GLuint textureId;
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // 创建一个1x1像素的纹理
        GLubyte pixel[4] = {r, g, b, a};
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

        // 设置纹理参数
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // 解绑纹理
        glBindTexture(GL_TEXTURE_2D, 0);

        // 创建并返回TextureAsset对象
        return create(textureId);
    }

private:
    inline TextureAsset(GLuint textureId) : textureID_(textureId) {} // 构造函数，私有化以限制创建方式
    static std::shared_ptr<TextureAsset> create(GLuint textureId) {
        return std::shared_ptr<TextureAsset>(new TextureAsset(textureId));
    }

    GLuint textureID_; // OpenGL纹理ID
};

#endif //ANDROIDGLINVESTIGATIONS_TEXTUREASSET_H
