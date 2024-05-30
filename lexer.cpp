#include "lexer.hpp"
#include <iostream>
#include <fstream>
#include <cctype>
#include <stdexcept>

size_t line_number = 0;

void print_token(const Token& token) {
    std::cout << "TOKEN VALUE: '" << token.value << "'\n";
    std::cout << "line number: " << token.line_num;

    switch (token.type) {
    case TokenType::INT:
        std::cout << " TOKEN TYPE: INT\n";
        break;
    case TokenType::KEYWORD:
        std::cout << " TOKEN TYPE: KEYWORD\n";
        break;
    case TokenType::SEPARATOR:
        std::cout << " TOKEN TYPE: SEPARATOR\n";
        break;
    case TokenType::OPERATOR:
        std::cout << " TOKEN TYPE: OPERATOR\n";
        break;
    case TokenType::IDENTIFIER:
        std::cout << " TOKEN TYPE: IDENTIFIER\n";
        break;
    case TokenType::STRING:
        std::cout << " TOKEN TYPE: STRING\n";
        break;
    case TokenType::COMP:
        std::cout << " TOKEN TYPE: COMPARATOR\n";
        break;
    case TokenType::END_OF_TOKENS:
        std::cout << " END OF TOKENS\n";
        break;
    case TokenType::BEGINNING:
        std::cout << "BEGINNING\n";
        break;
    }
}

Token generate_number(const std::string& current, size_t& current_index) {
    Token token;
    token.line_num = line_number;
    token.type = TokenType::INT;
    std::string value;
    
    while (std::isdigit(current[current_index]) && current[current_index] != '\0') {
        value += current[current_index];
        current_index++;
    }
    token.value = value;
    return token;
}

Token generate_keyword_or_identifier(const std::string& current, size_t& current_index) {
    Token token;
    token.line_num = line_number;
    std::string keyword;
    
    while (std::isalpha(current[current_index]) && current[current_index] != '\0') {
        keyword += current[current_index];
        current_index++;
    }
    
    if (keyword == "exit") {
        token.type = TokenType::KEYWORD;
        token.value = "EXIT";
    } else if (keyword == "int") {
        token.type = TokenType::KEYWORD;
        token.value = "INT";
    } else if (keyword == "if") {
        token.type = TokenType::KEYWORD;
        token.value = "IF";
    } else if (keyword == "while") {
        token.type = TokenType::KEYWORD;
        token.value = "WHILE";
    } else if (keyword == "write") {
        token.type = TokenType::KEYWORD;
        token.value = "WRITE";
    } else if (keyword == "eq") {
        token.type = TokenType::COMP;
        token.value = "EQ";
    } else if (keyword == "neq") {
        token.type = TokenType::COMP;
        token.value = "NEQ";
    } else if (keyword == "less") {
        token.type = TokenType::COMP;
        token.value = "LESS";
    } else if (keyword == "greater") {
        token.type = TokenType::COMP;
        token.value = "GREATER";
    } else {
        token.type = TokenType::IDENTIFIER;
        token.value = keyword;
    }
    return token;
}

Token generate_string_token(const std::string& current, size_t& current_index) {
    Token token;
    token.line_num = line_number;
    std::string value;
    current_index++;
    
    while (current[current_index] != '"') {
        value += current[current_index];
        current_index++;
    }
    
    token.type = TokenType::STRING;
    token.value = value;
    return token;
}

Token generate_separator_or_operator(const std::string& current, size_t& current_index, TokenType type) {
    Token token;
    token.value = current[current_index];
    token.line_num = line_number;
    token.type = type;
    return token;
}

std::vector<Token> lexer(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file");
    }

    file.seekg(0, std::ios::end);
    size_t length = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::string current(length, ' ');
    file.read(&current[0], length);
    file.close();
    
    size_t current_index = 0;
    std::vector<Token> tokens;
    
    while (current[current_index] != '\0') {
        if (current[current_index] == ';' || current[current_index] == ',' || 
            current[current_index] == '(' || current[current_index] == ')' || 
            current[current_index] == '{' || current[current_index] == '}' || 
            current[current_index] == '=' || current[current_index] == '+' || 
            current[current_index] == '-' || current[current_index] == '*' || 
            current[current_index] == '/' || current[current_index] == '%') {
            Token token = generate_separator_or_operator(current, current_index, TokenType::SEPARATOR);
            tokens.push_back(token);
        } else if (current[current_index] == '"') {
            Token token = generate_string_token(current, current_index);
            tokens.push_back(token);
        } else if (std::isdigit(current[current_index])) {
            Token token = generate_number(current, current_index);
            tokens.push_back(token);
            current_index--;
        } else if (std::isalpha(current[current_index])) {
            Token token = generate_keyword_or_identifier(current, current_index);
            tokens.push_back(token);
            current_index--;
        } else if (current[current_index] == '\n') {
            line_number++;
        }
        current_index++;
    }
    tokens.push_back({ TokenType::END_OF_TOKENS, "", line_number });
    return tokens;
}

int main() {
    std::string filename = "input.txt";
    try {
        std::vector<Token> tokens = lexer(filename);

        for (const Token& token : tokens) {
            print_token(token);
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    return 0;
}
