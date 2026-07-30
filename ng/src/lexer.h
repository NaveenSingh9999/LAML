#pragma once
#include <string>
#include <vector>
#include <optional>
#include "ast.h"

class Lexer {
public:
    explicit Lexer(std::string input);

    Token nextToken();
    std::vector<Token> tokenize();

private:
    std::string input;
    size_t pos = 0;
    size_t readPos = 0;
    char ch = 0;
    int line = 1;
    int col = 0;
    bool atLineStart = true;
    int parenDepth = 0;
    int braceDepth = 0;
    int bracketDepth = 0;

    void readChar();
    char peek() const;
    void skipWhitespace(bool skipNewlines);
    std::string readIdent();
    std::string readNumber(bool& isFloat);
    std::string readString();
    void skipComment();
    Token makeToken(Token::Type t, const std::string& lit);
    Token makeNewline();
};
