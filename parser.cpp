#include <iostream>
#include <vector>
#include <stack>
#include <cstring>
#include <cstdlib>

#include "lexer.hpp"

#define MAX_CURLY_STACK_LENGTH 64

struct Node {
    std::string value;
    TokenType type;
    Node *right;
    Node *left;

    Node(const std::string &val, TokenType t) : value(val), type(t), right(nullptr), left(nullptr) {}
};

struct CurlyStack {
    std::vector<Node*> content;
    int top;

    CurlyStack() : top(-1) {
        content.reserve(MAX_CURLY_STACK_LENGTH);
    }

    Node* peek() {
        return content[top];
    }

    void push(Node* element) {
        top++;
        if (top < MAX_CURLY_STACK_LENGTH) {
            content.push_back(element);
        }
    }

    Node* pop() {
        Node* result = content[top];
        content.pop_back();
        top--;
        return result;
    }
};

Node* peek_curly(CurlyStack &stack) {
    return stack.peek();
}

void push_curly(CurlyStack &stack, Node* element) {
    stack.push(element);
}

Node* pop_curly(CurlyStack &stack) {
    return stack.pop();
}

void print_tree(Node *node, int indent, const std::string &identifier) {
    if (node == nullptr) {
        return;
    }
    std::cout << std::string(indent, ' ') << identifier << " -> " << node->value << "\n";
    print_tree(node->left, indent + 1, "left");
    print_tree(node->right, indent + 1, "right");
}

Node* init_node(const std::string &value, TokenType type) {
    return new Node(value, type);
}

void print_error(const std::string &error_type, size_t line_number) {
    std::cerr << "ERROR: " << error_type << " on line number: " << line_number << "\n";
    std::exit(1);
}

Node* parse_expression(Token* &current_token) {
    Node* expr_node = init_node(current_token->value, current_token->type);
    current_token++;
    if (current_token->type != OPERATOR) {
        return expr_node;
    }
    return expr_node;
}

Token* generate_operation_nodes(Token* &current_token, Node* &current_node) {
    Node* oper_node = init_node(current_token->value, OPERATOR);
    current_node->left->left = oper_node;
    current_node = oper_node;
    current_token--;
    if (current_token->type == INT) {
        Node* expr_node = init_node(current_token->value, INT);
        current_node->left = expr_node;
    } else if (current_token->type == IDENTIFIER) {
        Node* identifier_node = init_node(current_token->value, IDENTIFIER);
        current_node->left = identifier_node;
    } else {
        std::cerr << "ERROR: expected int or identifier\n";
        std::exit(1);
    }
    current_token++;
    current_token++;
    while (current_token->type == INT || current_token->type == IDENTIFIER || current_token->type == OPERATOR) {
        if (current_token->type == INT || current_token->type == IDENTIFIER) {
            if ((current_token->type != INT && current_token->type != IDENTIFIER) || current_token == nullptr) {
                std::cerr << "Syntax Error here\n";
                std::exit(1);
            }
            current_token++;
            if (current_token->type != OPERATOR) {
                current_token--;
                if (current_token->type == INT) {
                    Node* second_expr_node = init_node(current_token->value, INT);
                    current_node->right = second_expr_node;
                } else if (current_token->type == IDENTIFIER) {
                    Node* second_identifier_node = init_node(current_token->value, IDENTIFIER);
                    current_node->right = second_identifier_node;
                } else {
                    std::cerr << "ERROR: Expected Integer or Identifier\n";
                    std::exit(1);
                }
            }
        }
        if
        (current_token->type == OPERATOR) {
            Node* next_oper_node = init_node(current_token->value, OPERATOR);
            current_node->right = next_oper_node;
            current_node = next_oper_node;
            current_token--;
            if (current_token->type == INT) {
                Node* second_expr_node = init_node(current_token->value, INT);
                current_node->left = second_expr_node;
            } else if (current_token->type == IDENTIFIER) {
                Node* second_identifier_node = init_node(current_token->value, IDENTIFIER);
                current_node->left = second_identifier_node;
            } else {
                std::cerr << "ERROR: Expected IDENTIFIER or INT\n";
                std::exit(1);
            }
            current_token++;
        }
        current_token++;
    }
    return current_token;
}

