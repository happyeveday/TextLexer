#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <stack>
#include <queue>
#include <cctype>
#include <algorithm>
using namespace std;

// 单词符号类型编码（复用词法分析器的定义）
enum TokenType
{
    TOKEN_ID,      // 标识符
    TOKEN_NUM,     // 整常数
    TOKEN_FLOAT,   // 浮点数
    TOKEN_BOOL,    // 布尔常量
    TOKEN_KEYWORD, // 关键字
    TOKEN_OP,      // 运算符
    TOKEN_SEP,     // 分隔符
    TOKEN_ERROR    // 错误
};

// 语法树节点类型
enum NodeType
{
    NODE_EXPR,    // 表达式
    NODE_BOOL,    // 布尔表达式
    NODE_DECLS,   // 声明语句
    NODE_STMTS,   // 执行语句
    NODE_ASSIGN,  // 赋值语句
    NODE_IF,      // if语句
    NODE_WHILE,   // while语句
    NODE_FOR,     // for语句
    NODE_READ,    // read语句
    NODE_WRITE,   // write语句
    NODE_BLOCK,   // 语句块
    NODE_OP,      // 运算符
    NODE_ID,      // 标识符
    NODE_NUM,     // 数字常量
    NODE_FLOAT,   // 浮点数常量
    NODE_BOOLVAL, // 布尔值
    NODE_TYPE,    // 类型
    NODE_LIST     // 列表
};

// 语法树节点结构
struct TreeNode
{
    NodeType type;
    string value;
    vector<TreeNode *> children;

    TreeNode(NodeType t, const string &v = "") : type(t), value(v) {}

    ~TreeNode()
    {
        for (auto child : children)
        {
            delete child;
        }
    }
};

// 单词符号的二元组（复用词法分析器的定义）
struct Token
{
    TokenType type;
    string value;
};

// 语法分析器类
class Parser
{
private:
    vector<Token> tokens;
    size_t current = 0;

    TreeNode* parseDecl() {
        cerr << "DEBUG: Parsing declaration, current token: " << peek().value << endl;
        // 解析类型关键字
        string type;
        if (match(TOKEN_KEYWORD, "int")) {
            type = "int";
        } else if (match(TOKEN_KEYWORD, "float")) {
            type = "float";
        } else if (match(TOKEN_KEYWORD, "bool")) {
            type = "bool";
        } else {
            error("Expected type keyword in declaration");
        }
        
        TreeNode* declNode = new TreeNode(NODE_LIST);
        declNode->children.push_back(new TreeNode(NODE_TYPE, type));
    
        // 解析变量声明（允许带初始化）
        do {
            consume(TOKEN_ID, "Expected variable name");
            TreeNode* idNode = new TreeNode(NODE_ID, previous().value);
            declNode->children.push_back(idNode);
    
            // 处理初始化
            if (match(TOKEN_OP, "=")) {
                TreeNode* initNode = (type == "bool") ? parseBoolExpr() : parseArithmeticExpr();
                declNode->children.push_back(initNode);
            }
        } while (match(TOKEN_SEP, ",")); // 支持多变量声明，如 int a,b=2;
    
        consume(TOKEN_SEP, ";", "Expected ';' after declaration");
        return declNode;
    }

    TreeNode* parseStmts() {
        TreeNode* stmtsNode = new TreeNode(NODE_STMTS);
        while (!isAtEnd() && !check(TOKEN_SEP, "}")) {
            TreeNode* stmt = parseStmt();
            if (stmt) {
                stmtsNode->children.push_back(stmt);
            }
        }
        return stmtsNode;
    }

    // 查看当前token
    Token peek() const
    {
        if (current < tokens.size())
        {
            return tokens[current];
        }
        return {TOKEN_ERROR, ""};
    }

    // 查看前一个token
    Token previous() const
    {
        if (current > 0)
        {
            return tokens[current - 1];
        }
        return {TOKEN_ERROR, ""};
    }

    // 检查是否到达末尾
    bool isAtEnd() const
    {
        return peek().type == TOKEN_ERROR && peek().value.empty();
    }

    // 前进到下一个token
    Token advance()
    {
        if (!isAtEnd())
            current++;
        return previous();
    }

    // 检查当前token是否匹配给定类型
    bool check(TokenType type) const
    {
        if (isAtEnd())
            return false;
        return peek().type == type;
    }

