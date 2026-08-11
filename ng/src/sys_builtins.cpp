#include "sys_builtins.h"
#include "env.h"
#include "value.h"
#include "evaluator.h"
#include "lexer.h"
#include "parser.h"
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

static std::vector<std::string> gArgs;

// Imported modules' ASTs must outlive the functions they declare (FuncData
// holds a pointer into the AST for its body). Keep them alive for the
// process lifetime — module count is small and bounded.
static std::vector<std::unique_ptr<ASTNode>> gImportedAsts;

void sysInit(int argc, char** argv) {
    gArgs.clear();
    for (int i = 0; i < argc; i++) gArgs.push_back(argv[i]);
}

static Value builtinArgs(const std::vector<Value>&) {
    std::vector<Value> out;
    for (const auto& a : gArgs) out.push_back(Value::makeString(a));
    return Value::makeArray(out);
}

static Value builtinEnv(const std::vector<Value>& args) {
    if (args.empty() || args[0].type != ValType::String)
        return Value::makeError("env: expected a name string");
    const char* v = std::getenv(args[0].strVal.c_str());
    return Value::makeString(v ? v : "");
}

static Value builtinExit(const std::vector<Value>& args) {
    int code = 0;
    if (!args.empty() && args[0].type == ValType::Int) code = (int)args[0].intVal;
    // _Exit: std::exit runs static destructors while scheduler workers still
    // hold locks (destroyed mutex -> abort). Immediate termination is correct
    // for a CLI exit; the OS reclaims fds.
    std::_Exit(code);
}

static Value builtinWriteFile(const std::vector<Value>& args) {
    if (args.size() < 2 || args[0].type != ValType::String ||
        args[1].type != ValType::String)
        return Value::makeError("writeFile: expected (path, data)");
    std::ofstream f(args[0].strVal, std::ios::binary);
    if (!f.is_open()) return Value::makeError("writeFile: cannot open " + args[0].strVal);
    f << args[1].strVal;
    return NIL;
}

static Value builtinAppend(const std::vector<Value>& args) {
    if (args.size() < 2 || args[0].type != ValType::Array || !args[0].arrVal)
        return Value::makeError("append: expected (array, value)");
    args[0].arrVal->push_back(args[1]);
    return NIL;
}

static Value builtinTry(const std::vector<Value>& args) {
    if (args.empty() || args[0].type != ValType::Func)
        return Value::makeError("try: expected a function");
    Evaluator* ev = Evaluator::current();
    if (!ev) return Value::makeError("try: no evaluator");
    Value result = ev->applyFunc(args[0], {});
    if (result.type == ValType::Error)
        return Value::makeArray({FALSE_V, Value::makeString(result.errMsg)});
    return Value::makeArray({TRUE_V, result});
}

static Value builtinImport(const std::vector<Value>& args) {
    if (args.empty() || args[0].type != ValType::String)
        return Value::makeError("import: expected a path string");
    std::ifstream f(args[0].strVal);
    if (!f.is_open()) return Value::makeError("import: cannot open " + args[0].strVal);
    std::stringstream ss;
    ss << f.rdbuf();
    Lexer lexer(ss.str());
    Parser parser(lexer);
    auto ast = std::make_unique<ASTNode>(parser.parseProgram());
    auto errors = parser.getErrors();
    if (!errors.empty())
        return Value::makeError("import: parse error in " + args[0].strVal + ": " + errors[0]);
    Evaluator* ev = Evaluator::current();
    if (!ev) return Value::makeError("import: no evaluator");
    Value result = ev->eval(*ast, ev->globalEnv());
    if (result.type == ValType::Error) return result;
    gImportedAsts.push_back(std::move(ast));
    return NIL;
}

void registerSysBuiltins(std::shared_ptr<Env> env) {
    auto reg = [&](const std::string& name, auto fn) {
        env->set(name, Value::makeBuiltin({fn}));
    };
    reg("args", builtinArgs);
    reg("env", builtinEnv);
    reg("exit", builtinExit);
    reg("writeFile", builtinWriteFile);
    reg("append", builtinAppend);
    reg("try", builtinTry);
    reg("import", builtinImport);
}
