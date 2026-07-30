#include "parser.h"
#include <cstdlib>
#include <cerrno>

Parser::Parser(Lexer& lexer) : lexer(lexer) {
    next();
    next();
}

void Parser::next() {
    cur = peek;
    peek = lexer.nextToken();
    // Skip past NEWLINE tokens between statements (but not inside expressions)
    // Actually let's keep newlines - they're important for expression termination
}

bool Parser::curIs(Token::Type t) const { return cur.type == t; }
bool Parser::peekIs(Token::Type t) const { return peek.type == t; }

bool Parser::expectPeek(Token::Type t) {
    if (peekIs(t)) { next(); return true; }
    errors.push_back("expected " + std::to_string(t) + " but got " + peek.literal);
    return false;
}

ASTNode Parser::parseProgram() {
    ASTNode prog = ASTNode::make(ASTNode::Kind::Program);
    while (!curIs(Token::EOF_T)) {
        ASTNode stmt = parseStatement();
        if (stmt.kind != ASTNode::Kind::Program) {
            prog.children.push_back(std::move(stmt));
        }
        while (curIs(Token::NEWLINE)) next();
    }
    return prog;
}

ASTNode Parser::parseStatement() {
    switch (cur.type) {
    case Token::SAY: return parseSay();
    case Token::VAL: return parseValDecl();
    case Token::LET: return parseLetDecl();
    case Token::IF: return parseIf();
    case Token::WHILE: return parseWhile();
    case Token::LOOP: return parseLoop();
    case Token::RETURN: return parseReturn();
    case Token::FUNC: {
        next(); // skip 'func'
        ASTNode node = ASTNode::make(ASTNode::Kind::FuncDecl);
        if (curIs(Token::IDENT)) {
            node.strVal = cur.literal;
            next();
        }
        if (curIs(Token::LPAREN)) {
            next(); // skip (
            while (!curIs(Token::RPAREN) && !curIs(Token::EOF_T)) {
                if (curIs(Token::IDENT)) {
                    node.children.push_back(ASTNode::ident(cur.literal));
                }
                next();
                if (curIs(Token::COMMA)) next();
            }
            if (curIs(Token::RPAREN)) next();
        }
        if (curIs(Token::LBRACE)) {
            node.children.push_back(parseBlock());
        } else {
            node.children.push_back(parseExprStmt());
        }
        return node;
    }
    case Token::CLOSC: return parseClosc();
    case Token::LBRACE: return parseBlock();
    case Token::NEWLINE: next(); return ASTNode(); // skip
    case Token::SEMICOLON: next(); return ASTNode(); // skip
    default: return parseExprStmt();
    }
}

ASTNode Parser::parseValDecl() {
    ASTNode node = ASTNode::make(ASTNode::Kind::ValDecl);
    next();
    if (curIs(Token::IDENT)) {
        node.strVal = cur.literal;
        next();
    }
    if (curIs(Token::ASSIGN)) {
        next();
        node.children.push_back(parseExpression(0));
    }
    return node;
}

ASTNode Parser::parseLetDecl() {
    ASTNode node = ASTNode::make(ASTNode::Kind::LetDecl);
    next();
    if (curIs(Token::IDENT)) {
        node.strVal = cur.literal;
        next();
    }
    if (curIs(Token::ASSIGN)) {
        next();
        node.children.push_back(parseExpression(0));
    }
    return node;
}

ASTNode Parser::parseSay() {
    ASTNode node = ASTNode::make(ASTNode::Kind::Say);
    next();
    node.children.push_back(parseExpression(0));
    return node;
}

ASTNode Parser::parseIf() {
    ASTNode node = ASTNode::make(ASTNode::Kind::If);
    next(); // skip 'if'
    node.children.push_back(parseExpression(0)); // condition
    // consequence
    if (curIs(Token::LBRACE)) {
        node.children.push_back(parseBlock());
    } else {
        // Single statement
        node.children.push_back(parseExprStmt());
    }
    // optional else
    if (curIs(Token::ELSE)) {
        next();
        if (curIs(Token::LBRACE)) {
            node.children.push_back(parseBlock());
        } else {
            node.children.push_back(parseExprStmt());
        }
    }
    return node;
}

ASTNode Parser::parseWhile() {
    ASTNode node = ASTNode::make(ASTNode::Kind::While);
    next();
    node.children.push_back(parseExpression(0));
    if (curIs(Token::LBRACE)) {
        node.children.push_back(parseBlock());
    } else {
        node.children.push_back(parseExprStmt());
    }
    return node;
}