    // 检查当前token是否匹配给定类型和值
    bool check(TokenType type, const string &value) const
    {
        if (isAtEnd())
            return false;
        return peek().type == type && peek().value == value;
    }

    // 检查当前token是否匹配给定值（类型不限）
    bool check(const string &value) const
    {
        if (isAtEnd())
            return false;
        return peek().value == value;
    }

    // 匹配给定类型
    bool match(TokenType type)
    {
        if (check(type))
        {
            advance();
            return true;
        }
        return false;
    }

    // 匹配给定类型和值
    bool match(TokenType type, const string &value)
    {
        if (check(type, value))
        {
            advance();
            return true;
        }
        return false;
    }

    // 匹配给定值（类型不限）
    bool match(const string &value)
    {
        if (check(value))
        {
            advance();
            return true;
        }
        return false;
    }

    // 错误处理
    void error(const string &message)
    {
        cerr << "Syntax error: " << message << " at token: " << peek().value << endl;
        exit(1);
    }

    // 消耗一个token，如果不匹配则报错
    void consume(TokenType type, const string &message)
    {
        if (check(type))
        {
            advance();
            return;
        }
        error(message);
    }

    // 消耗一个token（指定值和类型），如果不匹配则报错
    void consume(TokenType type, const string &value, const string &message)
    {
        if (check(type, value))
        {
            advance();
            return;
        }
        error(message + " (Actual: " + peek().value + ")");
    }

    // 消耗一个token（指定值），如果不匹配则报错
    void consume(const string &value, const string &message)
    {
        if (check(value))
        {
            advance();
            return;
        }
        error(message);
    }

    TreeNode *parseArithmeticExpr() {
        // 添加空表达式检查
        if (check(TOKEN_SEP, ";")) {
            error("Empty expression not allowed here");
        }
        
        // 使用算符优先分析法处理算术表达式
        stack<TreeNode *> nodeStack;
        stack<string> opStack;
    
        // 算符优先级表
        unordered_map<string, int> precedence = {
            {"||", 1}, {"&&", 2},
            {"==", 3}, {"!=", 3}, {"<", 3}, {"<=", 3}, {">", 3}, {">=", 3},
            {"+", 4}, {"-", 4},
            {"*", 5}, {"/", 5}, {"%", 5},
            {"!", 6}, {"++", 6}, {"--", 6}};
    
        auto processOp = [&]() {
            string op = opStack.top();
            opStack.pop();
    
            TreeNode *node = new TreeNode(NODE_OP, op);
    
            // 处理一元运算符
            if (op == "!" || op == "++" || op == "--") {
                if (nodeStack.empty())
                    error("Missing operand for unary operator");
                node->children.push_back(nodeStack.top());
                nodeStack.pop();
            }
            // 处理二元运算符
            else {
                if (nodeStack.size() < 2)
                    error("Missing operands for binary operator");
                TreeNode *right = nodeStack.top();
                nodeStack.pop();
                TreeNode *left = nodeStack.top();
                nodeStack.pop();
    
                node->children.push_back(left);
                node->children.push_back(right);
            }
    
            nodeStack.push(node);
        };
    
        while (!isAtEnd() && !check(TOKEN_SEP, ";") && !check(TOKEN_SEP, ",") &&
            !check(TOKEN_KEYWORD, "then") && !check(TOKEN_KEYWORD, "do") &&
            !check(TOKEN_KEYWORD, "else") && !check(TOKEN_SEP, "{")) {  // 添加对{的检查
            if (match(TOKEN_SEP, "(")) {
                opStack.push("(");
            } else if (match(TOKEN_SEP, ")")) {
                while (!opStack.empty() && opStack.top() != "(") {
                    processOp();
                }
                if (opStack.empty()) {
                    error("Unmatched parentheses");
                }
                opStack.pop(); // 弹出 "("
            } else if (match(TOKEN_OP)) {
                string op = previous().value;
    
                // 处理负号（减号和负号的歧义）
                if (op == "-" && (nodeStack.empty() ||
                                (!opStack.empty() && opStack.top() == "("))) {
                    op = "neg"; // 标记为一元负号
                }
    
                while (!opStack.empty() && opStack.top() != "(" &&
                    precedence[opStack.top()] >= precedence[op]) {
                    processOp();
                }
                opStack.push(op);
            } else {
                // 处理操作数
                TreeNode *operand = nullptr;
                if (match(TOKEN_ID)) {
                    operand = new TreeNode(NODE_ID, previous().value);
                } else if (match(TOKEN_NUM)) {
                    operand = new TreeNode(NODE_NUM, previous().value);
                } else if (match(TOKEN_FLOAT)) {
                    operand = new TreeNode(NODE_FLOAT, previous().value);
                } else if (match(TOKEN_BOOL)) {
                    operand = new TreeNode(NODE_BOOLVAL, previous().value);
                } else {
                    error("Expected operand in expression");
                }
                nodeStack.push(operand);
            }
        }
    
        // 处理剩余的运算符
        while (!opStack.empty()) {
            if (opStack.top() == "(") {
                error("Unmatched parentheses");
            }
            processOp();
        }
    
        if (nodeStack.empty()) {
            error("Empty expression");
        }
        if (nodeStack.size() > 1) {
            error("Malformed expression");
        }
    
        TreeNode *exprNode = new TreeNode(NODE_EXPR);
        exprNode->children.push_back(nodeStack.top());
        return exprNode;
    }

