package compiler

import (
	"fmt"
	"io/ioutil"
	"os"
	"os/exec"
	"path/filepath"
	"strings"

	"laml/internal/ast"
	"laml/internal/console"
	"laml/internal/evaluator"
	"laml/internal/lexer"
	"laml/internal/object"
	"laml/internal/parser"
)

// Compiler represents the LAML compiler
type Compiler struct {
	env *object.Environment
}

// New creates a new compiler instance
func New() *Compiler {
	return &Compiler{
		env: object.NewEnvironment(),
	}
}

// CompileFile compiles a LAML source file
func (c *Compiler) CompileFile(filename string) error {
	// Read the source file
	input, err := ioutil.ReadFile(filename)
	if err != nil {
		return fmt.Errorf("failed to read file %s: %v", filename, err)
	}

	// Tokenize
	l := lexer.New(string(input))

	// Parse
	p := parser.New(l)
	program := p.ParseProgram()

	// Check for parser errors
	errors := p.Errors()
	if len(errors) > 0 {
		for _, err := range errors {
			console.PrintError(0, err)
		}
		return fmt.Errorf("parser errors occurred")
	}

	// Generate output filename
	outputFile := strings.TrimSuffix(filename, ".lm") + ".bin"

	// For now, we'll create a simple Go executable
	// In a real implementation, you might want to generate actual binary code
	goCode := c.generateGoCode(program)

	// Write Go code to temporary file
	tempDir, err := ioutil.TempDir("", "laml_compile")
	if err != nil {
		return fmt.Errorf("failed to create temp directory: %v", err)
	}
	defer os.RemoveAll(tempDir)

	goFile := filepath.Join(tempDir, "main.go")
	err = ioutil.WriteFile(goFile, []byte(goCode), 0644)
	if err != nil {
		return fmt.Errorf("failed to write Go code: %v", err)
	}

	// Compile Go code to binary
	cmd := exec.Command("go", "build", "-o", outputFile, goFile)
	output, err := cmd.CombinedOutput()
	if err != nil {
		return fmt.Errorf("Go compilation failed: %v\nOutput: %s", err, string(output))
	}

	return nil
}

// CompileAndRun compiles and runs a LAML source file
func (c *Compiler) CompileAndRun(filename string) error {
	// Read the source file
	input, err := ioutil.ReadFile(filename)
	if err != nil {
		return fmt.Errorf("failed to read file %s: %v", filename, err)
	}

	// Tokenize
	l := lexer.New(string(input))

	// Parse
	p := parser.New(l)
	program := p.ParseProgram()

	// Check for parser errors
	errors := p.Errors()
	if len(errors) > 0 {
		for _, err := range errors {
			console.PrintError(0, err)
		}
		return fmt.Errorf("parser errors occurred")
	}

	// Execute using interpreter
	result := evaluator.Eval(program, c.env)
	if result.Type() == object.ERROR_OBJ {
		console.PrintError(0, fmt.Sprintf("Runtime error: %s", result.Inspect()))
		return fmt.Errorf("runtime error occurred")
	}

	return nil
}

