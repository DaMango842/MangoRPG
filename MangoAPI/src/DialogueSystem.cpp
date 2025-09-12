// DialogueSystem.cpp
#include "DialogueSystem.h"
#include <iostream>
#include <fstream>
#include <algorithm>

#include <Window.h>

DialogueSystem::DialogueSystem(const sf::Font& font, unsigned int charSize)
    : m_font(font), m_textDisplay(font, charSize) {

    // 对话框设置 - 底部对话框风格
    m_bg.setFillColor(sf::Color(0, 0, 0, 220));
    m_bg.setSize({ 800.f, 200.f }); // 全宽度
    m_bg.setPosition(0.f, 400.f);   // 底部对齐

    // 角色名称框 - 角标风格
    m_nameBg.setFillColor(sf::Color(70, 130, 180, 230)); // 钢蓝色
    m_nameBg.setSize({ 200.f, 35.f });
    m_nameBg.setPosition(20.f, 365.f); // 稍微重叠在对话框上

    m_nameText.setFont(m_font);
    m_nameText.setCharacterSize(charSize);
    m_nameText.setFillColor(sf::Color::White);
    m_nameText.setPosition(30.f, 370.f);
    m_nameText.setStyle(sf::Text::Bold);

    // 文本显示位置 - 更大的边距
    m_textDisplay.setPosition({ 40.f, 430.f });

    // 选项框设置 - 居中显示
    m_optionsBg.setFillColor(sf::Color(30, 30, 40, 230));
    m_optionsBg.setOutlineColor(sf::Color(100, 100, 140));
    m_optionsBg.setOutlineThickness(1.5f);

    m_optionsPosition = { 250.f, 220.f };
    m_optionSpacing = 10.f;
}

DialogueSystem::~DialogueSystem() {
    // 智能指针自动管理内存，无需手动删除
}

// 实现访问者接口
void DialogueSystem::visit(TextNode& node) {
    m_textDisplay.setText(node.text, node.displaySpeed);
    m_textDisplay.setColor(node.textColor);
    m_selectedChoice = -1;
    m_currentChoices.clear();

    if (node.onDisplayComplete) {
        node.onDisplayComplete();
    }
}

void DialogueSystem::visit(ChoiceNode& node) {
    m_textDisplay.complete();
    m_currentChoices = node.options;
    m_selectedChoice = -1;

    for (size_t i = 0; i < m_currentChoices.size(); ++i) {
        if (m_currentChoices[i] && m_currentChoices[i]->enabled) {
            m_selectedChoice = static_cast<int>(i);
            break;
        }
    }
}

void DialogueSystem::visit(BranchNode& node) {
    for (const auto& conditionPair : node.conditionNodes) {
        if (m_currentNode->condition && m_currentNode->condition()) {
            advanceToNode(conditionPair.second);
            return;
        }
    }

    if (!node.defaultNode.empty()) {
        advanceToNode(node.defaultNode);
    }
    else {
        endDialogue();
    }
}

void DialogueSystem::visit(ActionNode& node) {
    if (node.action) {
        node.action();
    }

    if (!node.nextNode.empty()) {
        advanceToNode(node.nextNode);
    }
    else {
        endDialogue();
    }
}

void DialogueSystem::executeCurrentNode() {
    if (m_currentNode) {
        m_currentNode->accept(*this);
    }
}

void DialogueSystem::handleEvent(const sf::Event& event) {
    const auto& window = Window::getInstance().getWindow();
    handleEvent(event, window);
}