    // 布尔表达式（复用算术表达式的算符优先分析）
    TreeNode *parseBoolExpr() {
        TreeNode *left = parseArithmeticExpr();
        
        if (check(TOKEN_SEP, "{")) {
            return left;
        }

        // 处理比较运算符
        if (check(TOKEN_OP, ">") || check(TOKEN_OP, "<") || 
            check(TOKEN_OP, ">=") || check(TOKEN_OP, "<=") ||
            check(TOKEN_OP, "==") || check(TOKEN_OP, "!=")) {
            Token op = advance();
            TreeNode *right = parseArithmeticExpr();
            
            TreeNode *boolNode = new TreeNode(NODE_BOOL, op.value);
            boolNode->children.push_back(left);
            boolNode->children.push_back(right);
            return boolNode;
        }
        
        // 如果没有比较运算符，直接返回算术表达式
        return left;
    }
    
    // 声明语句
    TreeNode *parseDecls()
    {
        TreeNode* declsNode = new TreeNode(NODE_DECLS);
        while (!isAtEnd()) {
            if (check(TOKEN_KEYWORD, "int") || check(TOKEN_KEYWORD, "float") || check(TOKEN_KEYWORD, "bool")) {
                string type;
                if (match(TOKEN_KEYWORD, "int")) {
                    type = "int";
                } else if (match(TOKEN_KEYWORD, "float")) {
                    type = "float";
                } else if (match(TOKEN_KEYWORD, "bool")) {
                    type = "bool";
                } // 闭合if语句块
    
                TreeNode* typeNode = new TreeNode(NODE_TYPE, type);
                TreeNode* declNode = new TreeNode(NODE_LIST);
                declNode->children.push_back(typeNode);
    
                do {
                    if (match(TOKEN_SEP, ";")) break; // 允许空声明
                    consume(TOKEN_ID, "Expected variable name in declaration");
                    TreeNode* idNode = new TreeNode(NODE_ID, previous().value);
                    declNode->children.push_back(idNode);
    
                    if (match(TOKEN_OP, "=")) {
                        TreeNode* initNode = (type == "bool") ? parseBoolExpr() : parseArithmeticExpr();
                        declNode->children.push_back(initNode);
                    }
                } while (match(TOKEN_SEP, ","));
    
                consume(TOKEN_SEP, ";", "Expected ';' after declaration");
                declsNode->children.push_back(declNode);
            } else {
                break; // 无更多声明
            }
        }
        return declsNode;
    }

    // 赋值语句
    TreeNode *parseAssignStmt(bool inForLoop = false) {
        consume(TOKEN_ID, "Expected identifier in assignment");
        TreeNode *idNode = new TreeNode(NODE_ID, previous().value);

        string op = peek().value;
        
        // 处理自增/自减运算符
        if (op == "++" || op == "--") {
            consume(TOKEN_OP, "Expected operator");
            TreeNode *assignNode = new TreeNode(NODE_ASSIGN, op);
            assignNode->children.push_back(idNode);
            if (!inForLoop) {
                consume(TOKEN_SEP, ";", "Expected ';' after assignment");
            }
            return assignNode;
        }
        
        consume(TOKEN_OP, "Expected assignment operator");
        TreeNode *assignNode = new TreeNode(NODE_ASSIGN, op);
        assignNode->children.push_back(idNode);

        if (op == "=") {
            // 需要判断是算术表达式还是布尔表达式
            if (check(TOKEN_BOOL) || check(TOKEN_OP, "!") ||
                check(TOKEN_ID) || check(TOKEN_SEP, "(")) {
                assignNode->children.push_back(parseBoolExpr());
            } else {
                assignNode->children.push_back(parseArithmeticExpr());
            }
        } else {
            // 复合赋值运算符如 +=, -= 等
            assignNode->children.push_back(parseArithmeticExpr());
        }

        if (!inForLoop) {
            consume(TOKEN_SEP, ";", "Expected ';' after assignment");
        }
        return assignNode;
    }

