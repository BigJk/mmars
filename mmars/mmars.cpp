#include "mmars.hpp"

#define arith(op) \
       switch (ir.mod) { \
       case modifier::a: \
          _core[(pc + wpb) % core_size].a = \
             (irb.a op ira.a) % core_size \
          ; \
          break; \
       case modifier::b: \
          _core[(pc + wpb) % core_size].b = \
             (irb.b op ira.b) % core_size \
          ; \
          break; \
       case modifier::ab: \
          _core[(pc + wpb) % core_size].b = \
             (irb.b op ira.a) % core_size \
          ; \
          break; \
       case modifier::ba: \
          _core[(pc + wpb) % core_size].a = \
             (irb.b op ira.a) % core_size \
          ; \
          break; \
       case modifier::f: \
       case modifier::i: \
          _core[(pc + wpb) % core_size].a = \
             (irb.a op ira.a) % core_size \
          ; \
          _core[(pc + wpb) % core_size].b = \
             (irb.b op ira.b) % core_size \
          ; \
          break; \
       case modifier::x: \
          _core[(pc + wpb) % core_size].b = \
             (irb.a op ira.b) % core_size \
          ; \
          _core[(pc + wpb) % core_size].a = \
             (irb.b op ira.a) % core_size \
          ; \
          break; \
       default: \
          throw std::runtime_error("unsupported operation"); \
       }; \
       queue(ri, (pc + 1) % core_size); \
       break;

#define arith_div(op) \
       switch (ir.mod) { \
       case modifier::a: \
          if (ira.a != 0) \
             _core[(pc + wpb) % core_size].a = irb.a op ira.a; \
          else do_queue = false; \
          break; \
       case modifier::b: \
          if (ira.b != 0) \
             _core[(pc + wpb) % core_size].b = irb.b op ira.b; \
          else do_queue = false; \
          break; \
       case modifier::ab: \
          if (ira.a != 0) \
             _core[(pc + wpb) % core_size].b = irb.b op ira.a; \
          else do_queue = false; \
          break; \
       case modifier::ba: \
          if (ira.b != 0) \
             _core[(pc + wpb) % core_size].a = irb.a op ira.b; \
          else do_queue = false; \
          break; \
       case modifier::f: \
       case modifier::i: \
          if (ira.a != 0) \
             _core[(pc + wpb) % core_size].a = irb.a op ira.a; \
          if (ira.b != 0) \
             _core[(pc + wpb) % core_size].b = irb.b op ira.b; \
          if ((ira.a == 0) || (ira.b == 0)) \
             do_queue = false; \
          break; \
       case modifier::x: \
          if (ira.a != 0) \
             _core[(pc + wpb) % core_size].b = irb.b op ira.a; \
          if (ira.b != 0) \
             _core[(pc + wpb) % core_size].a = irb.a op ira.b; \
          if ((ira.a == 0) || (ira.b == 0)) \
             do_queue = false; \
          break; \
       default: \
          throw std::runtime_error("unsupported operation"); \
       }; \
       if(do_queue) queue(ri, (pc + 1) % core_size); \
       break;

inline uint32_t mmars::fold(uint32_t ptr, uint32_t limit) const
{
    uint32_t res = ptr % limit;
    if(res > (limit / 2)) res += core_size - limit;
    return res;
}

inline void mmars::queue(int wi, uint32_t ptr)
{
    _task_queue[wi].enqueue(ptr);
}

inline int mmars::random()
{
    _seed = 16807 * (_seed % 127773) - 2836 * (_seed / 127773);
    if (_seed < 0)
        _seed += 2147483647;
    return _seed;
}

uint32_t mmars::copy_warrior(int wi, uint32_t p)
{
    auto w = _warriors[wi];
    for (uint32_t i = 0; i < w->code.size(); ++i)
    {
        _core[i + p] = w->code[i];
    }
    _task_queue[wi].enqueue(p + w->start);
    return p + (uint32_t)w->code.size();
}