ASTNode Parser::parseLoop() {
    next(); // skip 'loop'
    // loop x in start to end { body }
    if (curIs(Token::IDENT) && peekIs(Token::IN)) {
        ASTNode node = ASTNode::make(ASTNode::Kind::RangeLoop);
        node.strVal = cur.literal;
        next(); next(); // skip identifier and 'in'
        node.children.push_back(parseExpression(0)); // start
        if (curIs(Token::TO)) {
            next();
            node.children.push_back(parseExpression(0)); // end
        }
        if (curIs(Token::LBRACE)) {
            node.children.push_back(parseBlock());
        } else {
            node.children.push_back(parseExprStmt());
        }
        return node;
    }
    // regular loop with expression
    ASTNode n = ASTNode::make(ASTNode::Kind::Loop);
    n.children.push_back(parseExpression(0));
    if (curIs(Token::LBRACE)) {
        n.children.push_back(parseBlock());
    } else {
        n.children.push_back(parseExprStmt());
    }
    return n;
}

ASTNode Parser::parseReturn() {
    ASTNode node = ASTNode::make(ASTNode::Kind::Return);
    next();
    if (!curIs(Token::NEWLINE) && !curIs(Token::RBRACE) && !curIs(Token::EOF_T)) {
        node.children.push_back(parseExpression(0));
    }
    return node;
}

ASTNode Parser::parseBlock() {
    ASTNode node = ASTNode::make(ASTNode::Kind::Block);
    next(); // skip '{'
    while (!curIs(Token::RBRACE) && !curIs(Token::EOF_T)) {
        ASTNode stmt = parseStatement();
        if (stmt.kind != ASTNode::Kind::Program) {
            node.children.push_back(std::move(stmt));
        }
        while (curIs(Token::NEWLINE)) next();
    }
    if (curIs(Token::RBRACE)) next();
    return node;
}

ASTNode Parser::parseExprStmt() {
    ASTNode node = parseExpression(0);
    ASTNode wrapper = ASTNode::make(ASTNode::Kind::ExprStmt);
    wrapper.children.push_back(std::move(node));
    return wrapper;
}

ASTNode Parser::parseClosc() {
    ASTNode node = ASTNode::make(ASTNode::Kind::CloscDecl);
    next(); // skip 'closc'
    // optional priority name
    if (curIs(Token::IDENT)) {
        node.strVal = cur.literal; // name
        next();
    }
    if (curIs(Token::LBRACE)) {
        node.children.push_back(parseBlock());
    } else {
        node.children.push_back(parseExprStmt());
    }
    return node;
}

// Pratt parser
int precedence(Token::Type t) {
    switch (t) {
    case Token::DOT:
    case Token::LBRACKET:
    case Token::LPAREN: return 8;
    case Token::STAR: case Token::SLASH: case Token::PERCENT: return 5;
    case Token::PLUS: return 4;
    case Token::MINUS: return 4;
    case Token::EQ: case Token::NEQ: case Token::LT: case Token::GT:
    case Token::LTE: case Token::GTE: return 3;
    case Token::AND: case Token::OR: return 2;
    case Token::ASSIGN: return 1;
    default: return 0;
    }
}

int Parser::getPrec(Token::Type t) const { return precedence(t); }

ASTNode Parser::parseExpression(int prec) {
    ASTNode left = parsePrefix();
    if (left.kind == ASTNode::Kind::Program) return left;

    while (prec < getPrec(cur.type)) {
        switch (cur.type) {
        case Token::PLUS: case Token::MINUS:
        case Token::STAR: case Token::SLASH: case Token::PERCENT:
        case Token::EQ: case Token::NEQ: case Token::LT:
        case Token::GT: case Token::LTE: case Token::GTE:
        case Token::AND: case Token::OR:
        case Token::ASSIGN: {
            std::string op = cur.literal;
            int opPrec = getPrec(cur.type);
            next();
            left = parseInfix(std::move(left), op, opPrec);
            break;
        }
        case Token::LPAREN:
            next();
            left = parseCall(std::move(left));
            break;
        case Token::LBRACKET:
            next();
            left = parseIndex(std::move(left));
            break;
        case Token::DOT:
            next();
            left = parseDot(std::move(left));
            break;
        default:
            return left;
        }
    }
    return left;
}

