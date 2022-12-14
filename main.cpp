#include <pthread.h>
#include <chrono>
#include <thread>
#include <iostream>
#include <string>
#include <utility>
#include <ctime>
#include <unistd.h>
#include <random>

/*
 * Philosopher struct
 */
struct Philosopher {
    std::string name;
    size_t left_fork;
    size_t right_fork;
    unsigned seed;

    Philosopher() {
        this->name = "";
        this->left_fork = 0;
        this->right_fork = 0;
        seed = time(0);
    }

    Philosopher(std::string name, size_t left_fork, size_t right_fork, unsigned seed) {
        this->name = std::move(name);
        this->left_fork = left_fork;
        this->right_fork = right_fork;
        this->seed = seed * 100;
    }
};

struct Table {
    std::vector<pthread_mutex_t> forks;

    explicit Table(size_t n) {
        forks.resize(n);
        for (auto & fork : this->forks) {
            pthread_mutex_init(&fork, nullptr);
        }
    }
};

struct Philosopher_args {
    Philosopher* philosopher;
    Table* table;
};

pthread_mutex_t entry_point = PTHREAD_MUTEX_INITIALIZER;

int intRand(const int& min, const int& max) {
    static thread_local std::mt19937 generator;
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(generator);
}

void* life_cycle(void* args) {
    auto* arg = (Philosopher_args*) args;
    Philosopher* philosopher = arg->philosopher;
    Table* table = arg->table;

    while (true) {
        //lock taking forks
        pthread_mutex_lock(&entry_point);
        printf("%s started dinner\n", philosopher->name.c_str());
        //take left fork
        pthread_mutex_lock(&table->forks[philosopher->left_fork]);

        int time_r = intRand(0, 100 + philosopher->seed);
        std::this_thread::sleep_for(std::chrono::milliseconds(time_r));
        //take right fork
        pthread_mutex_lock(&table->forks[philosopher->right_fork]);
        pthread_mutex_unlock(&entry_point);

        printf("%s ended dinner after %d ms\n", philosopher->name.c_str(), time_r);

        pthread_mutex_unlock(&table->forks[philosopher->right_fork]);
        pthread_mutex_unlock(&table->forks[philosopher->left_fork]);

        printf("%s put back forks\n", philosopher->name.c_str());

        time_r = intRand(0, 100 + philosopher->seed);

        printf("%s took rest for %d ms\n", philosopher->name.c_str(), time_r);
        std::this_thread::sleep_for(std::chrono::milliseconds(time_r));
    }
}

int main(int argc, char** argv) {
    size_t n;

    std::cin >> n;

    if (n < 3 || n > 100) {
        std::cout << "incorrect input\n";
        return 0;
    }

    pthread_t threads[n];
    Philosopher philosophers[n];
    Philosopher_args args[n];
    Table table(n);

    for (size_t i = 0; i < n; ++i) {
        philosophers[i] = Philosopher(std::to_string(i + 1) + " philosopher", i, (i + 1) % n, i);
        args[i].philosopher = &philosophers[i];
        args[i].table = &table;
    }

    for (size_t i = 0; i < n; i++) {
        pthread_create(&threads[i], nullptr, life_cycle, &args[i]);
    }

    for (size_t i = 0; i < n; i++) {
        pthread_join(threads[i], nullptr);
    }
}