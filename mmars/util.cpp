#include "util.hpp"

std::string util::instruction_to_string(instruction i)
{
    std::string out;

    switch (i.op) {
    case op_code::dat:
        out += "DAT."; break;
    case op_code::nop:
        out += "NOP."; break;
    case op_code::spl:
        out += "SPL."; break;
    case op_code::jmp:
        out += "JMP."; break;
    case op_code::mov:
        out += "MOV."; break;
    case op_code::add:
        out += "ADD."; break;
    case op_code::sub:
        out += "SUB."; break;
    case op_code::mul:
        out += "MUL."; break;
    case op_code::div:
        out += "DIV."; break;
    case op_code::mod:
        out += "MOD."; break;
    case op_code::jmz:
        out += "JMZ."; break;
    case op_code::jmn:
        out += "JMN."; break;
    case op_code::djn:
        out += "DJN."; break;
    case op_code::slt:
        out += "SLT."; break;
    case op_code::cmp:
        out += "CMP."; break;
    case op_code::sne:
        out += "SNE."; break;
    default:;
    }

    switch (i.mod)
    {
    case modifier::i:
        out += "I\t\t"; break;
    case a:
        out += "A\t\t"; break;
    case b:
        out += "B\t\t"; break;
    case ab:
        out += "AB\t\t"; break;
    case ba:
        out += "BA\t\t"; break;
    case f:
        out += "F\t\t"; break;
    case x:
        out += "X\t\t"; break;
    default:;
    }

    switch (i.a_mode)
    {
    case im:
        out += "#\t"; break;
    case dir:
        out += "$\t"; break;
    case ind_b:
        out += "@\t"; break;
    case pre_dec_b:
        out += "<\t"; break;
    case post_inc_b:
        out += ">\t"; break;
    case ind_a:
        out += "*\t"; break;
    case pre_dec_a:
        out += "{\t"; break;
    case post_inc_a:
        out += "}\t"; break;
    default:;
    }

    out += std::to_string(i.a) + "\t,\t";

    switch (i.b_mode)
    {
    case im:
        out += "#\t"; break;
    case dir:
        out += "$\t"; break;
    case ind_b:
        out += "@\t"; break;
    case pre_dec_b:
        out += "<\t"; break;
    case post_inc_b:
        out += ">\t"; break;
    case ind_a:
        out += "*\t"; break;
    case pre_dec_a:
        out += "{\t"; break;
    case post_inc_a:
        out += "}\t"; break;
    default:;
    }

    return out + std::to_string(i.b);
}
