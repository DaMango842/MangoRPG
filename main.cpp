#include "Game.h"
#include "Utils.h"
#include "ExceptionMonitor.h"
#include "Logger.h"
#include "Translator.h"


#include <Enemy.h>

#include <MangoString.hpp>

int main(int argc, char* argv[])
{
    enableANSIColor();

    Translator::get().setLanguage("zh_CN");

    LOG_DEBUG("日志已启动");
    
    ExceptionMonitor::Execute([&] {
        Game game;
        game.run();
    });

    
    return 0;
}

//#include "DialogueSystem.h"
//#include <SFML/Graphics.hpp>
//
//#include "Window.h"
//
//#include <iostream>
//
//int main() {
//    enableANSIColor();
//
//    auto& windowInstance = Window::getInstance();
//
//    windowInstance.create(sf::VideoMode(800, 600), "Dialogue Demo");
//
//    sf::Font font;
//    font.loadFromFile("Assets/Font/fusion-pixel-12px.ttf");
//
//    DialogueParser parser;
//    auto nodes = parser.parseFromFile("Assets/Dialogues/testDialogue.json");
//
//    // 创建对话系统
//
//    DialogueSystem dialogue(font);
//    for (auto& [id, node] : nodes) {
//        dialogue.addNode(std::move(node));
//    }
//
//    dialogue.setOptionsBoxStyle(
//        sf::Color(0, 0, 0, 200),  // 填充颜色
//        sf::Color::White,          // 边框颜色
//        2.f                        // 边框厚度
//    );
//
//    // 设置选项间距
//    dialogue.setOptionSpacing(15.f);
//
//	// 从JSON文件加载对话
//   
//
//
//    // 设置角色名称
//    //dialogue.setCharacterName("player", "玩家");
//    //dialogue.setCharacterName("npc", "神秘老人");
//
//    //dialogue.setTextSpeedNormal();
//
//    //// 创建对话节点
//    //// 第一个文本节点
//    //MangoPtr<TextNode> node1 = make_mango_ptr<TextNode>();
//    //node1->id = "start";
//    //node1->characterName = "神秘老人";
//    //node1->text = "你好，旅行者！你需要帮助吗？";
//    ////node1->displaySpeed = 0.05f;
//    //node1->textColor = sf::Color::White;
//    //node1->nextNode = "choices";
//
//    //// 选项节点
//    //MangoPtr<ChoiceNode> choices = make_mango_ptr<ChoiceNode>();
//    //choices->id = "choices";
//
//    //// 选项1
//    //MangoPtr<ChoiceNode::Option> option1 = make_mango_ptr<ChoiceNode::Option>();
//    //option1->text = "是的，我需要指引";
//    //option1->callback = [] { std::cout << "选择了选项1\n"; };
//    //option1->normal = sf::Color::White;
//    //option1->hover = sf::Color::Yellow;
//    //option1->nextNode = "help_response";
//
//    //// 选项2
//    //MangoPtr<ChoiceNode::Option> option2 = make_mango_ptr<ChoiceNode::Option>();
//    //option2->text = "不，我只是路过";
//    //option2->callback = [] { std::cout << "选择了选项2\n"; };
//    //option2->normal = sf::Color::White;
//    //option2->hover = sf::Color::Yellow;
//    //option2->nextNode = "bye_response";
//
//    //// 选项3
//    //MangoPtr<ChoiceNode::Option> option3 = make_mango_ptr<ChoiceNode::Option>();
//    //option3->text = "你能告诉我更多信息吗？";
//    //option3->callback = [] { std::cout << "选择了选项3\n"; };
//    //option3->normal = sf::Color::White;
//    //option3->hover = sf::Color::Yellow;
//    //option3->nextNode = "more_info";
//
//    //choices->options.push_back(option1);
//    //choices->options.push_back(option2);
//    //choices->options.push_back(option3);
//
//    //// 回应节点
//    //MangoPtr<TextNode> helpResponse = make_mango_ptr<TextNode>();
//    //helpResponse->id = "help_response";
//    //helpResponse->characterName = "神秘老人";
//    //helpResponse->text = "很好，我会指引你前进的道路。";
//    //helpResponse->nextNode = ""; // 结束对话
//
//    //MangoPtr<TextNode> byeResponse = make_mango_ptr<TextNode>();
//    //byeResponse->id = "bye_response";
//    //byeResponse->characterName = "神秘老人";
//    //byeResponse->text = "好吧，祝你旅途愉快。";
//    //byeResponse->nextNode = "exit_game"; // 结束对话
//
//    //MangoPtr<TextNode> moreInfo = make_mango_ptr<TextNode>();
//    //moreInfo->id = "more_info";
//    //moreInfo->characterName = "神秘老人";
//    //moreInfo->text = "这个世界充满了神秘和危险，但也充满了机遇。";
//    //moreInfo->nextNode = "choices"; // 返回选项
//
//    //MangoPtr<ActionNode> exitAction = make_mango_ptr<ActionNode>();
//    //exitAction->id = "exit_game";
//    //exitAction->action = [&] {
//    //    std::exit(0);
//    //    };
//
//
//    //// 添加节点到对话系统
//    //dialogue.addNode(node1);
//    //dialogue.addNode(choices);
//    //dialogue.addNode(helpResponse);
//    //dialogue.addNode(byeResponse);
//    //dialogue.addNode(moreInfo);
//    //dialogue.addNode(exitAction);
//
//    // 设置起始节点
//    //dialogue.setStartNode("start");
//	dialogue.setStartNode("testDialogue_1");
//
//    // 开始对话
//    dialogue.startDialogue();
//
//    sf::Clock clock;
//    while (windowInstance.isOpen()) {
//        sf::Event event;
//        while (windowInstance.pollEvent(event)) {
//            if (event.type == sf::Event::Closed)
//                windowInstance.close();
//            dialogue.handleEvent(event);
//        }
//
//        dialogue.update(clock.restart().asSeconds());
//
//        windowInstance.clear(sf::Color(50, 50, 50)); // 深灰色背景
//        dialogue.render(windowInstance.getWindow());
//        windowInstance.display();
//    }
//
//    return 0;
//}

//int main()
//{
//	String name{ "Mango" };
//
//	std::cout << "Hello, " << name << "!" << std::endl;
//
//	return 0;
//}
