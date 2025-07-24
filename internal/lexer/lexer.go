package lexer

import (
	"fmt"
)

// TokenType represents the type of a token
type TokenType int

const (
	// Special tokens
	ILLEGAL TokenType = iota
	EOF

	// Identifiers and literals
	IDENT  // variable names, function names
	INT    // integers
	FLOAT  // floating point numbers
	STRING // string literals
	BOOL   // true, false

	// Keywords
	BRING  // bring (import)
	VAL    // val (mutable variable)
	LET    // let (backward compatibility)
	CONST  // const
	FUNC   // func
	TYPE   // type (OOP-lite)
	OBJ    // obj (backward compatibility)
	IF     // if
	ELSE   // else
	LOOP   // loop
	WHILE  // while
	IN     // in
	TO     // to
	RETURN // return
	SAY    // say (print)
	TRUE   // true
	FALSE  // false
	MAIN   // main
	PTR    // ptr (raw pointer ops)

	// Operators
	ASSIGN   // =
	PLUS     // +
	MINUS    // -
	MULTIPLY // *
	DIVIDE   // /
	MODULO   // %

	// Comparison operators
	EQ     // ==
	NOT_EQ // !=
	LT     // <
	GT     // >
	LTE    // <=
	GTE    // >=

	// Logical operators
	AND // &&
	OR  // ||
	NOT // !

	// Delimiters
	COMMA     // ,
	SEMICOLON // ;
	DOT       // .
	COLON     // : (for type annotations)

	// Increment/Decrement
	INCREMENT // ++
	DECREMENT // --

	// Brackets
	LPAREN   // (
	RPAREN   // )
	LBRACE   // {
	RBRACE   // }
	LBRACKET // [
	RBRACKET // ]

	// Comments
	COMMENT       // // comment (legacy)
	SINGLECOMMENT // ~ comment
	MULTICOMMENT  // {~ ... ~}
)

// Token represents a single token
type Token struct {
	Type    TokenType
	Literal string
	Line    int
	Column  int
}

// String returns a string representation of the token type
func (t TokenType) String() string {
	switch t {
	case ILLEGAL:
		return "ILLEGAL"
	case EOF:
		return "EOF"
	case IDENT:
		return "IDENT"
	case INT:
		return "INT"
	case FLOAT:
		return "FLOAT"
	case STRING:
		return "STRING"
	case BOOL:
		return "BOOL"
	case BRING:
		return "BRING"
	case VAL:
		return "VAL"
	case LET:
		return "LET"
	case CONST:
		return "CONST"
	case FUNC:
		return "FUNC"
	case TYPE:
		return "TYPE"
	case OBJ:
		return "OBJ"
	case IF:
		return "IF"
	case ELSE:
		return "ELSE"
	case LOOP:
		return "LOOP"
	case WHILE:
		return "WHILE"
	case IN:
		return "IN"
	case TO:
		return "TO"
	case RETURN:
		return "RETURN"
	case SAY:
		return "SAY"
	case TRUE:
		return "TRUE"
	case FALSE:
		return "FALSE"
	case MAIN:
		return "MAIN"
	case ASSIGN:
		return "ASSIGN"
	case PLUS:
		return "PLUS"
	case MINUS:
		return "MINUS"
	case MULTIPLY:
		return "MULTIPLY"
	case DIVIDE:
		return "DIVIDE"
	case MODULO:
		return "MODULO"
	case EQ:
		return "EQ"
	case NOT_EQ:
		return "NOT_EQ"
	case LT:
		return "LT"
	case GT:
		return "GT"
	case LTE:
		return "LTE"
	case GTE:
		return "GTE"
	case AND:
		return "AND"
	case OR:
		return "OR"
	case NOT:
		return "NOT"
	case COMMA:
		return "COMMA"
	case SEMICOLON:
		return "SEMICOLON"
	case DOT:
		return "DOT"
	case COLON:
		return "COLON"
	case INCREMENT:
		return "INCREMENT"
	case DECREMENT:
		return "DECREMENT"
	case LPAREN:
		return "LPAREN"
	case RPAREN:
		return "RPAREN"
	case LBRACE:
		return "LBRACE"
	case RBRACE:
		return "RBRACE"
	case LBRACKET:
		return "LBRACKET"
	case RBRACKET:
		return "RBRACKET"
	case COMMENT:
		return "COMMENT"
	case SINGLECOMMENT:
		return "SINGLECOMMENT"
	case MULTICOMMENT:
		return "MULTICOMMENT"
	case PTR:
		return "PTR"
	default:
		return "UNKNOWN"
	}
}

// Keywords map
var keywords = map[string]TokenType{
	"bring":  BRING,
	"val":    VAL,
	"let":    LET,
	"const":  CONST,
	"func":   FUNC,
	"type":   TYPE,
	"obj":    OBJ,
	"if":     IF,
	"else":   ELSE,
	"loop":   LOOP,
	"while":  WHILE,
	"in":     IN,
	"to":     TO,
	"return": RETURN,
	"say":    SAY,
	"true":   TRUE,
	"false":  FALSE,
	"ptr":    PTR,
}