    // if语句
    TreeNode* parseIfStmt() {
        cerr << "DEBUG: Enter parseIfStmt, current token: " << peek().value << endl;
        consume(TOKEN_KEYWORD, "if", "Expected 'if'");
        consume(TOKEN_SEP, "(", "Expected '(' after 'if'");
        
        // 解析条件表达式
        TreeNode* cond = parseBoolExpr();
        cerr << "DEBUG: After parseBoolExpr, current token: " << peek().value << endl;
        consume(TOKEN_SEP, ")", "Expected ')' after condition");

        // 确保消耗{
        consume(TOKEN_SEP, "{", "Expected '{' to start if block");
        
        // 解析then分支
        TreeNode* thenBranch = nullptr;
        if (match(TOKEN_SEP, "{")) {
            thenBranch = parseBlock();
        } else {
            // 单条语句的情况
            thenBranch = parseStmt();
            // 如果stmt以分号结束，需要消耗分号
            if (check(TOKEN_SEP, ";")) {
                advance();
            }
        }
        
        TreeNode* ifNode = new TreeNode(NODE_IF);
        ifNode->children.push_back(cond);
        ifNode->children.push_back(thenBranch);
        
        // 解析else分支
        if (match(TOKEN_KEYWORD, "else")) {
            // 确保消耗{
            consume(TOKEN_SEP, "{", "Expected '{' to start else block");
            TreeNode* elseBranch = parseBlock();
            ifNode->children.push_back(elseBranch);
        }
        return ifNode;
    }

    // while语句
    TreeNode* parseWhileStmt() 
{
    consume(TOKEN_KEYWORD, "while", "Expected 'while'");
    consume(TOKEN_SEP, "(", "Expected '(' after 'while'");
    
    TreeNode* whileNode = new TreeNode(NODE_WHILE);
    whileNode->children.push_back(parseBoolExpr());
    
    // 确保消耗右括号
    consume(TOKEN_SEP, ")", "Expected ')' after condition");
    
    // 直接解析循环体（可以是语句块或单条语句）
    if (check(TOKEN_SEP, "{")) {
        whileNode->children.push_back(parseBlock());
    } else {
        whileNode->children.push_back(parseStmt());
    }
    
    return whileNode;
}

    // for语句
    TreeNode* parseForStmt() {
        cerr << "DEBUG: Parsing for statement, current token: " << peek().value << endl;
        consume(TOKEN_KEYWORD, "for", "Expected 'for'");
        consume(TOKEN_SEP, "(", "Expected '(' after 'for'");
        
        TreeNode* forNode = new TreeNode(NODE_FOR);
        
        // 初始化部分
        if (!check(TOKEN_SEP, ";")) {
            cerr << "DEBUG: Parsing for initializer" << endl;
            if (check(TOKEN_KEYWORD, "int") || check(TOKEN_KEYWORD, "float") || check(TOKEN_KEYWORD, "bool")) {
                cerr << "DEBUG: Found type declaration in for initializer" << endl;
                // 使用parseDecl()来处理类型声明，parseDecl()已经消耗了分号
                TreeNode* decl = parseDecl();
                forNode->children.push_back(decl);
            } else {
                TreeNode* assign = parseAssignStmt();
                forNode->children.push_back(assign);
                // 只有非类型声明的情况才需要消耗分号
                consume(TOKEN_SEP, ";", "Expected ';' after for initializer");
            }
        } else {
            forNode->children.push_back(nullptr);
            consume(TOKEN_SEP, ";", "Expected ';' after for initializer");
        }
        
        // 条件部分
        cerr << "DEBUG: Before condition, current token: " << peek().value << endl;
        if (!check(TOKEN_SEP, ";")) {
            forNode->children.push_back(parseBoolExpr());
        } else {
            forNode->children.push_back(nullptr);
        }
        consume(TOKEN_SEP, ";", "Expected ';' after for condition");
        cerr << "DEBUG: After condition, current token: " << peek().value << endl;
        
        // 迭代表达式
        if (!check(TOKEN_SEP, ")")) {
            TreeNode* updateNode = parseAssignStmt(true); // 传递true表示在for循环中
            forNode->children.push_back(updateNode);
        } else {
            forNode->children.push_back(nullptr);
        }
        consume(TOKEN_SEP, ")", "Expected ')' after for update");
        
        // 循环体 - 允许语句块或单条语句
        if (check(TOKEN_SEP, "{")) {
            forNode->children.push_back(parseBlock());
        } else {
            // 单条语句的情况
            TreeNode* stmtNode = new TreeNode(NODE_BLOCK);
            stmtNode->children.push_back(parseStmt());
            forNode->children.push_back(stmtNode);
        }
        
        return forNode;
    }

