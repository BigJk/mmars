#include <fstream>
#include <chrono>

#include "mmars.hpp"
#include "parser.hpp"
#include "cli11.hpp"
#include "benchmark.hpp"

int main(int argc, char *argv[])
{
    /*
     * CLI Setup
     */
    CLI::App app{
            "    _____ ___  ____ ___  ____ ___________\n"
                   "  __/ __ `__ \\/ __ `__ \\/ __ `/ ___/ ___/\n"
                   " __/ / / / / / / / / / / /_/ / /  (__  )\n"
                   " _/_/ /_/ /_/_/ /_/ /_/\\__,_/_/  /____/  by BigJk\n\n"
                   "mmars - c++ modern mars" };

    std::vector<std::string> warrior_paths;
    app.add_option("-w,--w,--warrior", warrior_paths, "a list of warrior paths");

    int core_size = 8000;
    int max_cycles = 80000;
    int max_process = 8000;
    int max_length = 200;
    int min_separation = 200;

    int read_limit = 0;
    int write_limit = 0;
    int rounds = 100;
    int initial_pos = 0;

    app.add_option("-s,--s,--core_size", core_size, "Core size");
    app.add_option("-c,--c,--max_cycle", max_cycles, "Maximum cycles");
    app.add_option("-p,--p,--max_process", max_process, "Maximum processes");
    app.add_option("-l,--l,--max_length", max_length, "Maximum length");
    app.add_option("-d,--d,--min_separation", min_separation, "Minimum separation");
    app.add_option("-r,--r,--rounds", rounds, "Rounds to fight");
    app.add_option("-f,--f,--fixed_pos", initial_pos, "Fixed position for the first round (will also be used as seed)");
    app.add_option("--rl,--read_limit", read_limit, "Read limit (defaults to core size)");
    app.add_option("--wl,--write_limit", write_limit, "Write limit (defaults to core size)");

    int benchmark_threads = std::max(1, (int)std::thread::hardware_concurrency());
    std::string benchmark_path = "";
    app.add_option("-b,--b,--bench_path", benchmark_path, "The path to a folder that contains the warriors to benchmark against");
    app.add_option("-t,--t,--bench_threads", benchmark_threads, "The amount of threads to use for the benchmark");

    try {
        app.parse(argc, argv);
    }
    catch (const CLI::ParseError &e) {
        return app.exit(e);
    }

    if(warrior_paths.empty())
    {
        printf(app.help().c_str());
        return 0;
    }

    if(!benchmark_path.empty() && warrior_paths.size() > 1)
    {
        printf("You can only benchmark one warrior. Please try again with one warrior path!");
        return 0;
    }

    /*
     * Parse Warrior
     */
    std::vector<std::shared_ptr<warrior>> parsed;
    for (auto && path : warrior_paths)
    {
        std::ifstream f(path);
        if (!f || !f.is_open() || f.bad())
        {
            printf("can't open warrior: %s", path.c_str());
            return 0;
        }

        auto w = parser::parse(f, core_size);
        parsed.push_back(w);

        f.close();
    }

    /*
     * Benchmark
     */
    if (!benchmark_path.empty())
    {
        benchmark b(core_size, max_cycles, max_process, max_length, min_separation, read_limit, write_limit, rounds, benchmark_threads);
        b.add_directory(benchmark_path);

        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        float res = std::any_cast<float>(b.run(parsed[0]));
        int64_t time_taken = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin).count();

        b.shutdown();

        if (parsed[0]->name.empty()) printf("Warrior=%-20d score=%.03f\n", i, res);
        else printf("Warrior=%-20s score=%.03f\n", parsed[0]->name.c_str(), res);
        printf("Finished in %lldms (%.2fms/round)", time_taken, (float)time_taken / (float)(rounds * b.warriors.size()));

        return 0;
    }

    /*
     * Create Mars
     */
    mmars m(core_size, max_cycles, max_process, max_length, min_separation);
    if (read_limit > 0) m.read_limit = read_limit;
    if (write_limit > 0) m.write_limit = write_limit;

    for (auto && w : parsed)
    {
        m.add_warrior(w);
    }

    /*
     * Seed
     */
    if (initial_pos > 0) m.set_seed(initial_pos - min_separation);
    else m.set_seed((uint32_t)time(nullptr));

    /*
     * Simulate
     */
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    m.run(rounds);
    int64_t time_taken = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin).count();

    /*
     * Print Results
    */
    int i = 0;
    for (auto && warrior : parsed)
    {
        auto res = m.get_result(warrior);
        if (warrior->name.empty()) printf("Warrior=%-20d win=%d loss=%d tie=%d\n", i, res.win, res.loss, res.tie);
        else printf("Warrior=%-20s win=%d loss=%d tie=%d\n", warrior->name.c_str(), res.win, res.loss, res.tie);
        i++;
    }
    printf("Finished in %lldms (%.2fms/round)", time_taken, (float)time_taken / (float)rounds);
}
