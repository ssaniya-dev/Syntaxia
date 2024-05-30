#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <stack>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include "lexer.hpp"

#define MAX_STACK_SIZE_SIZE 1024

std::stack<std::string> curly_stack;
size_t curly_stack_size = 0;
int curly_count = 0;
int global_curly = 0;
size_t stack_size = 0;
int current_stack_size_size = 0;
int label_number = 0;
int loop_label_number = 0;
int text_label = 0;
std::vector<size_t> current_stack_size(MAX_STACK_SIZE_SIZE);
const unsigned initial_size = 100;
std::unordered_map<std::string, size_t> hashmap;

enum class OperatorType {
    ADD,
    SUB,
    DIV,
    MUL,
    MOD,
    NOT_OPERATOR
};

struct Node {
    std::string value;
    TokenType type;
    Node* left;
    Node* right;
};

void create_label(std::ofstream &file, int num) {
    label_number--;
    file << "label" << num << ":\n";
}

void create_end_loop(std::ofstream &file) {
    loop_label_number--;
    file << " jmp loop" << loop_label_number << "\n";
}

void create_loop_label(std::ofstream &file) {
    file << "loop" << loop_label_number << ":\n";
    loop_label_number++;
}

void if_label(std::ofstream &file, const std::string &comp, int num) {
    if (comp == "EQ") {
        file << "  jne label" << num << "\n";
    } else if (comp == "NEQ") {
        file << "  je label" << num << "\n";
    } else if (comp == "LESS") {
        file << "  jge label" << num << "\n";
    } else if (comp == "GREATER") {
        file << "  jle label" << label_number << "\n";
    } else {
        std::cerr << "ERROR: Unexpected comparator\n";
        exit(1);
    }
    label_number++;
}

void stack_push(size_t value) {
    current_stack_size_size++;
    current_stack_size[current_stack_size_size] = value;
}

size_t stack_pop() {
    if (current_stack_size_size == 0) {
        std::cerr << "ERROR: stack is already empty\n";
        exit(1);
    }
    size_t result = current_stack_size[current_stack_size_size];
    current_stack_size_size--;
    return result;
}

void curly_stack_push(const std::string &value) {
    curly_stack.push(value);
    curly_stack_size++;
}

std::string curly_stack_pop() {
    if (curly_stack.empty()) {
        return "";
    }
    std::string result = curly_stack.top();
    curly_stack.pop();
    curly_stack_size--;
    return result;
}

std::string curly_stack_peek() {
    if (curly_stack.empty()) {
        return "";
    }
    return curly_stack.top();
}

static int log_and_free_out_of_scope(void* const context, struct hashmap_element_s* const e) {
    (void)(context);
    if (*(size_t*)e->data > (current_stack_size[current_stack_size_size] + 1)) {
        if (hashmap_remove(&hashmap, e->key, strlen(e->key)) != 0) {
            std::cerr << "COULD NOT REMOVE ELEMENT\n";
        }
    }
    return 0;
}

void push(const std::string &reg, std::ofstream &file) {
    file << "  push " << reg << "\n";
    stack_size++;
}

void push_var(size_t stack_pos, const std::string &var_name, std::ofstream &file) {
    file << "  push QWORD [rsp + " << (stack_size - stack_pos) * 8 << "]\n";
    stack_size++;
}

void modify_var(size_t stack_pos, const std::string &new_value, const std::string &var_name, std::ofstream &file) {
    file << "  mov QWORD [rsp + " << ((stack_size) - (stack_pos)) * 8 << "], " << new_value << "\n";
    file << "  push QWORD [rsp + " << (stack_size - stack_pos) * 8 << "]\n";
}

void pop(const std::string &reg, std::ofstream &file) {
    stack_size--;
    file << "  pop " << reg << "\n";
    if (stack_size > 1000) {
        exit(1);
    }
}

void mov(const std::string &reg1, const std::string &reg2, std::ofstream &file) {
    file << "  mov " << reg1 << ", " << reg2 << "\n";
}

OperatorType check_operator(Node *node) {
    if (node->type != TokenType::OPERATOR) {
        return OperatorType::NOT_OPERATOR;
    }

    if (node->value == "+") {
        return OperatorType::ADD;
    }
    if (node->value == "-") {
        return OperatorType::SUB;
    }
    if (node->value == "/") {
        return OperatorType::DIV;
    }
    if (node->value == "*") {
        return OperatorType::MUL;
    }
    if (node->value == "%") {
        return OperatorType::MOD;
    }
    return OperatorType::NOT_OPERATOR;
}

int mov_if_var_or_not(const std::string &reg, Node *node, std::ofstream &file) {
    if (node->type == TokenType::IDENTIFIER) {
        auto it = hashmap.find(node->value);
        if (it == hashmap.end()) {
            std::cerr << "ERROR: Variable " << node->value << " not declared in current scope\n";
            exit(1);
        }
        push_var(it->second, node->value, file);
        pop(reg, file);
        return 0;
    }
    if (node->type == TokenType::INT) {
        file << "  mov " << reg << ", " << node->value << "\n";
        return 0;
    }
    return -1;
}

