#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <memory>
#include <vector>
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
#include "rt_loop.h"
#include "rt_timer.h"
#include <thread>
#include <chrono>

static std::shared_ptr<Env> globalEnv;
static Evaluator* gEval = nullptr;

void handleSignal(int sig) {
    std::cerr << "\n[RT] Signal " << sig << " received. Draining..." << std::endl;
    RtLoop::instance().stop();
    // Fall through: main() wait loops check running() and exit cleanly.
    // Hard fallback if still stuck after 6s:
    static std::atomic<bool> fired{false};
    if (fired.exchange(true)) std::_Exit(sig);
    std::thread([sig]{ std::this_thread::sleep_for(std::chrono::seconds(6)); std::_Exit(sig); }).detach();
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
    // Kept process-wide: closc bodies + handler FuncData hold raw ASTNode*
    // into this tree, and closc threads outlive this frame (see gRunAsts).
    // Same pattern as sys_builtins gImportedAsts.
    static std::vector<std::unique_ptr<ASTNode>> gRunAsts;
    auto ast = std::make_unique<ASTNode>(parser.parseProgram());

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

    Value result = evaluator.eval(*ast, globalEnv);
    if (result.type == ValType::Error) {
        std::cerr << "Runtime error: " << result.errMsg << std::endl;
    }
    gRunAsts.push_back(std::move(ast));

    // v4.1: if serve() started the RT loop, block here until SIGTERM/SIGINT.
    // (Previously runFile returned instantly and main() tore down the loop.)
    while (RtLoop::instance().running()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    // Wait for CLOSC sections to be registered and started
    // Then wait for the main program (CLOSC runs in background)

    gEval = nullptr;
}

void runRepl() {
    std::cout << "LAML v4.1.0 — dynamic language for realtime servers" << std::endl;
    std::cout << "Enter code (Ctrl+D to exit):" << std::endl;

    Evaluator evaluator(globalEnv);
    std::string line;

    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;

        if (line.empty()) continue;

        Lexer lexer(line);
        Parser parser(lexer);
        // Same lifetime rule as runFile: closc/func bodies point into the AST.
        static std::vector<std::unique_ptr<ASTNode>> gReplAsts;
        auto ast = std::make_unique<ASTNode>(parser.parseProgram());

        auto errors = parser.getErrors();
        if (!errors.empty()) {
            for (const auto& e : errors) {
                std::cerr << "  " << e << std::endl;
            }
            continue;
        }

        Value result = evaluator.eval(*ast, globalEnv);
        gReplAsts.push_back(std::move(ast));
        if (result.type == ValType::Error) {
            std::cerr << "Error: " << result.errMsg << std::endl;
        } else if (result.type != ValType::Nil) {
            std::cout << result.inspect() << std::endl;
        }
    }
    std::cout << std::endl;
}

#define LAML_VERSION "4.1.0"

static void printVersion() {
    std::cout << "LAML v" LAML_VERSION " — dynamic language for realtime servers (C++20)" << std::endl;
}

static void printHelp() {
    printVersion();
    std::cout <<
        "\nUsage:\n"
        "  laml run <file.lm>   Run a LAML program\n"
        "  laml <file.lm>        Same as run\n"
        "  laml repl            Interactive shell\n"
        "  laml version         Print version\n"
        "  laml --version       Print version\n"
        "  laml --help          This help\n"
        "\nExamples:\n"
        "  laml run hello.lm\n"
        "  echo 'say \"hi\"' | laml repl\n"
        "\nDocs: https://naveensingh9999.github.io/LAML/learn.html\n";
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
    } else if (cmd == "version" || cmd == "--version" || cmd == "-v") {
        printVersion();
    } else if (cmd == "--help" || cmd == "-h" || cmd == "help") {
        printHelp();
    } else {
        runFile(cmd);
    }

    CloscManager::instance().waitAll();
    CloscManager::instance().stopAll();
    RtLoop::instance().stop();
    TimerWheel::instance().stop();
    Scheduler::instance().stop();
    return 0;
}
