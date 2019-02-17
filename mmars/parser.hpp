#pragma once

#include <unordered_map>
#include <regex>
#include <memory>
#include <stack>

#include "warrior.hpp"

/**
 * \brief The types a token can have
 */
enum class token_type
{
    label,
    opcode,
    preprocessor,
    modifier,
    mode,
    number,
    comma,
    maths,
    eol,
    comment,
    unknown
};

/**
 * \brief Token of a redcode token stream
 */
class token
{
public:
    int             line;
    int             position;
    std::string     text;
    token_type      type;

    token(int line, int position, const std::string& text, token_type type)
        : line(line),
          position(position),
          text(text),
          type(type)
    { }

    token() = default;
};

/**
 * \brief Implementation of the shunting yard algorithm and evaluation of postfix formula
 */
class shunting_yard
{
private:
    class prec_operator
    {
    public:
        int precedence;
        bool right_associative;

        prec_operator(int precedence, bool right_associative)
            : precedence(precedence),
            right_associative(right_associative)
        { }

        prec_operator() = default;
    };

    std::unordered_map<std::string, prec_operator> _operators = {
        {"*", prec_operator(10, false)},
        {"/", prec_operator(10, false)},
        {"%", prec_operator(10, false)},
        {"-", prec_operator(9, false)},
        {"+", prec_operator(8, false)},
    };

public:
    explicit shunting_yard() = default;

    int eval(std::vector<token> expression);
};

class parser
{
private:
    std::unordered_map<std::string, op_code> str_opcodes = {
        {"SUB", op_code::sub},
        {"ADD", op_code::add},
        {"CMP", op_code::cmp},
        {"DAT", op_code::dat},
        {"DIV", op_code::div},
        {"DJN", op_code::djn},
        {"JMN", op_code::jmn},
        {"JMP", op_code::jmp},
        {"JMZ", op_code::jmz},
        {"MOD", op_code::mod},
        {"MOV", op_code::mov},
        {"CMP", op_code::cmp},
        {"SEQ", op_code::cmp},
        {"NOP", op_code::nop},
        {"SNE", op_code::sne},
        {"SLT", op_code::slt},
        {"MUL", op_code::mul},
        {"SPL", op_code::spl},
    };

    std::unordered_map<std::string, modifier> str_modifiers = {
        {".X", modifier::x},
        {".A", modifier::a},
        {".B", modifier::b},
        {".AB", modifier::ab},
        {".BA", modifier::ba},
        {".I", modifier::i},
        {".F", modifier::f},
    };

    std::unordered_map<std::string, addr_mode> str_addr_mode = {
        {"#", addr_mode::im},
        {"$", addr_mode::dir},
        {"@", addr_mode::ind_b},
        {"<", addr_mode::pre_dec_b},
        {">", addr_mode::post_inc_b},
        {"*", addr_mode::ind_a},
        {"{", addr_mode::pre_dec_a},
        {"}", addr_mode::post_inc_a},
    };

    std::regex _re_label = std::regex("[A-Z_][A-Z_0-9]*\\:?", std::regex_constants::icase);
    std::regex _re_opcode = std::regex("(DAT|MOV|ADD|SUB|MUL|DIV|MOD|JMP|JMZ|JMN|DJN|CMP|SLT|SPL|SEQ|SNE|NOP)(?!\\w)", std::regex_constants::icase);
    std::regex _re_preproccesor = std::regex("(EQU|END|ORG|FOR|ROF)(?!\\w)", std::regex_constants::icase);
    std::regex _re_modifier = std::regex("\\.(AB|BA|A|B|F|X|I)", std::regex_constants::icase);
    std::regex _re_mode = std::regex("(#|\\$|@|<|>|\\{|\\}|\\*)", std::regex_constants::icase);
    std::regex _re_number = std::regex("[0-9]+", std::regex_constants::icase);
    std::regex _re_comma = std::regex(",", std::regex_constants::icase);
    std::regex _re_maths = std::regex("(\\+|\\-|\\*|\\/|%|\\(|\\))", std::regex_constants::icase);
    std::regex _re_comment = std::regex(";.*", std::regex_constants::icase);

    int _line;
    int _position;
    std::vector<token> _org;
    std::vector<token> _tokens;
    std::unordered_map<std::string, std::vector<token>> _equs;
    std::unordered_map <std::string, int> _labels;
    std::shared_ptr<warrior> _result;

    void throw_error(int line, int pos, const std::string& message) const;

    static modifier default_modifier(op_code op, addr_mode a_mode, addr_mode b_mode);

    void transform_token(token_type t, std::string text);
    void add_token(token_type t, std::string text);

    void accumulator(std::string text);
    void comment(std::string text);
    void eol();
    void tokenize(std::istream& input);
    void filter();

    void process_for();
    void process_equs();
    void process_org();
    void process_labels();
    void process_expressions();

    std::vector<token> read_line(uint32_t start);
    void pop(uint32_t start, uint32_t size);

public:
    uint32_t core_size = 8000;
    uint32_t max_cycles = 80000;
    uint32_t max_process = 8000;
    uint32_t max_length = 200;
    uint32_t min_separation = 200;

    uint32_t read_limit = 8000;
    uint32_t write_limit = 8000;

    parser(uint32_t core_size, uint32_t max_cycles, uint32_t max_process, uint32_t max_length,
        uint32_t min_separation)
        : _line(0),
          _position(0),
          core_size(core_size),
          max_cycles(max_cycles),
          max_process(max_process),
          max_length(max_length),
          min_separation(min_separation)
    {
    }

    /**
     * \brief Parses a warrior
     * \param input Input stream of the warrior text
     * \return The parsed warrior
     */
    std::shared_ptr<warrior> parse(std::istream& input);
};