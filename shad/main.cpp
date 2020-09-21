#include "functions.hpp"
#include "genetic.hpp"

#include <iostream>
#include <random>

class GeneticFunctionHelper: public genetic::GeneticHelper<functions::Function> {
public:
    GeneticFunctionHelper() {
        
    }
    
    virtual std::unique_ptr<functions::Function> newInstance(genetic::Random& random) {
        return std::make_unique<functions::NoArgFunction>(10);
    }
    
    virtual std::unique_ptr<functions::Function> mutate(const functions::Function& instance, genetic::Random& random) {
        return std::make_unique<functions::NoArgFunction>(10);
    }
    
    virtual std::unique_ptr<functions::Function> cross(const functions::Function& left, const functions::Function& right, genetic::Random& random) {
        return std::make_unique<functions::NoArgFunction>(10);
    }
    
    virtual double score(const functions::Function& instance) {
        return 0;
    }
    
    virtual ~GeneticFunctionHelper() {}
};

class Random: public genetic::Random {
private:
    std::mt19937 engine;
    std::uniform_real_distribution<> dist;
public:
    Random(unsigned int seed): engine(seed), dist(0, 1) {}
    
    virtual int randomInt(size_t bound) {
        return std::uniform_int_distribution<>(0, static_cast<unsigned int>(bound))(engine);
    }
    
    virtual double randomDouble()  {
        return dist(engine);
    }
    
    virtual bool randomBool() {
        return dist(engine) > 0.5;
    }
       
    virtual ~Random() {}
};


int main() {
    GeneticFunctionHelper helper;
    Random random(42);
    
    genetic::GeneticAlgorithm<functions::Function> genetic(50, helper, random);
    
    genetic.train(10);
    
    functions::Function& best = genetic.best();
}
