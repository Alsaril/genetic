#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <functional>
#include <vector>

namespace functions {

class ArgumentProvider {
public:
    virtual bool has(const std::string& name) = 0;
    virtual double get(const std::string& name) = 0;
    virtual ~ArgumentProvider() {}
};

class OneVariableProvider: public ArgumentProvider {
private:
    std::string name;
    double value;
public:
    OneVariableProvider(const std::string& name): name(name) {}
    virtual bool has(const std::string& name);
    virtual double get(const std::string& name);
    void set(double value);
    virtual ~OneVariableProvider() {}
};

class FallbackProvider: public ArgumentProvider {
private:
    std::vector<std::reference_wrapper<ArgumentProvider>> providers;
public:
    FallbackProvider(std::vector<std::reference_wrapper<ArgumentProvider>>&& providers): providers(std::move(providers)) {}
    virtual bool has(const std::string& name);
    virtual double get(const std::string& name);
    virtual ~FallbackProvider() {}
};

class Function {
public:
    virtual double eval(ArgumentProvider& provider) = 0;
    virtual ~Function() {}
};

class NoArgFunction: public Function {
private:
    const double value;
    
public:
    NoArgFunction(double value): value(value) {}
    virtual double eval(ArgumentProvider& provider);
};

class OneArgFunction: public Function {
private:
    const std::function<double(double)> fun;
    const std::unique_ptr<Function> arg;
    
public:
    OneArgFunction(std::function<double(double)> fun, std::unique_ptr<Function> arg): fun(fun), arg(std::move(arg)) {}
    virtual double eval(ArgumentProvider& provider);
};

class VariableFunction: public Function {
private:
    const std::string name;
    
public:
    VariableFunction(const std::string& name): name(name) {}
    virtual double eval(ArgumentProvider& provider);
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
    virtual double eval(ArgumentProvider& provider);
};

class NumericFunction: public Function {
private:
    double left, leftValue, right, rightValue, dx;
    std::vector<double> values;
    std::string variable;
public:
    NumericFunction(double left, double leftValue, double right, double rightValue, double dx, std::vector<double>&& values, const std::string& variable);
    virtual double eval(ArgumentProvider& provider);
    virtual ~NumericFunction() {}
};

// scope should be as the original function
class BindedFunction: public Function {
private:
    Function& original;
    ArgumentProvider& defaultProvider;
public:
    BindedFunction(Function& f, ArgumentProvider& defaultProvider): original(f), defaultProvider(defaultProvider) {}
    virtual double eval(ArgumentProvider& provider);
    virtual ~BindedFunction() {}
};

std::unique_ptr<Function> bind(Function& f, ArgumentProvider& provider);
std::unique_ptr<Function> integrate(Function& f, double left, double right, double dx, const std::string& variable, ArgumentProvider& provider);
double product(Function& f1, Function& f2, double left, double right, double dx);

}

