#include "mmars.hpp"
#include "parser.hpp"

/*
 * Compile with emscripten:
 * emcc wasm.cpp mmars.cpp parser.cpp -o mmars.js -std=c++1z -s EXPORTED_FUNCTIONS='["_simulate"]' -s EXTRA_EXPORTED_RUNTIME_METHODS='["stringToUTF8", "setValue"]' -s MODULARIZE=1 -s NO_EXIT_RUNTIME=1 -s DISABLE_EXCEPTION_CATCHING=2 -O3
 */

extern "C" {
    void simulate(char** warriors, int size, int core_size, int max_cycles, int max_process, int max_length, int min_separation, int seed, int rounds, int* results)
    {
        parser p(core_size, max_cycles, max_process, max_length, min_separation);
        mmars m(core_size, max_cycles, max_process, max_length, min_separation);

        std::vector<std::shared_ptr<warrior>> parsed_warriors;
        for (int i = 0; i < size; ++i) {
            std::stringstream s;
            s << warriors[i];
            parsed_warriors.push_back(p.parse(s));
            m.add_warrior(parsed_warriors[parsed_warriors.size()-1]);
        }

        m.set_seed(seed > 0 ? seed : rand() % 100000);
        m.run(rounds);

        for (int i = 0; i < parsed_warriors.size(); ++i) {
            auto res = m.get_result(parsed_warriors[i]);

            results[i*3+0] = res.win;
            results[i*3+1] = res.loss;
            results[i*3+2] = res.tie;
        }
    }
}