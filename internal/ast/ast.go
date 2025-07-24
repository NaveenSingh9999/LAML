package ast

import (
	"bytes"
	"laml/internal/lexer"
	"strings"
)

// Node represents any AST node
type Node interface {
	TokenLiteral() string
	String() string
}

// Statemen// TypedParameter represents a parameter with type annotation
type TypedParameter struct {
	Name *Identifier
	Type *Identifier // optional type annotation
}

func (tp *TypedParameter) String() string {
	if tp.Type != nil {
		return tp.Name.String() + ":" + tp.Type.String()
	}
	return tp.Name.String()
}

// FunctionStatement represents function definitions
type FunctionStatement struct {
	Token      lexer.Token // the 'func' token
	Name       *Identifier
	Parameters []*TypedParameter // updated to support type annotations
	ReturnType *Identifier       // optional return type
	Body       *BlockStatement
}

// Statement represents any statement node
type Statement interface {
	Node
	statementNode()
}

// Expression represents any expression node
type Expression interface {
	Node
	expressionNode()
}

// Program represents the root of every AST
type Program struct {
	Statements []Statement
}

func (p *Program) TokenLiteral() string {
	if len(p.Statements) > 0 {
		return p.Statements[0].TokenLiteral()
	}
	return ""
}

func (p *Program) String() string {
	var out bytes.Buffer
	for _, s := range p.Statements {
		out.WriteString(s.String())
	}
	return out.String()
}

// ImportStatement represents import statements (bring)
type ImportStatement struct {
	Token lexer.Token // the 'bring' token
	Value Expression
}

func (is *ImportStatement) statementNode()       {}
func (is *ImportStatement) TokenLiteral() string { return is.Token.Literal }
func (is *ImportStatement) String() string {
	var out bytes.Buffer
	out.WriteString(is.TokenLiteral() + " ")
	if is.Value != nil {
		out.WriteString(is.Value.String())
	}
	out.WriteString(";")
	return out.String()
}

// ValStatement represents variable declarations (val keyword)
type ValStatement struct {
	Token lexer.Token // the 'val' token
	Name  *Identifier
	Value Expression
}

func (vs *ValStatement) statementNode()       {}
func (vs *ValStatement) TokenLiteral() string { return vs.Token.Literal }
func (vs *ValStatement) String() string {
	var out bytes.Buffer
	out.WriteString(vs.TokenLiteral() + " ")
	out.WriteString(vs.Name.String())
	out.WriteString(" = ")
	if vs.Value != nil {
		out.WriteString(vs.Value.String())
	}
	out.WriteString(";")
	return out.String()
}

// LetStatement represents variable declarations
type LetStatement struct {
	Token lexer.Token // the 'let' token
	Name  *Identifier
	Value Expression
}

func (ls *LetStatement) statementNode()       {}
func (ls *LetStatement) TokenLiteral() string { return ls.Token.Literal }
func (ls *LetStatement) String() string {
	var out bytes.Buffer
	out.WriteString(ls.TokenLiteral() + " ")
	out.WriteString(ls.Name.String())
	out.WriteString(" = ")
	if ls.Value != nil {
		out.WriteString(ls.Value.String())
	}
	out.WriteString(";")
	return out.String()
}

// ConstStatement represents constant declarations
type ConstStatement struct {
	Token lexer.Token // the 'const' token
	Name  *Identifier
	Value Expression
}

func (cs *ConstStatement) statementNode()       {}
func (cs *ConstStatement) TokenLiteral() string { return cs.Token.Literal }
func (cs *ConstStatement) String() string {
	var out bytes.Buffer
	out.WriteString(cs.TokenLiteral() + " ")
	out.WriteString(cs.Name.String())
	out.WriteString(" = ")
	if cs.Value != nil {
		out.WriteString(cs.Value.String())
	}
	out.WriteString(";")
	return out.String()
}

// ReturnStatement represents return statements
type ReturnStatement struct {
	Token       lexer.Token // the 'return' token
	ReturnValue Expression
}

func (rs *ReturnStatement) statementNode()       {}
func (rs *ReturnStatement) TokenLiteral() string { return rs.Token.Literal }
func (rs *ReturnStatement) String() string {
	var out bytes.Buffer
	out.WriteString(rs.TokenLiteral() + " ")
	if rs.ReturnValue != nil {
		out.WriteString(rs.ReturnValue.String())
	}
	out.WriteString(";")
	return out.String()
}