Node* handle_exit_syscall(Node* root, Token* &current_token, Node* current) {
    Node* exit_node = init_node(current_token->value, KEYWORD);
    current->right = exit_node;
    current = exit_node;
    current_token++;
    if (current_token->type == END_OF_TOKENS) {
        print_error("Invalid Syntax on OPEN", current_token->line_num);
    }
    if (current_token->value == "(" && current_token->type == SEPARATOR) {
        Node* open_paren_node = init_node(current_token->value, SEPARATOR);
        current->left = open_paren_node;
        current_token++;
        if (current_token->type == END_OF_TOKENS) {
            print_error("Invalid Syntax on INT", current_token->line_num);
        }
        if (current_token->type == INT || current_token->type == IDENTIFIER) {
            current_token++;
            if (current_token->type == OPERATOR && current_token != nullptr) {
                current_token = generate_operation_nodes(current_token, current);
                current_token--;
            } else {
                current_token--;
                Node* expr_node = init_node(current_token->value, current_token->type);
                current->left->left = expr_node;
            }
            current_token++;
            if (current_token->type == END_OF_TOKENS) {
                print_error("Invalid Syntax on CLOSE", current_token->line_num);
            }
            if (current_token->value == ")" && current_token->type == SEPARATOR && current_token->type != END_OF_TOKENS) {
                Node* close_paren_node = init_node(current_token->value, SEPARATOR);
                current->left->right = close_paren_node;
                current_token++;
                if (current_token->type == END_OF_TOKENS) {
                    print_error("Invalid Syntax on SEMI", current_token->line_num);
                }
                if (current_token->value == ";" && current_token->type == SEPARATOR) {
                    Node* semi_node = init_node(current_token->value, SEPARATOR);
                    current->right = semi_node;
                    current = semi_node;
                } else {
                    print_error("Invalid Syntax on SEMI", current_token->line_num);
                }
            } else {
                print_error("Invalid Syntax on CLOSE", current_token->line_num);
            }
        } else {
            print_error("Invalid Syntax INT", current_token->line_num);
        }
    } else {
        print_error("Invalid Syntax OPEN", current_token->line_num);
    }
    return current;
}

void handle_token_errors(const std::string &error_text, Token* current_token, TokenType type) {
    if (current_token->type == END_OF_TOKENS || current_token->type != type) {
        print_error(error_text, current_token->line_num);
    }
}

