#include <unordered_map>
#include <regex>

#include "parser.hpp"

std::unordered_map<std::string, op_code> parser::opcodes = {
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

std::unordered_map<std::string, modifier> parser::modifiers = {
    {"X", modifier::x},
    {"A", modifier::a},
    {"B", modifier::b},
    {"AB", modifier::ab},
    {"BA", modifier::ba},
    {"I", modifier::i},
    {"F", modifier::f},
};

std::unordered_map<std::string, addr_mode> parser::addr_mode = {
    {"#", addr_mode::im},
    {"$", addr_mode::dir},
    {"@", addr_mode::ind_b},
    {"<", addr_mode::pre_dec_b},
    {">", addr_mode::post_inc_b},
    {"*", addr_mode::ind_a},
    {"{", addr_mode::pre_dec_a},
    {"}", addr_mode::post_inc_a},
};

std::shared_ptr<warrior> parser::parse(std::istream& input, uint32_t core_size)
{
    static auto wrap = [](int i, int max)
    {
        i = i % max;
        return i < 0 ? i + max : i;
    };

    static std::regex line_pattern = std::regex("[ \\t]*([A-Z]{3}).([A-Z]{1,2})[ \\t]+([<>${}*@#])[ \\t]+([-0-9]+),[ \\t]+([<>${}*@#])[ \\t]+([-0-9]+)");
    static std::regex end_pattern = std::regex("[ \\t]*(end|org)[ \\t]*([-0-9]+)");
    static std::regex name_pattern = std::regex(";name (.+)");
    static std::regex author_pattern = std::regex(";author (.+)");

    std::shared_ptr<warrior> parsed = std::make_shared<warrior>();

    std::string name;
    std::string author;

    std::string line;
    while (std::getline(input, line))
    {
        std::smatch match;
        if (std::regex_search(line, match, line_pattern))
        {
            uint32_t a = (uint32_t)wrap(std::stoi(match[4].str()), (int)core_size);
            uint32_t b = (uint32_t)wrap(std::stoi(match[6].str()), (int)core_size);
            parsed->code.emplace_back(opcodes.at(match[1].str()), modifiers.at(match[2].str()), addr_mode.at(match[3].str()), a, addr_mode.at(match[5].str()), b);
        }
        else if (std::regex_search(line, match, end_pattern))
        {
            parsed->start = std::stoi(match[2].str());
        }
        else if (std::regex_search(line, match, name_pattern))
        {
            name = match[1].str();
        }
        else if (std::regex_search(line, match, author_pattern))
        {
            author = match[1].str();
        }
    }

    parsed->name = name;
    parsed->author = author;

    if (parsed->code.empty()) throw std::exception("warrior was empty");
    return parsed;
}
