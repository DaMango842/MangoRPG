// DialogueSystem.h
#pragma once
#include "BaseDialogue.h"
#include "TextDisplay.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <functional>
#include <unordered_map>
#include <memory>
#include <istream>
#include <fstream>
#include <ostream>
#include <MangoString.hpp>
#include <MangoPtr.hpp>
#include <MangoVector.hpp>

#include <nlohmann/json.hpp>

#include "GlobalCommand.h"

// 对话节点类型
enum class DialogueNodeType {
    Text,       // 纯文本
    Choice,     // 选项
    Branch,     // 分支（根据条件跳转）
    Action      // 执行动作
};

// 前向声明
struct TextNode;
struct ChoiceNode;
struct BranchNode;
struct ActionNode;

// 访问者接口
class DialogueNodeVisitor {
public:
    virtual ~DialogueNodeVisitor() = default;
    virtual void visit(TextNode& node) = 0;
    virtual void visit(ChoiceNode& node) = 0;
    virtual void visit(BranchNode& node) = 0;
    virtual void visit(ActionNode& node) = 0;
};

// 对话节点基类
struct DialogueNode {
    String id;
    DialogueNodeType type;
    std::function<bool()> condition; // 条件（用于分支节点）
	String nextNode;    // 下一个节点ID

	DialogueNode() { type = DialogueNodeType::Text; }
    virtual ~DialogueNode() = default;
    virtual void accept(DialogueNodeVisitor& visitor) = 0;
};

// 文本节点
struct TextNode : public DialogueNode {
    String characterName;    // 角色名
    String text;             // 对话文本
    float displaySpeed = 0.05f; // 显示速度
    sf::Color textColor = sf::Color::White;
    std::function<void()> onDisplayComplete;

    TextNode() { type = DialogueNodeType::Text; }

    void accept(DialogueNodeVisitor& visitor) override;
};

// 选项节点
struct ChoiceNode : public DialogueNode {
    struct Option {
        String text;
        std::function<void()> callback;
        String nextNode;     // 选择后的下一个节点
        sf::Color normal = sf::Color::White;
        sf::Color hover = sf::Color::Yellow;
        bool enabled = true;
    };

    std::vector<MangoPtr<Option>> options;

    ChoiceNode() { type = DialogueNodeType::Choice; }
    void accept(DialogueNodeVisitor& visitor) override;
};

// 分支节点
struct BranchNode : public DialogueNode {
    std::unordered_map<String, String> conditionNodes; // 条件->节点ID映射
    String defaultNode; // 默认节点

    BranchNode() { type = DialogueNodeType::Branch; }
    void accept(DialogueNodeVisitor& visitor) override;
};

// 动作节点
struct ActionNode : public DialogueNode {
    std::string command{ "" }; 
    std::function<void()> action;

    ActionNode() { type = DialogueNodeType::Action; }
    void accept(DialogueNodeVisitor& visitor) override;
};

class DialogueSystem : public BaseDialogue, public DialogueNodeVisitor {
public:
    explicit DialogueSystem(const sf::Font& font, unsigned int charSize = 30);
    ~DialogueSystem() override;

    // 实现访问者接口
    void visit(TextNode& node) override;
    void visit(ChoiceNode& node) override;
    void visit(BranchNode& node) override;
    void visit(ActionNode& node) override;

    // 核心接口
    void handleEvent(const sf::Event& event) override;
	void handleEvent(const sf::Event& event, const sf::RenderTarget& target) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

    void startDialogue() override;
    void endDialogue() override;
    void pauseDialogue() override;
    void resumeDialogue() override;
    State getState() const override { return m_state; }

    // 对话管理
    void addNode(MangoPtr<DialogueNode> node);
    MangoPtr<DialogueNode> getNode(const String& id);
    void setStartNode(const String& id) { m_startNodeId = id; }
    void clearNodes() { m_nodes.clear(); }

    // 变量系统
    void setVariable(const String& name, int value) { m_variables[name] = value; }
    int getVariable(const String& name) const;

