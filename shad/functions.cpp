#include "functions.hpp"

#include <cassert>
#include <algorithm>

namespace functions {

bool OneVariableProvider::has(const std::string& name) const {
    return name == this->name;
}

double OneVariableProvider::get(const std::string& name) const {
    assert(has(name));
    return value;
}

void OneVariableProvider::set(double value) {
    this->value = value;
}

bool FallbackProvider::has(const std::string& name) const {
    return std::any_of(providers.begin(), providers.end(), [&name](auto& p){ return p->has(name); });
}

double FallbackProvider::get(const std::string& name) const {
    for (auto& provider: providers) {
        if (provider->has(name)) {
            return provider->get(name);
        }
    }
    throw 1;
}

double ConstFunction::eval(const ArgumentProvider& provider) const {
    return value;
}

double OneArgFunction::eval(const ArgumentProvider& provider) const {
    return fun(arg->eval(provider));
}

double VariableFunction::eval(const ArgumentProvider& provider) const {
    return provider.get(name);
}

double TwoArgFunction::eval(const ArgumentProvider& provider) const {
    return fun(left->eval(provider), right->eval(provider));
}

NumericFunction::NumericFunction(double left,
                                 double leftValue,
                                 double right,
                                 double rightValue,
                                 double dx,
                                 std::vector<double>&& values,
                                 const std::string& variable):
                                    left(left),
                                    leftValue(leftValue),
                                    right(right),
                                    rightValue(rightValue),
                                    dx(dx),
                                    values(std::move(values)),
                                    variable(variable){
    assert(left < right);
    assert(values.size() == (right - left) / dx + 1);
}

double NumericFunction::eval(const ArgumentProvider& provider) const {
    double x = provider.get(variable);
    if (x < left) {
        return leftValue;
    }
    if (x > right) {
        return rightValue;
    }
    return values[static_cast<int>((x - left) / dx)];
}

double BindedFunction::eval(const ArgumentProvider& provider) const {
    FallbackProvider fallbackProvider({ &defaultProvider, &provider });
    return original.eval(fallbackProvider);
}

std::unique_ptr<Function> bind(const Function& f, const ArgumentProvider& provider) {
    return std::make_unique<BindedFunction>(f, provider);
}

std::unique_ptr<Function> integrate(const Function& f, double left, double right, double dx, const std::string& variable, const ArgumentProvider& provider) {
    std::vector<double> result;
    double sum = 0.0;
    result.reserve(static_cast<int>((right - left) / dx));
    std::unique_ptr<Function> binded = bind(f, provider);
    OneVariableProvider oneVariableProvider(variable);
    while (left < right) {
        oneVariableProvider.set(left);
        sum += binded->eval(oneVariableProvider);
        result.push_back(sum);
        left += dx;
    }
    return std::make_unique<NumericFunction>(left, 0.0, right, sum, dx, std::move(result), variable);
}

double product(const Function& f1, const Function& f2, double left, double right, double dx, const std::string& variable, const ArgumentProvider& provider) {
    std::unique_ptr<Function> b1 = bind(f1, provider);
    std::unique_ptr<Function> b2 = bind(f2, provider);
    OneVariableProvider oneVariableProvider(variable);
    
    double mul = 0.0, sum1 = 0.0, sum2 = 0.0;
    
    while (left < right) {
        oneVariableProvider.set(left);
        
        double v1 = b1->eval(oneVariableProvider);
        double v2 = b2->eval(oneVariableProvider);
        
        mul += v1 * v2;
        sum1 += v1 * v1;
        sum2 += v2 * v2;
        
        left += dx;
    }
    
    return mul / sqrt(sum1 * sum2);
}

}