Node* create_variable_reusage(Token* &current_token, Node* current) {
    Node* main_identifier_node = init_node(current_token->value, IDENTIFIER);
    current->left = main_identifier_node;
    current = main_identifier_node;
    current_token++;

    handle_token_errors("Invalid syntax after identifier", current_token, OPERATOR);

    if (current_token->type == OPERATOR) {
        if (current_token->value != "=") {
            print_error("Invalid Variable Syntax on =", current_token->line_num);
        }
        Node* equals_node = init_node(current_token->value, OPERATOR);
        current->left = equals_node;
        current = equals_node;
        current_token++;
    }
    if (current_token->type == END_OF_TOKENS || (current_token->type != INT && current_token->type != IDENTIFIER)) {
        print_error("Invalid Syntax After Equals", current_token->line_num);
    }

    current_token++;
    if (current_token->type == OPERATOR) {
        Node* oper_node = init_node(current_token->value, OPERATOR);
        current->left = oper_node;
        current = oper_node;
        current_token--;
        if (current_token->type == INT) {
            Node* expr_node = init_node(current_token->value, INT);
            oper_node->left = expr_node;
            current_token++;
            current_token++;
        } else if (current_token->type == IDENTIFIER) {
            Node* identifier_node = init_node(current_token->value, IDENTIFIER);
            oper_node->left = identifier_node;
            current_token++;
            current_token++;
        } else {
            print_error("ERROR: Expected IDENTIFIER or INT", current_token->line_num);
        }
        current_token++;

        if (current_token->type == OPERATOR) {
            Node* oper_node = init_node(current_token->value, OPERATOR);
            current->right = oper_node;
            current = oper_node;
            int operation = 1;
            current_token--;
            current_token--;
            while (operation) {
                current_token++;
                if (current_token->type == INT) {
                    Node* expr_node = init_node(current_token->value, INT);
                    current->left = expr_node;
                } else if (current_token->type == IDENTIFIER) {
                    Node* identifier_node = init_node(current_token->value, IDENTIFIER);
                    current->left = identifier_node;
                } else {
                    std::cerr << "ERROR: Unexpected Token\n";
                    std::exit(1);
                }
                current_token++;
                if (current_token->type == OPERATOR) {
                    current_token++;
                    current_token++;
                    if (current_token->type != OPERATOR) {
                        current_token--;
                        if (current_token->type == INT) {
                            Node* expr_node = init_node(current_token->value, INT);
                            current->right = expr_node;
                            current_token++;
                        } else if (current_token->type == IDENTIFIER) {
                            Node* identifier_node = init_node(current_token->value, IDENTIFIER);
                            current->right = identifier_node;
                            current_token++;
                        } else {
                            std::cerr << "ERROR: UNRECOGNIZED TOKEN!\n";
                            std::exit(1);
                        }
                        operation = 0;
                    } else {
                        current_token--;
                        current_token--;
                        Node* oper_node = init_node(current_token->value, OPERATOR);
                        current->right = oper_node;
                        current = oper_node;
                    }
                } else {
                    operation = 0;
                }
            }
        } else {
            current_token--;
            if (current_token->type == INT) {
                Node* expr_node = init_node(current_token->value, INT);
                oper_node->right = expr_node;
            } else if (current_token->type == IDENTIFIER) {
                Node* identifier_node = init_node(current_token->value, IDENTIFIER);
                oper_node->right = identifier_node;
            }
            current_token++;
        }
    } else {
        current_token--;
        if (current_token->type == INT) {
            Node* expr_node = init_node(current_token->value, INT);
            current->left = expr_node;
            current_token++;
        } else if (current_token->type == IDENTIFIER) {
            Node* identifier_node = init_node(current_token->value, IDENTIFIER);
            current->left = identifier_node;
            current_token++;
        }
    }
    handle_token_errors("Invalid Syntax After Expression", current_token, SEPARATOR);

    current = main_identifier_node;
    if (current_token->value == ";") {
        Node* semi_node = init_node(current_token->value, SEPARATOR);
        current->right = semi_node;
        current = semi_node;
    }
    return current;
}

