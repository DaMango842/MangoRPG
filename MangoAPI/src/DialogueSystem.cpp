#include "DialogueSystem.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cctype>

#include <Window.h>

DialogueSystem::DialogueSystem(const sf::Font& font, unsigned int charSize)
    : m_font(font), m_textDisplay(font, charSize) {

    // 对话框设置 - 底部对话框风格
    m_bg.setFillColor(sf::Color(128, 168, 255, 220));
    m_bg.setSize({ 750.f, 200.f }); // 全宽度
    m_bg.setPosition(25.f, 380.f);   // 底部对齐
    m_bg.setOutlineColor(sf::Color(168, 128, 240, 220));
    m_bg.setOutlineThickness(5.f);

	m_bg_shadow.setFillColor(sf::Color(0, 0, 0, 128));
	m_bg_shadow.setSize(m_bg.getSize());
	m_bg_shadow.setPosition(m_bg.getPosition() + sf::Vector2f(10.f, 10.f)); // 右下偏移5像素

    // 角色名称框 - 角标风格
    m_nameBg.setFillColor(sf::Color(70, 130, 180, 230)); // 钢蓝色
    m_nameBg.setSize({ 200.f, 35.f });
    m_nameBg.setPosition(25.f, 365.f); // 稍微重叠在对话框上

    m_nameText.setFont(m_font);
    m_nameText.setCharacterSize(charSize);
    m_nameText.setFillColor(sf::Color::White);
    m_nameText.setOutlineColor(sf::Color::Black);
	m_nameText.setOutlineThickness(1.f);

    m_nameText.setPosition(30.f, 365.f);
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
    std::cout << "[对话系统] 访问文本节点: " << node.id
        << ", 下一个节点: " << node.nextNode << std::endl;

    m_textDisplay.setText(node.text, node.displaySpeed);
    m_textDisplay.setColor(node.textColor);
    m_selectedChoice = -1;
    m_currentChoices.clear();

    if (node.onDisplayComplete) {
        node.onDisplayComplete();
    }
}

void DialogueSystem::visit(ChoiceNode& node) {
    std::cout << "[对话系统] 访问选项节点: " << node.id << std::endl;

    m_textDisplay.complete();
    m_currentChoices = node.options;
    m_selectedChoice = -1;

    for (size_t i = 0; i < m_currentChoices.size(); ++i) {
        if (m_currentChoices[i] && m_currentChoices[i]->enabled) {
            m_selectedChoice = static_cast<int>(i);
            std::cout << "[对话系统] 默认选择选项 " << i << ": " << m_currentChoices[i]->text << std::endl;
            break;
        }
    }
}

void DialogueSystem::visit(BranchNode& node) {
    std::cout << "[对话系统] 访问分支节点: " << node.id << std::endl;

    for (const auto& conditionPair : node.conditionNodes) {
        if (m_currentNode->condition && m_currentNode->condition()) {
            std::cout << "[对话系统] 满足条件，跳转到节点: " << conditionPair.second << std::endl;
            advanceToNode(conditionPair.second);
            return;
        }
    }

    if (!node.defaultNode.empty()) {
        std::cout << "[对话系统] 使用默认节点: " << node.defaultNode << std::endl;
        advanceToNode(node.defaultNode);
    }
    else {
        std::cout << "[对话系统] 没有默认节点，结束对话" << std::endl;
        endDialogue();
    }
}

void DialogueSystem::visit(ActionNode& node) {
    std::cout << "[对话系统] 访问动作节点: " << node.id << std::endl;

    if (!node.command.empty()) {
		std::cout << "[对话系统] 执行动作命令: " << node.command << std::endl;
    }

    if (node.action) {
        node.action();
    }

    if (!node.nextNode.empty()) {
        std::cout << "[对话系统] 动作完成，跳转到节点: " << node.nextNode << std::endl;
        advanceToNode(node.nextNode);
    }
    else {
        std::cout << "[对话系统] 动作完成，没有下一个节点，结束对话" << std::endl;
        endDialogue();
    }
}

void DialogueSystem::executeCurrentNode() {
    if (m_currentNode) {
        std::cout << "[对话系统] 执行当前节点: " << m_currentNode->id << std::endl;
        m_currentNode->accept(*this);
    }
}

void DialogueSystem::handleEvent(const sf::Event& event) {
    const auto& window = Window::getInstance().getWindow();
    handleEvent(event, window);
}

