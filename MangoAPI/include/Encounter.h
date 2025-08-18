#pragma once

#include <SFML/Graphics.hpp>
#include "Random.hpp"
#include "TileMap.h"
#include "Player.h"

class Encounter
{
public:
    Encounter(float baseProb = 0.05f, float stepIncrement = 0.01f, float maxProb = 0.8f)
        : base_probability(baseProb),
        step_increment(stepIncrement),
        max_probability(maxProb) {
    }

    bool update(const Player& player, const TileMap& tileMap)
    {
        if (isEncounterArea(player.getPosition(), tileMap)) {
            encounter_step++;
            if (Random::chance(calculateProbability())) {
                reset();
                return true;
            }
        }
        return false;
    }

    void reset() { encounter_step = 0; }

    // 设置方法
    void setEncounterTiles(const std::unordered_set<size_t>& tiles) { encounter_tiles = tiles; }
    void setEncounterLayer(size_t layer) { encounter_layer = layer; }

    // 获取方法
    unsigned int getStepCount() const { return encounter_step; }
    float getCurrentProbability() const { return calculateProbability(); }

private:
    float calculateProbability() const
    {
        return std::min(base_probability + (encounter_step * step_increment), max_probability);
    }

    bool isEncounterArea(const sf::Vector2f& position, const TileMap& tileMap) const
    {
        size_t tileX = static_cast<size_t>(position.x / tileMap.getTileWidth());
        size_t tileY = static_cast<size_t>(position.y / tileMap.getTileHeight());

        // 获取地图尺寸（以图块为单位）
        size_t mapWidth = static_cast<size_t>(tileMap.getMapSize().x / tileMap.getTileWidth());
        size_t mapHeight = static_cast<size_t>(tileMap.getMapSize().y / tileMap.getTileHeight());

        if (tileX >= mapWidth || tileY >= mapHeight) {
            return false;
        }

        // 直接检查当前图块是否在遇敌图块集合中
        size_t tileID = tileMap.getTileID(tileX, tileY, encounter_layer);
        return encounter_tiles.find(tileID) != encounter_tiles.end();
    }

private:
    unsigned int encounter_step = 0;
    float base_probability;
    float step_increment;
    float max_probability;

    std::unordered_set<size_t> encounter_tiles; // 可遇敌的图块ID集合
    size_t encounter_layer = 0; // 默认检查第0层
};
