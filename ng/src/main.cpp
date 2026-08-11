#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <memory>
#include <csignal>
#include <cstdlib>

#include "lexer.h"
#include "parser.h"
#include "evaluator.h"
#include "builtins.h"
#include "scheduler.h"
#include "closc.h"
#include "jit.h"
#include "sys_builtins.h"

static std::shared_ptr<Env> globalEnv;
static Evaluator* gEval = nullptr;

void handleSignal(int sig) {
    // Direct _Exit: a closc blocked in a blocking socket call never joins,
    // so graceful shutdown would hang. The OS reclaims fds on exit.
    std::cerr << "\n[SAFETY] Signal " << sig << " received. Exiting..." << std::endl;
    std::_Exit(sig);
}

std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: cannot open " << path << std::endl;
        return "";
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

void runFile(const std::string& path) {
    std::string source = readFile(path);
    if (source.empty()) return;

    Lexer lexer(source);
    Parser parser(lexer);
    ASTNode ast = parser.parseProgram();

    auto errors = parser.getErrors();
    if (!errors.empty()) {
        for (const auto& e : errors) {
            std::cerr << "Parse error: " << e << std::endl;
        }
        return;
    }

    Scheduler::instance().start();

    Evaluator evaluator(globalEnv);
    gEval = &evaluator;

    Value result = evaluator.eval(ast, globalEnv);
    if (result.type == ValType::Error) {
        std::cerr << "Runtime error: " << result.errMsg << std::endl;
    }

    // Wait for CLOSC sections to be registered and started
    // Then wait for the main program (CLOSC runs in background)

    gEval = nullptr;
}

void runRepl() {
    std::cout << "LAML-NG v1.0 — Ultra Simple, C-Powered, LLVM-Backed" << std::endl;
    std::cout << "Enter code (Ctrl+D to exit):" << std::endl;

    Evaluator evaluator(globalEnv);
    std::string line;

    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;

        if (line.empty()) continue;

        Lexer lexer(line);
        Parser parser(lexer);
        ASTNode ast = parser.parseProgram();

        auto errors = parser.getErrors();
        if (!errors.empty()) {
            for (const auto& e : errors) {
                std::cerr << "  " << e << std::endl;
            }
            continue;
        }

        Value result = evaluator.eval(ast, globalEnv);
        if (result.type == ValType::Error) {
            std::cerr << "Error: " << result.errMsg << std::endl;
        } else if (result.type != ValType::Nil) {
            std::cout << result.inspect() << std::endl;
        }
    }
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    signal(SIGINT, handleSignal);
    signal(SIGTERM, handleSignal);

    globalEnv = std::make_shared<Env>();
    registerBuiltins(globalEnv);
    sysInit(argc, argv);

    if (argc < 2) {
        runRepl();
        CloscManager::instance().waitAll();
        CloscManager::instance().stopAll();
        Scheduler::instance().stop();
        return 0;
    }

    std::string cmd = argv[1];

    if (cmd == "run" && argc >= 3) {
        runFile(argv[2]);
    } else if (cmd == "repl") {
        runRepl();
    } else if (cmd == "version") {
        std::cout << "LAML-NG v1.0 — LLVM-Native Next Generation" << std::endl;
    } else {
        runFile(cmd);
    }

    CloscManager::instance().waitAll();
    CloscManager::instance().stopAll();
    Scheduler::instance().stop();
    return 0;
}
