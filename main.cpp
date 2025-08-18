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

