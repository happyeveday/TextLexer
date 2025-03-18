#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cctype>
using namespace std;

// 单词符号类型编码
enum TokenType {
    TOKEN_ID,        // 标识符
    TOKEN_NUM,       // 整常数
    TOKEN_FLOAT,     // 浮点数
    TOKEN_BOOL,      // 布尔常量
    TOKEN_KEYWORD,   // 关键字
    TOKEN_OP,        // 运算符
    TOKEN_SEP,       // 分隔符
    TOKEN_ERROR      // 错误
};

// 符号表：关键字
unordered_map<string, TokenType> keywords = {
    {"int", TOKEN_KEYWORD},
    {"float", TOKEN_KEYWORD}, // 新增 float
    {"bool", TOKEN_KEYWORD},
    {"if", TOKEN_KEYWORD},
    {"else", TOKEN_KEYWORD},
    {"while", TOKEN_KEYWORD},
    {"for", TOKEN_KEYWORD}, // 新增 for
    {"read", TOKEN_KEYWORD},
    {"write", TOKEN_KEYWORD},
    {"true", TOKEN_BOOL},
    {"false", TOKEN_BOOL}
};

// 符号表：运算符
unordered_map<string, TokenType> operators = {
    {"+", TOKEN_OP}, 
    {"-", TOKEN_OP}, 
    {"*", TOKEN_OP}, 
    {"/", TOKEN_OP},
    {"=", TOKEN_OP}, 
    {"==", TOKEN_OP}, 
    {"!=", TOKEN_OP}, 
    {"<", TOKEN_OP},
    {"<=", TOKEN_OP}, 
    {">", TOKEN_OP}, 
    {">=", TOKEN_OP}, 
    {"&&", TOKEN_OP},
    {"||", TOKEN_OP}, 
    {"!", TOKEN_OP}, 
    {"++", TOKEN_OP}, 
    {"--", TOKEN_OP} // 添加自增、自减运算符
};


// 符号表：分隔符
unordered_map<string, TokenType> separators = {
    {";", TOKEN_SEP},
    {",", TOKEN_SEP},
    {"(", TOKEN_SEP},
    {")", TOKEN_SEP},
    {"{", TOKEN_SEP},
    {"}", TOKEN_SEP}
};

// 单词符号的二元组
struct Token {
    TokenType type;
    string value;
};

// 词法分析器
class Lexer {
private:
    string source; // 源程序
    size_t pos = 0; // 当前扫描位置

    // 读取下一个字符
    char peek() {
        return (pos < source.length()) ? source[pos] : '\0';
    }

    // 读取并移动指针
    char advance() {
        return (pos < source.length()) ? source[pos++] : '\0';
    }

    // 跳过空白字符
    void skipWhitespace() {
        if (peek() == '/' && source[pos + 1] == '/') {
            while (peek() != '\n' && peek() != '\0') advance();
        }
        if (peek() == '/' && source[pos + 1] == '*') {
            advance(); // 跳过 '/'
            advance(); // 跳过 '*'
            while (!(peek() == '*' && source[pos + 1] == '/')) {
                if (peek() == '\0') return; // 文件结束
                advance();
            }
            advance(); // 跳过 '*'
            advance(); // 跳过 '/'
        }
        while (isspace(peek())) advance();
    }

    // 识别标识符或关键字
    Token recognizeIdOrKeyword() {
        string value;
        while (isalnum(peek()) || peek() == '_') value += advance();
        if (keywords.find(value) != keywords.end()) {
            return {keywords[value], value};
        }
        return {TOKEN_ID, value};
    }

    // 识别整常数或浮点数
    Token recognizeNumber() {
        string value;
        while (isdigit(peek())) value += advance();
        if (peek() == '.') {
            value += advance(); // 读取小数点
            if (!isdigit(peek())) {
                return {TOKEN_ERROR, "非法浮点数格式: " + value + peek()};
            }
            while (isdigit(peek())) value += advance();
            return {TOKEN_FLOAT, value}; // 返回浮点数
        }
        return {TOKEN_NUM, value}; // 返回整常数
    }

    // 识别运算符或分隔符
    Token recognizeOpOrSep() {
        string value;
        value += advance(); // 先读取一个字符

        // 处理 ++ 和 --
        if ((value == "+" || value == "-") && peek() == value[0]) {
            value += advance();
            return {TOKEN_OP, value};
        }

        // 处理双字符运算符（如 >=, <=, ==, !=, &&, ||）
        if (operators.find(value + peek()) != operators.end()) {
        value += advance();
        }

        // 识别单字符运算符或分隔符
        if (operators.find(value) != operators.end()) {
            return {TOKEN_OP, value};
        }
        if (separators.find(value) != separators.end()) {
            return {TOKEN_SEP, value};
        }

        return {TOKEN_ERROR, "非法符号: " + value};
    }


public:
    Lexer(const string& src) : source(src) {}

    // 获取下一个单词符号
    Token getNextToken() {
        skipWhitespace();
        char ch = peek();
        if (isalpha(ch) || ch == '_') {
            return recognizeIdOrKeyword();
        } else if (isdigit(ch)) {
            return recognizeNumber();
        } else if (operators.find(string(1, ch)) != operators.end() || separators.find(string(1, ch)) != separators.end()) {
            return recognizeOpOrSep();
        } else if (ch == '\0') {
            return {TOKEN_ERROR, ""};
        } else {
            advance();
            return {TOKEN_ERROR, "非法字符: " + string(1, ch)};
        }
    }
};

// 驱动模块
int main() {
    // 读取源程序
    ifstream inFile("source.txt");
    if (!inFile) {
        cerr << "can't open source.txt" << endl;
        return 1;
    }
    string source((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
    inFile.close();

    // 词法分析
    Lexer lexer(source);
    vector<Token> tokens;
    ofstream outFile("lex_out.txt");
    if (!outFile) {
        cerr << "can't output lex_out.txt" << endl;
        return 1;
    }

    while (true) {
        Token token = lexer.getNextToken();
        if (token.type == TOKEN_ERROR && token.value.empty()) break;
        tokens.push_back(token);
        outFile << "(" << token.type << ", " << token.value << ")\n";
    }
    outFile.close();

    cout << "lex success lex_out.txt" << endl;
    return 0;
}