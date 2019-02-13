#pragma once

#include <cstdint>

/**
 * \brief A redcode opcode.
 */
enum class op_code : char {
    dat, nop, spl, 
    jmp, mov, add,
    sub, mul, div,
    mod, jmz, jmn,
    djn, slt, cmp,
    sne
};

/**
 * \brief A redcode modifier.
 */
enum modifier : char {
    i, a, b, ab, ba, f, x 
};

/**
 * \brief A redcode addressing mode.
 */
enum addr_mode : char {
    im, // #
    dir, // $
    ind_b, // @
    pre_dec_b, // <
    post_inc_b, // >
    ind_a, // *
    pre_dec_a, // {
    post_inc_a  // }
};

/**
 * \brief A redcode instruction.
 */
class instruction
{
public:
    op_code     op;
    modifier    mod;
    addr_mode   a_mode;
    uint32_t    a;
    addr_mode   b_mode;
    uint32_t    b;

    instruction(op_code op, modifier mod, addr_mode a_mode, uint32_t a, addr_mode b_mode, uint32_t b)
    {
        this->op        = op;
        this->mod       = mod;
        this->a_mode    = a_mode;
        this->a         = a;
        this->b_mode    = b_mode;
        this->b         = b;
    }

    instruction() : instruction(op_code::dat, modifier::f, addr_mode::dir, 0, addr_mode::dir, 0)
    { }

    instruction(const instruction& other) = default;

    instruction(instruction&& other) noexcept
        : op(other.op),
          mod(other.mod),
          a_mode(other.a_mode),
          a(other.a),
          b_mode(other.b_mode),
          b(other.b)
    { }

    instruction& operator=(const instruction& other)
    {
        if (this == &other)
            return *this;
        op = other.op;
        mod = other.mod;
        a_mode = other.a_mode;
        a = other.a;
        b_mode = other.b_mode;
        b = other.b;
        return *this;
    }

    instruction& operator=(instruction&& other) noexcept
    {
        if (this == &other)
            return *this;
        op = other.op;
        mod = other.mod;
        a_mode = other.a_mode;
        a = other.a;
        b_mode = other.b_mode;
        b = other.b;
        return *this;
    }
};