ASTNode Parser::parsePrefix() {
    switch (cur.type) {
    case Token::IDENT: return parseIdent();
    case Token::INT: return parseIntLit();
    case Token::FLOAT: return parseFloatLit();
    case Token::STRING: return parseStrLit();
    case Token::MINUS:
    case Token::NOT: {
        ASTNode node = ASTNode::make(ASTNode::Kind::Prefix);
        node.strVal = cur.literal;
        next();
        node.children.push_back(parseExpression(precedence(Token::NOT)));
        return node;
    }
    case Token::LPAREN: return parseGrouped();
    case Token::LBRACKET: return parseArray();
    case Token::FUNC:
        next();
        return parseFuncDecl();
    default:
        return ASTNode(); // nil node
    }
}

ASTNode Parser::parseInfix(ASTNode left, const std::string& op, int opPrec) {
    ASTNode node = ASTNode::make(ASTNode::Kind::Infix);
    node.strVal = op;
    node.children.push_back(std::move(left));
    node.children.push_back(parseExpression(opPrec));
    return node;
}

ASTNode Parser::parseCall(ASTNode callee) {
    ASTNode node = ASTNode::make(ASTNode::Kind::Call);
    node.children.push_back(std::move(callee));
    // '(' already consumed by parseExpression
    while (!curIs(Token::RPAREN) && !curIs(Token::EOF_T)) {
        node.children.push_back(parseExpression(0));
        if (curIs(Token::COMMA)) next();
    }
    if (curIs(Token::RPAREN)) next();
    return node;
}

ASTNode Parser::parseIndex(ASTNode left) {
    ASTNode node = ASTNode::make(ASTNode::Kind::Index);
    node.children.push_back(std::move(left));
    // '[' already consumed by parseExpression
    node.children.push_back(parseExpression(0));
    if (curIs(Token::RBRACKET)) next();
    return node;
}

ASTNode Parser::parseDot(ASTNode left) {
    ASTNode node = ASTNode::make(ASTNode::Kind::Dot);
    node.children.push_back(std::move(left));
    if (curIs(Token::IDENT)) {
        ASTNode prop = ASTNode::ident(cur.literal);
        node.children.push_back(std::move(prop));
        next();
    }
    return node;
}

ASTNode Parser::parseIdent() {
    ASTNode n = ASTNode::ident(cur.literal);
    next();
    return n;
}

ASTNode Parser::parseIntLit() {
    char* end;
    errno = 0;
    int64_t val = strtoll(cur.literal.c_str(), &end, 10);
    ASTNode n = ASTNode::intLit(val);
    next();
    return n;
}

ASTNode Parser::parseFloatLit() {
    double val = strtod(cur.literal.c_str(), nullptr);
    ASTNode n = ASTNode::floatLit(val);
    next();
    return n;
}

ASTNode Parser::parseStrLit() {
    std::string s = cur.literal;
    if (s == "true") {
        ASTNode n = ASTNode::boolLit(true);
        next();
        return n;
    }
    if (s == "false") {
        ASTNode n = ASTNode::boolLit(false);
        next();
        return n;
    }
    ASTNode n = ASTNode::strLit(s);
    next();
    return n;
}

ASTNode Parser::parseBoolLit() {
    ASTNode n = ASTNode::boolLit(cur.literal == "true");
    next();
    return n;
}

ASTNode Parser::parseGrouped() {
    next(); // skip '('
    ASTNode expr = parseExpression(0);
    if (curIs(Token::RPAREN)) next();
    return expr;
}

ASTNode Parser::parseArray() {
    ASTNode node = ASTNode::make(ASTNode::Kind::ArrayLit);
    next();
    while (!curIs(Token::RBRACKET) && !curIs(Token::EOF_T)) {
        node.children.push_back(parseExpression(0));
        if (curIs(Token::COMMA)) next();
    }
    if (curIs(Token::RBRACKET)) next();
    return node;
}

ASTNode Parser::parseFuncDecl() {
    ASTNode node = ASTNode::make(ASTNode::Kind::FuncDecl);
    if (curIs(Token::IDENT)) {
        node.strVal = cur.literal;
        next();
    }
    if (curIs(Token::LPAREN)) {
        next();
        while (!curIs(Token::RPAREN) && !curIs(Token::EOF_T)) {
            if (curIs(Token::IDENT)) {
                node.children.push_back(ASTNode::ident(cur.literal));
            }
            next();
            if (curIs(Token::COMMA)) next();
        }
        if (curIs(Token::RPAREN)) next();
    }
    // body
    if (curIs(Token::LBRACE)) {
        node.children.push_back(parseBlock());
    } else {
        node.children.push_back(parseExprStmt());
    }
    return node;
}
