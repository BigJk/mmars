#include <fstream>
#include <chrono>

#include "core.hpp"
#include "parser.hpp"
#include "cli11.hpp"

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

    /*
     * Create Mars
     */
    mmars m(core_size, max_cycles, max_process, max_length, min_separation);
    if (read_limit > 0) m.read_limit = read_limit;
    if (write_limit > 0) m.write_limit = write_limit;

    /*
     * Seed
     */
    if (initial_pos > 0) m.set_seed(initial_pos - min_separation);
    else m.set_seed(time(nullptr));

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

        auto w = parser::parse(f, m.core_size);
        m.add_warrior(w);
        parsed.push_back(w);

        f.close();
    }

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
