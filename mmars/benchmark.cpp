#include "benchmark.hpp"

#include "parser.hpp"

void benchmark::add_warrior(const std::shared_ptr<warrior>& w)
{
    warriors.push_back(w);
}

void benchmark::add_directory(const std::string& path)
{
    for (const auto & entry : fs::directory_iterator(path))
    {
        try
        {
            std::fstream f(entry.path());
            if (!f || f.bad() || !f.is_open()) continue;

            auto parsed = parser::parse(f, core_size);
            if (parsed != nullptr && parsed->code.size() <= max_length)
                warriors.push_back(parsed);

            f.close();
        }
        catch (...)
        {
        }
    }
}

std::any benchmark::run(const std::shared_ptr<warrior>& target)
{
    result sum;

    if(_pool != nullptr)
    {
        std::vector<std::future<result>> results;
        for (auto && enemy : warriors)
        {
            results.push_back(_pool->enqueue_work([&]()
            {
                mmars m(core_size, max_cycles, max_process, max_length, min_separation);
                if (read_limit > 0) m.read_limit = read_limit;
                if (write_limit > 0) m.write_limit = write_limit;

                m.set_seed((uint32_t)time(nullptr));
                m.add_warrior(target);
                m.add_warrior(enemy);
                m.run(rounds_per_enemy);

                return m.get_result(target);
            }));
        }

        for (auto && res : results)
        {
            res.wait();
            result r = res.get();
            sum.win += r.win;
            sum.loss += r.loss;
            sum.tie += r.tie;
        }
    }
    else
    {
        mmars m(core_size, max_cycles, max_process, max_length, min_separation);
        if (read_limit > 0) m.read_limit = read_limit;
        if (write_limit > 0) m.write_limit = write_limit;

        for (auto && enemy : warriors)
        {
            m.clear();
            m.set_seed((uint32_t)time(nullptr));
            m.add_warrior(target);
            m.add_warrior(enemy);
            m.run(rounds_per_enemy);
            
            result r = m.get_result(target);
            sum.win += r.win;
            sum.loss += r.loss;
            sum.tie += r.tie;
        }
    }

    return score_calc(sum, rounds_per_enemy, warriors.size());
}

void benchmark::shutdown()
{
    if(_pool != nullptr)
        _pool->shutdown();
}
