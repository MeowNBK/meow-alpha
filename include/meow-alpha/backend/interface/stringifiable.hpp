#pragma once

#include <string>

class Value;

class Stringifiable {
public:
    virtual ~Stringifiable() = default;

    virtual std::string toString() const = 0;
};

Stringifiable* toStringifiable(const Value& value);