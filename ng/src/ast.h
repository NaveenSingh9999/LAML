#pragma once
#include <string>
#include <vector>
#include <memory>

struct Token {
    enum Type {
        ILLEGAL, EOF_T, NEWLINE,
        IDENT, INT, FLOAT, STRING,
        SAY, VAL, LET, FUNC, IF, ELSE,
        LOOP, WHILE, IN, TO, RETURN,
        CLOSC,
        ASSIGN, PLUS, MINUS, STAR, SLASH, PERCENT,
        EQ, NEQ, LT, GT, LTE, GTE,
        AND, OR, NOT,
        LPAREN, RPAREN, LBRACE, RBRACE,
        LBRACKET, RBRACKET,
        COMMA, DOT, COLON, SEMICOLON,
        COMMENT
    } type = ILLEGAL;
    std::string literal;
    int line = 0;
    int col = 0;
};

struct ASTNode {
    enum class Kind {
        Program, Ident, IntLit, FloatLit, StrLit, BoolLit,
        ValDecl, LetDecl, Assign,
        Say, If, While, Loop, RangeLoop,
        FuncDecl, Return,
        Block, ExprStmt,
        Prefix, Infix, Postfix,
        Call, Index, Dot,
        ArrayLit,
        CloscDecl
    } kind;
    std::string strVal;
    int64_t intVal = 0;
    double floatVal = 0.0;
    bool boolVal = false;
    std::vector<ASTNode> children;

    ASTNode() : kind(Kind::Program) {}
    static ASTNode make(Kind k) { ASTNode n; n.kind = k; return n; }
    static ASTNode ident(const std::string& s) { ASTNode n; n.kind = Kind::Ident; n.strVal = s; return n; }
    static ASTNode intLit(int64_t v) { ASTNode n; n.kind = Kind::IntLit; n.intVal = v; return n; }
    static ASTNode floatLit(double v) { ASTNode n; n.kind = Kind::FloatLit; n.floatVal = v; return n; }
    static ASTNode strLit(const std::string& s) { ASTNode n; n.kind = Kind::StrLit; n.strVal = s; return n; }
    static ASTNode boolLit(bool b) { ASTNode n; n.kind = Kind::BoolLit; n.boolVal = b; return n; }
};