void mmars::insert_warriors()
{
    std::vector<uint32_t> target_positions = std::vector<uint32_t>(_warriors.size());

    if (_warriors.size() == 2) {
        target_positions[1] = min_separation + _seed % (core_size + 1 - (uint32_t)_warriors.size() * min_separation);
        random();
    }
    else if(_warriors.size() > 2)
    {
        bool failed = false;
        uint32_t pos = 1;
        int retries = 20;
        int retries_backtrack = 4;

        do
        {
            target_positions[pos] = random() % (core_size - 2 * min_separation + 1) + min_separation;

            uint32_t i;
            for (i = 1; i < pos; ++i)
            {
                uint32_t diff = (uint32_t)std::abs((int32_t)target_positions[pos] - (int32_t)target_positions[i]);
                if (diff < min_separation)
                    break;
            }

            if (i == pos)
            {
                ++pos;
            }
            else
            {
                if (!retries_backtrack) failed = true;
                if (!retries)
                {
                    pos = i;
                    --retries_backtrack;
                    retries = 20;
                }
                else
                {
                    --retries;
                }
            }
        } while (pos < _warriors.size() && !failed);

        if(failed)
        {
            // TODO: implement backup strategy
            throw std::runtime_error("couldn't position warriors");
        }
    }

    for (uint32_t i = 0; i < _warriors.size(); ++i)
    {
        copy_warrior(i, target_positions[i]);
    }
}

void mmars::clear()
{
    _warriors.clear();
    _results.clear();
    _task_queue.clear();
    _core.clear();
}

void mmars::add_warrior(std::shared_ptr<warrior> w)
{
    if (w->code.empty()) throw std::runtime_error("warrior has no code");
    if (w->code.size() > max_length) throw std::runtime_error("warrior code is too long");

    _warriors.push_back(w);
    _results.insert_or_assign(w, result());
}

result mmars::get_result(std::shared_ptr<warrior> w)
{
    if(!_results.count(w))
    {
        throw std::runtime_error("warrior is not part of the mars");
    }

    result res = _results[w];

    return res;
}

void mmars::set_seed(uint32_t seed)
{
    _seed = seed;
}

void mmars::setup()
{
    if (_core.capacity() != core_size || _core.size() != core_size) _core = std::vector<instruction>(core_size, instruction());
    else
    {
        for (uint32_t i = 0; i < core_size; ++i)
        {
            _core[i].op = op_code::dat;
            _core[i].mod = modifier::f;
            _core[i].a_mode = addr_mode::dir;
            _core[i].b_mode = addr_mode::dir;
            _core[i].a = 0;
            _core[i].b = 0;
        }
    }

    //_task_queue = std::vector<std::queue<uint32_t>>(_warriors.size(), std::queue<uint32_t>());
    if(_task_queue.size() != _warriors.size())
    {
        _task_queue = std::vector<task_queue>(_warriors.size(), task_queue(max_process));
    }
    else
    {
        for (auto& i : _task_queue)
        {
            i.clear();
        }
    }

    insert_warriors();
}

