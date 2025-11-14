#pragma once

#include "interface/callable.hpp"
#include "common/token.hpp"

#include "interface/indexable.hpp"
#include "interface/iterable.hpp"
#include "interface/stringifiable.hpp"

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

struct ObjectData;
class Value;
using Object = std::shared_ptr<ObjectData>;

class MeowScriptClass;
class MeowScriptInstance;
class MeowScriptBoundMethod;

using Class = std::shared_ptr<MeowScriptClass>;
using Instance = std::shared_ptr<MeowScriptInstance>;
using BoundMethod = std::shared_ptr<MeowScriptBoundMethod>;

class MeowScriptClass : public Callable, public Stringifiable, public Indexable, public std::enable_shared_from_this<MeowScriptClass> {
public:
    std::string name;
    Class superclass;
    std::unordered_map<std::string, Function> methods;
    std::unordered_map<std::string, Value> static_fields;

    MeowScriptClass(std::string name, Class super) : name(std::move(name)), superclass(std::move(super)) {}
    ~MeowScriptClass() override = default;

    Value call(Interpreter* engine, const std::vector<Value>& args) override;
    
    Arity arity() const override;

    Function findMethod(const std::string& methodName) const;

    Value get(const Value& key) override;
    void set(const Value& key, const Value& value) override;
    
    std::string toString() const override;
};

class InstanceIterator : public Iterator {
private:
    std::unique_ptr<Value> iteratorObject;
    Interpreter* engine;
    std::unique_ptr<Value> nextValue;
    bool isFinished;

    void advance();
public:
    InstanceIterator(MeowScriptInstance* data);
    ~InstanceIterator();
    bool hasNext() const override;
    Value next() override;
};

class MeowScriptInstance : public Indexable, public Iterable, public Callable, public Stringifiable, public std::enable_shared_from_this<MeowScriptInstance> {
public:
    Class klass; 

    Object fields = std::make_shared<ObjectData>();

    Interpreter* engine;

    MeowScriptInstance(Class k, Interpreter* e) : klass(std::move(k)), engine(std::move(e)) {}

    Value get(const Value& key) override;
    
    void set(const Value& key, const Value& value) override;

    Value call(Interpreter* engine, const std::vector<Value>& args) override;

    Arity arity() const override;

    std::unique_ptr<Iterator> makeIterator() override;
    
    std::string toString() const override;
};

class MeowScriptBoundMethod : public Callable, public Stringifiable, public Indexable {
public:
    Instance instance;
    Function function;

    MeowScriptBoundMethod(Instance inst, Function func) : instance(std::move(inst)), function(std::move(func)) {}

    Value call(Interpreter* engine, const std::vector<Value>& args) override;

    Arity arity() const override;

    Value get(const Value& key) override;
    
    void set(const Value& key, const Value& value) override;

    std::string toString() const override;
};