    // read语句
    TreeNode *parseReadStmt()
    {
        consume(TOKEN_KEYWORD, "read", "Expected 'read'");
        consume(TOKEN_SEP, "(", "Expected '(' after 'read'");

        TreeNode *readNode = new TreeNode(NODE_READ);

        do
        {
            consume(TOKEN_ID, "Expected variable name in read statement");
            readNode->children.push_back(new TreeNode(NODE_ID, previous().value));
        } while (match(TOKEN_SEP, ","));

        consume(TOKEN_SEP, ")", "Expected ')' after read arguments");
        consume(TOKEN_SEP, ";", "Expected ';' after read statement");
        return readNode;
    }

    // write语句
    TreeNode *parseWriteStmt() {
        consume(TOKEN_KEYWORD, "write", "Expected 'write'");
        
        TreeNode *writeNode = new TreeNode(NODE_WRITE);
        
        // 处理带括号的write语句
        if (match(TOKEN_SEP, "(")) {
            do {
                consume(TOKEN_ID, "Expected variable name in write statement");
                writeNode->children.push_back(new TreeNode(NODE_ID, previous().value));
            } while (match(TOKEN_SEP, ","));
            consume(TOKEN_SEP, ")", "Expected ')' after write arguments");
        } else {
            // 直接读取标识符，不需要括号
            consume(TOKEN_ID, "Expected variable name in write statement");
            writeNode->children.push_back(new TreeNode(NODE_ID, previous().value));
        }
        
        consume(TOKEN_SEP, ";", "Expected ';' after write statement");
        return writeNode;
    }

    // 语句序列
    TreeNode* parseStmt() {
        if (check(TOKEN_SEP, "{")) {
            return parseBlock();
        } else if (check(TOKEN_KEYWORD, "if")) {
            return parseIfStmt();
        } else if (check(TOKEN_KEYWORD, "while")) {
            return parseWhileStmt();
        } else if (check(TOKEN_KEYWORD, "for")) {
            return parseForStmt();
        } else if (check(TOKEN_KEYWORD, "read")) {
            return parseReadStmt();
        } else if (check(TOKEN_KEYWORD, "write")) {
            return parseWriteStmt();
        } else if (check(TOKEN_ID)) {
            return parseAssignStmt();
        } else if (match(TOKEN_SEP, ";")) {
            // 修改这里，直接返回空语句节点而不调用parseArithmeticExpr()
            return new TreeNode(NODE_STMTS, "empty_stmt"); 
        } else {
            error("Expected statement but found: " + peek().value);
            return nullptr;
        }
    }

    TreeNode *parseBlock() {
        // 这里确保消耗{
        consume(TOKEN_SEP, "{", "Expected '{' to start block");
        TreeNode* blockNode = new TreeNode(NODE_BLOCK);
        
        while (!isAtEnd() && !check(TOKEN_SEP, "}")) {
            TreeNode* stmt = parseStmt();
            if (stmt) {
                blockNode->children.push_back(stmt);
            }
        }
        
        consume(TOKEN_SEP, "}", "Expected '}' to end block");
        return blockNode;
    }


