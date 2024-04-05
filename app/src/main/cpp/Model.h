#ifndef ANDROIDGLINVESTIGATIONS_MODEL_H
#define ANDROIDGLINVESTIGATIONS_MODEL_H

#include <vector>
#include "TextureAsset.h" // 引入纹理资产的头文件

// 三维向量或位置数据的表示
union Vector3 {
    struct {
        float x, y, z; // 代表三维空间中的坐标或向量的分量
    };
    float idx[3]; // 通过索引访问相同的数据
};

// 二维向量，常用于纹理坐标
union Vector2 {
    struct {
        float x, y; // 二维空间中的点或向量的通用表示
    };
    struct {
        float u, v; // 纹理坐标的表示，u为纹理的水平坐标，v为垂直坐标
    };
    float idx[2]; // 通过索引访问相同的数据
};

struct Vector4 {
    float r, g, b, a;
};

struct Vertex {
    Vector3 position; // Vertex position
    Vector2 uv;       // Texture coordinates
    Vector4 color;    // Vertex color

    // Updated constructor to include color initialization
    Vertex(const Vector3 &inPosition, const Vector2 &inUV)
            : position(inPosition), uv(inUV) {}
};

typedef uint16_t Index; // 定义索引类型，用于索引缓冲

// 模型类，包含顶点、索引和纹理资产
class Model {
public:
    // 模型的构造函数，接收顶点列表、索引列表和纹理资源
    inline Model(
            std::vector<Vertex> vertices,
            std::vector<Index> indices,
            std::shared_ptr<TextureAsset> spTexture)
            : vertices_(std::move(vertices)),
              indices_(std::move(indices)),
              spTexture_(std::move(spTexture)) {}

    // 获取顶点数据的只读访问方法
    inline const Vertex *getVertexData() const {
        return vertices_.data();
    }

    // 获取索引数量的方法
    inline const size_t getIndexCount() const {
        return indices_.size();
    }

    // 获取索引数据的只读访问方法
    inline const Index *getIndexData() const {
        return indices_.data();
    }

    // 获取纹理资源的方法
    inline const TextureAsset &getTexture() const {
        return *spTexture_;
    }

private:
    std::vector<Vertex> vertices_; // 存储模型顶点的容器
    std::vector<Index> indices_;   // 存储顶点索引的容器
    std::shared_ptr<TextureAsset> spTexture_; // 模型纹理的智能指针
};

#endif //ANDROIDGLINVESTIGATIONS_MODEL_H