uint32_t mmars::step()
{
    uint32_t alive = 0;
    for (uint32_t i = 0; i < _warriors.size(); ++i)
    {
        uint32_t ri = (i + _round) % (uint32_t)_warriors.size();
        if (_task_queue[ri].empty()) continue;

        /*
         * Get Current Task
         */
        uint32_t pc = _task_queue[ri].dequeue();

        uint32_t rpa, wpa, rpb, wpb, pip;
        instruction ir = _core[pc];

        /*
         * Process A-Mode
         */
        if(ir.a_mode == im)
        {
            rpa = wpa = 0;
        }
        else
        {
            rpa = fold(ir.a, read_limit);
            wpa = fold(ir.a, write_limit);

            if (ir.a_mode != dir) {
                if(ir.a_mode == pre_dec_a)
                {
                    _core[(pc + wpa) % core_size].a = (_core[(pc + wpa) % core_size].a + core_size - 1) % core_size;
                }
                else if (ir.a_mode == pre_dec_b)
                {
                    _core[(pc + wpa) % core_size].b = (_core[(pc + wpa) % core_size].b + core_size - 1) % core_size;;
                }
                else if(ir.a_mode == post_inc_a || ir.a_mode == post_inc_b)
                {
                    pip = (pc + wpa) % core_size;
                }

                if(ir.a_mode == pre_dec_a || ir.a_mode == post_inc_a || ir.a_mode == ind_a)
                {
                    rpa = fold(rpa + _core[(pc + rpa) % core_size].a, read_limit);
                    wpa = fold(wpa + _core[(pc + wpa) % core_size].a, write_limit);
                }
                else
                {
                    rpa = fold(rpa + _core[(pc + rpa) % core_size].b, read_limit);
                    wpa = fold(wpa + _core[(pc + wpa) % core_size].b, write_limit);
                }
            }
        }

        instruction ira = _core[(pc + rpa) % core_size];

        /*
         * Process A-Mode Post Increment
         */
        if(ir.a_mode == post_inc_a)
        {
            _core[pip].a = (_core[pip].a + 1) % core_size;
        }
        else if(ir.a_mode == post_inc_b)
        {
            _core[pip].b = (_core[pip].b + 1) % core_size;
        }

        /*
         * Process B-Mode
         */
        if(ir.b_mode == im)
        {
            rpb = wpb = 0;
        } 
        else
        {
            rpb = fold(ir.b, read_limit);
            wpb = fold(ir.b, write_limit);

            if(ir.b_mode != dir)
            {
                if(ir.b_mode == pre_dec_a)
                {
                    _core[(pc + wpb) % core_size].a = (_core[(pc + wpb) % core_size].a + core_size - 1) % core_size;
                }
                else if (ir.b_mode == pre_dec_b)
                {
                    _core[(pc + wpb) % core_size].b = (_core[(pc + wpb) % core_size].b + core_size - 1) % core_size;;
                }
                else if(ir.b_mode == post_inc_a || ir.b_mode == post_inc_b)
                {
                    pip = (pc + wpb) % core_size;
                }

                if (ir.b_mode == pre_dec_a || ir.b_mode == post_inc_a || ir.b_mode == ind_a)
                {
                    rpb = fold(rpb + _core[(pc + rpb) % core_size].a, read_limit);
                    wpb = fold(wpb + _core[(pc + wpb) % core_size].a, write_limit);
                }
                else
                {
                    rpb = fold(rpb + _core[(pc + rpb) % core_size].b, read_limit);
                    wpb = fold(wpb + _core[(pc + wpb) % core_size].b, write_limit);
                }
            }
        }

        instruction irb = _core[(pc + rpb) % core_size];

        /*
         * Process B-Mode Post Increment
         */
        if (ir.b_mode == post_inc_a)
        {
            _core[pip].a = (_core[pip].a + 1) % core_size;
        }
        else if (ir.b_mode == post_inc_b)
        {
            _core[pip].b = (_core[pip].b + 1) % core_size;
        }

        /*
         * Process Instruction
         */
        bool do_queue = true;
        switch (ir.op)
        {
        case op_code::nop:
            queue(ri, (pc + 1) % core_size);
            break;
        case op_code::dat:
            break;
        case op_code::mov:
            switch (ir.mod)
            {
            case modifier::a:
                _core[(pc + wpb) % core_size].a = ira.a;
                break;
            case modifier::b:
                _core[(pc + wpb) % core_size].b = ira.b;
                break;
            case modifier::ab:
                _core[(pc + wpb) % core_size].b = ira.a;
                break;
            case modifier::ba:
                _core[(pc + wpb) % core_size].a = ira.b;
                break;
            case modifier::f:
                _core[(pc + wpb) % core_size].a = ira.a;
                _core[(pc + wpb) % core_size].b = ira.b;
                break;
            case modifier::x:
                _core[(pc + wpb) % core_size].b = ira.a;
                _core[(pc + wpb) % core_size].a = ira.b;
                break;
            case modifier::i:
                _core[(pc + wpb) % core_size] = ira;
                break;
            default: 
                throw std::runtime_error("unsupported operation");
            }
            queue(ri, (pc + 1) % core_size);
            break;
        case op_code::spl:
            queue(ri, (pc + 1) % core_size);
            queue(ri, (pc + rpa) % core_size);
            break;
        case op_code::jmp:
            queue(ri, (pc + rpa) % core_size);
            break;
        case op_code::add: arith(+)
        case op_code::sub: arith(+ core_size -)
        case op_code::mul: arith(*)
        case op_code::div: arith_div(/)
        case op_code::mod: arith_div(%)
        case op_code::jmz:
            switch (ir.mod)
            {
            case modifier::a:
            case modifier::ba:
                if(irb.a == 0) queue(ri, (pc + rpa) % core_size);
                else queue(ri, (pc + 1) % core_size);
                break;
            case modifier::b:
            case modifier::ab:
                if (irb.b == 0) queue(ri, (pc + rpa) % core_size);
                else queue(ri, (pc + 1) % core_size);
                break;
            case modifier::f:
            case modifier::x:
            case modifier::i:
                if (irb.b == 0 && irb.a == 0) queue(ri, (pc + rpa) % core_size);
                else queue(ri, (pc + 1) % core_size);
                break;
            default:
                throw std::runtime_error("unsupported operation");
            }
            break;
        case op_code::jmn:
            switch (ir.mod)
            {
            case modifier::a:
            case modifier::ba:
                if (irb.a != 0) queue(ri, (pc + rpa) % core_size);
                else queue(ri, (pc + 1) % core_size);
                break;
            case modifier::b:
            case modifier::ab:
                if (irb.b != 0) queue(ri, (pc + rpa) % core_size);
                else queue(ri, (pc + 1) % core_size);
                break;
            case modifier::f:
            case modifier::x:
            case modifier::i:
                if (irb.b != 0 || irb.a != 0) queue(ri, (pc + rpa) % core_size);
                else queue(ri, (pc + 1) % core_size);
                break;
            default:
                throw std::runtime_error("unsupported operation");
            }
            break;
        case op_code::djn:
            switch (ir.mod)
            {
            case modifier::a:
            case modifier::ba:
                _core[(pc + wpb) % core_size].a = (_core[(pc + wpb) % core_size].a + core_size - 1) % core_size;
                irb.a -= 1;

                if(irb.a != 0) queue(ri, (pc + rpa) % core_size);
                else queue(ri, (pc + 1) % core_size);
                break;
            case modifier::b:
            case modifier::ab:
                _core[(pc + wpb) % core_size].b = (_core[(pc + wpb) % core_size].b + core_size - 1) % core_size;
                irb.b -= 1;

                if (irb.b != 0) queue(ri, (pc + rpa) % core_size);
                else queue(ri, (pc + 1) % core_size);
                break;
            case modifier::f:
            case modifier::x:
            case modifier::i:
                _core[(pc + wpb) % core_size].a = (_core[(pc + wpb) % core_size].a + core_size - 1) % core_size;
                irb.a -= 1;

                _core[(pc + wpb) % core_size].b = (_core[(pc + wpb) % core_size].b + core_size - 1) % core_size;
                irb.b -= 1;

                if (irb.b != 0 || irb.a != 0) queue(ri, (pc + rpa) % core_size);
                else queue(ri, (pc + 1) % core_size);
                break;
            default:
                throw std::runtime_error("unsupported operation");
            }
            break;
        case op_code::sne:
            switch (ir.mod)
            {
            case modifier::a:
                if (ira.a != irb.a) queue(ri, (pc + 2) % core_size);
                else queue(ri, (pc + 1) % core_size);
                break;
            case modifier::b:
                if (ira.b != irb.b) queue(ri, (pc + 2) % core_size);
                else queue(ri, (pc + 1) % core_size);
                break;
            case modifier::ab:
                if (ira.a != irb.b) queue(ri, (pc + 2) % core_size);
                else queue(ri, (pc + 1) % core_size);
                break;
            case modifier::ba:
                if (ira.b != irb.a) queue(ri, (pc + 2) % core_size);
                else queue(ri, (pc + 1) % core_size);
                break;
            case modifier::f:
                if (ira.b != irb.b || ira.a != irb.a) queue(ri, (pc + 2) % core_size);
                else queue(ri, (pc + 1) % core_size);
                break;
            case modifier::x:
                if (ira.a != irb.b || ira.b != irb.a) queue(ri, (pc + 2) % core_size);
                else queue(ri, (pc + 1) % core_size);
                break;
            case modifier::i:
                if (ira.a != irb.a ||
                    ira.b != irb.b ||
                    ira.op != irb.op ||
                    ira.a_mode != irb.a_mode ||
                    ira.b_mode != irb.b_mode ||
                    ira.mod != irb.mod) queue(ri, (pc + 2) % core_size);
                else queue(ri, (pc + 1) % core_size);
                break;
            default:
                throw std::runtime_error("unsupported operation");
            }
            break;
        case op_code::cmp:
            switch (ir.mod)
            {
            case modifier::a: 
                if(ira.a == irb.a) queue(ri, (pc + 2) % core_size);
                else queue(ri, (pc + 1) % core_size);
                break;
            case modifier::b:
                if (ira.b == irb.b) queue(ri, (pc + 2) % core_size);
                else queue(ri, (pc + 1) % core_size);
                break;
            case modifier::ab:
                if (ira.a == irb.b) queue(ri, (pc + 2) % core_size);
                else queue(ri, (pc + 1) % core_size);
                break;
            case modifier::ba:
                if (ira.b == irb.a) queue(ri, (pc + 2) % core_size);
                else queue(ri, (pc + 1) % core_size);
                break;
            case modifier::f:
                if (ira.b == irb.b && ira.a == irb.a) queue(ri, (pc + 2) % core_size);
                else queue(ri, (pc + 1) % core_size);
                break;
            case modifier::x:
                if (ira.a == irb.b && ira.b == irb.a) queue(ri, (pc + 2) % core_size);
                else queue(ri, (pc + 1) % core_size);
                break;
            case modifier::i:
                if (ira.a == irb.a &&
                    ira.b == irb.b &&
                    ira.op == irb.op &&
                    ira.a_mode == irb.a_mode &&
                    ira.b_mode == irb.b_mode &&
                    ira.mod == irb.mod) queue(ri, (pc + 2) % core_size);
                else queue(ri, (pc + 1) % core_size);
                break;
            default:
                throw std::runtime_error("unsupported operation");
            }
            break;
        case op_code::slt:
            switch (ir.mod)
            {
            case modifier::a:
                if (ira.a < irb.a) queue(ri, (pc + 2) % core_size);
                else queue(ri, (pc + 1) % core_size);
                break;
            case modifier::b:
                if (ira.b < irb.b) queue(ri, (pc + 2) % core_size);
                else queue(ri, (pc + 1) % core_size);
                break;
            case modifier::ab:
                if (ira.a < irb.b) queue(ri, (pc + 2) % core_size);
                else queue(ri, (pc + 1) % core_size);
                break;
            case modifier::ba:
                if (ira.b < irb.a) queue(ri, (pc + 2) % core_size);
                else queue(ri, (pc + 1) % core_size);
                break;
            case modifier::f:
            case modifier::i:
                if (ira.a < irb.a &&
                    ira.b < irb.b) queue(ri, (pc + 2) % core_size);
                else queue(ri, (pc + 1) % core_size);
                break;
            case modifier::x:
                if (ira.a < irb.b &&
                    ira.b < irb.a) queue(ri, (pc + 2) % core_size);
                else queue(ri, (pc + 1) % core_size);
                break;
            default:
                throw std::runtime_error("unsupported operation");
            }
            break;
        default: ;
        }

        if (!_task_queue[ri].empty()) alive++;
    }

    return alive;
}

