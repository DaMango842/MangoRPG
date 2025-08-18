#pragma once

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Audio/Music.hpp>

#include <map>
#include <memory>
#include <string>
#include <stdexcept>

#include <nlohmann/json.hpp>

class ResourceLoader
{
public:
    ResourceLoader() = delete;
    ~ResourceLoader() = delete;

    // --- SFML 资源获取 ---
    static sf::Texture& getTexture(const std::string& filename);
    static sf::Font& getFont(const std::string& filename);
    static sf::SoundBuffer& getSoundBuffer(const std::string& filename);
    static sf::Shader& getShader(const std::string& vertexFile, const std::string& fragmentFile);
    static std::unique_ptr<sf::Music> loadMusic(const std::string& filename);

    // --- 文件与 JSON ---
    static const std::string& getFileContent(const std::string& filename);
    static const nlohmann::json& getJson(const std::string& filename);

    // --- 别名支持（可选使用）---
    static void registerAlias(const std::string& alias, const std::string& path);
    static void clearAliases();

    // --- 清理缓存 ---
    static void clearTextures();
    static void clearFonts();
    static void clearSoundBuffers();
    static void clearShaders();
    static void clearFiles();
    static void clearJson();
    static void clearAll();

private:
    static std::string resolvePath(const std::string& name);
    static std::string makeShaderKey(const std::string& vertexFile, const std::string& fragmentFile);

    static std::map<std::string, std::unique_ptr<sf::Texture>> textureCache;
    static std::map<std::string, std::unique_ptr<sf::Font>> fontCache;
    static std::map<std::string, std::unique_ptr<sf::SoundBuffer>> soundCache;
    static std::map<std::string, std::unique_ptr<sf::Shader>> shaderCache;
    static std::map<std::string, std::string> fileCache;
    static std::map<std::string, nlohmann::json> jsonCache;
    static std::map<std::string, std::string> aliasMap;
};
