#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <stdexcept>

class Window {
public:
    static Window& getInstance() {
        static Window instance;
        return instance;
    }

    void create(const sf::VideoMode& mode, const sf::String& title,
        bool fullscreen = false,
        const sf::ContextSettings& settings = sf::ContextSettings()) {
        m_isFullscreen = fullscreen;
        m_windowTitle = title;
        m_settings = settings;

        if (!fullscreen)
            m_windowedMode = mode;

        m_videoMode = fullscreen ? sf::VideoMode::getDesktopMode() : m_windowedMode;

        createWindow();
    }

    void toggleFullscreen() {
        if (!m_window) return;

        // 延迟应用切换，避免频繁重建
        m_isFullscreen = !m_isFullscreen;
        m_needsRecreation = true;
    }

    void setResolution(const sf::VideoMode& newMode) {
        if (!m_window || m_isFullscreen) return;

        if (newMode.width == m_windowedMode.width && newMode.height == m_windowedMode.height)
            return; // 避免重复设置相同分辨率

        m_windowedMode = newMode;
        m_videoMode = newMode;

        // 直接修改窗口大小，不重建窗口，性能好
        m_window->setSize(sf::Vector2u(newMode.width, newMode.height));

        // 更新视图，保证内容适配窗口
        sf::View view(sf::FloatRect(0, 0, static_cast<float>(newMode.width), static_cast<float>(newMode.height)));
        m_window->setView(view);
    }

    // 主循环调用，应用任何待处理的窗口重建请求
    void applyPendingChanges() {
        if (m_needsRecreation && m_window) {
            m_window->close();
            m_videoMode = m_isFullscreen ? sf::VideoMode::getDesktopMode() : m_windowedMode;
            createWindow();
            m_needsRecreation = false;

            // 重建窗口后可选设置刷新率、视图等
            if (m_vsyncEnabled) {
                m_window->setVerticalSyncEnabled(true);
            }
            if (m_framerateLimit > 0) {
                m_window->setFramerateLimit(m_framerateLimit);
            }
        }
    }

    void setVSyncEnabled(bool enabled) {
        m_vsyncEnabled = enabled;
        if (m_window) m_window->setVerticalSyncEnabled(enabled);
    }

    void setFramerateLimit(unsigned limit) {
        m_framerateLimit = limit;
        if (m_window) m_window->setFramerateLimit(limit);
    }

    sf::RenderWindow& getWindow() {
        if (!m_window)
            throw std::runtime_error("Window has not been created.");
        return *m_window;
    }

    bool isOpen() const {
        return m_window && m_window->isOpen();
    }

    void close() {
        if (m_window) m_window->close();
    }

    void destroy() {
        m_window.reset();
    }

    void clear(const sf::Color& color = sf::Color::Black) {
        if (m_window) m_window->clear(color);
    }

    void display() {
        if (m_window) m_window->display();
    }

    bool pollEvent(sf::Event& event) {
        return m_window && m_window->pollEvent(event);
    }

    bool isFullscreen() const {
        return m_isFullscreen;
    }

    sf::VideoMode getCurrentResolution() const {
        return m_videoMode;
    }

    const sf::VideoMode& getWindowedMode() const {
        return m_windowedMode;
    }

private:
    Window() = default;
    ~Window() = default;

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    void createWindow() {
        sf::Uint32 style = m_isFullscreen ? sf::Style::Fullscreen : sf::Style::Default;
        m_window = std::make_unique<sf::RenderWindow>(m_videoMode, m_windowTitle, style, m_settings);

        // 创建后设置刷新率和视图（防止丢失）
        if (m_vsyncEnabled) {
            m_window->setVerticalSyncEnabled(true);
        }
        if (m_framerateLimit > 0) {
            m_window->setFramerateLimit(m_framerateLimit);
        }

        sf::View view(sf::FloatRect(0, 0, static_cast<float>(m_videoMode.width), static_cast<float>(m_videoMode.height)));
        m_window->setView(view);
    }

    std::unique_ptr<sf::RenderWindow> m_window;

    bool m_isFullscreen = false;
    bool m_needsRecreation = false;

    bool m_vsyncEnabled = false;
    unsigned m_framerateLimit = 0;

    sf::VideoMode m_videoMode;
    sf::VideoMode m_windowedMode;
    sf::String m_windowTitle;
    sf::ContextSettings m_settings;
};