Node *generate_operator_code(Node *node, std::ofstream &file) {
    mov_if_var_or_not("rax", node->left, file);
    push("rax", file);
    Node *tmp = node;
    OperatorType oper_type = check_operator(tmp);
    while (tmp->type == TokenType::OPERATOR) {
        pop("rax", file);
        oper_type = check_operator(tmp);
        tmp = tmp->right;
        if (tmp->type != TokenType::OPERATOR) {
            break;
        }
        mov_if_var_or_not("rbx", tmp->left, file);
        switch (oper_type) {
            case OperatorType::ADD:
                file << "  add rax, rbx\n";
                break;
            case OperatorType::SUB:
                file << "  sub rax, rbx\n";
                break;
            case OperatorType::DIV:
                file << "  xor rdx, rdx\n";
                file << "  div rbx\n";
                break;
            case OperatorType::MUL:
                file << "  mul rbx\n";
                break;
            case OperatorType::MOD:
                file << "  xor rdx, rdx\n";
                file << "  div rbx\n";
                break;
            case OperatorType::NOT_OPERATOR:
                std::cerr << "ERROR: Invalid Syntax\n";
                exit(1);
                break;
        }
        if (oper_type != OperatorType::MOD) {
            push("rax", file);
        } else {
            push("rdx", file);
        }
        oper_type = check_operator(tmp);
    }
    mov_if_var_or_not("rbx", tmp, file);
    switch (oper_type) {
        case OperatorType::ADD:
            file << "  add rax, rbx\n";
            break;
        case OperatorType::SUB:
            file << "  sub rax, rbx\n";
            break;
        case OperatorType::DIV:
            file << "  xor rdx, rdx\n";
            file << "  div rbx\n";
            break;
        case OperatorType::MUL:
            file << "  mul rbx\n";
            break;
        case OperatorType::MOD:
            file << "  xor rdx, rdx\n";
            file << "  div rbx\n";
            break;
        case OperatorType::NOT_OPERATOR:
            std::cerr << "ERROR: Invalid Syntax\n";
            exit(1);
            break;
    }
    if (oper_type != OperatorType::MOD) {
        push("rax", file);
    } else {
        push("rdx", file);
    }
    node->left = nullptr;
    node->right = nullptr;
    return node;
}

