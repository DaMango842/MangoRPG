// DialogueSystem.h
#pragma once
#include "BaseDialogue.h"
#include "TextDisplay.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <functional>
#include <unordered_map>
#include <memory>
#include <MangoString.hpp>
#include <MangoPtr.hpp>

#include <nlohmann/json.hpp>

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
    String nextNode;         // 下一个节点ID

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
    std::function<void()> action;
    String nextNode; // 执行后的下一个节点

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

    bool isStringValid(const String& str) const;

    // 数据成员
    const sf::Font& m_font;
    TextDisplay m_textDisplay;
    sf::Text m_nameText;
    sf::RectangleShape m_bg;
    sf::RectangleShape m_nameBg;

    // 选项框相关
    sf::RectangleShape m_optionsBg;
    sf::Vector2f m_optionsPosition;
    float m_optionSpacing = 10.f;

    // 当前选项列表
    std::vector<MangoPtr<ChoiceNode::Option>> m_currentChoices;

    // 使用智能指针管理节点
    std::unordered_map<String, MangoPtr<DialogueNode>> m_nodes;
    std::unordered_map<String, String> m_characters;
    std::unordered_map<String, int> m_variables;

    MangoPtr<DialogueNode> m_currentNode = nullptr;
    String m_startNodeId;
    int m_selectedChoice = -1;

    // 对话结束回调
    std::function<void()> m_onDialogueEnded;

    // 自动结束计时器
    float m_autoEndTimer = 0.f;

    State m_state = State::Inactive;
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

                        if (optionData.contains("nextDialogueId") &&
                            !optionData["nextDialogueId"].is_null()) {
                            option->nextNode = optionData["nextDialogueId"].get<std::string>();
                        }

                        choiceNode->options.push_back(std::move(option));
                    }

                    nodes[nodeId] = std::move(choiceNode);
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

                    nodes[nodeId] = std::move(textNode);
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Failed to parse dialogue JSON: " << e.what() << std::endl;
        }

        return nodes;
    }
};