Node* create_variables(Token* &current_token, Node* current) {
    Node* var_node = init_node(current_token->value, KEYWORD);
    current->left = var_node;
    current = var_node;
    current_token++;
    handle_token_errors("Invalid syntax after INT", current_token, IDENTIFIER);
    if (current_token->type == IDENTIFIER) {
        Node* identifier_node = init_node(current_token->value, IDENTIFIER);
        current->left = identifier_node;
        current = identifier_node;
        current_token++;
    }
    handle_token_errors("Invalid Syntax After Identifier", current_token, OPERATOR);

    if (current_token->type == OPERATOR) {
        if (current_token->value != "=") {
            print_error("Invalid Variable Syntax on =", current_token->line_num);
        }
        Node* equals_node = init_node(current_token->value, OPERATOR);
        current->left = equals_node;
        current = equals_node;
        current_token++;
    }
    if (current_token->type == END_OF_TOKENS || (current_token->type != INT && current_token->type != IDENTIFIER)) {
        print_error("Invalid Syntax After Equals", current_token->line_num);
    }

    current_token++;
    if (current_token->type == OPERATOR) {
        Node* oper_node = init_node(current_token->value, OPERATOR);
        current->left = oper_node;
        current = oper_node;
        current_token--;
        if (current_token->type == INT) {
            Node* expr_node = init_node(current_token->value, INT);
            oper_node->left = expr_node;
            current_token++;
            current_token++;
        } else if (current_token->type == IDENTIFIER) {
            Node* identifier_node = init_node(current_token->value, IDENTIFIER);
            oper_node->left = identifier_node;
            current_token++;
            current_token++;
        } else {
            print_error("ERROR: Expected IDENTIFIER or INT", current_token->line_num);
        }
        current_token++;

        if (current_token->type == OPERATOR) {
            Node* oper_node = init_node(current_token->value, OPERATOR);
            current->right = oper_node;
            current = oper_node;
            int operation = 1;
            current_token--;
            current_token--;
            while (operation) {
                current_token++;
                if (current_token->type == INT) {
                    Node* expr_node = init_node(current_token->value, INT);
                    current->left = expr_node;
                } else if (current_token->type == IDENTIFIER) {
                    Node* identifier_node = init_node(current_token->value, IDENTIFIER);
                    current->left = identifier_node;
                } else {
                    std::cerr << "ERROR: Unexpected Token\n";
                    std::exit(1);
                }
                current_token++;
                if (current_token->type == OPERATOR) {
                    current_token++;
                    current_token++;
                    if (current_token->type != OPERATOR) {
                        current_token--;
                        if (current_token->type == INT) {
                            Node* expr_node = init_node(current_token->value, INT);
                            current->right = expr_node;
                            current_token++;
                        } else if (current_token->type == IDENTIFIER) {
                            Node* identifier_node = init_node(current_token->value, IDENTIFIER);
                            current->right = identifier_node;
                            current_token++;
                        } else {
                            std::cerr << "ERROR: UNRECOGNIZED TOKEN!\n";
                            std::exit(1);
                        }
                        operation = 0;
                    } else {
                        current_token--;
                        current_token--;
                        Node* oper_node = init_node(current_token->value, OPERATOR);
                        current->right = oper_node;
                        current = oper_node;
                    }
                } else {
                    operation = 0;
                }
            }
        } else {
            current_token--;
            if (current_token->type == INT) {
                Node* expr_node = init_node(current_token->value, INT);
                oper_node->right = expr_node;
            } else if (current_token->type == IDENTIFIER) {
                Node* identifier_node = init_node(current_token->value, IDENTIFIER);
                oper_node->right = identifier_node;
            }
            current_token++;
        }
    } else {
        current_token--;
        if (current_token->type == INT) {
            Node* expr_node = init_node(current_token->value, INT);
            current->left = expr_node;
            current_token++;
        } else if (current_token->type == IDENTIFIER) {
            Node* identifier_node = init_node(current_token->value, IDENTIFIER);
            current->left = identifier_node;
            current_token++;
        }
    }

    handle_token_errors("Invalid Syntax After Expression", current_token, SEPARATOR);

    current = var_node;
    if (current_token->value == ";") {
        Node* semi_node = init_node(current_token->value, SEPARATOR);
        current->right = semi_node;
        current = semi_node;
    }
    return current;
}

Token* generate_if_operation_nodes(Token* &current_token, Node* &current_node) {
    Node* oper_node = init_node(current_token->value, OPERATOR);
    current_node->left->left = oper_node;
    current_node = oper_node;
    current_token--;

    Node* expr_node = init_node(current_token->value, current_token->type);
    current_node->left = expr_node;

    current_token++;
    current_token++;
    while (current_token->type == INT || current_token->type == IDENTIFIER || current_token->type == OPERATOR) {
        if (current_token->type == INT || current_token->type == IDENTIFIER) {
            if ((current_token->type != INT && current_token->type != IDENTIFIER) || current_token == nullptr) {
                std::exit(1);
            }
            current_token++;
            if (current_token->type != OPERATOR || current_token->value == "=") {
                current_token--;
                if (current_token->type == INT) {
                    Node* second_expr_node = init_node(current_token->value, INT);
                    current_node->right = second_expr_node;
                } else if (current_token->type == IDENTIFIER) {
                    Node* second_identifier_node = init_node(current_token->value, IDENTIFIER);
                    current_node->right = second_identifier_node;
                } else {
                    std::cerr << "ERROR: Expected Integer or Identifier\n";
                    std::exit(1);
                }
            }
        }
        if (current_token->value == "=") {
            break;
        } else if (current_token->type == OPERATOR) {
            Node* next_oper_node = init_node(current_token->value, OPERATOR);
            current_node->right = next_oper_node;
            current_node = next_oper_node;
            current_token--;
            if (current_token->type == INT) {
                Node* second_expr_node = init_node(current_token->value, INT);
                current_node->left = second_expr_node;
            } else if (current_token->type == IDENTIFIER) {
                Node* second_identifier_node = init_node(current_token->value, IDENTIFIER);
                current_node->left = second_identifier_node;
            } else {
                std::cerr << "ERROR: Expected IDENTIFIER or INT\n";
                std::exit(1);
            }
            current_token++;
        }
        current_token++;
    }
    return current_token;
}

