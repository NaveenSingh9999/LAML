package evaluator

import (
	"fmt"
	"laml/internal/ast"
	"laml/internal/object"
)

var (
	NULL  = &object.Null{}
	TRUE  = &object.Boolean{Value: true}
	FALSE = &object.Boolean{Value: false}
)

// Eval evaluates AST nodes and returns objects
func Eval(node ast.Node, env *object.Environment) object.Object {
	switch node := node.(type) {
	// Statements
	case *ast.Program:
		return evalProgram(node.Statements, env)

	case *ast.ExpressionStatement:
		return Eval(node.Expression, env)

	case *ast.BlockStatement:
		return evalBlockStatement(node, env)

	case *ast.ValStatement:
		val := Eval(node.Value, env)
		if isError(val) {
			return val
		}
		env.Set(node.Name.Value, val)
		return val

	case *ast.LetStatement:
		val := Eval(node.Value, env)
		if isError(val) {
			return val
		}
		env.Set(node.Name.Value, val)
		return val

	case *ast.ConstStatement:
		val := Eval(node.Value, env)
		if isError(val) {
			return val
		}
		env.Set(node.Name.Value, val)
		return val

	case *ast.ReturnStatement:
		val := Eval(node.ReturnValue, env)
		if isError(val) {
			return val
		}
		return &object.ReturnValue{Value: val}

	case *ast.SayStatement:
		val := Eval(node.Value, env)
		if isError(val) {
			return val
		}
		fmt.Println(val.Inspect())
		return NULL

	case *ast.ImportStatement:
		// For now, just return NULL - imports will be handled by the module system
		return NULL

	case *ast.FunctionStatement:
		// Convert TypedParameter to Identifier for the object system
		params := make([]*ast.Identifier, len(node.Parameters))
		for i, param := range node.Parameters {
			params[i] = param.Name
		}
		fn := &object.Function{
			Parameters: params,
			Body:       node.Body,
			Env:        env,
		}
		env.Set(node.Name.Value, fn)
		return fn

	case *ast.TypeStatement:
		obj := &object.ObjectObj{
			Env: object.NewEnclosedEnvironment(env),
		}

		for _, prop := range node.Properties {
			Eval(prop, obj.Env)
		}

		env.Set(node.Name.Value, obj)
		return obj

	case *ast.ObjectStatement:
		obj := &object.ObjectObj{
			Env: object.NewEnclosedEnvironment(env),
		}

		for _, prop := range node.Properties {
			Eval(prop, obj.Env)
		}

		env.Set(node.Name.Value, obj)
		return obj

	case *ast.IfStatement:
		return evalIfExpression(node, env)

	case *ast.WhileStatement:
		return evalWhileStatement(node, env)

	case *ast.LoopStatement:
		return evalLoopStatement(node, env)

	case *ast.CStyleLoopStatement:
		return evalCStyleLoopStatement(node, env)

	// Expressions
	case *ast.IntegerLiteral:
		return &object.Integer{Value: node.Value}

	case *ast.FloatLiteral:
		return &object.Float{Value: node.Value}

	case *ast.StringLiteral:
		return &object.String{Value: node.Value}

	case *ast.BooleanLiteral:
		return nativeBoolToPyBool(node.Value)

	case *ast.PrefixExpression:
		right := Eval(node.Right, env)
		if isError(right) {
			return right
		}
		return evalPrefixExpression(node.Operator, right)

	case *ast.InfixExpression:
		left := Eval(node.Left, env)
		if isError(left) {
			return left
		}
		right := Eval(node.Right, env)
		if isError(right) {
			return right
		}
		return evalInfixExpression(node.Operator, left, right)

	case *ast.PostfixExpression:
		return evalPostfixExpression(node, env)

	case *ast.Identifier:
		return evalIdentifier(node, env)

	case *ast.CallExpression:
		function := Eval(node.Function, env)
		if isError(function) {
			return function
		}
		args := evalExpressions(node.Arguments, env)
		if len(args) == 1 && isError(args[0]) {
			return args[0]
		}
		return applyFunction(function, args)

	case *ast.ArrayLiteral:
		elements := evalExpressions(node.Elements, env)
		if len(elements) == 1 && isError(elements[0]) {
			return elements[0]
		}
		return &object.Array{Elements: elements}

	case *ast.IndexExpression:
		left := Eval(node.Left, env)
		if isError(left) {
			return left
		}
		index := Eval(node.Index, env)
		if isError(index) {
			return index
		}
		return evalIndexExpression(left, index)

	case *ast.DotExpression:
		left := Eval(node.Left, env)
		if isError(left) {
			return left
		}
		return evalDotExpression(left, node.Property, env)

	default:
		return newError("unknown node type: %T", node)
	}
}

