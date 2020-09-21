#pragma once

#include <memory>
#include <vector>
#include <utility>

namespace genetic {

class Random {
public:
    virtual int randomInt(size_t bound) = 0;
    virtual double randomDouble() = 0;
    virtual bool randomBool() = 0;
    
    virtual ~Random() {}
};

template <typename T>
class GeneticHelper {
public:
    virtual std::unique_ptr<T> newInstance(Random& random) = 0;
    virtual std::unique_ptr<T> mutate(const T& instance, Random& random) = 0;
    virtual std::unique_ptr<T> cross(const T& left, const T& right, Random& random) = 0;
    virtual double score(const T& instance) = 0;
    
    virtual ~GeneticHelper() {}
};

template <typename T>
class GeneticAlgorithm {
private:
    const size_t population_size;
    GeneticHelper<T>& helper;
    Random& random;
    std::vector<std::pair<std::unique_ptr<T>, double>> population; // sorted list of entities
    
public:
    GeneticAlgorithm(size_t population_size, GeneticHelper<T>& helper, Random& random);
    void train(size_t epochs);
    T& best();
};
}

#include <iostream>
#include <chrono>


namespace genetic {

static inline long long int currentMillis() {
    using namespace std::chrono;
    milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    return ms.count();
}

template <typename T>
static void emplaceBack(std::vector<std::pair<std::unique_ptr<T>, double>>& vector, std::unique_ptr<T> instance, GeneticHelper<T>& helper) {
    double score = helper.score(*instance);
    vector.emplace_back(std::move(instance), score);
}

template <typename T>
GeneticAlgorithm<T>::GeneticAlgorithm(size_t population_size,
                                      GeneticHelper<T>& helper,
                                      Random& random): population_size(population_size), helper(helper), random(random) {
    population.reserve(population_size);
    for (int i = 0; i < population_size; i++) {
        emplaceBack(population, std::move(helper.newInstance(random)), helper);
    }
    std::sort(population.begin(), population.end(), [](auto& a, auto& b){ return a.second < b.second; });
}



template <typename T>
void GeneticAlgorithm<T>::train(size_t epochs) {
    for (int i = 1; i <= epochs; i++) {

        auto start = currentMillis();
        
        std::vector<std::pair<std::unique_ptr<T>, double>> newPopulation;
        newPopulation.reserve(population_size);
        
        // then mutate first 5 elements
        for (int i = 0; i < 5; i++) {
            emplaceBack(newPopulation, helper.mutate(*population[i].first, random), helper);
        }
        
        // then cross first 5 elements with random from all population
        for (int i = 0; i < 5; i++) {
            emplaceBack(newPopulation,
                        helper.cross(*population[i].first, *population[random.randomInt(population.size())].first, random),
                        helper);
        }
        
        // then just random cross
        for (int i = 0; i < population_size - newPopulation.size() - 5; i++) {
           emplaceBack(newPopulation,
                       helper.cross(*population[random.randomInt(population.size())].first,
                                    *population[random.randomInt(population.size())].first,
                                    random),
                       helper);
        }
        
        // copy first 5 elements
        for (int i = 0; i < 5; i++) {
            emplaceBack(newPopulation, std::move(population[i].first), helper);
        }
        
        std::swap(population, newPopulation);
        std::sort(population.begin(), population.end(), [](auto& a, auto& b){ return a.second < b.second; });
        
        auto end = currentMillis();
        
        std::cout << "Epoch " << i << ": time " << (end - start) << " ms, score " << helper.score(*population[0].first) << '\n';
    }
}

template <typename T>
T& GeneticAlgorithm<T>::best() {
    return *population[0].first;
}

}
