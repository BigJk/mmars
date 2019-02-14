#pragma once

#include <any>
#include <cstdint>
#include <vector>
#include <memory>
#include <fstream>
#include <filesystem>

#include "thread_pool.hpp"
#include "warrior.hpp"
#include "mmars.hpp"

namespace fs = std::experimental::filesystem;

/**
 * \brief Benchmarks a warrior against a set of other warriors. Also supports multi-threading if threads > 1.
 */
class benchmark
{
private:
    std::shared_ptr<thread_pool<result>> _pool = nullptr;

public:
    uint32_t core_size = 8000;
    uint32_t max_cycles = 80000;
    uint32_t max_process = 8000;
    uint32_t max_length = 200;
    uint32_t min_separation = 200;

    uint32_t read_limit = 8000;
    uint32_t write_limit = 8000;

    uint32_t rounds_per_enemy = 100;

    std::vector<std::shared_ptr<warrior>> warriors;

    /**
     * \brief The score calculation function.
     */
    std::function<std::any(result, int, int)> score_calc = [](result res, int rounds, int warriors)
    {
        return std::any(((float)res.win * 3.0f + (float)res.tie) / ((float)warriors * (float)rounds) * 100.0f);
 
    };

    benchmark(uint32_t core_size, uint32_t max_cycles, uint32_t max_process, uint32_t max_length,
        uint32_t min_separation, uint32_t read_limit, uint32_t write_limit, uint32_t rounds_per_enemy, int threads = 1)
        : core_size(core_size),
          max_cycles(max_cycles),
          max_process(max_process),
          max_length(max_length),
          min_separation(min_separation),
          read_limit(read_limit),
          write_limit(write_limit),
          rounds_per_enemy(rounds_per_enemy)
    {
        if(threads > 1)
        {
            _pool = std::make_shared<thread_pool<result>>(threads);
        }
    }

    /**
     * \brief Adds a warrior to the benchmark.
     * \param w The warrior to add to the benchmark
     */
    void add_warrior(const std::shared_ptr<warrior>& w);

    /**
     * \brief Adds all fitting warriors that are inside the given path to the benchmark.
     * \param path The path to the warriors
     */
    void add_directory(const std::string& path);

    /**
     * \brief Runs a benchmark.
     * \param target The warrior to benchmark
     * \return A any with the type defined through the score_calc function
     */
    std::any run(const std::shared_ptr<warrior>& target);

    /**
     * \brief If the benchmark uses multiple threads this will free the thread pool.
     * Benchmark will be unusable after calling this function.
     */
    void shutdown();
};
