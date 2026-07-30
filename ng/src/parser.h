#pragma once
#include <vector>
#include <memory>
#include "ast.h"
#include "lexer.h"

class Parser {
public:
    explicit Parser(Lexer& lexer);

    ASTNode parseProgram();
    std::vector<std::string> getErrors() const { return errors; }

private:
    Lexer& lexer;
    Token cur;
    Token peek;
    std::vector<std::string> errors;

    void next();
    bool curIs(Token::Type t) const;
    bool peekIs(Token::Type t) const;
    bool expectPeek(Token::Type t);

    ASTNode parseStatement();
    ASTNode parseValDecl();
    ASTNode parseLetDecl();
    ASTNode parseSay();
    ASTNode parseIf();
    ASTNode parseWhile();
    ASTNode parseLoop();
    ASTNode parseReturn();
    ASTNode parseBlock();
    ASTNode parseExprStmt();
    ASTNode parseClosc();

    ASTNode parseExpression(int prec);
    ASTNode parsePrefix();
    ASTNode parseInfix(ASTNode left, const std::string& op, int opPrec);
    ASTNode parseCall(ASTNode callee);
    ASTNode parseIndex(ASTNode left);
    ASTNode parseDot(ASTNode left);

    ASTNode parseIdent();
    ASTNode parseIntLit();
    ASTNode parseFloatLit();
    ASTNode parseStrLit();
    ASTNode parseBoolLit();
    ASTNode parseGrouped();
    ASTNode parseArray();
    ASTNode parseFuncDecl();

    int getPrec(Token::Type t) const;
};

int precedence(Token::Type t);
