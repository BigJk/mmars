#include <unordered_map>
#include <cctype>
#include <regex>

#include "parser.hpp"

void parser::throw_error(int line, int pos, const std::string& message) const
{
    throw std::exception(("[" + std::to_string(line) + ":" + std::to_string(pos) + "] " + message).c_str());
}

modifier parser::default_modifier(op_code op, addr_mode a_mode, addr_mode b_mode)
{
    switch (op)
    {
    case op_code::dat: return f;
    case op_code::mov:
    case op_code::cmp:
    case op_code::sne:
        if(a_mode == addr_mode::im)
            return ab;
        if (b_mode == addr_mode::im)
            return b;
        return i;
    case op_code::add: 
    case op_code::sub: 
    case op_code::mul: 
    case op_code::div: 
    case op_code::mod:
        if (a_mode == addr_mode::im)
            return ab;
        if (b_mode == addr_mode::im)
            return b;
        return f;
    case op_code::slt:
        if (a_mode == addr_mode::im)
            return ab;
        return b;
    case op_code::nop: 
    case op_code::spl: 
    case op_code::jmp: 
    case op_code::jmz: 
    case op_code::jmn:
    case op_code::djn:
        return b;
    }
    return b;
}

void parser::transform_token(token_type t, std::string text)
{
    if(t == token_type::opcode || t == token_type::modifier || t == token_type::preprocessor)
        std::transform(text.begin(), text.end(), text.begin(), ::toupper);

    for (uint32_t i = 0; i < text.size(); ++i)
    {
        if (text[i] == ':')
        {
            text.erase(text.begin() + i, text.begin() + i + 1);
            i--;
        }
    }

    add_token(t, text);
}

void parser::add_token(token_type t, std::string text)
{
    _tokens.emplace_back(_line, _position, text, t);
}

void parser::accumulator(std::string text)
{
    auto match_token = [&](token_type t, std::regex pattern)
    {
        std::smatch match;
        if(std::regex_search(text, match, pattern) && text[0] == match[0].str()[0])
        {
            transform_token(t, match[0].str());
            _position += match[0].str().size();
            text = text.substr(match[0].str().size());
            return true;
        }
        return false;
    };

    while(!text.empty())
    {
        if (text[0] == ';') return;

        if (match_token(token_type::comma, _re_comma))
            continue;

        if (match_token(token_type::modifier, _re_modifier))
            continue;

        if (match_token(token_type::mode, _re_mode))
            continue;

        if (match_token(token_type::maths, _re_maths))
            continue;

        if (match_token(token_type::number, _re_number))
            continue;

        if (match_token(token_type::opcode, _re_opcode))
            continue;

        if (match_token(token_type::preprocessor, _re_preproccesor))
            continue;

        if (match_token(token_type::label, _re_label))
            continue;

        if (match_token(token_type::comment, _re_comment))
            continue;

        transform_token(token_type::unknown, "");
        text = "";
    }
}

void parser::comment(std::string text)
{
    if(text.find("author") == 0 || text.find("AUTHOR") == 0) _result->author = text.substr(7);
    else if (text.find("name") == 0 || text.find("NAME") == 0)  _result->name = text.substr(5);
    add_token(token_type::comment, text);
}

void parser::eol()
{
    add_token(token_type::eol, "\n");
}

void parser::tokenize(std::istream& input)
{
    std::string accum;
    std::string line;
    while (std::getline(input, line))
    {
        _position = 0;
        for (uint32_t c = 0; c < line.size(); ++c)
        {
            if (line[c] == ';')
            {
                if (!accum.empty())
                {
                    accumulator(accum);
                    accum = "";
                }

                _position = c + 1;
                comment(line.substr(c + 1));

                break;
            }

            if (!std::isspace(line[c]))
            {
                accum += line[c];
            }
            else
            {
                if (!accum.empty())
                {
                    accumulator(accum);
                    accum = "";
                }

                _position = c + 1;
            }
        }

        if (!accum.empty())
        {
            accumulator(accum);
            accum = "";
        }

        eol();
        _line++;
    }
}

