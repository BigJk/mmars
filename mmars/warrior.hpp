#pragma once

#include <string>
#include <vector>
#include <sstream>

#include "util.hpp"
#include "instruction.hpp"

/**
 * \brief Represents a redcode warrior.
 */
class warrior
{
public:
    std::string author;
    std::string name;

    std::vector<instruction> code;
    uint16_t start;

    void serialize(std::ostream& output)
    {
        output << ";author" << author   << "\n";
        output << ";name"   << name     << "\n";
        output << "ORG "    << start    << "\n";
        for (auto && i : code)
        {
            output << util::instruction_to_string(i) << "\n";
        }
    }

    std::string to_string()
    {
        std::stringstream s;
        serialize(s);
        return s.str();
    }
};
