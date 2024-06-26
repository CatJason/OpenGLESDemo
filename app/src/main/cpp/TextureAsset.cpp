#include "TextureAsset.h"
#include "AndroidOut.h"
#include "Utility.h"
#include "jsoncpp/json/json.h"

#include <android/imagedecoder.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>

// 加载资源的函数，使用共享指针管理TextureAsset资源
std::shared_ptr<TextureAsset>
TextureAsset::loadAsset(AAssetManager *assetManager, const std::string &assetPath) {
    aout << "执行函数 loadAsset" << std::endl;
    // 从资产管理器获取图像
    auto pAndroidRobotPng = AAssetManager_open(
            assetManager,
            assetPath.c_str(),
            AASSET_MODE_BUFFER);

    // 创建解码器将其转换为纹理
    AImageDecoder *pAndroidDecoder = nullptr;
    auto result = AImageDecoder_createFromAAsset(pAndroidRobotPng, &pAndroidDecoder);
    assert(result == ANDROID_IMAGE_DECODER_SUCCESS);

    // 确保输出是8位每通道的RGBA格式
    AImageDecoder_setAndroidBitmapFormat(pAndroidDecoder, ANDROID_BITMAP_FORMAT_RGBA_8888);

    // 获取图像头信息，帮助进行设置
    const AImageDecoderHeaderInfo *pAndroidHeader = nullptr;
    pAndroidHeader = AImageDecoder_getHeaderInfo(pAndroidDecoder);

    // 重要的度量用于发送到GL
    auto width = AImageDecoderHeaderInfo_getWidth(pAndroidHeader);
    auto height = AImageDecoderHeaderInfo_getHeight(pAndroidHeader);
    auto stride = AImageDecoder_getMinimumStride(pAndroidDecoder);

    // 获取图像的位图数据
    auto upAndroidImageData = std::make_unique<std::vector<uint8_t>>(height * stride);
    auto decodeResult = AImageDecoder_decodeImage(
            pAndroidDecoder,
            upAndroidImageData->data(),
            stride,
            upAndroidImageData->size());
    assert(decodeResult == ANDROID_IMAGE_DECODER_SUCCESS);

    // 获取OpenGL纹理
    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    // 设置为边缘紧贴，如果不这样做在进行Alpha混合时会得到奇怪的结果
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 将纹理加载到显存
    glTexImage2D(
            GL_TEXTURE_2D, // 目标
            0, // mip级别
            GL_RGBA, // 内部格式，通常建议使用BGR
            width, // 纹理宽度
            height, // 纹理高度
            0, // 边界（总是0）
            GL_RGBA, // 格式
            GL_UNSIGNED_BYTE, // 类型
            upAndroidImageData->data() // 上传的数据
    );

    // 生成mip层级。对于2D来说不是很需要，但最好这样做
    glGenerateMipmap(GL_TEXTURE_2D);

    // 清理辅助工具
    AImageDecoder_delete(pAndroidDecoder);
    AAsset_close(pAndroidRobotPng);

    // 创建共享指针，以便易于/自动清理
    return std::shared_ptr<TextureAsset>(new TextureAsset(textureId));
}

void TextureAsset::processGltfFile(const std::string& gltfFilename) {
    std::ifstream jsonFile(gltfFilename, std::ios::binary);
    Json::Value json;

    try {
        jsonFile >> json;
    } catch (const std::exception& e) {
        std::cerr << "Json parsing error: " << e.what() << std::endl;
    }
    jsonFile.close();

    std::string binFilename = json["buffers"][0]["uri"].asString();
    std::ifstream binFile(binFilename, std::ios::binary | std::ios::ate);
    size_t binLength = binFile.tellg();
    binFile.seekg(0);

    std::vector<char> bin(binLength);
    binFile.read(bin.data(), binLength);
    binFile.close();

    Json::Value& primitive = json["meshes"][0]["primitives"][0];
    Json::Value& positionAccessor = json["accessors"][primitive["attributes"]["POSITION"].asInt()];
    Json::Value& bufferView = json["bufferViews"][positionAccessor["bufferView"].asInt()];

    float* buffer = (float*)(bin.data() + bufferView["byteOffset"].asInt());

    for (int i = 0; i < positionAccessor["count"].asInt(); ++i) {
        std::cout << "(" << buffer[i*3] << ", " << buffer[i*3 + 1] << ", " << buffer[i*3 + 2] << ")" << std::endl;
    }

    std::cout << "Vertices: " << positionAccessor["count"].asInt() << std::endl;
}

void TextureAsset::processGlbFile(const std::string& glbFilename) {
    std::ifstream binFile(glbFilename, std::ios::binary);
    binFile.seekg(12);

    uint32_t jsonLength;
    binFile.read(reinterpret_cast<char*>(&jsonLength), sizeof(jsonLength));

    std::string jsonStr(jsonLength, ' ');
    binFile.seekg(20);
    binFile.read(&jsonStr[0], jsonLength);

    Json::Value json;
    Json::Reader reader;
    if (!reader.parse(jsonStr, json)) {
        std::cerr << "Problem parsing json: " << jsonStr << std::endl;
    }

    uint32_t binLength;
    binFile.read(reinterpret_cast<char*>(&binLength), sizeof(binLength));
    binFile.seekg(sizeof(uint32_t), std::ios_base::cur);

    std::vector<char> bin(binLength);
    binFile.read(bin.data(), binLength);

    Json::Value& primitive = json["meshes"][0]["primitives"][0];
    Json::Value& positionAccessor = json["accessors"][primitive["attributes"]["POSITION"].asInt()];
    Json::Value& bufferView = json["bufferViews"][positionAccessor["bufferView"].asInt()];

    float* buffer = (float*)(bin.data() + bufferView["byteOffset"].asInt());

    for (int i = 0; i < positionAccessor["count"].asInt(); ++i) {
        std::cout << "(" << buffer[i*3] << ", " << buffer[i*3 + 1] << ", " << buffer[i*3 + 2] << ")" << std::endl;
    }

    std::cout << "Vertices: " << positionAccessor["count"].asInt() << std::endl;
}

TextureAsset::~TextureAsset() {
    aout << "执行函数 ~TextureAsset" << std::endl;
    // 释放纹理资源
    glDeleteTextures(1, &textureID_);
    textureID_ = 0;
}