// ExpressionStatement represents expressions used as statements
type ExpressionStatement struct {
	Token      lexer.Token // the first token of the expression
	Expression Expression
}

func (es *ExpressionStatement) statementNode()       {}
func (es *ExpressionStatement) TokenLiteral() string { return es.Token.Literal }
func (es *ExpressionStatement) String() string {
	if es.Expression != nil {
		return es.Expression.String()
	}
	return ""
}

// BlockStatement represents block statements
type BlockStatement struct {
	Token      lexer.Token // the '{' token
	Statements []Statement
}

func (bs *BlockStatement) statementNode()       {}
func (bs *BlockStatement) TokenLiteral() string { return bs.Token.Literal }
func (bs *BlockStatement) String() string {
	var out bytes.Buffer
	for _, s := range bs.Statements {
		out.WriteString(s.String())
	}
	return out.String()
}

func (fs *FunctionStatement) statementNode()       {}
func (fs *FunctionStatement) TokenLiteral() string { return fs.Token.Literal }
func (fs *FunctionStatement) String() string {
	var out bytes.Buffer
	params := []string{}
	for _, p := range fs.Parameters {
		params = append(params, p.String())
	}
	out.WriteString(fs.TokenLiteral())
	out.WriteString(" ")
	out.WriteString(fs.Name.String())
	out.WriteString("(")
	out.WriteString(strings.Join(params, ", "))
	out.WriteString(")")
	if fs.ReturnType != nil {
		out.WriteString(":")
		out.WriteString(fs.ReturnType.String())
	}
	out.WriteString(" ")
	out.WriteString(fs.Body.String())
	return out.String()
}

// TypeStatement represents type definitions (OOP-lite)
type TypeStatement struct {
	Token      lexer.Token // the 'type' token
	Name       *Identifier
	Properties []Statement // can contain both variable declarations and functions
}

func (ts *TypeStatement) statementNode()       {}
func (ts *TypeStatement) TokenLiteral() string { return ts.Token.Literal }
func (ts *TypeStatement) String() string {
	var out bytes.Buffer
	out.WriteString(ts.TokenLiteral())
	out.WriteString(" ")
	out.WriteString(ts.Name.String())
	out.WriteString(" {\n")
	for _, prop := range ts.Properties {
		out.WriteString("  ")
		out.WriteString(prop.String())
		out.WriteString("\n")
	}
	out.WriteString("}")
	return out.String()
}

// ObjectStatement represents object definitions
type ObjectStatement struct {
	Token      lexer.Token // the 'obj' token
	Name       *Identifier
	Properties []Statement // can contain both variable declarations and functions
}

func (os *ObjectStatement) statementNode()       {}
func (os *ObjectStatement) TokenLiteral() string { return os.Token.Literal }
func (os *ObjectStatement) String() string {
	var out bytes.Buffer
	out.WriteString(os.TokenLiteral())
	out.WriteString(" ")
	out.WriteString(os.Name.String())
	out.WriteString(" {\n")
	for _, prop := range os.Properties {
		out.WriteString("  ")
		out.WriteString(prop.String())
		out.WriteString("\n")
	}
	out.WriteString("}")
	return out.String()
}

// IfStatement represents if statements
type IfStatement struct {
	Token       lexer.Token // the 'if' token
	Condition   Expression
	Consequence *BlockStatement
	Alternative *BlockStatement
}

func (ifs *IfStatement) statementNode()       {}
func (ifs *IfStatement) TokenLiteral() string { return ifs.Token.Literal }
func (ifs *IfStatement) String() string {
	var out bytes.Buffer
	out.WriteString("if")
	out.WriteString(ifs.Condition.String())
	out.WriteString(" ")
	out.WriteString(ifs.Consequence.String())
	if ifs.Alternative != nil {
		out.WriteString("else ")
		out.WriteString(ifs.Alternative.String())
	}
	return out.String()
}

// WhileStatement represents while loops
type WhileStatement struct {
	Token     lexer.Token // the 'while' token
	Condition Expression
	Body      *BlockStatement
}