    // 打印语法树（辅助函数）
    void printTree(const TreeNode *node, ofstream &outFile, int depth = 0)
    {
        if (!node)
            return;

        // 缩进
        for (int i = 0; i < depth; ++i)
        {
            outFile << "  ";
        }

        // 打印节点类型和值
        outFile << "[" << nodeTypeToString(node->type) << "]";
        if (!node->value.empty())
        {
            outFile << " " << node->value;
        }
        outFile << endl;

        // 打印子节点
        for (const auto &child : node->children)
        {
            printTree(child, outFile, depth + 1);
        }
    }

    // 节点类型转字符串（辅助函数）
    string nodeTypeToString(NodeType type) const
    {
        switch (type)
        {
        case NODE_EXPR:
            return "EXPR";
        case NODE_BOOL:
            return "BOOL";
        case NODE_DECLS:
            return "DECLS";
        case NODE_STMTS:
            return "STMTS";
        case NODE_ASSIGN:
            return "ASSIGN";
        case NODE_IF:
            return "IF";
        case NODE_WHILE:
            return "WHILE";
        case NODE_FOR:
            return "FOR";
        case NODE_READ:
            return "READ";
        case NODE_WRITE:
            return "WRITE";
        case NODE_BLOCK:
            return "BLOCK";
        case NODE_OP:
            return "OP";
        case NODE_ID:
            return "ID";
        case NODE_NUM:
            return "NUM";
        case NODE_FLOAT:
            return "FLOAT";
        case NODE_BOOLVAL:
            return "BOOLVAL";
        case NODE_TYPE:
            return "TYPE";
        case NODE_LIST:
            return "LIST";
        default:
            return "UNKNOWN";
        }
    }

public:
    Parser(const vector<Token> &t) : tokens(t) {}

    // 解析入口
    TreeNode *parse()
    {
        TreeNode *programNode = new TreeNode(NODE_BLOCK); // 用BLOCK作为程序根节点

        // 先解析声明部分
        programNode->children.push_back(parseDecls());

        // 然后解析语句部分
        programNode->children.push_back(parseStmts());

        return programNode;
    }

    // 输出语法树到文件
    void outputTree(const TreeNode *root, const string &filename)
    {
        ofstream outFile(filename);
        if (!outFile)
        {
            cerr << "Can't open output file: " << filename << endl;
            return;
        }

        printTree(root, outFile);
        outFile.close();
        cout << "Parse success. Output written to " << filename << endl;
    }
};

// 从文件读取token序列
vector<Token> readTokens(const string &filename)
{
    ifstream inFile(filename);
    if (!inFile)
    {
        cerr << "Can't open input file: " << filename << endl;
        exit(1);
    }

    vector<Token> tokens;
    string line;

    while (getline(inFile, line))
    {
        if (line.empty() || line.find('(') == string::npos || line.find(')') == string::npos)
        {
            continue;
        }

        // 提取类型和值（格式：(TYPE, VALUE)）
        size_t typeStart = line.find('(') + 1;
        size_t typeEnd = line.find(',');
        if (typeEnd == string::npos)
            continue;

        string typeStr = line.substr(typeStart, typeEnd - typeStart);
        size_t end = line.rfind(')'); // 找到最后一个')'
        string value = line.substr(typeEnd + 1, end - typeEnd - 1);
        value.erase(remove_if(value.begin(), value.end(), ::isspace), value.end());

        // 去除值中的空格
        value.erase(remove_if(value.begin(), value.end(), ::isspace), value.end());

        TokenType type;
        if (typeStr == "0")
            type = TOKEN_ID;
        else if (typeStr == "1")
            type = TOKEN_NUM;
        else if (typeStr == "2")
            type = TOKEN_FLOAT;
        else if (typeStr == "3")
            type = TOKEN_BOOL;
        else if (typeStr == "4")
            type = TOKEN_KEYWORD;
        else if (typeStr == "5")
            type = TOKEN_OP;
        else if (typeStr == "6")
            type = TOKEN_SEP;
        else
            type = TOKEN_ERROR;

        tokens.push_back({type, value});
    }

    inFile.close();
    return tokens;
}

// 主函数
int main()
{
    // 读取token序列
    vector<Token> tokens = readTokens("lex_out.txt");

    for (const auto &token : tokens)
    {
        cout << "Token: type=" << token.type << ", value=" << token.value << endl;
    }

    // 语法分析
    Parser parser(tokens);
    TreeNode *syntaxTree = parser.parse();

    // 输出语法树
    parser.outputTree(syntaxTree, "parse_out.txt");

    // 释放内存
    delete syntaxTree;

    return 0;
}