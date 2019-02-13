#pragma once

#include <vector>

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
};
