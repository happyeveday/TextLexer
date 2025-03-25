#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cctype>
using namespace std;

// 单词符号类型编码
enum TokenType
{
    TOKEN_ID,      // 标识符
    TOKEN_NUM,     // 整常数
    TOKEN_FLOAT,   // 浮点数
    TOKEN_BOOL,    // 布尔常量
    TOKEN_KEYWORD, // 关键字
    TOKEN_OP,      // 运算符
    TOKEN_SEP,     // 分隔符
    TOKEN_BIT_OP,  // 位运算符
    TOKEN_ERROR    // 错误
};

// 符号表：关键字
static const unordered_map<string, TokenType> keywords = {
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
    {"false", TOKEN_BOOL}};

// 符号表：运算符
static const unordered_map<string, TokenType> operators = {
    {"+", TOKEN_OP},
    {"-", TOKEN_OP},
    {"*", TOKEN_OP},
    {"/", TOKEN_OP},
    {"=", TOKEN_OP},
    {"&", TOKEN_OP},
    {"|", TOKEN_OP},
    {"+=", TOKEN_OP},
    {"-=", TOKEN_OP},
    {"*=", TOKEN_OP},
    {"/=", TOKEN_OP},
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
    {"--", TOKEN_OP}};

// 符号表：分隔符
static const unordered_map<string, TokenType> separators = {
    {";", TOKEN_SEP},
    {",", TOKEN_SEP},
    {"(", TOKEN_SEP},
    {")", TOKEN_SEP},
    {"{", TOKEN_SEP},
    {"}", TOKEN_SEP},
    {"[]", TOKEN_SEP},
    {"]", TOKEN_SEP},
    {":", TOKEN_SEP}};

// 符号表：位运算符
static const unordered_map<string, TokenType> bit_operators = {
    {"&", TOKEN_BIT_OP},
    {"|", TOKEN_BIT_OP},
    {"~", TOKEN_BIT_OP},
    {"^", TOKEN_BIT_OP},
    {"<<", TOKEN_BIT_OP},
    {">>", TOKEN_BIT_OP}};

// 单词符号的二元组
struct Token
{
    TokenType type;
    string value;
    int line;
    int column;
};

// 词法分析器
class Lexer
{
private:
    string source;  // 源程序
    size_t pos = 0; // 当前扫描位置
    int line = 1;   // 当前行号
    int column = 1; // 当前列号

    // 读取下一个字符
    char peek()
    {
        return (pos < source.length()) ? source[pos] : '\0';
    }

    // 读取并移动指针
    char advance()
    {
        char c = (pos < source.length()) ? source[pos++] : '\0';
        if (c == '\n')
        {
            line++;
            column = 1;
        }
        else
        {
            column++;
        }
        return c;
    }

    // 跳过空白字符
    void skipWhitespace()
    {
        while (true)
        {
            if (peek() == '#' || (peek() == '/' && source[pos + 1] == '/'))
            {
                while (peek() != '\n' && peek() != '\0')
                    advance();
            }
            else if (peek() == '/' && source[pos + 1] == '*')
            {
                advance();
                advance(); // 跳过/*
                while (!(peek() == '*' && source[pos + 1] == '/'))
                {
                    if (peek() == '\0')
                        return;
                    advance();
                }
                advance();
                advance(); // 跳过*/
            }
            else if (isspace(peek()))
            {
                advance();
            }
            else
                break;
        }
    }

    // 输出错误信息
    Token makeToken(TokenType type, const string &value)
    {
        return {type, value, line, column - value.length()};
    }

    Token makeError(const string &msg)
    {
        return {TOKEN_ERROR, msg + " at line " + to_string(line) + ":" + to_string(column), line, column};
    }

    // 识别标识符或关键字
    Token recognizeIdOrKeyword()
    {
        string value;
        int start_col = column;

        if (isdigit(peek()))
        {
            while (isdigit(peek()) || isalpha(peek()))
                value += advance();
            return makeError("非法标识符（不能以数字开头）: " + value);
        }

        while (isalnum(peek()) || peek() == '_')
            value += advance();

        if (keywords.count(value))
        {
            return makeToken(keywords.at(value), value);
        }
        return makeToken(TOKEN_ID, value);
    }

