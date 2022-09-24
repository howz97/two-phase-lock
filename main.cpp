#include <vector>
#include <thread>
#include <algorithm>
#include <mutex>
#include <shared_mutex>

#define S_SIZE 100000
#define WORKER_NUM 1000
#define WORKER_OP 10000

uint64_t S[S_SIZE];
std::shared_mutex MUTEXES[S_SIZE];

void operation()
{
    uint32_t i = rand() % S_SIZE;
    uint32_t j = rand() % S_SIZE;
    std::vector<uint32_t> locks;
    locks.push_back(i);
    locks.push_back(i + 1 % S_SIZE);
    locks.push_back(i + 2 % S_SIZE);
    if (std::find(locks.begin(), locks.end(), j) == locks.end())
    {
        locks.push_back(j);
    }
    // acquire lock in order to avoid dead-lock
    std::sort(locks.begin(), locks.end());
    for (uint32_t l : locks)
    {
        if (l == j)
        {
            MUTEXES[l].lock();
        }
        else
        {
            MUTEXES[l].lock_shared();
        }
    }
    S[j] = S[i] + S[i + 1] + S[i + 2];
    for (uint32_t l : locks)
    {
        if (l == j)
        {
            MUTEXES[l].unlock();
        }
        else
        {
            MUTEXES[l].unlock_shared();
        }
    }
}

void worker()
{
    for (int i = 0; i < WORKER_OP; ++i)
    {
        operation();
    }
}

int main()
{
    // initialize S
    for (int i = 0; i < S_SIZE; ++i)
    {
        S[i] = 1;
    }

    std::vector<std::thread> workers;
    workers.reserve(WORKER_NUM);
    for (int i = 0; i < WORKER_NUM; ++i)
    {
        workers.emplace_back(worker);
    }

    for (auto &w : workers)
    {
        w.join();
    }
}

// g++ -std=c++17 main.cpp -o main -lpthread