// evalProgram evaluates program statements
func evalProgram(stmts []ast.Statement, env *object.Environment) object.Object {
	var result object.Object

	for _, statement := range stmts {
		result = Eval(statement, env)

		switch result := result.(type) {
		case *object.ReturnValue:
			return result.Value
		case *object.Error:
			return result
		}
	}

	// After executing all statements, look for and call main function
	if mainFunc, ok := env.Get("main"); ok {
		if fn, ok := mainFunc.(*object.Function); ok {
			// Call main function with no arguments
			extendedEnv := extendFunctionEnv(fn, []object.Object{})
			evaluated := Eval(fn.Body, extendedEnv)
			return unwrapReturnValue(evaluated)
		}
	}

	return result
}

// evalBlockStatement evaluates block statements
func evalBlockStatement(block *ast.BlockStatement, env *object.Environment) object.Object {
	var result object.Object

	for _, statement := range block.Statements {
		result = Eval(statement, env)

		if result != nil {
			rt := result.Type()
			if rt == object.RETURN_VALUE_OBJ || rt == object.ERROR_OBJ {
				return result
			}
		}
	}

	return result
}

// evalPrefixExpression evaluates prefix expressions
func evalPrefixExpression(operator string, right object.Object) object.Object {
	switch operator {
	case "!":
		return evalBangOperatorExpression(right)
	case "-":
		return evalMinusPrefixOperatorExpression(right)
	default:
		return newError("unknown operator: %s%s", operator, right.Type())
	}
}

// evalBangOperatorExpression evaluates bang operator expressions
func evalBangOperatorExpression(right object.Object) object.Object {
	switch right {
	case TRUE:
		return FALSE
	case FALSE:
		return TRUE
	case NULL:
		return TRUE
	default:
		return FALSE
	}
}

// evalMinusPrefixOperatorExpression evaluates minus prefix operator expressions
func evalMinusPrefixOperatorExpression(right object.Object) object.Object {
	switch right := right.(type) {
	case *object.Integer:
		return &object.Integer{Value: -right.Value}
	case *object.Float:
		return &object.Float{Value: -right.Value}
	default:
		return newError("unknown operator: -%s", right.Type())
	}
}

// evalPostfixExpression evaluates postfix expressions (i++, i--)
func evalPostfixExpression(node *ast.PostfixExpression, env *object.Environment) object.Object {
	// For postfix operators, we need to get the current value, then modify the variable
	if ident, ok := node.Left.(*ast.Identifier); ok {
		currentVal, ok := env.Get(ident.Value)
		if !ok {
			return newError("identifier not found: " + ident.Value)
		}

		switch currentVal := currentVal.(type) {
		case *object.Integer:
			// Store the current value to return
			result := &object.Integer{Value: currentVal.Value}

			// Update the variable
			switch node.Operator {
			case "++":
				env.Set(ident.Value, &object.Integer{Value: currentVal.Value + 1})
			case "--":
				env.Set(ident.Value, &object.Integer{Value: currentVal.Value - 1})
			default:
				return newError("unknown postfix operator: %s", node.Operator)
			}

			return result
		default:
			return newError("postfix operator not supported for type: %s", currentVal.Type())
		}
	}

	return newError("postfix operator can only be applied to identifiers")
}

// evalInfixExpression evaluates infix expressions
func evalInfixExpression(operator string, left, right object.Object) object.Object {
	switch {
	case left.Type() == object.INTEGER_OBJ && right.Type() == object.INTEGER_OBJ:
		return evalIntegerInfixExpression(operator, left, right)
	case left.Type() == object.FLOAT_OBJ && right.Type() == object.FLOAT_OBJ:
		return evalFloatInfixExpression(operator, left, right)
	case left.Type() == object.STRING_OBJ && right.Type() == object.STRING_OBJ:
		return evalStringInfixExpression(operator, left, right)
	case operator == "+" && (left.Type() == object.STRING_OBJ || right.Type() == object.STRING_OBJ):
		// Handle string concatenation with mixed types
		leftStr := left.Inspect()
		rightStr := right.Inspect()
		return &object.String{Value: leftStr + rightStr}
	case operator == "==":
		return nativeBoolToPyBool(left == right)
	case operator == "!=":
		return nativeBoolToPyBool(left != right)
	default:
		return newError("unknown operator: %s %s %s", left.Type(), operator, right.Type())
	}
}