func (ws *WhileStatement) statementNode()       {}
func (ws *WhileStatement) TokenLiteral() string { return ws.Token.Literal }
func (ws *WhileStatement) String() string {
	var out bytes.Buffer
	out.WriteString("while ")
	out.WriteString(ws.Condition.String())
	out.WriteString(" ")
	out.WriteString(ws.Body.String())
	return out.String()
}

// CStyleLoopStatement represents C-style loops
type CStyleLoopStatement struct {
	Token     lexer.Token // the 'loop' token
	Init      Statement   // initialization statement
	Condition Expression  // loop condition
	Update    Statement   // update statement (e.g., i++)
	Body      *BlockStatement
}

func (csl *CStyleLoopStatement) statementNode()       {}
func (csl *CStyleLoopStatement) TokenLiteral() string { return csl.Token.Literal }
func (csl *CStyleLoopStatement) String() string {
	var out bytes.Buffer
	out.WriteString("loop ")
	if csl.Init != nil {
		out.WriteString(csl.Init.String())
	}
	out.WriteString(" ")
	if csl.Condition != nil {
		out.WriteString(csl.Condition.String())
	}
	out.WriteString("; ")
	if csl.Update != nil {
		out.WriteString(csl.Update.String())
	}
	out.WriteString(" ")
	out.WriteString(csl.Body.String())
	return out.String()
}

// LoopStatement represents range-based loops
type LoopStatement struct {
	Token    lexer.Token // the 'loop' token
	Variable *Identifier
	Start    Expression
	End      Expression
	Body     *BlockStatement
}

func (ls *LoopStatement) statementNode()       {}
func (ls *LoopStatement) TokenLiteral() string { return ls.Token.Literal }
func (ls *LoopStatement) String() string {
	var out bytes.Buffer
	out.WriteString("loop ")
	out.WriteString(ls.Variable.String())
	out.WriteString(" in ")
	out.WriteString(ls.Start.String())
	out.WriteString(" to ")
	out.WriteString(ls.End.String())
	out.WriteString(" ")
	out.WriteString(ls.Body.String())
	return out.String()
}

// SayStatement represents print statements
type SayStatement struct {
	Token lexer.Token // the 'say' token
	Value Expression
}

func (ss *SayStatement) statementNode()       {}
func (ss *SayStatement) TokenLiteral() string { return ss.Token.Literal }
func (ss *SayStatement) String() string {
	var out bytes.Buffer
	out.WriteString(ss.TokenLiteral() + " ")
	if ss.Value != nil {
		out.WriteString(ss.Value.String())
	}
	out.WriteString(";")
	return out.String()
}

// Identifier represents identifiers
type Identifier struct {
	Token lexer.Token // the token.IDENT token
	Value string
}

func (i *Identifier) expressionNode()      {}
func (i *Identifier) TokenLiteral() string { return i.Token.Literal }
func (i *Identifier) String() string       { return i.Value }

// IntegerLiteral represents integer literals
type IntegerLiteral struct {
	Token lexer.Token // the token.INT token
	Value int64
}

func (il *IntegerLiteral) expressionNode()      {}
func (il *IntegerLiteral) TokenLiteral() string { return il.Token.Literal }
func (il *IntegerLiteral) String() string       { return il.Token.Literal }

// FloatLiteral represents float literals
type FloatLiteral struct {
	Token lexer.Token // the token.FLOAT token
	Value float64
}

func (fl *FloatLiteral) expressionNode()      {}
func (fl *FloatLiteral) TokenLiteral() string { return fl.Token.Literal }
func (fl *FloatLiteral) String() string       { return fl.Token.Literal }

// StringLiteral represents string literals
type StringLiteral struct {
	Token lexer.Token // the token.STRING token
	Value string
}

func (sl *StringLiteral) expressionNode()      {}
func (sl *StringLiteral) TokenLiteral() string { return sl.Token.Literal }
func (sl *StringLiteral) String() string       { return "\"" + sl.Token.Literal + "\"" }

// BooleanLiteral represents boolean literals
type BooleanLiteral struct {
	Token lexer.Token // the token.TRUE or token.FALSE token
	Value bool
}

