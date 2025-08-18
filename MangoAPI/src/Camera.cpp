#include "Camera.h"
#include "Window.h"
#include <cmath>
#include <random>
#include <algorithm>

Camera::Camera(const sf::Vector2f& size)
{
    setSizeAndCenter(size);
}

void Camera::setCenter(const sf::Vector2f& center)
{
    m_view.setCenter(center);
    LOG_DEBUG("Camera center set to: {}, {}", center.x, center.y);
}

void Camera::move(const sf::Vector2f& offset)
{
    m_view.move(offset);
}

void Camera::zoom(float factor)
{
    m_view.zoom(factor);
}

void Camera::setTarget(const sf::Vector2f* target)
{
    m_target = target;
    LOG_INFO("Camera target set.");
}

void Camera::setTarget(const sf::Vector2f& target)
{
    m_target = &target;
    LOG_INFO("Camera target set (by ref).");
}

void Camera::setBounds(const sf::FloatRect& bounds)
{
    m_bounds = bounds;
    LOG_INFO("Camera bounds set: {}x{}", bounds.width, bounds.height);
}

void Camera::setSize(const sf::Vector2f& value)
{
    m_view.setSize(value);
    m_view.setCenter(value / 2.f);
}

void Camera::setSizeAndCenter(const sf::Vector2f& value)
{
    m_view.setSize(value);
    m_view.setCenter(value / 2.f);
    LOG_INFO("Camera size and center set to: {}, {}", value.x, value.y);
}

void Camera::shake(float duration, float intensity)
{
    m_shakeDuration = duration;
    m_shakeTime = duration;
    m_shakeIntensity = intensity;
    LOG_WARN("Camera shake started. Duration: {}, Intensity: {}", duration, intensity);
}

void Camera::update(float deltaTime)
{
    sf::Vector2f center = m_view.getCenter();

    if (m_target)
    {
        sf::Vector2f targetCenter = *m_target;
        center += (targetCenter - center) * m_lerpSpeed * deltaTime;
    }

    center += getShakenOffset();

    if (m_bounds.has_value())
    {
        const auto& b = m_bounds.value();
        sf::Vector2f halfSize = m_view.getSize() / 2.f;

        float minX = b.left + halfSize.x;
        float maxX = b.left + b.width - halfSize.x;
        float minY = b.top + halfSize.y;
        float maxY = b.top + b.height - halfSize.y;

        bool validX = minX <= maxX;
        bool validY = minY <= maxY;

        if (!validX || !validY)
        {
            LOG_WARN("Invalid clamp range, skipping clamping.");
        }
        else
        {
            center.x = std::clamp(center.x, minX, maxX);
            center.y = std::clamp(center.y, minY, maxY);
        }
    }

    m_view.setCenter(center);

    if (m_shakeTime > 0.f)
        m_shakeTime -= deltaTime;
}

void Camera::applyTo(sf::RenderTarget& target) const
{
    target.setView(m_view);
}

void Camera::resetCamera()
{
    auto& window = Window::getInstance().getWindow();
    sf::Vector2f size = static_cast<sf::Vector2f>(window.getSize());
    setSizeAndCenter(size);
    LOG_DEBUG("Reset camera with window size: {} x {}", size.x, size.y);
}

void Camera::resetCamera(const sf::Vector2f& size)
{
    setSizeAndCenter(size);
    LOG_DEBUG("Reset camera with window size: {} x {}", size.x, size.y);
}

sf::Vector2f Camera::getShakenOffset() const
{
    if (m_shakeTime <= 0.f)
        return {};

    static std::default_random_engine engine(std::random_device{}());
    static std::uniform_real_distribution<float> dist(-1.f, 1.f);

    float strength = m_shakeIntensity * (m_shakeTime / m_shakeDuration);
    return { dist(engine) * strength, dist(engine) * strength };
}

bool Camera::isViewWithinBounds() const
{
    if (!m_bounds.has_value()) return true;

    auto& center = m_view.getCenter();
    auto halfSize = m_view.getSize() / 2.f;
    const auto& b = m_bounds.value();

    return center.x - halfSize.x >= b.left &&
        center.x + halfSize.x <= b.left + b.width &&
        center.y - halfSize.y >= b.top &&
        center.y + halfSize.y <= b.top + b.height;
}