// evalIntegerInfixExpression evaluates integer infix expressions
func evalIntegerInfixExpression(operator string, left, right object.Object) object.Object {
	leftVal := left.(*object.Integer).Value
	rightVal := right.(*object.Integer).Value

	switch operator {
	case "+":
		return &object.Integer{Value: leftVal + rightVal}
	case "-":
		return &object.Integer{Value: leftVal - rightVal}
	case "*":
		return &object.Integer{Value: leftVal * rightVal}
	case "/":
		return &object.Integer{Value: leftVal / rightVal}
	case "%":
		return &object.Integer{Value: leftVal % rightVal}
	case "<":
		return nativeBoolToPyBool(leftVal < rightVal)
	case ">":
		return nativeBoolToPyBool(leftVal > rightVal)
	case "<=":
		return nativeBoolToPyBool(leftVal <= rightVal)
	case ">=":
		return nativeBoolToPyBool(leftVal >= rightVal)
	case "==":
		return nativeBoolToPyBool(leftVal == rightVal)
	case "!=":
		return nativeBoolToPyBool(leftVal != rightVal)
	default:
		return newError("unknown operator: %s", operator)
	}
}

// evalFloatInfixExpression evaluates float infix expressions
func evalFloatInfixExpression(operator string, left, right object.Object) object.Object {
	leftVal := left.(*object.Float).Value
	rightVal := right.(*object.Float).Value

	switch operator {
	case "+":
		return &object.Float{Value: leftVal + rightVal}
	case "-":
		return &object.Float{Value: leftVal - rightVal}
	case "*":
		return &object.Float{Value: leftVal * rightVal}
	case "/":
		return &object.Float{Value: leftVal / rightVal}
	case "<":
		return nativeBoolToPyBool(leftVal < rightVal)
	case ">":
		return nativeBoolToPyBool(leftVal > rightVal)
	case "<=":
		return nativeBoolToPyBool(leftVal <= rightVal)
	case ">=":
		return nativeBoolToPyBool(leftVal >= rightVal)
	case "==":
		return nativeBoolToPyBool(leftVal == rightVal)
	case "!=":
		return nativeBoolToPyBool(leftVal != rightVal)
	default:
		return newError("unknown operator: %s", operator)
	}
}

// evalStringInfixExpression evaluates string infix expressions
func evalStringInfixExpression(operator string, left, right object.Object) object.Object {
	leftVal := left.(*object.String).Value
	rightVal := right.(*object.String).Value

	switch operator {
	case "+":
		return &object.String{Value: leftVal + rightVal}
	case "==":
		return nativeBoolToPyBool(leftVal == rightVal)
	case "!=":
		return nativeBoolToPyBool(leftVal != rightVal)
	default:
		return newError("unknown operator: %s", operator)
	}
}

// evalIfExpression evaluates if expressions
func evalIfExpression(ie *ast.IfStatement, env *object.Environment) object.Object {
	condition := Eval(ie.Condition, env)
	if isError(condition) {
		return condition
	}

	if isTruthy(condition) {
		return Eval(ie.Consequence, env)
	} else if ie.Alternative != nil {
		return Eval(ie.Alternative, env)
	} else {
		return NULL
	}
}

// evalWhileStatement evaluates while statements
func evalWhileStatement(ws *ast.WhileStatement, env *object.Environment) object.Object {
	var result object.Object = NULL

	for {
		condition := Eval(ws.Condition, env)
		if isError(condition) {
			return condition
		}

		if !isTruthy(condition) {
			break
		}

		result = Eval(ws.Body, env)
		if result != nil {
			rt := result.Type()
			if rt == object.RETURN_VALUE_OBJ || rt == object.ERROR_OBJ {
				return result
			}
		}
	}

	return result
}

// evalCStyleLoopStatement evaluates C-style loop statements
func evalCStyleLoopStatement(cls *ast.CStyleLoopStatement, env *object.Environment) object.Object {
	var result object.Object = NULL
	loopEnv := object.NewEnclosedEnvironment(env)

	// Execute initialization
	if cls.Init != nil {
		initResult := Eval(cls.Init, loopEnv)
		if isError(initResult) {
			return initResult
		}
	}

	for {
		// Check condition
		if cls.Condition != nil {
			condition := Eval(cls.Condition, loopEnv)
			if isError(condition) {
				return condition
			}
			if !isTruthy(condition) {
				break
			}
		}

		// Execute body
		result = Eval(cls.Body, loopEnv)
		if result != nil {
			rt := result.Type()
			if rt == object.RETURN_VALUE_OBJ || rt == object.ERROR_OBJ {
				return result
			}
		}

		// Execute update
		if cls.Update != nil {
			updateResult := Eval(cls.Update, loopEnv)
			if isError(updateResult) {
				return updateResult
			}
		}
	}

	return result
}

