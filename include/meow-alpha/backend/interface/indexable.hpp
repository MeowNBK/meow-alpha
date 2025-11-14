#pragma once

class Value;

class Indexable {
public:
    virtual ~Indexable() = default;

    virtual Value get(const Value& key) = 0;

    virtual void set(const Value& key, const Value& value) = 0;
};

Indexable* toIndexable(const Value& value);