// LookupIdent checks if an identifier is a keyword
func LookupIdent(ident string) TokenType {
	if tok, ok := keywords[ident]; ok {
		return tok
	}
	return IDENT
}

// Lexer represents the lexical analyzer
type Lexer struct {
	input        string
	position     int  // current position in input (points to current char)
	readPosition int  // current reading position in input (after current char)
	ch           byte // current char under examination
	line         int  // current line number
	column       int  // current column number
}

// New creates a new lexer instance
func New(input string) *Lexer {
	l := &Lexer{
		input:  input,
		line:   1,
		column: 0,
	}
	l.readChar()
	return l
}

// readChar gives us the next character and advances our position in the input string
func (l *Lexer) readChar() {
	if l.readPosition >= len(l.input) {
		l.ch = 0 // ASCII NUL character represents "nothing"
	} else {
		l.ch = l.input[l.readPosition]
	}
	l.position = l.readPosition
	l.readPosition++

	if l.ch == '\n' {
		l.line++
		l.column = 0
	} else {
		l.column++
	}
}

// peekChar returns the next character without advancing our position
func (l *Lexer) peekChar() byte {
	if l.readPosition >= len(l.input) {
		return 0
	}
	return l.input[l.readPosition]
}

// skipWhitespace skips whitespace characters
func (l *Lexer) skipWhitespace() {
	for l.ch == ' ' || l.ch == '\t' || l.ch == '\n' || l.ch == '\r' {
		l.readChar()
	}
}

// readIdentifier reads an identifier or keyword
func (l *Lexer) readIdentifier() string {
	position := l.position
	for isLetter(l.ch) || isDigit(l.ch) {
		l.readChar()
	}
	return l.input[position:l.position]
}

// readNumber reads a number (integer or float)
func (l *Lexer) readNumber() (string, TokenType) {
	position := l.position
	tokenType := INT

	for isDigit(l.ch) {
		l.readChar()
	}

	// Check for decimal point
	if l.ch == '.' && isDigit(l.peekChar()) {
		tokenType = FLOAT
		l.readChar() // consume '.'
		for isDigit(l.ch) {
			l.readChar()
		}
	}

	return l.input[position:l.position], tokenType
}

// readString reads a string literal
func (l *Lexer) readString() string {
	position := l.position + 1 // skip opening quote
	for {
		l.readChar()
		if l.ch == '"' || l.ch == 0 {
			break
		}
	}
	return l.input[position:l.position]
}

// readComment reads a single-line comment
func (l *Lexer) readComment() string {
	position := l.position
	for l.ch != '\n' && l.ch != 0 {
		l.readChar()
	}
	return l.input[position:l.position]
}

// readSingleComment reads a single-line comment starting with ~
func (l *Lexer) readSingleComment() string {
	position := l.position
	for l.ch != '\n' && l.ch != 0 {
		l.readChar()
	}
	return l.input[position:l.position]
}

