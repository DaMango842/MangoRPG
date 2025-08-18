#include "TileMap.h"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

bool TileMap::loadFromJSON(const std::string& jsonPath, const std::string& tilesetImage)
{
    std::ifstream in(jsonPath);
    if (!in.is_open())
    {
        std::cerr << "Failed to open JSON: " << jsonPath << std::endl;
        return false;
    }

    json j;
    in >> j;
    json& testMap = j[0]["test_map"];

    m_mapWidth = testMap["width"];
    m_mapHeight = testMap["height"];
    m_tileWidth = testMap["tile_width"];
    m_tileHeight = testMap["tile_height"];

    if (!m_tileset.loadFromFile(tilesetImage))
    {
        std::cerr << "Failed to load tileset image!" << std::endl;
        return false;
    }

    size_t tilesetColumns = m_tileset.getSize().x / m_tileWidth;

    // 清空旧图层数据
    m_layers.clear();
    m_layerNames.clear();
    m_layerVisible.clear();
    m_tileIndicesPerLayer.clear();

    for (const auto& layer : testMap["layers"])
    {
        std::string layerName = layer.value("name", "unnamed");
        std::vector<size_t> tiles = layer["data"];

        sf::VertexArray vertices;
        vertices.setPrimitiveType(sf::Quads);
        vertices.resize(m_mapWidth * m_mapHeight * 4);

        for (size_t y = 0; y < m_mapHeight; ++y)
        {
            for (size_t x = 0; x < m_mapWidth; ++x)
            {
                size_t tileIndex = y * m_mapWidth + x;
                size_t tileNumber = tiles[tileIndex];

                if (tileNumber == 0)
                    continue;

                size_t tu = tileNumber % tilesetColumns;
                size_t tv = tileNumber / tilesetColumns;

                sf::Vertex* quad = &vertices[tileIndex * 4];

                quad[0].position = sf::Vector2f(x * m_tileWidth, y * m_tileHeight);
                quad[1].position = sf::Vector2f((x + 1) * m_tileWidth, y * m_tileHeight);
                quad[2].position = sf::Vector2f((x + 1) * m_tileWidth, (y + 1) * m_tileHeight);
                quad[3].position = sf::Vector2f(x * m_tileWidth, (y + 1) * m_tileHeight);

                quad[0].texCoords = sf::Vector2f(tu * m_tileWidth, tv * m_tileHeight);
                quad[1].texCoords = sf::Vector2f((tu + 1) * m_tileWidth, tv * m_tileHeight);
                quad[2].texCoords = sf::Vector2f((tu + 1) * m_tileWidth, (tv + 1) * m_tileHeight);
                quad[3].texCoords = sf::Vector2f(tu * m_tileWidth, (tv + 1) * m_tileHeight);
            }
        }

        m_layers.push_back(vertices);
        m_tileIndicesPerLayer.push_back(tiles);
        m_layerNames.push_back(layerName);
        m_layerVisible.push_back(true); // 默认可见
    }

    return true;
}

void TileMap::setSolidTiles(const std::unordered_set<size_t>& solidTiles)
{
    m_solidTiles = solidTiles;
}

bool TileMap::isSolidTileAt(size_t tileX, size_t tileY) const
{
    if (tileX >= m_mapWidth || tileY >= m_mapHeight)
        return false;

    auto it = std::find(m_layerNames.begin(), m_layerNames.end(), "Collision");
    if (it == m_layerNames.end())
        return false;

    size_t layerIndex = std::distance(m_layerNames.begin(), it);
    const auto& tiles = m_tileIndicesPerLayer[layerIndex];

    size_t index = tileY * m_mapWidth + tileX;
    if (index >= tiles.size())
        return false;

    size_t tileNumber = tiles[index];
    return m_solidTiles.count(tileNumber) > 0;
}

void TileMap::setLayerVisible(const std::string& layerName, bool visible)
{
    auto it = std::find(m_layerNames.begin(), m_layerNames.end(), layerName);
    if (it != m_layerNames.end())
    {
        size_t index = std::distance(m_layerNames.begin(), it);
        if (index < m_layerVisible.size())
            m_layerVisible[index] = visible;
    }
}

size_t TileMap::getTileWidth() const
{
    return m_tileWidth;
}

size_t TileMap::getTileHeight() const
{
    return m_tileHeight;
}

void TileMap::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform *= getTransform();
    states.texture = &m_tileset;

    for (size_t i = 0; i < m_layers.size(); ++i)
    {
        if (i < m_layerVisible.size() && m_layerVisible[i])
            target.draw(m_layers[i], states);
    }
}
