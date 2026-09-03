#include "rt_bridge.h"
#include "rt_loop.h"
#include "rt_room.h"
#include "rt_timer.h"
#include "rt_json.h"
#include "scheduler.h"
#include "lexer.h"
#include "parser.h"
#include <sys/resource.h>
#include <fstream>
#include <sstream>
#include "env.h"
#include "value.h"
#include "evaluator.h"
#include <mutex>
#include <unordered_map>
#include <memory>
#include <atomic>
#include <chrono>
#include <chrono>

namespace {
std::shared_ptr<Env> gGlobal;
std::mutex hmtx;
// event -> func Value (holds FuncData with AST body + closure)
std::unordered_map<std::string, Value> handlers;
// timer id -> func Value
std::mutex tmtx;
std::unordered_map<int64_t, Value> timerFuncs;

Value err(const std::string& m) { return Value::makeError(m); }

Value getHandler(const std::string& ev) {
    std::lock_guard<std::mutex> l(hmtx);
    auto it = handlers.find(ev);
    if (it == handlers.end()) return Value();
    return it->second;
}
bool hasAnyWsHandler() {
    std::lock_guard<std::mutex> l(hmtx);
    return handlers.count("message") || handlers.count("connect");
}

// Build LAML req obj: {method,path,query,body,fd}
Value makeReqObj(const RtLoop::HttpRequestSnap& s, int fd) {
    auto e = std::make_shared<Env>();
    e->declare("method", Value::makeString(s.method));
    e->declare("path", Value::makeString(s.path));
    e->declare("query", Value::makeString(s.query));
    e->declare("body", Value::makeString(s.body));
    e->declare("fd", Value::makeInt(fd));
    return Value::makeObj(e);
}

// Wrap handler call in try-like isolation: Error => string fallback, never throw.
Value callIsolated(const Value& h, std::vector<Value> args) {
    if (h.type != ValType::Func || !gGlobal) return Value::makeError("no handler");
    Evaluator ev(gGlobal); // thread_local current set in ctor
    Value r = ev.applyFunc(h, args);
    r.returning = false; r.breaking = false; r.continuing = false;
    return r;
}

RtLoop::HttpResponse dispatchRequest(RtLoop::HttpRequestSnap snap, int fd) {
    RtLoop::HttpResponse out;
    Value h = getHandler("request");
    if (h.type != ValType::Func || !gGlobal) {
        out.body = "LAML v4.1 rt: no on(\"request\") handler.\n";
        return out;
    }
    Value reqObj = makeReqObj(snap, fd);
    Value r = callIsolated(h, {reqObj});
    if (r.type == ValType::Error) {
        out.body = "handler error: " + r.errMsg + "\n";
        return out;
    }
    if (r.type == ValType::Obj && r.objVal) {
        auto sse = r.objVal->env->get("sse");
        if (sse && sse->isTruthy()) {
            RtLoop::instance().sendRaw(fd,
                "HTTP/1.1 200 OK\r\nContent-Type: text/event-stream\r\n"
                "Cache-Control: no-cache\r\nConnection: keep-alive\r\n\r\n");
            RtLoop::instance().markSse(fd);
            out.sse = true;
            return out;
        }
        auto b = r.objVal->env->get("body");
        if (b) {
            if (b->type == ValType::String) out.body = b->strVal;
            else out.body = b->inspect();
            return out;
        }
        return out; // {} -> empty 200
    }
    if (r.type == ValType::String) { out.body = r.strVal; return out; }
    if (r.type == ValType::Nil) return out;
    out.body = r.inspect();
    return out;
}

void dispatchConnect(int fd, const std::string& path) {
    Value h = getHandler("connect");
    if (h.type != ValType::Func || !gGlobal) return;
    callIsolated(h, {Value::makeInt(fd), Value::makeString(path)});
}

void dispatchMessage(int fd, std::string payload, uint8_t opcode) {
    (void)opcode; // binary vs text both surface as string bytes in M2
    Value h = getHandler("message");
    if (h.type != ValType::Func || !gGlobal) return;
    callIsolated(h, {Value::makeInt(fd), Value::makeString(std::move(payload))});
}

void dispatchClose(int fd) {
    RoomBus::instance().leaveAll(fd); // always clean rooms, even without handler
    Value h = getHandler("close");
    if (h.type != ValType::Func || !gGlobal) return;
    callIsolated(h, {Value::makeInt(fd)});
}

std::string toSendStr(const Value& v) {
    if (v.type == ValType::String) return v.strVal;
    return v.inspect();
}

Value bServe(const std::vector<Value>& args) {
    if (args.empty() || args[0].type != ValType::Int)
        return err("serve: expected serve(port[, opts])");
    int port = (int)args[0].intVal;
    int workers = 4;
    if (args.size() >= 2 && args[1].type == ValType::Obj && args[1].objVal) {
        auto w = args[1].objVal->env->get("workers");
        if (w && w->type == ValType::Int && w->intVal > 0 && w->intVal <= 32)
            workers = (int)w->intVal;
        auto st = args[1].objVal->env->get("stats");
        if (st && st->isTruthy())
            RtLoop::instance().statsEndpoint_ = true;
    }
    RtLoop::instance().onRequest = dispatchRequest;
    RtLoop::instance().onConnect = dispatchConnect;
    RtLoop::instance().onMessage = dispatchMessage;
    RtLoop::instance().onClose = dispatchClose;
    RtLoop::instance().hasWsHandler = hasAnyWsHandler;
    TimerWheel::instance().start();
    if (!RtLoop::instance().start(port, workers))
        return err("serve: bind/listen failed on port " + std::to_string(port));
    return NIL;
}

Value bOn(const std::vector<Value>& args) {
    if (args.size() < 2 || args[0].type != ValType::String || args[1].type != ValType::Func)
        return err("on: expected on(event, func)");
    { std::lock_guard<std::mutex> l(hmtx); handlers[args[0].strVal] = args[1]; }
    RtLoop::instance().onRequest = dispatchRequest;
    RtLoop::instance().onConnect = dispatchConnect;
    RtLoop::instance().onMessage = dispatchMessage;
    RtLoop::instance().onClose = dispatchClose;
    RtLoop::instance().hasWsHandler = hasAnyWsHandler;
    return NIL;
}

Value bStats(const std::vector<Value>&) {
    auto s = RtLoop::instance().stats();
    auto e = std::make_shared<Env>();
    e->declare("conns", Value::makeInt((int64_t)s.active));
    e->declare("accepts", Value::makeInt((int64_t)s.accepts));
    e->declare("reqs", Value::makeInt((int64_t)s.reqs));
    e->declare("wsMsgs", Value::makeInt((int64_t)s.wsMsgs));
    e->declare("dropped", Value::makeInt((int64_t)s.dropped));
    e->declare("rooms", Value::makeInt((int64_t)RoomBus::instance().roomCount()));
    int64_t rssMB = 0;
    struct rusage ru;
    if (getrusage(RUSAGE_SELF, &ru) == 0) rssMB = (int64_t)(ru.ru_maxrss / 1024); // Linux KB -> MB
    e->declare("rssMB", Value::makeInt(rssMB));
    e->declare("rps", Value::makeInt((int64_t)s.rpsAvg));
    e->declare("p50Ms", Value::makeInt((int64_t)s.p50Ms));
    e->declare("p99Ms", Value::makeInt((int64_t)s.p99Ms));
    e->declare("latCount", Value::makeInt((int64_t)s.latCount));
    return Value::makeObj(e);
}

Value bClose(const std::vector<Value>& args) {
    if (args.empty() || args[0].type != ValType::Int)
        return err("close: expected close(fd)");
    RtLoop::instance().closeFd((int)args[0].intVal);
    return NIL;
}

Value bSend(const std::vector<Value>& args) {
    if (args.size() < 2 || args[0].type != ValType::Int)
        return err("send: expected send(fd, msg)");
    RtLoop::instance().sendSmart((int)args[0].intVal, toSendStr(args[1]));
    return NIL;
}

Value bJoin(const std::vector<Value>& args) {
    if (args.size() < 2 || args[0].type != ValType::Int || args[1].type != ValType::String)
        return err("joinRoom: expected joinRoom(fd, room)");
    RoomBus::instance().join((int)args[0].intVal, args[1].strVal);
    return NIL;
}
Value bLeave(const std::vector<Value>& args) {
    if (args.size() < 2 || args[0].type != ValType::Int || args[1].type != ValType::String)
        return err("leaveRoom: expected leaveRoom(fd, room)");
    RoomBus::instance().leave((int)args[0].intVal, args[1].strVal);
    return NIL;
}
Value bLeaveAll(const std::vector<Value>& args) {
    if (args.empty() || args[0].type != ValType::Int)
        return err("leaveAll: expected leaveAll(fd)");
    RoomBus::instance().leaveAll((int)args[0].intVal);
    return NIL;
}
Value bMembers(const std::vector<Value>& args) {
    if (args.empty() || args[0].type != ValType::String)
        return err("members: expected members(room)");
    auto fds = RoomBus::instance().members(args[0].strVal);
    std::vector<Value> out;
    for (int fd : fds) out.push_back(Value::makeInt(fd));
    return Value::makeArray(out);
}
Value bBroadcast(const std::vector<Value>& args) {
    if (args.size() < 2 || args[0].type != ValType::String)
        return err("broadcast: expected broadcast(room, msg[, exceptFd])");
    std::string payload = toSendStr(args[1]);
    int except = -1;
    if (args.size() >= 3 && args[2].type == ValType::Int) except = (int)args[2].intVal;
    int n = RoomBus::instance().broadcast(args[0].strVal, [&](int fd){
        RtLoop::instance().sendSmart(fd, payload);
    }, except);
    return Value::makeInt(n);
}

Value bSetTimeout(const std::vector<Value>& args) {
    if (args.size() < 2 || args[0].type != ValType::Func || args[1].type != ValType::Int)
        return err("setTimeout: expected setTimeout(func, ms)");
    Value fn = args[0];
    int64_t ms = args[1].intVal;
    int64_t id = TimerWheel::instance().add(ms, 0, [fn]() mutable {
        callIsolated(fn, {});
    });
    std::lock_guard<std::mutex> l(tmtx);
    timerFuncs[id] = fn;
    return Value::makeInt(id);
}
Value bSetInterval(const std::vector<Value>& args) {
    if (args.size() < 2 || args[0].type != ValType::Func || args[1].type != ValType::Int)
        return err("setInterval: expected setInterval(func, ms)");
    Value fn = args[0];
    int64_t ms = args[1].intVal;
    if (ms < 10) ms = 10;
    int64_t id = TimerWheel::instance().add(ms, ms, [fn]() mutable {
        callIsolated(fn, {});
    });
    std::lock_guard<std::mutex> l(tmtx);
    timerFuncs[id] = fn;
    return Value::makeInt(id);
}
Value bCancelTimer(const std::vector<Value>& args) {
    if (args.empty() || args[0].type != ValType::Int)
        return err("cancelTimer: expected cancelTimer(id)");
    TimerWheel::instance().cancel(args[0].intVal);
    std::lock_guard<std::mutex> l(tmtx);
    timerFuncs.erase(args[0].intVal);
    return NIL;
}

// Reloaded files' ASTs must outlive the handlers they define (FuncData holds
// a raw body pointer). Same pattern as sys_builtins gImportedAsts.
static std::vector<std::unique_ptr<ASTNode>> gReloadedAsts;

Value bReload(const std::vector<Value>& args) {
    if (args.empty() || args[0].type != ValType::String)
        return err("reload: expected reload(path)");
    if (!gGlobal) return err("reload: no evaluator");
    std::ifstream f(args[0].strVal);
    if (!f.is_open()) return err("reload: cannot open " + args[0].strVal);
    std::stringstream ss;
    ss << f.rdbuf();
    Lexer lexer(ss.str());
    Parser parser(lexer);
    auto ast = std::make_unique<ASTNode>(parser.parseProgram());
    auto errors = parser.getErrors();
    if (!errors.empty())
        return err("reload: parse error in " + args[0].strVal + ": " + errors[0]);
    // Snapshot handlers: all-or-nothing swap. on() calls inside the file
    // mutate the live map one by one; on failure restore the snapshot.
    std::unordered_map<std::string, Value> snap;
    { std::lock_guard<std::mutex> l(hmtx); snap = handlers; }
    Evaluator ev(gGlobal);
    Value r = ev.eval(*ast, gGlobal);
    if (r.type == ValType::Error) {
        std::lock_guard<std::mutex> l(hmtx);
        handlers = std::move(snap);
        return r;
    }
    gReloadedAsts.push_back(std::move(ast));
    return NIL;
}

Value bAsync(const std::vector<Value>& args) {
    if (args.empty() || args[0].type != ValType::Func)
        return err("async: expected async(func)");
    Value fn = args[0];
    uint64_t n = (uint64_t)std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    std::string id = "async:" + std::to_string(n);
    if (Scheduler::instance().workerCount() == 0) {
        // No pool (REPL without serve): run inline, stash result.
        Value r = callIsolated(fn, {});
        Scheduler::instance().markDone(id, r);
        return Value::makeString(id);
    }
    auto t = std::make_shared<Task>();
    t->name = id;
    t->fn = [fn]() mutable { return callIsolated(fn, {}); };
    Scheduler::instance().submit(t);
    return Value::makeString(id);
}
Value bWaitFor(const std::vector<Value>& args) {
    if (args.empty() || args[0].type != ValType::String)
        return err("waitFor: expected waitFor(asyncId)");
    Scheduler::instance().waitFor(args[0].strVal);
    auto r = Scheduler::instance().getResult(args[0].strVal);
    if (!r) return err("waitFor: no such task");
    return *r;
}

Value bJsonParse(const std::vector<Value>& args) {
    if (args.empty() || args[0].type != ValType::String)
        return err("jsonParse: expected jsonParse(string)");
    return RtJson::parse(args[0].strVal);
}
Value bJsonStringify(const std::vector<Value>& args) {
    if (args.empty()) return Value::makeString("null");
    return RtJson::stringify(args[0]);
}
Value bSseSend(const std::vector<Value>& args) {
    if (args.size() < 2 || args[0].type != ValType::Int)
        return err("sseSend: expected sseSend(fd, data)");
    std::string data = toSendStr(args[1]);
    RtLoop::instance().sendRaw((int)args[0].intVal, "data: " + data + "\n\n");
    return NIL;
}
} // namespace

void rtBridgeSetGlobal(std::shared_ptr<Env> env) { gGlobal = std::move(env); }

void registerRtBuiltins(std::shared_ptr<Env> env) {
    rtBridgeSetGlobal(env);
    auto reg = [&](const std::string& n, auto fn) { env->set(n, Value::makeBuiltin({fn})); };
    reg("serve", bServe);
    reg("on", bOn);
    reg("stats", bStats);
    reg("close", bClose);
    reg("send", bSend);
    reg("joinRoom", bJoin);
    reg("leaveRoom", bLeave);
    reg("leaveAll", bLeaveAll);
    reg("members", bMembers);
    reg("broadcast", bBroadcast);
    reg("setTimeout", bSetTimeout);
    reg("setInterval", bSetInterval);
    reg("cancelTimer", bCancelTimer);
    reg("jsonParse", bJsonParse);
    reg("jsonStringify", bJsonStringify);
    reg("sseSend", bSseSend);
    reg("async", bAsync);
    reg("waitFor", bWaitFor);
    reg("reload", bReload);
}
