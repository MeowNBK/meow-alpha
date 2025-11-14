#pragma once

#include <memory>

class Value;

class Iterator {
public:
    virtual ~Iterator() = default;
    virtual bool hasNext() const = 0;
    virtual Value next() = 0;
};

class Iterable {
public:
    virtual ~Iterable() = default;
    virtual std::unique_ptr<Iterator> makeIterator() = 0;
};

Iterable* toIterable(const Value& value);