// evalLoopStatement evaluates loop statements
func evalLoopStatement(ls *ast.LoopStatement, env *object.Environment) object.Object {
	start := Eval(ls.Start, env)
	if isError(start) {
		return start
	}

	end := Eval(ls.End, env)
	if isError(end) {
		return end
	}

	startInt, ok := start.(*object.Integer)
	if !ok {
		return newError("loop start must be integer, got %T", start)
	}

	endInt, ok := end.(*object.Integer)
	if !ok {
		return newError("loop end must be integer, got %T", end)
	}

	var result object.Object = NULL
	loopEnv := object.NewEnclosedEnvironment(env)

	for i := startInt.Value; i <= endInt.Value; i++ {
		loopEnv.Set(ls.Variable.Value, &object.Integer{Value: i})
		result = Eval(ls.Body, loopEnv)
		if result != nil {
			rt := result.Type()
			if rt == object.RETURN_VALUE_OBJ || rt == object.ERROR_OBJ {
				return result
			}
		}
	}

	return result
}

// evalIdentifier evaluates identifiers
func evalIdentifier(node *ast.Identifier, env *object.Environment) object.Object {
	val, ok := env.Get(node.Value)
	if !ok {
		return newError("identifier not found: " + node.Value)
	}

	return val
}

// evalExpressions evaluates a list of expressions
func evalExpressions(exps []ast.Expression, env *object.Environment) []object.Object {
	var result []object.Object

	for _, e := range exps {
		evaluated := Eval(e, env)
		if isError(evaluated) {
			return []object.Object{evaluated}
		}
		result = append(result, evaluated)
	}

	return result
}

// applyFunction applies a function with arguments
func applyFunction(fn object.Object, args []object.Object) object.Object {
	switch fn := fn.(type) {
	case *object.Function:
		extendedEnv := extendFunctionEnv(fn, args)
		evaluated := Eval(fn.Body, extendedEnv)
		return unwrapReturnValue(evaluated)
	case *object.Builtin:
		return fn.Fn(args...)
	default:
		return newError("not a function: %T", fn)
	}
}

// extendFunctionEnv extends the function environment with parameters
func extendFunctionEnv(fn *object.Function, args []object.Object) *object.Environment {
	env := object.NewEnclosedEnvironment(fn.Env)

	for paramIdx, param := range fn.Parameters {
		env.Set(param.Value, args[paramIdx])
	}

	return env
}

// unwrapReturnValue unwraps return values
func unwrapReturnValue(obj object.Object) object.Object {
	if returnValue, ok := obj.(*object.ReturnValue); ok {
		return returnValue.Value
	}
	return obj
}

// evalIndexExpression evaluates index expressions
func evalIndexExpression(left, index object.Object) object.Object {
	switch {
	case left.Type() == object.ARRAY_OBJ && index.Type() == object.INTEGER_OBJ:
		return evalArrayIndexExpression(left, index)
	default:
		return newError("index operator not supported: %s", left.Type())
	}
}

// evalArrayIndexExpression evaluates array index expressions
func evalArrayIndexExpression(array, index object.Object) object.Object {
	arrayObject := array.(*object.Array)
	idx := index.(*object.Integer).Value
	max := int64(len(arrayObject.Elements) - 1)

	if idx < 0 || idx > max {
		return NULL
	}

	return arrayObject.Elements[idx]
}

// evalDotExpression evaluates dot expressions
func evalDotExpression(left object.Object, property *ast.Identifier, env *object.Environment) object.Object {
	switch obj := left.(type) {
	case *object.ObjectObj:
		val, ok := obj.Env.Get(property.Value)
		if !ok {
			return newError("property not found: %s", property.Value)
		}
		return val
	default:
		return newError("dot operator not supported: %s", left.Type())
	}
}

// isTruthy checks if an object is truthy
func isTruthy(obj object.Object) bool {
	switch obj {
	case NULL:
		return false
	case TRUE:
		return true
	case FALSE:
		return false
	default:
		return true
	}
}

// nativeBoolToPyBool converts Go bool to LAML bool
func nativeBoolToPyBool(input bool) *object.Boolean {
	if input {
		return TRUE
	}
	return FALSE
}

// newError creates a new error object
func newError(format string, a ...interface{}) *object.Error {
	return &object.Error{Message: fmt.Sprintf(format, a...)}
}

// isError checks if an object is an error
func isError(obj object.Object) bool {
	if obj != nil {
		return obj.Type() == object.ERROR_OBJ
	}
	return false
}