Token* generate_if_operation_nodes_right(Token* &current_token, Node* &current_node) {
    Node* oper_node = init_node(current_token->value, OPERATOR);
    current_node->left->right = oper_node;
    current_node = oper_node;
    current_token--;

    Node* expr_node = init_node(current_token->value, current_token->type);
    current_node->left = expr_node;

    current_token++;
    current_token++;
    while (current_token->type == INT || current_token->type == IDENTIFIER || current_token->type == OPERATOR) {
        if (current_token->type == INT || current_token->type == IDENTIFIER) {
            if ((current_token->type != INT && current_token->type != IDENTIFIER) || current_token == nullptr) {
                std::cerr << "Syntax Error here\n";
                std::exit(1);
            }
            current_token++;
            if (current_token->type != OPERATOR || current_token->value == "=") {
                current_token--;
                if (current_token->type == INT) {
                    Node* second_expr_node = init_node(current_token->value, INT);
                    current_node->right = second_expr_node;
                } else if (current_token->type == IDENTIFIER) {
                    Node* second_identifier_node = init_node(current_token->value, IDENTIFIER);
                    current_node->right = second_identifier_node;
                } else {
                    std::cerr << "ERROR: Expected Integer or Identifier\n";
                    std::exit(1);
                }
            }
        }
        if (current_token->value == "=") {
            break;
        } else if (current_token->type == OPERATOR) {
            Node* next_oper_node = init_node(current_token->value, OPERATOR);
            current_node->right = next_oper_node;
            current_node = next_oper_node;
            current_token--;
            if (current_token->type == INT) {
                Node* second_expr_node = init_node(current_token->value, INT);
                current_node->left = second_expr_node;
            } else if (current_token->type == IDENTIFIER) {
                Node* second_identifier_node = init_node(current_token->value, IDENTIFIER);
                current_node->left = second_identifier_node;
            } else {
                std::cerr << "ERROR: Expected IDENTIFIER or INT\n";
                std::exit(1);
            }
            current_token++;
        }
        current_token++;
    }
    return current_token;
}

Node* create_if_statement(Token* &current_token, Node* current) {
    Node* if_node = init_node(current_token->value, current_token->type);
    current->left = if_node;
    current = if_node;
    current_token++;

    handle_token_errors("ERROR: Expected (", current_token, SEPARATOR);

    Node* open_paren_node = init_node(current_token->value, SEPARATOR);
    current->left = open_paren_node;
    current = open_paren_node;

    current_token++;

    if (current_token->type != IDENTIFIER && current_token->type != INT) {
        std::cerr << "ERROR: Expected Identifier or INT\n";
        std::exit(1);
    }

    while (current_token->type != END_OF_TOKENS && current_token->type != COMP) {
        current_token++;
    }

    if (current_token->type != COMP) {
        std::cerr << "ERROR: Expected =\n";
        std::exit(1);
    }
    Node* comp_node = init_node(current_token->value, current_token->type);
    open_paren_node->left = comp_node;

    while (current_token->type != SEPARATOR) {
        current_token--;
    }

    current_token++;
    current_token++;

    if (current_token->type != OPERATOR || current_token->type == COMP) {
        current_token--;
        Node* expr_node = init_node(current_token->value, current_token->type);
        comp_node->left = expr_node;
    } else {
        current_token = generate_if_operation_nodes(current_token, current);
    }

    current_token++;
    while ((current_token->type != END_OF_TOKENS && current_token->type != OPERATOR && current_token->type != SEPARATOR) || current_token->value == "=") {
        current_token++;
    }
    if (current_token->type == SEPARATOR) {
        current_token--;
        Node* expr_node = init_node(current_token->value, current_token->type);
        comp_node->right = expr_node;
    } else {
        current_token = generate_if_operation_nodes_right(current_token, current);
    }

    Node* close_paren_node = init_node(")", SEPARATOR);
    open_paren_node->right = close_paren_node;
    current = close_paren_node;

    return current;
}

