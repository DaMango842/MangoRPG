#pragma once

#include <functional>
#include <vector>

template<typename... Args>
class Signal {
public:
    using SlotType = std::function<void(Args...)>;

    void connect(const SlotType& slot) {
        slots.push_back(slot);
    }

    void emit(Args... args) {
        for (auto& slot : slots) {
            slot(args...);
        }
    }

private:
    std::vector<SlotType> slots;
};

