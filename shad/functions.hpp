#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <functional>
#include <vector>

namespace functions {

class ArgumentProvider {
public:
    virtual bool has(const std::string& name) const = 0;
    virtual double get(const std::string& name) const = 0;
    virtual ~ArgumentProvider() {}
};

class EmptyArgumentProvider: public ArgumentProvider {
public:
    virtual bool has(const std::string& name) const {
        return false;
    }
    virtual double get(const std::string& name) const {
        throw 1;
    }
    virtual ~EmptyArgumentProvider() {}
};

class OneVariableProvider: public ArgumentProvider {
private:
    std::string name;
    double value;
public:
    OneVariableProvider(const std::string& name): name(name) {}
    virtual bool has(const std::string& name) const;
    virtual double get(const std::string& name) const;
    void set(double value);
    virtual ~OneVariableProvider() {}
};

class FallbackProvider: public ArgumentProvider {
private:
    std::vector<const ArgumentProvider*> providers;
public:
    FallbackProvider(const std::vector<const ArgumentProvider*>& providers): providers(providers) {}
    virtual bool has(const std::string& name) const;
    virtual double get(const std::string& name) const;
    virtual ~FallbackProvider() {}
};

class Function {
public:
    virtual double eval(const ArgumentProvider& provider) const = 0;
    virtual ~Function() {}
};

class ConstFunction: public Function {
private:
    const double value;
    
public:
    ConstFunction(double value): value(value) {}
    virtual double eval(const ArgumentProvider& provider) const;
};

class OneArgFunction: public Function {
private:
    const std::function<double(double)> fun;
    const std::unique_ptr<Function> arg;
    
public:
    OneArgFunction(std::function<double(double)> fun, std::unique_ptr<Function> arg): fun(fun), arg(std::move(arg)) {}
    virtual double eval(const ArgumentProvider& provider) const;
};

class VariableFunction: public Function {
private:
    const std::string name;
    
public:
    VariableFunction(const std::string& name): name(name) {}
    virtual double eval(const ArgumentProvider& provider) const;
};

class TwoArgFunction: public Function {
private:
    const std::function<double(double, double)> fun;
    const std::unique_ptr<Function> left;
    const std::unique_ptr<Function> right;
    
public:
    TwoArgFunction(std::function<double(double, double)> fun,
                   std::unique_ptr<Function> left,
                   std::unique_ptr<Function> right): fun(fun), left(std::move(left)), right(std::move(right)) {}
    virtual double eval(const ArgumentProvider& provider) const;
};

class NumericFunction: public Function {
private:
    double left, leftValue, right, rightValue, dx;
    std::vector<double> values;
    const std::string variable;
public:
    NumericFunction(double left, double leftValue, double right, double rightValue, double dx, std::vector<double>&& values, const std::string& variable);
    virtual double eval(const ArgumentProvider& provider) const;
    virtual ~NumericFunction() {}
};

// scope should be as the original function
class BindedFunction: public Function {
private:
    const Function& original;
    const ArgumentProvider& defaultProvider;
public:
    BindedFunction(const Function& f, const ArgumentProvider& defaultProvider): original(f), defaultProvider(defaultProvider) {}
    virtual double eval(const ArgumentProvider& provider) const;
    virtual ~BindedFunction() {}
};

std::unique_ptr<Function> bind(Function& f, ArgumentProvider& provider);
std::unique_ptr<Function> integrate(const Function& f, double left, double right, double dx, const std::string& variable, const ArgumentProvider& provider);
double product(const Function& f1, const Function& f2, double left, double right, double dx, const std::string& variable, const ArgumentProvider& provider);

}

