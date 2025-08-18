#include "ResourceLoader.h"
#include <fstream>
#include <sstream>

std::map<std::string, std::unique_ptr<sf::Texture>> ResourceLoader::textureCache;
std::map<std::string, std::unique_ptr<sf::Font>> ResourceLoader::fontCache;
std::map<std::string, std::unique_ptr<sf::SoundBuffer>> ResourceLoader::soundCache;
std::map<std::string, std::unique_ptr<sf::Shader>> ResourceLoader::shaderCache;
std::map<std::string, std::string> ResourceLoader::fileCache;
std::map<std::string, nlohmann::json> ResourceLoader::jsonCache;
std::map<std::string, std::string> ResourceLoader::aliasMap;

std::string ResourceLoader::resolvePath(const std::string& name)
{
    auto it = aliasMap.find(name);
    return (it != aliasMap.end()) ? it->second : name;
}

void ResourceLoader::registerAlias(const std::string& alias, const std::string& path)
{
    aliasMap[alias] = path;
}

void ResourceLoader::clearAliases()
{
    aliasMap.clear();
}

sf::Texture& ResourceLoader::getTexture(const std::string& filename)
{
    std::string resolved = resolvePath(filename);
    auto it = textureCache.find(resolved);
    if (it != textureCache.end())
        return *it->second;

    auto texture = std::make_unique<sf::Texture>();
    if (!texture->loadFromFile(resolved))
        throw std::runtime_error("Failed to load texture: " + resolved);

    auto& ref = *texture;
    textureCache[resolved] = std::move(texture);
    return ref;
}

sf::Font& ResourceLoader::getFont(const std::string& filename)
{
    std::string resolved = resolvePath(filename);
    auto it = fontCache.find(resolved);
    if (it != fontCache.end())
        return *it->second;

    auto font = std::make_unique<sf::Font>();
    if (!font->loadFromFile(resolved))
        throw std::runtime_error("Failed to load font: " + resolved);

    auto& ref = *font;
    fontCache[resolved] = std::move(font);
    return ref;
}

sf::SoundBuffer& ResourceLoader::getSoundBuffer(const std::string& filename)
{
    std::string resolved = resolvePath(filename);
    auto it = soundCache.find(resolved);
    if (it != soundCache.end())
        return *it->second;

    auto buffer = std::make_unique<sf::SoundBuffer>();
    if (!buffer->loadFromFile(resolved))
        throw std::runtime_error("Failed to load sound buffer: " + resolved);

    auto& ref = *buffer;
    soundCache[resolved] = std::move(buffer);
    return ref;
}

sf::Shader& ResourceLoader::getShader(const std::string& vertexFile, const std::string& fragmentFile)
{
    std::string vertexResolved = resolvePath(vertexFile);
    std::string fragmentResolved = resolvePath(fragmentFile);
    std::string key = makeShaderKey(vertexResolved, fragmentResolved);

    auto it = shaderCache.find(key);
    if (it != shaderCache.end())
        return *it->second;

    auto shader = std::make_unique<sf::Shader>();
    if (!shader->loadFromFile(vertexResolved, fragmentResolved))
        throw std::runtime_error("Failed to load shader: " + vertexResolved + " & " + fragmentResolved);

    auto& ref = *shader;
    shaderCache[key] = std::move(shader);
    return ref;
}

std::unique_ptr<sf::Music> ResourceLoader::loadMusic(const std::string& filename)
{
    std::string resolved = resolvePath(filename);
    auto music = std::make_unique<sf::Music>();
    if (!music->openFromFile(resolved))
        throw std::runtime_error("Failed to load music: " + resolved);
    return music;
}

const std::string& ResourceLoader::getFileContent(const std::string& filename)
{
    std::string resolved = resolvePath(filename);
    auto it = fileCache.find(resolved);
    if (it != fileCache.end())
        return it->second;

    std::ifstream file(resolved);
    if (!file)
        throw std::runtime_error("Failed to open file: " + resolved);

    std::ostringstream ss;
    ss << file.rdbuf();
    fileCache[resolved] = ss.str();
    return fileCache[resolved];
}

const nlohmann::json& ResourceLoader::getJson(const std::string& filename)
{
    std::string resolved = resolvePath(filename);
    auto it = jsonCache.find(resolved);
    if (it != jsonCache.end())
        return it->second;

    std::ifstream file(resolved);
    if (!file)
        throw std::runtime_error("Failed to open JSON file: " + resolved);

    try {
        nlohmann::json jsonData;
        file >> jsonData;
        jsonCache[resolved] = std::move(jsonData);
        return jsonCache[resolved];
    }
    catch (const std::exception& e) {
        throw std::runtime_error("JSON parse error in file: " + resolved + " - " + e.what());
    }
}

void ResourceLoader::clearTextures() { textureCache.clear(); }
void ResourceLoader::clearFonts() { fontCache.clear(); }
void ResourceLoader::clearSoundBuffers() { soundCache.clear(); }
void ResourceLoader::clearShaders() { shaderCache.clear(); }
void ResourceLoader::clearFiles() { fileCache.clear(); }
void ResourceLoader::clearJson() { jsonCache.clear(); }

void ResourceLoader::clearAll()
{
    clearTextures();
    clearFonts();
    clearSoundBuffers();
    clearShaders();
    clearFiles();
    clearJson();
}

std::string ResourceLoader::makeShaderKey(const std::string& vertexFile, const std::string& fragmentFile)
{
    return vertexFile + "::" + fragmentFile;
}
