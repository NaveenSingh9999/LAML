#include "value.h"

std::string Value::inspect() const {
    switch (type) {
    case ValType::Nil: return "nil";
    case ValType::Int: return std::to_string(intVal);
    case ValType::Float: {
        std::ostringstream os;
        os << floatVal;
        return os.str();
    }
    case ValType::String: return strVal;
    case ValType::Bool: return boolVal ? "true" : "false";
    case ValType::Array: {
        std::string s = "[";
        if (arrVal) {
            for (size_t i = 0; i < arrVal->size(); i++) {
                if (i > 0) s += ", ";
                s += (*arrVal)[i].inspect();
            }
        }
        return s + "]";
    }
    case ValType::Func: return "<func>";
    case ValType::Builtin: return "<builtin>";
    case ValType::Obj: return "<obj>";
    case ValType::Closc: return "<closc:" + (closcVal ? closcVal->name : "?") + ">";
    case ValType::Error: return "ERROR: " + errMsg;
    }
    return "<?>";
}

bool Value::isTruthy() const {
    switch (type) {
    case ValType::Nil: return false;
    case ValType::Bool: return boolVal;
    case ValType::Int: return intVal != 0;
    case ValType::Float: return floatVal != 0.0;
    case ValType::String: return !strVal.empty();
    case ValType::Array: return arrVal && !arrVal->empty();
    default: return true;
    }
}

Value Value::copy() const {
    Value v;
    v.type = type;
    v.intVal = intVal;
    v.floatVal = floatVal;
    v.boolVal = boolVal;
    v.strVal = strVal;
    v.errMsg = errMsg;
    v.returning = returning;
    if (arrVal) v.arrVal = std::make_shared<std::vector<Value>>(*arrVal);
    if (funcVal) v.funcVal = std::make_unique<FuncData>(*funcVal);
    if (closcVal) v.closcVal = std::make_unique<CloscData>(*closcVal);
    if (builtinVal) v.builtinVal = std::make_unique<BuiltinData>(*builtinVal);
    if (objVal) v.objVal = std::make_unique<ObjData>(*objVal);
    return v;
}