    // 识别整常数或浮点数
    Token recognizeNumber()
    {
        string value;
        bool hasDecimalPoint = false; // 是否包含小数点
        bool isError = false;         // 是否非法浮点数
        int start_col = column;

        // 读取整数部分
        while (isdigit(peek()))
            value += advance();

        // 读取小数点和小数部分
        if (peek() == '.')
        {
            value += advance(); // 读取小数点
            hasDecimalPoint = true;

            // 读取小数部分
            if (!isdigit(peek()))
            {
                isError = true; // 小数点后没有数字，非法浮点数
            }
            else
            {
                while (isdigit(peek()))
                    value += advance();
            }

            // 检查是否有多余的小数点
            if (peek() == '.')
            {
                isError = true;     // 多个小数点，非法浮点数
                value += advance(); // 读取多余的小数点
                while (isdigit(peek()))
                    value += advance(); // 继续读取后续数字
            }
        }

        // 检查是否以字母或其他非法字符结尾
        if (isalpha(peek()) || peek() == '_')
        {
            isError = true; // 数字后接字母或下划线，非法标识符
            while (isalnum(peek()) || peek() == '_')
                value += advance(); // 继续读取后续字符
        }

        // 返回结果
        if (isError)
        {
            return makeError("非法数字格式: " + value);
        }
        return makeToken(hasDecimalPoint ? TOKEN_FLOAT : TOKEN_NUM, value);
    }

    // 识别运算符或分隔符
    Token recognizeOpOrSep()
    {
        string value;
        int start_col = column;
        value += advance();

        // 检查双字符运算符（包括位运算符）
        string double_op = value + peek();
        if (operators.count(double_op) || bit_operators.count(double_op))
        {
            value += advance();
            return makeToken(operators.count(double_op) ? operators.at(double_op) : bit_operators.at(double_op), value);
        }

        // 检查单字符运算符或位运算符
        if (operators.count(value))
            return makeToken(TOKEN_OP, value);
        if (bit_operators.count(value))
            return makeToken(TOKEN_BIT_OP, value);
        if (separators.count(value))
            return makeToken(TOKEN_SEP, value);

        return makeError("无法识别的符号: " + value);
    }

public:
    Lexer(const string &src) : source(src) {}

    // 获取下一个单词符号
    Token getNextToken()
    {
        skipWhitespace();
        char ch = peek();
        int start_line = line, start_col = column;
        if (isalpha(ch) || ch == '_')
        {
            return recognizeIdOrKeyword();
        }
        else if (isdigit(ch))
        {
            return recognizeNumber();
        }
        else if (operators.find(string(1, ch)) != operators.end() || separators.find(string(1, ch)) != separators.end())
        {
            return recognizeOpOrSep();
        }
        else if (ch == '\0')
        {
            return {TOKEN_ERROR, ""};
        }
        else
        {
            advance();
            return {TOKEN_ERROR, "非法字符: " + string(1, ch)};
        }
    }
};

// 驱动模块
int main()
{
    ifstream inFile("source.txt");
    if (!inFile)
    {
        cerr << "can't open source.txt" << endl;
        return 1;
    }

    string source((istreambuf_iterator<char>(inFile)), {});
    inFile.close();

    Lexer lexer(source);
    ofstream outFile("lex_out.txt");
    if (!outFile)
    {
        cerr << "can't build lex_out.txt" << endl;
        return 1;
    }

    while (true)
    {
        Token token = lexer.getNextToken();
        if (token.type == TOKEN_ERROR && token.value.empty())
            break;

        // 输出格式：(类型, 值, 行, 列)
        outFile << "(" << token.type << ", \"" << token.value << "\", "
                << token.line << ", " << token.column << ")\n";
    }

    outFile.close();
    cout << "lex success，result in lex_out.txt" << endl;
    return 0;
}