#pragma once
#include <functional>
#include <iostream>
#include <exception>

class ExceptionMonitor {
public:
    using Callback = std::function<void(const std::exception&)>;

    static void SetHandler(Callback cb) {
        GetInstance().handler = std::move(cb);
    }

    static void Execute(const std::function<void()>& func) {
        try {
            func();
        }
        catch (const std::exception& e) {
            if (GetInstance().handler) GetInstance().handler(e);
            else std::cerr << "[Unhandled Exception] " << e.what() << std::endl;
        }
        catch (...) {
            std::cerr << "[Unknown Exception] Caught" << std::endl;
        }
    }

private:
    Callback handler;

    static ExceptionMonitor& GetInstance() {
        static ExceptionMonitor instance;
        return instance;
    }
};