void mmars::run(int rounds)
{
    _round = 0;

    _results.clear();
    for (auto && w : _warriors)
    {
        _results.insert_or_assign(w, result());
    }

    for (int r = 0; r < rounds; ++r)
    {
        setup();

        uint32_t alive = 0;
        for (uint32_t c = 0; c < max_cycles; ++c)
        {
            alive = step();
            if (alive <= 1)
                break;
            
        }

        for (uint32_t i = 0; i < _warriors.size(); ++i)
        {
            bool survived = !_task_queue[i].empty();

            if (survived && alive == 1 && _warriors.size() > 1) _results[_warriors[i]].win++;
            else if (!survived) _results[_warriors[i]].loss++;
            else _results[_warriors[i]].tie++;
        }

        _round++;
    }
}

instruction mmars::get_instruction(uint32_t i)
{
    return _core[fold(i, core_size)];
}

std::vector<uint32_t> mmars::get_tasks(std::shared_ptr<warrior> w)
{
    for (uint32_t i = 0; i < _warriors.size(); ++i)
    {
        if(_warriors[i] == w)
        {
            std::vector<uint32_t> res = std::vector<uint32_t>(_task_queue[i].count());
            for (uint32_t j = 0; j < _task_queue[i].count(); ++j)
            {
                res.push_back(_task_queue[i].peek(j));
            }
            return res;
        }
    }
    throw std::runtime_error("warrior not found");
}
