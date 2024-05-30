#ifndef LEXER_HPP_
#define LEXER_HPP_

#include <string>
#include <vector>
#include <cstddef>

enum class TokenType {
    BEGINNING,
    INT,
    KEYWORD,
    SEPARATOR,
    OPERATOR,
    IDENTIFIER,
    STRING,
    COMP,
    END_OF_TOKENS,
};

struct Token {
    TokenType type;
    std::string value;
    size_t line_num;
};

void print_token(const Token& token);
Token generate_number(const std::string& current, size_t& current_index);
Token generate_keyword_or_identifier(const std::string& current, size_t& current_index);
Token generate_separator_or_operator(const std::string& current, size_t& current_index, TokenType type);
std::vector<Token> lexer(const std::string& filename);

#endif 