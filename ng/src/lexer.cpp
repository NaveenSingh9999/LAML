#include "lexer.h"
#include <cctype>
#include <unordered_set>

Lexer::Lexer(std::string input) : input(std::move(input)) {
    readChar();
}

void Lexer::readChar() {
    if (readPos >= input.size()) {
        ch = 0;
    } else {
        ch = input[readPos];
    }
    pos = readPos;
    readPos++;
    if (ch == '\n') {
        line++;
        col = 0;
    } else {
        col++;
    }
}

char Lexer::peek() const {
    if (readPos >= input.size()) return 0;
    return input[readPos];
}

void Lexer::skipWhitespace(bool skipNewlines) {
    while (true) {
        if (ch == ' ' || ch == '\t' || ch == '\r') {
            readChar();
        } else if (ch == '\n' && skipNewlines) {
            readChar();
        } else {
            break;
        }
    }
}

std::string Lexer::readIdent() {
    size_t start = pos;
    while (std::isalnum(ch) || ch == '_') readChar();
    return input.substr(start, pos - start);
}

std::string Lexer::readNumber(bool& isFloat) {
    size_t start = pos;
    isFloat = false;
    while (std::isdigit(ch)) readChar();
    if (ch == '.' && std::isdigit(peek())) {
        isFloat = true;
        readChar();
        while (std::isdigit(ch)) readChar();
    }
    return input.substr(start, pos - start);
}

std::string Lexer::readString() {
    readChar();
    std::string result;
    while (ch != '"' && ch != 0 && ch != '\n') {
        if (ch == '\\') {
            readChar();
            switch (ch) {
            case 'n': result += '\n'; break;
            case 'r': result += '\r'; break;
            case 't': result += '\t'; break;
            case '\\': result += '\\'; break;
            case '"': result += '"'; break;
            default: result += ch; break;
            }
        } else {
            result += ch;
        }
        readChar();
    }
    if (ch == '"') readChar();
    return result;
}

void Lexer::skipComment() {
    while (ch != '\n' && ch != 0) readChar();
}

Token Lexer::makeToken(Token::Type t, const std::string& lit) {
    Token tok;
    tok.type = t;
    tok.literal = lit;
    tok.line = line;
    tok.col = col;
    return tok;
}

Token Lexer::makeNewline() {
    int nl = line;
    while (ch == '\n') readChar();
    skipWhitespace(true);
    Token tok;
    tok.type = Token::NEWLINE;
    tok.literal = "\n";
    tok.line = nl;
    tok.col = 0;
    return tok;
}