void DialogueSystem::handleEvent(const sf::Event& event, const sf::RenderTarget& target)
{
    if (getState() != State::Active) return;
    switch (event.type) {
    case sf::Event::KeyPressed:
        if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Space) {
            if (!m_textDisplay.isComplete()) {
                m_textDisplay.complete();
            }
            else if (m_currentNode) {
                // 使用安全的类型检查
                if (auto textNode = m_currentNode.cast_static<TextNode>()) {
                    if (!textNode->nextNode.empty()) {
                        advanceToNode(textNode->nextNode);
                    }
                    else {
                        m_autoEndTimer = 0.5f;
                    }
                }
                else if (auto choiceNode = m_currentNode.cast_static<ChoiceNode>()) {
                    if (m_selectedChoice >= 0) executeCurrentChoice();
                }
            }
        }
        else if (event.key.code == sf::Keyboard::Up) {
            selectPreviousChoice();
        }
        else if (event.key.code == sf::Keyboard::Down) {
            selectNextChoice();
        }
        break;
    case sf::Event::MouseButtonPressed:
        if (event.mouseButton.button == sf::Mouse::Left) {
            if (!m_textDisplay.isComplete()) {
                m_textDisplay.complete();
            }
            else if (m_currentNode) {
                sf::Vector2f mousePos = target.mapPixelToCoords({ event.mouseButton.x, event.mouseButton.y });
                updateSelection(mousePos);
                if (auto choiceNode = m_currentNode.cast_static<ChoiceNode>()) {
                    if (m_selectedChoice >= 0) executeCurrentChoice();
                }
                else if (auto textNode = m_currentNode.cast_static<TextNode>()) {
                    if (!textNode->nextNode.empty()) {
                        advanceToNode(textNode->nextNode);
                    }
                    else {
                        m_autoEndTimer = 0.5f;
                    }
                }
            }
        }
        break;
    case sf::Event::MouseMoved:
        {
            sf::Vector2f mousePos = target.mapPixelToCoords({ event.mouseMove.x, event.mouseMove.y });
            updateSelection(mousePos);
        }
        break;
    default:
        break;
	}
}

void DialogueSystem::update(float deltaTime) {
    if (getState() != State::Active) return;

    // 更新文本显示
    m_textDisplay.update(deltaTime);

    // 处理自动结束计时器
    if (m_autoEndTimer > 0) {
        m_autoEndTimer -= deltaTime;
        if (m_autoEndTimer <= 0) {
            endDialogue();
        }
    }
}

void DialogueSystem::render(sf::RenderTarget& target) {
    if (getState() != State::Active) return;

    if (m_currentNode && m_currentNode->type == DialogueNodeType::Choice && m_textDisplay.isComplete()) {
        renderOptions(target);
    }

    target.draw(m_bg);

    if (auto textNode = m_currentNode.cast_static<TextNode>()) {
        if (!textNode->characterName.empty() && isStringValid(textNode->characterName)) {
            target.draw(m_nameBg);
            m_nameText.setString(textNode->characterName);
            target.draw(m_nameText);
        }
    }

    m_textDisplay.draw(target);
}

void DialogueSystem::renderOptions(sf::RenderTarget& target) {
    if (m_currentChoices.empty()) return;

    // 计算选项框的大小
    float totalHeight = 0.f;
    float maxWidth = 300.f;

    // 创建一个临时的文本对象用于测量
    sf::Text tempText;
    tempText.setFont(m_font);
    tempText.setCharacterSize(m_textDisplay.getCharacterSize());

    // 计算总高度 - 添加安全检查
    for (const auto& option : m_currentChoices) {
        if (!option->enabled) continue;

        // 安全检查
        if (!isStringValid(option->text)) {
            tempText.setString("Invalid option text");
        }
        else {
            tempText.setString(option->text);
        }

        totalHeight += tempText.getLocalBounds().height + m_optionSpacing;
    }

    // 添加内边距
    totalHeight += 20.f;

    // 设置选项框的位置和大小
    float optionsX = (800.f - maxWidth) / 2.f;
    float optionsY = 250.f;

    m_optionsBg.setSize({ maxWidth, totalHeight });
    m_optionsBg.setPosition(optionsX, optionsY);
    target.draw(m_optionsBg);

    // 渲染选项 - 添加安全检查
    float yPos = optionsY + 10.f;

    for (size_t i = 0; i < m_currentChoices.size(); ++i) {
        if (!m_currentChoices[i]->enabled) continue;

        sf::Text optionText;
        optionText.setFont(m_font);
        optionText.setCharacterSize(m_textDisplay.getCharacterSize());

        // 安全检查
        if (!isStringValid(m_currentChoices[i]->text)) {
            optionText.setString("Invalid option");
        }
        else {
            optionText.setString(m_currentChoices[i]->text);
        }

        // 文本居中
        float textWidth = optionText.getLocalBounds().width;
        float xPos = optionsX + (maxWidth - textWidth) / 2.f;
        optionText.setPosition(xPos, yPos);

        // 选中状态
        if (i == static_cast<size_t>(m_selectedChoice)) {
            // 选中背景
            sf::RectangleShape highlight;
            highlight.setFillColor(sf::Color(80, 80, 80, 180));
            highlight.setSize({ maxWidth - 20.f, optionText.getLocalBounds().height + 8.f });
            highlight.setPosition(optionsX + 10.f, yPos - 4.f);
            target.draw(highlight);

            optionText.setFillColor(sf::Color::Yellow);
        }
        else {
            optionText.setFillColor(sf::Color::White);
        }

        target.draw(optionText);

        yPos += optionText.getLocalBounds().height + m_optionSpacing;
    }
}