// generateGoCode generates Go code from the AST
func (c *Compiler) generateGoCode(program *ast.Program) string {
	var code strings.Builder

	code.WriteString(`package main

import (
	"fmt"
)

func main() {
`)

	// Generate code for each statement
	for _, stmt := range program.Statements {
		c.generateStatement(stmt, &code, "\t")
	}

	code.WriteString("}\n")

	return code.String()
} // generateStatement generates Go code for a statement
func (c *Compiler) generateStatement(stmt ast.Statement, code *strings.Builder, indent string) {
	switch stmt := stmt.(type) {
	case *ast.SayStatement:
		code.WriteString(indent + "fmt.Println(")
		c.generateExpression(stmt.Value, code)
		code.WriteString(")\n")

	case *ast.ValStatement:
		code.WriteString(indent + stmt.Name.Value + " := ")
		c.generateExpression(stmt.Value, code)
		code.WriteString("\n")

	case *ast.LetStatement:
		code.WriteString(indent + stmt.Name.Value + " := ")
		c.generateExpression(stmt.Value, code)
		code.WriteString("\n")

	case *ast.ConstStatement:
		code.WriteString(indent + "const " + stmt.Name.Value + " = ")
		c.generateExpression(stmt.Value, code)
		code.WriteString("\n")

	case *ast.ReturnStatement:
		code.WriteString(indent + "return ")
		if stmt.ReturnValue != nil {
			c.generateExpression(stmt.ReturnValue, code)
		}
		code.WriteString("\n")

	case *ast.ExpressionStatement:
		code.WriteString(indent)
		c.generateExpression(stmt.Expression, code)
		code.WriteString("\n")

	case *ast.BlockStatement:
		for _, s := range stmt.Statements {
			c.generateStatement(s, code, indent)
		}

	case *ast.IfStatement:
		code.WriteString(indent + "if ")
		c.generateExpression(stmt.Condition, code)
		code.WriteString(" {\n")
		c.generateStatement(stmt.Consequence, code, indent+"\t")
		if stmt.Alternative != nil {
			code.WriteString(indent + "} else {\n")
			c.generateStatement(stmt.Alternative, code, indent+"\t")
		}
		code.WriteString(indent + "}\n")

	case *ast.WhileStatement:
		code.WriteString(indent + "for ")
		c.generateExpression(stmt.Condition, code)
		code.WriteString(" {\n")
		c.generateStatement(stmt.Body, code, indent+"\t")
		code.WriteString(indent + "}\n")

	case *ast.LoopStatement:
		code.WriteString(indent + "for " + stmt.Variable.Value + " := ")
		c.generateExpression(stmt.Start, code)
		code.WriteString("; " + stmt.Variable.Value + " <= ")
		c.generateExpression(stmt.End, code)
		code.WriteString("; " + stmt.Variable.Value + "++ {\n")
		c.generateStatement(stmt.Body, code, indent+"\t")
		code.WriteString(indent + "}\n")

	case *ast.FunctionStatement:
		if stmt.Name.Value == "main" {
			// Don't generate a separate main function, integrate it into the Go main
			c.generateStatement(stmt.Body, code, indent)
		} else {
			code.WriteString(indent + "func " + stmt.Name.Value + "(")
			for i, param := range stmt.Parameters {
				if i > 0 {
					code.WriteString(", ")
				}
				code.WriteString(param.Name.Value + " interface{}")
			}
			code.WriteString(") interface{} {\n")
			c.generateStatement(stmt.Body, code, indent+"\t")
			code.WriteString(indent + "}\n")
		}
	}
}

// generateExpression generates Go code for an expression
func (c *Compiler) generateExpression(expr ast.Expression, code *strings.Builder) {
	switch expr := expr.(type) {
	case *ast.IntegerLiteral:
		code.WriteString(fmt.Sprintf("%d", expr.Value))

	case *ast.FloatLiteral:
		code.WriteString(fmt.Sprintf("%g", expr.Value))

	case *ast.StringLiteral:
		code.WriteString(fmt.Sprintf(`"%s"`, expr.Value))

	case *ast.BooleanLiteral:
		code.WriteString(fmt.Sprintf("%t", expr.Value))

	case *ast.Identifier:
		code.WriteString(expr.Value)

	case *ast.PrefixExpression:
		code.WriteString(expr.Operator)
		c.generateExpression(expr.Right, code)

	case *ast.InfixExpression:
		code.WriteString("(")
		c.generateExpression(expr.Left, code)
		code.WriteString(" " + expr.Operator + " ")
		c.generateExpression(expr.Right, code)
		code.WriteString(")")

	case *ast.CallExpression:
		c.generateExpression(expr.Function, code)
		code.WriteString("(")
		for i, arg := range expr.Arguments {
			if i > 0 {
				code.WriteString(", ")
			}
			c.generateExpression(arg, code)
		}
		code.WriteString(")")

	case *ast.ArrayLiteral:
		code.WriteString("[]interface{}{")
		for i, elem := range expr.Elements {
			if i > 0 {
				code.WriteString(", ")
			}
			c.generateExpression(elem, code)
		}
		code.WriteString("}")

	case *ast.IndexExpression:
		c.generateExpression(expr.Left, code)
		code.WriteString("[")
		c.generateExpression(expr.Index, code)
		code.WriteString("]")

	case *ast.DotExpression:
		c.generateExpression(expr.Left, code)
		code.WriteString(".")
		code.WriteString(expr.Property.Value)
	}
}