void parser::filter()
{
    for (uint32_t i = 0; i < _tokens.size(); ++i)
    {
        switch (_tokens[i].type)
        {
        case token_type::comment:
            _tokens.erase(_tokens.begin() + i, _tokens.begin() + i + 1);
            i--;
            break;
        case token_type::eol:
            if(i == 0 || i + 1 < _tokens.size() && _tokens[i+1].type == token_type::eol)
            {
                _tokens.erase(_tokens.begin() + i, _tokens.begin() + i + 1);
                i--;
            }
            break;
        case token_type::label:
            while(i < _tokens.size() - 1 && i > 0 && _tokens[i - 1].type == token_type::eol && _tokens[i+1].type == token_type::eol)
            {
                _tokens.erase(_tokens.begin() + i + 1, _tokens.begin() + i + 2);
            }
        case token_type::preprocessor:
            if(_tokens[i].text == "END")
            {
                for (uint32_t j = i; j < _tokens.size(); ++j)
                {
                    if(_tokens[j].type == token_type::eol)
                    {
                        _tokens.erase(_tokens.begin() + j + 1, _tokens.end());
                    }
                }
            }
            break;
        }
    }
}

void parser::process_equs()
{
    int current_pos = 0;
    std::vector<token> current_line = read_line(current_pos);
    while(!current_line.empty())
    {
        if (current_line.size() > 2 && current_line[0].type == token_type::label && current_line[1].type == token_type::preprocessor && current_line[1].text == "EQU")
        {
            std::string name = current_line[0].text;
            if (_equs.count(name)) throw_error(current_line[0].line, current_line[0].position, current_line[0].text + "> equ redefinition");

            current_line.erase(current_line.begin(), current_line.begin() + 2);
            _equs.insert_or_assign(name, current_line);
            pop(current_pos, current_line.size() + 3);
        } 
        else
        {
            current_pos += current_line.size() + 1;
        }

        current_line = read_line(current_pos);
    }

    for (auto && equ : _equs)
    {
        for (uint32_t i = 0; i < equ.second.size(); ++i)
        {
            if(equ.second[i].type != token_type::label || !_equs.count(equ.second[i].text)) continue;
            auto replace = _equs[equ.second[i].text];
            for (int j = replace.size() - 1; j >= 0; --j)
            {
                equ.second.insert(equ.second.begin() + i, replace[j]);
            }
            equ.second.erase(equ.second.begin() + replace.size() + i, equ.second.begin() + replace.size() + i + 1);
            i--;
        }
    }

    for (uint32_t i = 0; i < _tokens.size(); ++i)
    {
        if (_tokens[i].type != token_type::label || !_equs.count(_tokens[i].text)) continue;
        auto replace = _equs[_tokens[i].text];
        for (int j = replace.size() - 1; j >= 0; --j)
        {
            _tokens.insert(_tokens.begin() + i, replace[j]);
        }
        _tokens.erase(_tokens.begin() + replace.size() + i, _tokens.begin() + replace.size() + i + 1);
        i--;
    }
}

void parser::process_org()
{
    int current_pos = 0;
    std::vector<token> current_line = read_line(current_pos);
    while (!current_line.empty())
    {
        if (!current_line.empty() && current_line[0].type == token_type::preprocessor && (current_line[0].text == "END" || current_line[0].text == "ORG"))
        {
            if(current_line.size() > 1)
            {
                current_line.erase(current_line.begin(), current_line.begin() + 1);
                _org = current_line;
                pop(current_pos, current_line.size() + 2);
            }
            else
            {
                pop(current_pos, 2);
            }
        }
        else
        {
            current_pos += current_line.size() + 1;
        }

        current_line = read_line(current_pos);
    }
}