void traverse_tree(Node *node, int is_left, std::ofstream &file, int syscall_number) {
    if (node == nullptr) {
        return;
    }
    if (node->value == "EXIT") {
        syscall_number = 60;
    }
    if (node->value == "INT") {
        Node *value = node->left->left->left;
        if (value->type == TokenType::IDENTIFIER) {
            auto it = hashmap.find(value->value);
            if (it == hashmap.end()) {
                std::cerr << "ERROR: " << value->value << " Not Declared In Current Context\n";
                exit(1);
            }
            push_var(it->second, value->value, file);
        } else if (value->type == TokenType::INT) {
            push(value->value, file);
        } else if (value->type == TokenType::OPERATOR) {
            generate_operator_code(value, file);
        } else {
            std::cerr << "ERROR\n";
            exit(1);
        }
        size_t cur_size = stack_size;
        if (hashmap.find(node->left->value) != hashmap.end()) {
            std::cerr << "ERROR: Variable " << node->left->value << " is already declared in current scope\n";
            exit(1);
        }
        hashmap[node->left->value] = cur_size;
        node->left = nullptr;
    } else if (node->value == "IF") {
        curly_stack_push("IF");
        Node *current = node->left->left;
        if (current->left->type == TokenType::INT || current->left->type == TokenType::IDENTIFIER) {
            mov_if_var_or_not("rax", current->left, file);
            push("rax", file);
        } else {
            generate_operator_code(current->left, file);
        }
        if (current->right->type == TokenType::INT || current->right->type == TokenType::IDENTIFIER) {
            mov_if_var_or_not("rbx", current->right, file);
            push("rbx", file);
        } else {
            generate_operator_code(current->right, file);
        }
        pop("rax", file);
        pop("rbx", file);
        file << "  cmp rax, rbx\n";
        if_label(file, current->value, curly_count);
        node->left->left = nullptr;
    } else if (node->value == "WHILE") {
        curly_stack_push("W");
        create_loop_label(file);
        Node *current = node->left->left;
        if (current->left->type == TokenType::INT || current->left->type == TokenType::IDENTIFIER) {
            mov_if_var_or_not("rax", current->left, file);
            push("rax", file);
        } else {
            generate_operator_code(current->left, file);
        }
        if (current->right->type == TokenType::INT || current->right->type == TokenType::IDENTIFIER) {
            mov_if_var_or_not("rbx", current->right, file);
            push("rbx", file);
        } else {
            generate_operator_code(current->right, file);
        }
        pop("rbx", file);
        pop("rax", file);
        file << "  cmp rax, rbx\n";
        if (current->value == "EQ") {
            if_label(file, "EQ", curly_count);
        } else if (current->value == "NEQ") {
            if_label(file, "NEQ", curly_count);
        } else if (current->value == "LESS") {
            if_label(file, "LESS", curly_count);
        } else if (current->value == "GREATER") {
            if_label(file, "GREATER", curly_count);
        } else {
            std::cerr << "ERROR: Unknown Operator\n";
            exit(1);
        }
        node->left->left = nullptr;
    } else if (node->value == "WRITE") {
        if (node->left->type == TokenType::IDENTIFIER) {
            auto it = hashmap.find(node->left->value);
            if (it == hashmap.end()) {
                std::cerr << "ERROR: Value is not defined\n";
                exit(1);
            }
            push_var(it->second, node->right->value, file);
            mov("rdi", "printf_format", file);
            pop("rsi", file);
            file << "  xor rax, rax\n";
            file << "  call printf WRT ..plt\n";
        } else {
            std::string text = "text" + std::to_string(text_label);
            file << "section .data\n";
            file << " " << text << " db \"" << node->left->value << "\", 10\n";
            file << "section .text\n";
            mov("rax", "1", file);
            mov("rdx", node->right->value, file);
            mov("rdi", "1", file);
            mov("rsi", text, file);
            text_label++;
            file << "  syscall\n";
        }
        node = node->right->right;
        node->right = nullptr;
    }

    if (node->value == "(") {
        // Handle the "(" case
    }
    if (node->type == TokenType::OPERATOR) {
        if (node->value[0] == '=') {
            // Handle the "=" case
        } else {
            generate_operator_code(node, file);
        }
    }
    if (node->type == TokenType::INT) {
        file << "  mov rax, " << node->value << "\n";
        push("rax", file);
    }
    if (node->type == TokenType::IDENTIFIER) {
        if (syscall_number == 60) {
            auto it = hashmap.find(node->value);
            if (it == hashmap.end()) {
                std::cerr << "ERROR: Not Declared in current scope: " << node->value << "\n";
                exit(1);
            }
            push_var(it->second, node->value, file);
            pop("rdi", file);
            file << "  mov rax, " << syscall_number << "\n";
            file << "  syscall\n";
            syscall_number = 0;
        } else {
            auto it = hashmap.find(node->value);
            if (it == hashmap.end()) {
                std::cerr << "ERROR: Variable " << node->value << " is not declared in current scope\n";
                exit(1);
            }
            Node *value = node->left->left;
            auto var_location = hashmap.find(node->value)->second;
            if (value->type == TokenType::IDENTIFIER) {
                auto var_value = hashmap.find(value->value);
                if (var_value == hashmap.end()) {
                    std::cerr << "ERROR: " << value->value << " Not Declared In Current Context\n";
                    exit(1);
                }
            } else if (value->type == TokenType::INT) {
                push(value->value, file);
            } else if (value->type == TokenType::OPERATOR) {
                generate_operator_code(value, file);
            } else {
                std::cerr << "ERROR\n";
                exit(1);
            }
            size_t cur_size = stack_size;
            pop("rax", file);
            modify_var(var_location + 1, "rax", node->value, file);
            node->left = nullptr;
        }
    }
    if (node->value == ")") {
        // Handle the ")" case
    }

    if (node->value == "{") {
        stack_push(stack_size);
        curly_count++;
        curly_stack_push(std::to_string(curly_count));
    }

    if (node->value == "}") {
        std::string current_curly = curly_stack_pop();
        std::string next_curly = curly_stack_pop();

        if (next_curly == "IF") {
            create_label(file, std::stoi(current_curly) - 1);
            global_curly = std::stoi(current_curly);
        } else if (next_curly == "W") {
            create_end_loop(file);
            create_label(file, std::stoi(current_curly) - 1);
            global_curly = std::stoi(current_curly);
        }

        size_t stack_value = stack_pop();
        while (stack_size != stack_value) {
            pop("rsi", file);
        }

        for (auto it = hashmap.begin(); it != hashmap.end();) {
            if (it->second > current_stack_size[current_stack_size_size] + 1) {
                it = hashmap.erase(it);
            } else {
                ++it;
            }
        }
    }

    if (node->value == ";") {
        if (syscall_number == 60) {
            file << "  mov rax, " << syscall_number << "\n";
            file << "  pop rdi\n";
            file << "  syscall\n";
            syscall_number = 0;
        }
    }

    traverse_tree(node->left, 1, file, syscall_number);
    traverse_tree(node->right, 0, file, syscall_number);
}

int generate_code(Node *root, const std::string &filename) {
    std::ofstream file(filename);
    assert(file.is_open() && "FILE COULD NOT BE OPENED\n");

    file << "section .data\n";
    file << "  printf_format: db '%s', 10, 0\n";
    file << "extern printf\n";
    file << "global main\n";
    file << "section .text\n";
    file << "main:\n";

    traverse_tree(root, 0, file, 0);
    file.close();

    return 0;
}

int main() {
    // Test and use the translated functions here
    return 0;
}
