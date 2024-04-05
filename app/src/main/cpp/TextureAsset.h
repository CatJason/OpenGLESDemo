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

private:
    inline TextureAsset(GLuint textureId) : textureID_(textureId) {} // 构造函数，私有化以限制创建方式

    GLuint textureID_; // OpenGL纹理ID
};

#endif //ANDROIDGLINVESTIGATIONS_TEXTUREASSET_H