    // 角色管理
    void setCharacterName(const String& characterId, const String& displayName);
    String getCharacterName(const String& characterId) const;

    // 选项框设置
    void setOptionsPosition(const sf::Vector2f& position);
    void setOptionSpacing(float spacing);
    void setOptionsBoxStyle(const sf::Color& fillColor, const sf::Color& outlineColor, float outlineThickness);

    // 对话框设置
    void setDialogBoxStyle(const sf::Vector2f& size, const sf::Vector2f& position,
        const sf::Color& fillColor, const sf::Color& outlineColor, float outlineThickness = 0.f);

    // 对话结束回调
    void setOnDialogueEndedCallback(std::function<void()> callback) {
        m_onDialogueEnded = callback;
    }

    void setCanSkip(bool value) {
        m_canSkip = value;
    }

    bool canSkip() const { return m_canSkip; }

    void skipDialogue();

    // From TextDisplay
    float getDisplaySpeed() const { return m_textDisplay.getDisplaySpeed(); }
	void setDisplaySpeed(float speed) { m_textDisplay.setDisplaySpeed(speed); }

    // 文本速度预设组
	void setTextSpeedVerySlow() { m_textDisplay.setDisplaySpeed(0.05f); }
	void setTextSpeedSlow() { m_textDisplay.setDisplaySpeed(0.25f); }
	void setTextSpeedNormal() { m_textDisplay.setDisplaySpeed(0.5f); }
	void setTextSpeedFast() { m_textDisplay.setDisplaySpeed(1.f); }
	void setTextSpeedVeryFast() { m_textDisplay.setDisplaySpeed(2.f); }
	void setTextSpeedInstant() { m_textDisplay.setDisplaySpeed(-1.f); }

    void setInstantDisplay(bool instant) {
        if (instant) {
            // 切换到即时模式前保存当前速度
            if (getDisplaySpeed() != -1.f) {
                m_lastNormalSpeed = getDisplaySpeed();
            }
            setDisplaySpeed(-1.f);
        }
        else {
            // 恢复到之前保存的正常速度
            setDisplaySpeed(m_lastNormalSpeed);
        }
    }

    bool isInstantDisplay() const {
        return m_textDisplay.getDisplaySpeed() == -1.0f;
    }

    // 快速切换
    void toggleInstantDisplay() {
        setInstantDisplay(!isInstantDisplay());
    }

    std::string getSpeedDescription() const {
        float speed = getDisplaySpeed();
        if (speed == -1.f) return "即时";
        if (speed <= 0.08f) return "极慢";
        if (speed <= 0.25f) return "慢速";
        if (speed <= 0.5f) return "正常";
        if (speed <= 1.f) return "快速";
        return "极快";
    }

    // 调试
    void debugPrintOptions() const;

private:
    void advanceToNode(const String& nodeId);
    void executeCurrentNode();

    void renderOptions(sf::RenderTarget& target);
    void updateSelection(const sf::Vector2f& mousePos);
    void selectNextChoice();
    void selectPreviousChoice();
    void executeCurrentChoice();
    void selectFirstAvailableChoice();

    bool isStringValid(const String& str) const;

    // 数据成员
    // 1. 核心组件和引用
    const sf::Font& m_font;
    State m_state = State::Inactive;
    MangoPtr<DialogueNode> m_currentNode = nullptr;
    String m_startNodeId;

    // 2. 文本显示相关
    TextDisplay m_textDisplay;
    sf::Text m_nameText;
    float m_lastNormalSpeed = 0.5f; // 用于保存切换到即时显示前的速度

    // 3. 对话框UI元素
    sf::RectangleShape m_bg;
    sf::RectangleShape m_bg_shadow;
    sf::RectangleShape m_nameBg;

    // 4. 选项系统相关
    sf::RectangleShape m_optionsBg;
    sf::Vector2f m_optionsPosition;
    float m_optionSpacing = 10.f;
    std::vector<MangoPtr<ChoiceNode::Option>> m_currentChoices;
    int m_selectedChoice = -1;

    // 5. 游戏数据存储
    std::unordered_map<String, MangoPtr<DialogueNode>> m_nodes;
    std::unordered_map<String, String> m_characters;
    std::unordered_map<String, int> m_variables;