void DialogueSystem::handleEvent(const sf::Event& event, const sf::RenderTarget& target) {
    if (getState() != State::Active) return;

    switch (event.type) {
    case sf::Event::KeyPressed:
        if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Space) {
            if (!m_textDisplay.isComplete()) {
                // 文本显示中：加速显示
                m_textDisplay.complete();
            }
            else if (m_currentNode) {
                // 文本显示完成：根据节点类型处理
                if (m_currentNode->type == DialogueNodeType::Text) {
                    // 文本节点：直接进入下一个节点
                    if (auto textNode = m_currentNode.cast_static<TextNode>()) {
                        std::cout << "[对话系统] 文本节点显示完成，准备跳转到: " << textNode->nextNode << std::endl;
                        if (!textNode->nextNode.empty()) {
                            advanceToNode(textNode->nextNode);
                        }
                        else {
                            m_autoEndTimer = 0.5f; // 自动结束
                        }
                    }
                }
                else if (m_currentNode->type == DialogueNodeType::Choice) {
                    // 选项节点：执行当前选中的选项
                    if (m_selectedChoice >= 0 &&
                        static_cast<size_t>(m_selectedChoice) < m_currentChoices.size() &&
                        m_currentChoices[m_selectedChoice]->enabled) {
                        executeCurrentChoice();
                    }
                    else {
                        // 如果没有有效选择，尝试选择第一个可用选项
                        selectFirstAvailableChoice();
                        if (m_selectedChoice >= 0) {
                            executeCurrentChoice();
                        }
                    }
                }
                else {
                    // 其他类型节点：默认行为
                    if (!m_currentNode->nextNode.empty()) {
                        advanceToNode(m_currentNode->nextNode);
                    }
                    else {
                        endDialogue();
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
        else if (event.key.code == sf::Keyboard::BackSpace && m_canSkip) {
            skipDialogue();
        }
        break;

    case sf::Event::MouseButtonPressed:
        if (event.mouseButton.button == sf::Mouse::Left) {
            if (!m_textDisplay.isComplete()) {
                // 文本显示中：加速显示
                m_textDisplay.complete();
            }
            else if (m_currentNode) {
                sf::Vector2f mousePos = target.mapPixelToCoords({ event.mouseButton.x, event.mouseButton.y });

                if (m_currentNode->type == DialogueNodeType::Choice) {
                    // 选项节点：检查是否点击了选项
                    updateSelection(mousePos);
                    if (m_selectedChoice >= 0 &&
                        static_cast<size_t>(m_selectedChoice) < m_currentChoices.size() &&
                        m_currentChoices[m_selectedChoice]->enabled) {
                        executeCurrentChoice();
                    }
                }
                else {
                    // 非选项节点：直接进入下一个节点
                    if (m_currentNode->type == DialogueNodeType::Text) {
                        if (auto textNode = m_currentNode.cast_static<TextNode>()) {
                            if (!textNode->nextNode.empty()) {
                                advanceToNode(textNode->nextNode);
                            }
                            else {
                                m_autoEndTimer = 0.5f;
                            }
                        }
                    }
                    else {
                        // 其他节点类型
                        if (!m_currentNode->nextNode.empty()) {
                            advanceToNode(m_currentNode->nextNode);
                        }
                        else {
                            endDialogue();
                        }
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

    target.draw(m_bg_shadow);
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

        float textWidth = tempText.getLocalBounds().width;
        if (textWidth + 40.f > maxWidth) { // 40.f 是内边距
            maxWidth = textWidth + 40.f;
        }

        totalHeight += tempText.getLocalBounds().height + m_optionSpacing;
    }

    maxWidth = std::min(maxWidth, 600.f);

    // 添加内边距
    totalHeight += 20.f;

    // 设置选项框的位置和大小
    float optionsX = (800.f - maxWidth) / 2.f;
    float optionsY = 200.f;

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
            highlight.setSize({ maxWidth - 20.f, optionText.getLocalBounds().height + 10.f });
            highlight.setPosition(optionsX + 10.f, (yPos - 2.f) + 2.5f);
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
    std::cout << "[对话系统] 开始对话，起始节点: " << m_startNodeId << std::endl;

    if (m_nodes.empty() || m_startNodeId.empty()) {
        std::cerr << "[对话系统错误] 没有节点或未设置起始节点!" << std::endl;
        return;
    }

    // 列出所有可用节点
    std::cout << "[对话系统] 可用节点列表:" << std::endl;
    for (const auto& pair : m_nodes) {
        std::cout << "[对话系统]   - " << pair.first << std::endl;
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
        std::cout << "[对话系统] 结束对话" << std::endl;

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
        std::cout << "[对话系统] 添加节点: " << node->id << std::endl;
        m_nodes[node->id] = node;
    }
    else if (node) {
        std::cerr << "[对话系统警告] 尝试添加空ID的节点" << std::endl;
    }
}

MangoPtr<DialogueNode> DialogueSystem::getNode(const String& id) {
    // 检查ID是否有效
    if (!isStringValid(id)) {
        std::cerr << "[对话系统错误] 尝试查找无效的节点ID: " << id << std::endl;
        return nullptr;
    }

    auto it = m_nodes.find(id);
    if (it == m_nodes.end()) {
        std::cerr << "[对话系统错误] 节点不在列表中: " << id << std::endl;
        return nullptr;
    }
    return it->second;
}

void DialogueSystem::advanceToNode(const String& nodeId) {
    // 验证节点ID
    if (!isStringValid(nodeId)) {
        std::cerr << "[对话系统错误] 无效的节点ID: " << nodeId << std::endl;
        endDialogue();
        return;
    }

    if (nodeId.empty()) {
        std::cout << "[对话系统] 节点ID为空，结束对话" << std::endl;
        endDialogue();
        return;
    }

    std::cout << "[对话系统] 尝试跳转到节点: " << nodeId << " (长度: " << nodeId.size() << ")" << std::endl;

    auto node = getNode(nodeId);
    if (!node) {
        std::cerr << "[对话系统错误] 未找到节点: " << nodeId << std::endl;
        std::cerr << "[对话系统错误] 节点ID详情: 长度=" << nodeId.size() << ", 内容=\"" << nodeId << "\"" << std::endl;

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
    // 简化安全检查，只检查必要的条件
    if (m_selectedChoice < 0 || static_cast<size_t>(m_selectedChoice) >= m_currentChoices.size()) {
        std::cout << "[对话系统错误] 无效的选择: " << m_selectedChoice << std::endl;
        return;
    }

    MangoPtr<ChoiceNode::Option> option = m_currentChoices[m_selectedChoice];
    if (!option || !option->enabled) {
        std::cout << "[对话系统错误] 选项为空或已禁用" << std::endl;
        return;
    }

    std::cout << "[对话系统] 执行选择: " << option->text << "，下一个节点: " << option->nextNode << std::endl;

    // 执行回调
    if (option->callback) {
        option->callback();
    }

    // 保存下一个节点ID
    String nextNode = option->nextNode;

    // 清除当前选项状态
    m_currentChoices.clear();
    m_selectedChoice = -1;

    // 进入下一个节点
    if (!nextNode.empty()) {
        advanceToNode(nextNode);
    }
    else {
        endDialogue();
    }
}

void DialogueSystem::selectFirstAvailableChoice()
{
    if (m_currentNode && m_currentNode->type == DialogueNodeType::Choice) {
        for (size_t i = 0; i < m_currentChoices.size(); ++i) {
            if (m_currentChoices[i] && m_currentChoices[i]->enabled) {
                m_selectedChoice = static_cast<int>(i);
                return;
            }
        }
        m_selectedChoice = -1; // 没有可用选项
    }
}

bool DialogueSystem::isStringValid(const String& str) const {
    // 检查字符串大小是否合理
    if (str.size() > 10000) {
        return false;
    }

    // 检查数据指针
    if (str.data() == nullptr) {
        return false;
    }

    // 检查是否为空字符串
    if (str.empty()) {
        return false;
    }

    // 宽松检查：只拒绝明显的控制字符
    for (size_t i = 0; i < str.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(str[i]);

        // 拒绝明显的控制字符（除了常见的空白字符）
        if (c < 32 && c != '\t' && c != '\n' && c != '\r') {
            return false;
        }
        if (c == 127) { // DEL 字符
            return false;
        }
        // 其他所有字符都允许，包括中文字符
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

void DialogueSystem::skipDialogue()
{
    if (!m_canSkip || getState() != State::Active) return;

    // 如果当前是选项节点，不允许跳过
    if (m_currentNode && m_currentNode->type == DialogueNodeType::Choice) {
        return; // 选项必须由玩家选择，不能跳过
    }

    if (!m_textDisplay.isComplete()) {
        // 文本显示中：立即完成显示
        m_textDisplay.complete();
    }
    else {
        // 文本显示完成：根据节点类型处理
        if (m_currentNode) {
            switch (m_currentNode->type) {
            case DialogueNodeType::Text:
                if (auto textNode = m_currentNode.cast_static<TextNode>()) {
                    if (!textNode->nextNode.empty()) {
                        advanceToNode(textNode->nextNode);
                    }
                    else {
                        endDialogue();
                    }
                }
                break;

            case DialogueNodeType::Action:
            case DialogueNodeType::Branch:
                // 这些节点可以快速跳过
                if (!m_currentNode->nextNode.empty()) {
                    advanceToNode(m_currentNode->nextNode);
                }
                else {
                    endDialogue();
                }
                break;

            case DialogueNodeType::Choice:
                // 选项节点已经在开头检查过，这里不会执行
                break;

            default:
                endDialogue();
                break;
            }
        }
    }
}

void DialogueSystem::debugPrintOptions() const {
    if (m_currentNode && m_currentNode->type == DialogueNodeType::Choice) {
        std::cout << "选项数量: " << m_currentChoices.size() << std::endl;
        for (size_t i = 0; i < m_currentChoices.size(); ++i) {
            std::cout << "选项 " << i << ": " << m_currentChoices[i]->text
                << " (启用: " << m_currentChoices[i]->enabled << ")" << std::endl;
        }
        std::cout << "选中的选项: " << m_selectedChoice << std::endl;
    }
}