void DialogueSystem::startDialogue() {
    if (m_nodes.empty() || m_startNodeId.empty()) {
        std::cerr << "DialogueSystem: No nodes or start node not set!\n";
        return;
    }

    m_state = State::Active;
    m_textDisplay.reset();
    m_selectedChoice = -1;
    m_currentChoices.clear();
    m_autoEndTimer = 0.f;

    advanceToNode(m_startNodeId);
}

void DialogueSystem::endDialogue() {
    if (m_state != State::Finished) {
        m_state = State::Finished;
        m_textDisplay.reset();
        m_currentNode = nullptr;
        m_nameText.setString("");
        m_selectedChoice = -1;
        m_currentChoices.clear();
        m_autoEndTimer = 0.f;

        // 调用结束回调
        if (m_onDialogueEnded) {
            m_onDialogueEnded();
        }
    }
}

void DialogueSystem::pauseDialogue() {
    if (m_state == State::Active) {
        m_state = State::Paused;
    }
}

void DialogueSystem::resumeDialogue() {
    if (m_state == State::Paused) {
        m_state = State::Active;
    }
}

int DialogueSystem::getVariable(const String& name) const {
    auto it = m_variables.find(name);
    return it != m_variables.end() ? it->second : 0;
}

void DialogueSystem::setCharacterName(const String& characterId, const String& displayName) {
    m_characters[characterId] = displayName;
}

String DialogueSystem::getCharacterName(const String& characterId) const {
    auto it = m_characters.find(characterId);
    return it != m_characters.end() ? it->second : characterId;
}

void DialogueSystem::addNode(MangoPtr<DialogueNode> node) {
    if (node && !node->id.empty()) {
        m_nodes[node->id] = node;
    }
}

MangoPtr<DialogueNode> DialogueSystem::getNode(const String& id) {
    auto it = m_nodes.find(id);
    return it != m_nodes.end() ? it->second : nullptr;
}

void DialogueSystem::advanceToNode(const String& nodeId) {
    if (nodeId.empty()) {
        endDialogue();
        return;
    }

    auto node = getNode(nodeId);
    if (!node) {
        std::cerr << "DialogueSystem: Node not found: " << nodeId << "\n";
        endDialogue();
        return;
    }

    m_currentNode = node;
    executeCurrentNode();
}

void DialogueSystem::updateSelection(const sf::Vector2f& mousePos) {
    if (!m_currentNode || m_currentNode->type != DialogueNodeType::Choice) return;

    m_selectedChoice = -1;

    // 获取选项框的位置和大小
    float optionsX = m_optionsBg.getPosition().x;
    float optionsY = m_optionsBg.getPosition().y;
    float maxWidth = m_optionsBg.getSize().x;

    float yPos = optionsY + 10.f;

    for (size_t i = 0; i < m_currentChoices.size(); ++i) {
        if (!m_currentChoices[i]->enabled) continue;

        sf::Text tempText;
        tempText.setFont(m_font);
        tempText.setCharacterSize(m_textDisplay.getCharacterSize());
        tempText.setString(m_currentChoices[i]->text);

        // 计算选项的点击区域
        sf::FloatRect bounds(
            optionsX + 10.f,
            yPos - 4.f,
            maxWidth - 20.f,
            tempText.getLocalBounds().height + 8.f
        );

        if (bounds.contains(mousePos)) {
            m_selectedChoice = static_cast<int>(i);
            break;
        }

        yPos += tempText.getLocalBounds().height + m_optionSpacing;
    }
}