    // 6. 控制标志和计时器
    bool m_canSkip{ true };
    float m_autoEndTimer = 0.f;

    // 7. 回调函数
    std::function<void()> m_onDialogueEnded;
};

inline void TextNode::accept(DialogueNodeVisitor& visitor) { visitor.visit(*this); }
inline void ChoiceNode::accept(DialogueNodeVisitor& visitor) { visitor.visit(*this); }
inline void BranchNode::accept(DialogueNodeVisitor& visitor) { visitor.visit(*this); }
inline void ActionNode::accept(DialogueNodeVisitor& visitor) { visitor.visit(*this); }


class DialogueParser {
public:
    static std::unordered_map<String, MangoPtr<DialogueNode>> parseFromJSON(const std::string& jsonStr) {
        std::unordered_map<String, MangoPtr<DialogueNode>> nodes;

        try {
            nlohmann::json jsonData = nlohmann::json::parse(jsonStr);

            for (auto& item : jsonData.items()) {
                String nodeId = item.key();
                auto& nodeData = item.value();

                // 检查节点类型
                if (nodeData.contains("options")) {
                    // ChoiceNode
                    auto choiceNode = make_mango_ptr<ChoiceNode>();
                    choiceNode->id = nodeId;

                    for (auto& optionData : nodeData["options"]) {
                        auto option = make_mango_ptr<ChoiceNode::Option>();
                        option->text = optionData["text"].get<std::string>();
                        option->enabled = true;

                        if (optionData.contains("nextNode") &&
                            !optionData["nextNode"].is_null()) {
                            option->nextNode = optionData["nextNode"].get<std::string>();
                        }

                        choiceNode->options.push_back(std::move(option));
                    }

                    nodes[nodeId] = std::move(choiceNode);
                }
                else if (nodeData.contains("command")) {
                    auto actionNode = make_mango_ptr<ActionNode>();
                    actionNode->id = nodeId;
                    actionNode->command = nodeData["command"].get<std::string>();

                    // 直接哈希查找，O(1)复杂度
                    auto it = dialogue_action_commands.find(actionNode->command);
                    if (it != dialogue_action_commands.end()) {
                        actionNode->action = it->second;
                    }
                    else {
                        // 处理命令不存在的情况
                        std::cout << "Unknown command: " << actionNode->command << "\n";
                    }

                    nodes[nodeId] = std::move(actionNode);
                }
                else if (nodeData.contains("text")) {
                    // TextNode
                    auto textNode = make_mango_ptr<TextNode>();
                    textNode->id = nodeId;
                    textNode->text = nodeData["text"].get<std::string>();

                    if (nodeData.contains("characterName") &&
                        !nodeData["characterName"].is_null()) {
                        textNode->characterName = nodeData["characterName"].get<std::string>();
                    }

                    if (nodeData.contains("nextNode") &&
                        !nodeData["nextNode"].is_null()) {
                        textNode->nextNode = nodeData["nextNode"].get<std::string>();
                    }

                    nodes[nodeId] = std::move(textNode);
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Failed to parse dialogue JSON: " << e.what() << std::endl;
        }

        return nodes;
    }

    static std::unordered_map<String, MangoPtr<DialogueNode>> parseFromFile(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary | std::ios::ate); // 以二进制模式打开，定位到文件末尾
        if (!file.is_open()) {
            std::cerr << "无法打开对话文件: " << filePath << std::endl;
            return {};
        }

        // 获取文件大小
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg); // 回到文件开头

        // 读取完整内容
        std::string jsonContent(size, '\0');
        if (!file.read(jsonContent.data(), size)) {
            std::cerr << "读取文件失败: " << filePath << std::endl;
            return {};
        }
        file.close();

        std::cout << "[对话系统] 文件完整大小: " << size << " 字节" << std::endl;
        //std::cout << "[对话系统] 文件开头50字符: " << jsonContent.substr(0, 50) << std::endl;

        return parseFromJSON(jsonContent);
    }
};