func (bl *BooleanLiteral) expressionNode()      {}
func (bl *BooleanLiteral) TokenLiteral() string { return bl.Token.Literal }
func (bl *BooleanLiteral) String() string       { return bl.Token.Literal }

// PrefixExpression represents prefix expressions
type PrefixExpression struct {
	Token    lexer.Token // the prefix token, e.g. !
	Operator string
	Right    Expression
}

func (pe *PrefixExpression) expressionNode()      {}
func (pe *PrefixExpression) TokenLiteral() string { return pe.Token.Literal }
func (pe *PrefixExpression) String() string {
	var out bytes.Buffer
	out.WriteString("(")
	out.WriteString(pe.Operator)
	out.WriteString(pe.Right.String())
	out.WriteString(")")
	return out.String()
}

// PostfixExpression represents postfix expressions (i++, i--)
type PostfixExpression struct {
	Token    lexer.Token // the operator token, e.g. ++
	Left     Expression
	Operator string
}

func (pe *PostfixExpression) expressionNode()      {}
func (pe *PostfixExpression) TokenLiteral() string { return pe.Token.Literal }
func (pe *PostfixExpression) String() string {
	var out bytes.Buffer
	out.WriteString("(")
	out.WriteString(pe.Left.String())
	out.WriteString(pe.Operator)
	out.WriteString(")")
	return out.String()
}

// InfixExpression represents infix expressions
type InfixExpression struct {
	Token    lexer.Token // the operator token, e.g. +
	Left     Expression
	Operator string
	Right    Expression
}

func (ie *InfixExpression) expressionNode()      {}
func (ie *InfixExpression) TokenLiteral() string { return ie.Token.Literal }
func (ie *InfixExpression) String() string {
	var out bytes.Buffer
	out.WriteString("(")
	out.WriteString(ie.Left.String())
	out.WriteString(" " + ie.Operator + " ")
	out.WriteString(ie.Right.String())
	out.WriteString(")")
	return out.String()
}

// CallExpression represents function calls
type CallExpression struct {
	Token     lexer.Token // the '(' token
	Function  Expression  // Identifier or FunctionLiteral
	Arguments []Expression
}

func (ce *CallExpression) expressionNode()      {}
func (ce *CallExpression) TokenLiteral() string { return ce.Token.Literal }
func (ce *CallExpression) String() string {
	var out bytes.Buffer
	args := []string{}
	for _, a := range ce.Arguments {
		args = append(args, a.String())
	}
	out.WriteString(ce.Function.String())
	out.WriteString("(")
	out.WriteString(strings.Join(args, ", "))
	out.WriteString(")")
	return out.String()
}

// IndexExpression represents array/object indexing
type IndexExpression struct {
	Token lexer.Token // the '[' token
	Left  Expression
	Index Expression
}

func (ie *IndexExpression) expressionNode()      {}
func (ie *IndexExpression) TokenLiteral() string { return ie.Token.Literal }
func (ie *IndexExpression) String() string {
	var out bytes.Buffer
	out.WriteString("(")
	out.WriteString(ie.Left.String())
	out.WriteString("[")
	out.WriteString(ie.Index.String())
	out.WriteString("])")
	return out.String()
}

// DotExpression represents property access
type DotExpression struct {
	Token    lexer.Token // the '.' token
	Left     Expression
	Property *Identifier
}

func (de *DotExpression) expressionNode()      {}
func (de *DotExpression) TokenLiteral() string { return de.Token.Literal }
func (de *DotExpression) String() string {
	var out bytes.Buffer
	out.WriteString(de.Left.String())
	out.WriteString(".")
	out.WriteString(de.Property.String())
	return out.String()
}

// ArrayLiteral represents array literals
type ArrayLiteral struct {
	Token    lexer.Token // the '[' token
	Elements []Expression
}

func (al *ArrayLiteral) expressionNode()      {}
func (al *ArrayLiteral) TokenLiteral() string { return al.Token.Literal }
func (al *ArrayLiteral) String() string {
	var out bytes.Buffer
	elements := []string{}
	for _, e := range al.Elements {
		elements = append(elements, e.String())
	}
	out.WriteString("[")
	out.WriteString(strings.Join(elements, ", "))
	out.WriteString("]")
	return out.String()
}