Token Lexer::nextToken() {
    skipWhitespace(false);

    bool inBrackets = (parenDepth > 0 || braceDepth > 0 || bracketDepth > 0);

    if (ch == '\n') {
        if (inBrackets) {
            readChar();
            return nextToken();
        }
        if (!atLineStart) {
            atLineStart = true;
            return makeNewline();
        }
        readChar();
        return nextToken();
    }

    atLineStart = false;

    Token tok;
    tok.line = line;
    tok.col = col;

    switch (ch) {
    case 0:
        tok.type = Token::EOF_T; tok.literal = "";
        return tok;
    case '=':
        if (peek() == '=') { readChar(); tok.type = Token::EQ; tok.literal = "=="; }
        else { tok.type = Token::ASSIGN; tok.literal = "="; }
        readChar(); return tok;
    case '+':
        if (peek() == '+') { readChar(); tok.type = Token::PLUS; tok.literal = "++"; }
        else { tok.type = Token::PLUS; tok.literal = "+"; }
        readChar(); return tok;
    case '-':
        if (peek() == '-') { readChar(); tok.type = Token::MINUS; tok.literal = "--"; }
        else { tok.type = Token::MINUS; tok.literal = "-"; }
        readChar(); return tok;
    case '*': tok.type = Token::STAR; tok.literal = "*"; readChar(); return tok;
    case '/': tok.type = Token::SLASH; tok.literal = "/"; readChar(); return tok;
    case '%': tok.type = Token::PERCENT; tok.literal = "%"; readChar(); return tok;
    case '!':
        if (peek() == '=') { readChar(); tok.type = Token::NEQ; tok.literal = "!="; }
        else { tok.type = Token::NOT; tok.literal = "!"; }
        readChar(); return tok;
    case '<':
        if (peek() == '=') { readChar(); tok.type = Token::LTE; tok.literal = "<="; }
        else { tok.type = Token::LT; tok.literal = "<"; }
        readChar(); return tok;
    case '>':
        if (peek() == '=') { readChar(); tok.type = Token::GTE; tok.literal = ">="; }
        else { tok.type = Token::GT; tok.literal = ">"; }
        readChar(); return tok;
    case '&':
        if (peek() == '&') { readChar(); tok.type = Token::AND; tok.literal = "&&"; readChar(); }
        else { tok.type = Token::ILLEGAL; tok.literal = "&"; readChar(); }
        return tok;
    case '|':
        if (peek() == '|') { readChar(); tok.type = Token::OR; tok.literal = "||"; readChar(); }
        else { tok.type = Token::ILLEGAL; tok.literal = "|"; readChar(); }
        return tok;
    case '(': parenDepth++; tok.type = Token::LPAREN; tok.literal = "("; readChar(); return tok;
    case ')': parenDepth = std::max(0, parenDepth - 1); tok.type = Token::RPAREN; tok.literal = ")"; readChar(); return tok;
    case '{': braceDepth++; tok.type = Token::LBRACE; tok.literal = "{"; readChar(); return tok;
    case '}': braceDepth = std::max(0, braceDepth - 1); tok.type = Token::RBRACE; tok.literal = "}"; readChar(); return tok;
    case '[': bracketDepth++; tok.type = Token::LBRACKET; tok.literal = "["; readChar(); return tok;
    case ']': bracketDepth = std::max(0, bracketDepth - 1); tok.type = Token::RBRACKET; tok.literal = "]"; readChar(); return tok;
    case ',': tok.type = Token::COMMA; tok.literal = ","; readChar(); return tok;
    case '.': tok.type = Token::DOT; tok.literal = "."; readChar(); return tok;
    case ':': tok.type = Token::COLON; tok.literal = ":"; readChar(); return tok;
    case ';': tok.type = Token::SEMICOLON; tok.literal = ";"; readChar(); return tok;
    case '"':
        tok.type = Token::STRING;
        tok.literal = readString();
        return tok;
    case '~':
        skipComment();
        return nextToken();
    case '#':
        skipComment();
        return nextToken();
    default:
        if (std::isalpha(ch) || ch == '_') {
            std::string ident = readIdent();
            static const std::unordered_map<std::string, Token::Type> keywords = {
                {"say", Token::SAY}, {"val", Token::VAL}, {"let", Token::LET},
                {"func", Token::FUNC}, {"if", Token::IF}, {"else", Token::ELSE},
                {"loop", Token::LOOP}, {"while", Token::WHILE}, {"in", Token::IN},
                {"to", Token::TO}, {"return", Token::RETURN}, {"closc", Token::CLOSC},
                {"for", Token::FOR}, {"break", Token::BREAK}, {"continue", Token::CONTINUE},
                {"true", Token::BOOL}, {"false", Token::BOOL}
            };
            auto it = keywords.find(ident);
            if (it != keywords.end()) {
                tok.type = it->second;
                tok.literal = ident;
                return tok;
            }
            tok.type = Token::IDENT;
            tok.literal = ident;
            return tok;
        }
        if (std::isdigit(ch)) {
            bool isFloat = false;
            std::string num = readNumber(isFloat);
            tok.type = isFloat ? Token::FLOAT : Token::INT;
            tok.literal = num;
            return tok;
        }
        tok.type = Token::ILLEGAL;
        tok.literal = std::string(1, ch);
        readChar();
        return tok;
    }
}
