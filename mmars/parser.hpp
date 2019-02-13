#pragma once

#include <memory>
#include <unordered_map>

#include "warrior.hpp"

namespace parser
{
    /**
     * \brief String to opcode transformation map.
     */
    extern std::unordered_map<std::string, op_code> opcodes;

    /**
     * \brief String to modifier transformation map.
     */
    extern std::unordered_map<std::string, modifier> modifiers;

    /**
     * \brief String to addressing mode transformation map.
     */
    extern std::unordered_map<std::string, addr_mode> addr_mode;

    /**
     * \brief Parses a redcode load file.
     * \param Input Input stream
     * \param core_size Target core size
     * \return Returns A shared_ptr to the parsed warrior
     */
    std::shared_ptr<warrior> parse(std::istream& input, uint32_t core_size);
}
