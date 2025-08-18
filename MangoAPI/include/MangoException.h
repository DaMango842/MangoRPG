#pragma once

#include <exception>
#include <string>
#include <sstream>
#include "Attribute.h"

class AttributeException : public std::exception
{
public:
    explicit AttributeException(const std::string& message)
        : msg(message)
    {
    }

    explicit AttributeException(const Attribute& attr)
    {
        std::ostringstream oss;
        bool hasError = false;

        for (const auto& key : attributeDisplayOrder) {
            if (!attr.has(key)) {
                oss << "Missing attribute: " << key << "\n";
                hasError = true;
            }
        }

        if (!hasError) {
            oss << "All required attributes are present.";
        }

        msg = oss.str();
    }

    virtual const char* what() const noexcept override
    {
        return msg.c_str();
    }

private:
    std::string msg;
};
