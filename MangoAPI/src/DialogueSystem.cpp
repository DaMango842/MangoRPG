// DialogueSystem.cpp
#include "DialogueSystem.h"
#include <iostream>
#include <fstream>
#include <algorithm>

DialogueSystem::DialogueSystem(const sf::Font& font, unsigned int charSize)
    : m_font(font), m_textDisplay(font, charSize) {

    // 对话框设置
    m_bg.setFillColor(sf::Color(0, 0, 0, 200));
    m_bg.setSize({ 700.f, 150.f });
    m_bg.setPosition(50.f, 400.f);

    // 角色名称框
    m_nameBg.setFillColor(sf::Color(0, 0, 100, 200));
    m_nameBg.setSize({ 200.f, 30.f });
    m_nameBg.setPosition(50.f, 370.f);

    m_nameText.setFont(m_font);
    m_nameText.setCharacterSize(charSize - 2);
    m_nameText.setFillColor(sf::Color::White);
    m_nameText.setPosition(60.f, 375.f);

    // 文本显示位置
    m_textDisplay.setPosition({ 70.f, 420.f });

    // 选项框设置
    m_optionsBg.setFillColor(sf::Color(0, 0, 0, 200));
    m_optionsBg.setOutlineColor(sf::Color::White);
    m_optionsBg.setOutlineThickness(2.f);

    // 默认选项框位置
    m_optionsPosition = { 150.f, 250.f };
    m_optionSpacing = 15.f;
}

DialogueSystem::~DialogueSystem() {
    // 智能指针自动管理内存，无需手动删除
}

void DialogueSystem::handleEvent(const sf::Event& event) {
    if (getState() != State::Active) return;

    switch (event.type) {
    case sf::Event::KeyPressed:
        if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Space) {
            if (!m_textDisplay.isComplete()) {
                m_textDisplay.complete();
            }
            else if (m_currentNode && m_currentNode->type == DialogueNodeType::Choice) {
                if (m_selectedChoice >= 0) executeCurrentChoice();
            }
            else {
                // 推进到下一个节点
                if (m_currentNode && m_currentNode->type == DialogueNodeType::Text) {
                    TextNode* textNode = static_cast<TextNode*>(m_currentNode.get());
                    if (!textNode->nextNode.empty()) {
                        advanceToNode(textNode->nextNode);
                    }
                    else {
                        // 如果没有下一个节点，设置自动结束
                        m_autoEndTimer = 0.5f;
                    }
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
            else if (m_currentNode && m_currentNode->type == DialogueNodeType::Choice) {
                updateSelection({ float(event.mouseButton.x), float(event.mouseButton.y) });
                if (m_selectedChoice >= 0) executeCurrentChoice();
            }
            else {
                // 推进到下一个节点
                if (m_currentNode && m_currentNode->type == DialogueNodeType::Text) {
                    TextNode* textNode = static_cast<TextNode*>(m_currentNode.get());
                    if (!textNode->nextNode.empty()) {
                        advanceToNode(textNode->nextNode);
                    }
                    else {
                        // 如果没有下一个节点，设置自动结束
                        m_autoEndTimer = 0.5f;
                    }
                }
            }
        }
        break;
    case sf::Event::MouseMoved:
        updateSelection({ float(event.mouseMove.x), float(event.mouseMove.y) });
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

    // 先渲染选项框（如果存在）
    if (m_currentNode && m_currentNode->type == DialogueNodeType::Choice && m_textDisplay.isComplete()) {
        renderOptions(target);
    }

    // 渲染对话框
    target.draw(m_bg);

    // 显示角色名
    if (m_currentNode && m_currentNode->type == DialogueNodeType::Text) {
        TextNode* textNode = static_cast<TextNode*>(m_currentNode.get());
        if (!textNode->characterName.empty()) {
            target.draw(m_nameBg);
            m_nameText.setString(textNode->characterName);
            target.draw(m_nameText);
        }
    }

    // 显示文本
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

    // 计算总高度
    for (const auto& option : m_currentChoices) {
        if (!option->enabled) continue;

        tempText.setString(option->text);
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

    // 渲染选项
    float yPos = optionsY + 10.f;

    for (size_t i = 0; i < m_currentChoices.size(); ++i) {
        if (!m_currentChoices[i]->enabled) continue;

        sf::Text optionText;
        optionText.setFont(m_font);
        optionText.setCharacterSize(m_textDisplay.getCharacterSize());
        optionText.setString(m_currentChoices[i]->text);

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

void DialogueSystem::executeCurrentNode() {
    if (!m_currentNode) return;

    switch (m_currentNode->type) {
    case DialogueNodeType::Text:
        handleTextNode(static_cast<TextNode*>(m_currentNode.get()));
        break;
    case DialogueNodeType::Choice:
        handleChoiceNode(static_cast<ChoiceNode*>(m_currentNode.get()));
        break;
    case DialogueNodeType::Branch:
        handleBranchNode(static_cast<BranchNode*>(m_currentNode.get()));
        break;
    case DialogueNodeType::Action:
        handleActionNode(static_cast<ActionNode*>(m_currentNode.get()));
        break;
    default:
        break;
    }
}

void DialogueSystem::handleTextNode(TextNode* node) {
    m_textDisplay.setText(node->text, node->displaySpeed);
    m_textDisplay.setColor(node->textColor);
    m_selectedChoice = -1;
    m_currentChoices.clear();

    // 设置文本显示完成回调
    if (node->onDisplayComplete) {
        node->onDisplayComplete();
    }
}

void DialogueSystem::handleChoiceNode(ChoiceNode* node) {
    // 确保文本显示完成
    m_textDisplay.complete();

    // 存储当前选项
    m_currentChoices = node->options;

    m_selectedChoice = -1;
    // 找到第一个可用的选项
    for (size_t i = 0; i < m_currentChoices.size(); ++i) {
        if (m_currentChoices[i]->enabled) {
            m_selectedChoice = static_cast<int>(i);
            break;
        }
    }
}

void DialogueSystem::handleBranchNode(BranchNode* node) {
    // 检查所有条件，找到第一个满足的条件
    for (const auto& conditionPair : node->conditionNodes) {
        if (m_currentNode->condition && m_currentNode->condition()) {
            advanceToNode(conditionPair.second);
            return;
        }
    }

    // 如果没有条件满足，使用默认节点
    if (!node->defaultNode.empty()) {
        advanceToNode(node->defaultNode);
    }
    else {
        endDialogue();
    }
}

void DialogueSystem::handleActionNode(ActionNode* node) {
    if (node->action) {
        node->action();
    }

    if (!node->nextNode.empty()) {
        advanceToNode(node->nextNode);
    }
    else {
        endDialogue();
    }
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