Node* handle_write_node(Token* &current_token, Node* current) {
    Node* write_node = init_node(current_token->value, current_token->type);
    current->left = write_node;
    current = write_node;
  
    current_token++;

    handle_token_errors("ERROR: Expected (", current_token, SEPARATOR);

    current_token++;
    if (current_token->type != STRING && current_token->type != IDENTIFIER) {
        handle_token_errors("ERROR: Expected String Literal", current_token, STRING);
    }

    Node* string_node = init_node(current_token->value, current_token->type);
    current->left = string_node;

    current_token++;

    handle_token_errors("ERROR: Expected ,", current_token, SEPARATOR);

    current_token++;

    Node* number_node = init_node(current_token->value, current_token->type);
    current->right = number_node;

    current_token++;

    handle_token_errors("ERROR: Expected )", current_token, SEPARATOR);

    current_token++;

    if (current_token->value != ";") {
        print_error("ERROR: Expected ;", current_token->line_num);
    }

    Node* semi_node = init_node(current_token->value, current_token->type);
    number_node->right = semi_node; 
    current = semi_node;
    return current;
}

Node* parser(Token* tokens) {
    Token* current_token = &tokens[0];
    Node* root = init_node("PROGRAM", BEGINNING);

    Node* current = root;

    Node* open_curly = nullptr;

    CurlyStack stack;

    while (current_token->type != END_OF_TOKENS) {
        if (current == nullptr) {
            break;
        }
        switch (current_token->type) {
            case KEYWORD:
                if (current_token->value == "EXIT") {
                    current = handle_exit_syscall(root, current_token, current);
                } else if (current_token->value == "INT") {
                    current = create_variables(current_token, current);
                } else if (current_token->value == "IF") {
                    current = create_if_statement(current_token, current);
                } else if (current_token->value == "WHILE") {
                    current = create_if_statement(current_token, current);
                } else if (current_token->value == "WRITE") {
                    current = handle_write_node(current_token, current);
                }
                break;
            case SEPARATOR:
                if (current_token->value == "{") {
                    open_curly = init_node(current_token->value, SEPARATOR);
                    current->left = open_curly;
                    current = open_curly;
                    push_curly(stack, open_curly);
                    current = peek_curly(stack);
                }
                if (current_token->value == "}") {
                    Node* close_curly = new Node(current_token->value, current_token->type);
                    open_curly = pop_curly(stack);
                    if (open_curly == nullptr) {
                        std::cerr << "ERROR: Expected Open Parenthesis!\n";
                        std::exit(1);
                    }
                    close_curly->right = open_curly;
                    current->right = close_curly;
                    current = close_curly;
                }
                break; 
            case OPERATOR:
                break;
            case INT:
                break;
            case IDENTIFIER:
                current_token--;
                if (current_token->type == SEPARATOR && ((current_token->value == ";") || (current_token->value == "}") || (current_token->value == "{"))) {
                    current_token++;
                    current = create_variable_reusage(current_token, current);
                } else {
                    current_token++;
                }
                break;
            case STRING:
                break;
            case COMP:
                break;
            case BEGINNING:
                break;
            case END_OF_TOKENS:
                break;
        }
        current_token++;
    }
    return root;
}