// readMultiComment reads a multi-line comment {~ ... ~}
func (l *Lexer) readMultiComment() string {
	position := l.position
	l.readChar() // skip first '{'
	l.readChar() // skip '~'

	for l.ch != 0 {
		if l.ch == '~' && l.peekChar() == '}' {
			l.readChar() // consume '~'
			l.readChar() // consume '}'
			break
		}
		if l.ch == '\n' {
			l.line++
			l.column = 0
		}
		l.readChar()
	}
	return l.input[position:l.position]
} // NextToken returns the next token from the input
func (l *Lexer) NextToken() Token {
	var tok Token

	l.skipWhitespace()

	tok.Line = l.line
	tok.Column = l.column

	switch l.ch {
	case '=':
		if l.peekChar() == '=' {
			ch := l.ch
			l.readChar()
			tok = Token{Type: EQ, Literal: string(ch) + string(l.ch), Line: l.line, Column: l.column}
		} else {
			tok = newToken(ASSIGN, l.ch, l.line, l.column)
		}
	case '+':
		if l.peekChar() == '+' {
			ch := l.ch
			l.readChar()
			tok = Token{Type: INCREMENT, Literal: string(ch) + string(l.ch), Line: l.line, Column: l.column}
		} else {
			tok = newToken(PLUS, l.ch, l.line, l.column)
		}
	case '-':
		if l.peekChar() == '-' {
			ch := l.ch
			l.readChar()
			tok = Token{Type: DECREMENT, Literal: string(ch) + string(l.ch), Line: l.line, Column: l.column}
		} else {
			tok = newToken(MINUS, l.ch, l.line, l.column)
		}
	case '*':
		tok = newToken(MULTIPLY, l.ch, l.line, l.column)
	case '/':
		if l.peekChar() == '/' {
			tok.Type = COMMENT
			tok.Literal = l.readComment()
			return tok
		} else {
			tok = newToken(DIVIDE, l.ch, l.line, l.column)
		}
	case '%':
		tok = newToken(MODULO, l.ch, l.line, l.column)
	case '!':
		if l.peekChar() == '=' {
			ch := l.ch
			l.readChar()
			tok = Token{Type: NOT_EQ, Literal: string(ch) + string(l.ch), Line: l.line, Column: l.column}
		} else {
			tok = newToken(NOT, l.ch, l.line, l.column)
		}
	case '<':
		if l.peekChar() == '=' {
			ch := l.ch
			l.readChar()
			tok = Token{Type: LTE, Literal: string(ch) + string(l.ch), Line: l.line, Column: l.column}
		} else {
			tok = newToken(LT, l.ch, l.line, l.column)
		}
	case '>':
		if l.peekChar() == '=' {
			ch := l.ch
			l.readChar()
			tok = Token{Type: GTE, Literal: string(ch) + string(l.ch), Line: l.line, Column: l.column}
		} else {
			tok = newToken(GT, l.ch, l.line, l.column)
		}
	case '&':
		if l.peekChar() == '&' {
			ch := l.ch
			l.readChar()
			tok = Token{Type: AND, Literal: string(ch) + string(l.ch), Line: l.line, Column: l.column}
		} else {
			tok = newToken(ILLEGAL, l.ch, l.line, l.column)
		}
	case '|':
		if l.peekChar() == '|' {
			ch := l.ch
			l.readChar()
			tok = Token{Type: OR, Literal: string(ch) + string(l.ch), Line: l.line, Column: l.column}
		} else {
			tok = newToken(ILLEGAL, l.ch, l.line, l.column)
		}
	case ',':
		tok = newToken(COMMA, l.ch, l.line, l.column)
	case ';':
		tok = newToken(SEMICOLON, l.ch, l.line, l.column)
	case '.':
		tok = newToken(DOT, l.ch, l.line, l.column)
	case ':':
		tok = newToken(COLON, l.ch, l.line, l.column)
	case '(':
		tok = newToken(LPAREN, l.ch, l.line, l.column)
	case ')':
		tok = newToken(RPAREN, l.ch, l.line, l.column)
	case '{':
		if l.peekChar() == '~' {
			l.readMultiComment() // Read but don't return the comment
			return l.NextToken() // Get the next actual token
		} else {
			tok = newToken(LBRACE, l.ch, l.line, l.column)
		}
	case '}':
		tok = newToken(RBRACE, l.ch, l.line, l.column)
	case '[':
		tok = newToken(LBRACKET, l.ch, l.line, l.column)
	case ']':
		tok = newToken(RBRACKET, l.ch, l.line, l.column)
	case '~':
		l.readSingleComment() // Read but don't return the comment
		return l.NextToken()  // Get the next actual token
	case '"':
		tok.Type = STRING
		tok.Literal = l.readString()
		tok.Line = l.line
		tok.Column = l.column
	case 0:
		tok.Literal = ""
		tok.Type = EOF
		tok.Line = l.line
		tok.Column = l.column
	default:
		if isLetter(l.ch) {
			tok.Literal = l.readIdentifier()
			tok.Type = LookupIdent(tok.Literal)
			tok.Line = l.line
			tok.Column = l.column
			return tok // early return to avoid calling readChar() again
		} else if isDigit(l.ch) {
			tok.Literal, tok.Type = l.readNumber()
			tok.Line = l.line
			tok.Column = l.column
			return tok // early return to avoid calling readChar() again
		} else {
			tok = newToken(ILLEGAL, l.ch, l.line, l.column)
		}
	}

	l.readChar()
	return tok
}

// newToken creates a new token
func newToken(tokenType TokenType, ch byte, line, column int) Token {
	return Token{
		Type:    tokenType,
		Literal: string(ch),
		Line:    line,
		Column:  column,
	}
}

// isLetter checks if the character is a letter
func isLetter(ch byte) bool {
	return 'a' <= ch && ch <= 'z' || 'A' <= ch && ch <= 'Z' || ch == '_'
}

// isDigit checks if the character is a digit
func isDigit(ch byte) bool {
	return '0' <= ch && ch <= '9'
}

// TokenizeString tokenizes a string and returns all tokens
func TokenizeString(input string) ([]Token, error) {
	lexer := New(input)
	var tokens []Token

	for {
		tok := lexer.NextToken()
		if tok.Type == EOF {
			tokens = append(tokens, tok)
			break
		}
		if tok.Type == ILLEGAL {
			return nil, fmt.Errorf("illegal token '%s' at line %d, column %d", tok.Literal, tok.Line, tok.Column)
		}
		tokens = append(tokens, tok)
	}

	return tokens, nil
}