void DialogueSystem::selectNextChoice() {
    if (!m_currentNode || m_currentNode->type != DialogueNodeType::Choice) return;
    if (m_currentChoices.empty()) return;

    int start = m_selectedChoice;
    do {
        m_selectedChoice = (m_selectedChoice + 1) % static_cast<int>(m_currentChoices.size());
    } while (!m_currentChoices[m_selectedChoice]->enabled && m_selectedChoice != start);
}

void DialogueSystem::selectPreviousChoice() {
    if (!m_currentNode || m_currentNode->type != DialogueNodeType::Choice) return;
    if (m_currentChoices.empty()) return;

    int start = m_selectedChoice;
    do {
        m_selectedChoice = (m_selectedChoice - 1 + static_cast<int>(m_currentChoices.size()))
            % static_cast<int>(m_currentChoices.size());
    } while (!m_currentChoices[m_selectedChoice]->enabled && m_selectedChoice != start);
}

void DialogueSystem::executeCurrentChoice() {
    if (!m_currentNode || m_currentNode->type != DialogueNodeType::Choice) return;
    if (m_selectedChoice < 0) return;
    if (static_cast<size_t>(m_selectedChoice) >= m_currentChoices.size()) return;

    MangoPtr<ChoiceNode::Option> option = m_currentChoices[m_selectedChoice];

    if (option->enabled) {
        if (option->callback) {
            option->callback();
        }

        // 清除当前选项
        m_currentChoices.clear();
        m_selectedChoice = -1;

        if (!option->nextNode.empty()) {
            advanceToNode(option->nextNode);
        }
        else {
            endDialogue();
        }
    }
}

bool DialogueSystem::isStringValid(const String& str) const {
    // 检查字符串大小是否合理
    if (str.size() > 10000) { // 设置合理的上限
        return false;
    }

    // 检查数据指针
    if (str.data() == nullptr) {
        return false;
    }

    // 检查是否包含异常字符（可选）
    for (size_t i = 0; i < str.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(str.data()[i]);
        if (c < 32 && c != '\n' && c != '\t' && c != '\r') {
            // 包含控制字符，可能有问题
            return false;
        }
    }

    return true;
}

void DialogueSystem::setOptionsPosition(const sf::Vector2f& position) {
    m_optionsPosition = position;
}

void DialogueSystem::setOptionSpacing(float spacing) {
    m_optionSpacing = spacing;
}

void DialogueSystem::setOptionsBoxStyle(const sf::Color& fillColor, const sf::Color& outlineColor, float outlineThickness) {
    m_optionsBg.setFillColor(fillColor);
    m_optionsBg.setOutlineColor(outlineColor);
    m_optionsBg.setOutlineThickness(outlineThickness);
}

void DialogueSystem::setDialogBoxStyle(const sf::Vector2f& size, const sf::Vector2f& position,
    const sf::Color& fillColor, const sf::Color& outlineColor, float outlineThickness) {
    m_bg.setSize(size);
    m_bg.setPosition(position);
    m_bg.setFillColor(fillColor);
    m_bg.setOutlineColor(outlineColor);
    m_bg.setOutlineThickness(outlineThickness);

    // 自动调整名称框位置
    m_nameBg.setPosition(position.x + 10.f, position.y - 30.f);
    m_nameText.setPosition(position.x + 20.f, position.y - 25.f);
    m_textDisplay.setPosition(position.x + 20.f, position.y + 20.f);
}

void DialogueSystem::debugPrintOptions() const {
    if (m_currentNode && m_currentNode->type == DialogueNodeType::Choice) {
        std::cout << "Options count: " << m_currentChoices.size() << std::endl;
        for (size_t i = 0; i < m_currentChoices.size(); ++i) {
            std::cout << "Option " << i << ": " << m_currentChoices[i]->text
                << " (enabled: " << m_currentChoices[i]->enabled << ")" << std::endl;
        }
        std::cout << "Selected option: " << m_selectedChoice << std::endl;
    }
}
