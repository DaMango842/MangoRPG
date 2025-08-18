#pragma once

#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include <unordered_set>

// TileMap
class TileMap : public sf::Drawable, public sf::Transformable
{
public:
    bool loadFromJSON(const std::string& jsonPath, const std::string& tilesetImage);
    void setSolidTiles(const std::unordered_set<size_t>& solidTiles);
    bool isSolidTileAt(size_t tileX, size_t tileY) const;
    void setLayerVisible(const std::string& layerName, bool visible);

    size_t getTileWidth() const;
    size_t getTileHeight() const;

    // 计算地图整体尺寸
    sf::Vector2f getMapSize() const {
        return sf::Vector2f(static_cast<float>(m_tileWidth * m_mapWidth), static_cast<float>(m_tileHeight * m_mapHeight));
    }

    // 获取指定图层上 (x,y) 位置的图块ID，默认图层为0
    size_t getTileID(size_t x, size_t y, size_t layerIdx = 0) const
    {
        // 图层索引越界或图层数据为空，返回0
        if (layerIdx >= m_tileIndicesPerLayer.size() || m_tileIndicesPerLayer.empty())
            return 0;

        // 坐标越界，返回0
        if (x >= m_mapWidth || y >= m_mapHeight)
            return 0;

        const auto& layer = m_tileIndicesPerLayer[layerIdx];
        size_t index = y * m_mapWidth + x;

        // 防止意外越界
        if (index >= layer.size())
            return 0;

        return layer[index];
    }

    const std::vector<size_t>& getLayerTileIndices(size_t layerIndex) const
    {
        assert(layerIndex < m_tileIndicesPerLayer.size());
        return m_tileIndicesPerLayer[layerIndex];
    }


    size_t getTileIndexAt(size_t layer, size_t x, size_t y) const {
        // 参数校验...
        return m_tileIndicesPerLayer[layer][y * m_mapWidth + x];
    }

private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    sf::Texture m_tileset;
    size_t m_tileWidth = 0;
    size_t m_tileHeight = 0;
    size_t m_mapWidth = 0;
    size_t m_mapHeight = 0;

    std::vector<std::string> m_layerNames;
    std::vector<sf::VertexArray> m_layers;
    std::vector<std::vector<size_t>> m_tileIndicesPerLayer;
    std::vector<bool> m_layerVisible;

    std::unordered_set<size_t> m_solidTiles;
};