void parser::process_labels()
{
    int current_line = 0;
    for (uint32_t i = 0; i < _tokens.size() - 1; ++i)
    {
        if (_tokens[i].type == token_type::eol) current_line++;
        if (_tokens[i].type != token_type::label || _tokens[i + 1].type != token_type::opcode && _tokens[i + 1].type != token_type::label) continue;
        if (_equs.count(_tokens[i].text) || _labels.count(_tokens[i].text)) throw_error(_tokens[i].line, _tokens[i].position, _tokens[i].text + "> label redefinition");
        _labels.insert_or_assign(_tokens[i].text, current_line);
        _tokens.erase(_tokens.begin() + i, _tokens.begin() +  i + 1);
        i--;
    }

    if (_tokens[_tokens.size() - 1].text == "END" && _tokens[_tokens.size() - 2].type == token_type::label && _tokens[_tokens.size() - 3].type == token_type::eol)
    {
        if (_equs.count(_tokens[_tokens.size() - 2].text) || _labels.count(_tokens[_tokens.size() - 2].text)) throw_error(_tokens[_tokens.size() - 2].line, _tokens[_tokens.size() - 2].position, _tokens[_tokens.size() - 2].text + "> label redefinition");
        _labels.insert_or_assign(_tokens[_tokens.size() - 2].text, current_line);
        _tokens.erase(_tokens.begin() + _tokens.size() - 2, _tokens.begin() + _tokens.size() - 1);
    }

    current_line = 0;
    for (uint32_t i = 0; i < _tokens.size(); ++i)
    {
        if (_tokens[i].type == token_type::eol) current_line++;
        if (_tokens[i].type != token_type::label) continue;
        if (!_labels.count(_tokens[i].text)) throw_error(_tokens[i].line, _tokens[i].position, _tokens[i].text + "> label not defined");
        auto pos = _labels[_tokens[i].text];
        _tokens[i].type = token_type::number;
        _tokens[i].text = std::to_string(pos - current_line);
        i--;
    }

    for (auto && org : _org)
    {
        if (org.type != token_type::label) continue;
        if (!_labels.count(org.text)) throw_error(_tokens[i].line, _tokens[i].position, _tokens[i].text + "> label not defined");
        org.type = token_type::number;
        org.text = std::to_string(_labels[org.text]);
    }
}

void parser::process_expressions()
{
    static auto wrap = [](int i, int max)
    {
        i = i % max;
        return i < 0 ? i + max : i;
    };

    shunting_yard yard;
    _result->start = yard.eval(_org);

    int current_pos = 0;
    std::vector<token> current_line = read_line(current_pos);
    while (!current_line.empty())
    {
        current_pos += current_line.size() + 1;
        
        if (!current_line.empty() && current_line[0].type == token_type::opcode)
        {
            instruction ins;

            ins.op = str_opcodes[current_line[0].text];

            bool got_math = false;
            bool in_a = true;
            bool mod_found = false;

            std::vector<token> expr;

            while(!current_line.empty())
            {
                if (current_line[0].type == token_type::modifier)
                {
                    mod_found = true;
                    ins.mod = str_modifiers[current_line[0].text];
                } 
                else if (current_line[0].type == token_type::mode) {
                    if(got_math)
                    {
                        expr.push_back(current_line[0]);
                    }
                    else
                    {
                        if(in_a)
                            ins.a_mode = str_addr_mode[current_line[0].text];
                        else
                            ins.b_mode = str_addr_mode[current_line[0].text]; 
                    }
                } 
                else if (current_line[0].type == token_type::maths || current_line[0].type == token_type::number) {
                    got_math = true;
                    expr.push_back(current_line[0]);
                } 
                else if (current_line[0].type == token_type::comma)
                {
                    got_math = false;
                    in_a = false;
                    ins.a = wrap(yard.eval(expr), core_size);
                    expr.clear();
                }

                current_line.erase(current_line.begin(), current_line.begin() + 1);
            }

            ins.b = wrap(yard.eval(expr), core_size);

            if(!mod_found)
            {
                ins.mod = default_modifier(ins.op, ins.a_mode, ins.b_mode);
            }

            _result->code.push_back(ins);
        }

        current_line = read_line(current_pos);
    }
}

std::vector<token> parser::read_line(uint32_t start)
{
    if (start >= _tokens.size()) return {};
    std::vector<token> line;
    for (uint32_t i = start; i < _tokens.size() && _tokens[i].type != token_type::eol; ++i)
    {
        line.push_back(_tokens[i]);
    }
    return line;
}

void parser::pop(uint32_t start, uint32_t size)
{
    if (start >= _tokens.size()) return;
    if (start + size >= _tokens.size()) size = _tokens.size() - start;
    _tokens.erase(_tokens.begin() + start, _tokens.begin() + start + size);
}

std::shared_ptr<warrior> parser::parse(std::istream& input)
{
    _result = std::make_shared<warrior>();
    _position = 0;
    _line = 0;
    _tokens.clear();
    _equs.clear();
    _labels.clear();

    tokenize(input);
    for (int i = 0; i < 3; ++i) filter();
    process_equs();
    process_org();
    process_labels();
    process_expressions();

    return _result;
}
