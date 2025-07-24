#!/bin/bash

echo "🌐 LAML - Low Abstraction Machine Language Compiler Test Suite"
echo "============================================================="
echo

# Build the compiler
echo "📦 Building LAML compiler..."
go build -o laml main.go
if [ $? -eq 0 ]; then
    echo "✅ Compiler built successfully"
else
    echo "❌ Compiler build failed"
    exit 1
fi
echo

# Show version
echo "🔢 LAML Version:"
./laml version
echo

# Test all examples
echo "🧪 Testing LAML Examples:"
echo

echo "1. Hello World Example:"
echo "----------------------"
cat examples/hello.lm
echo
echo "Output:"
./laml run examples/hello.lm
echo

echo "2. Basic Features Example:"
echo "-------------------------"
echo "Output:"
./laml run examples/basic.lm
echo

echo "3. Functions Example:"
echo "--------------------"
echo "Output:"
./laml run examples/functions.lm
echo

# Test compilation
echo "⚙️ Testing Compilation:"
echo

echo "Compiling hello.lm to binary..."
./laml compile examples/hello.lm
if [ $? -eq 0 ]; then
    echo "✅ Compilation successful"
    
    echo "Running compiled binary:"
    ./examples/hello.bin
    echo
    
    echo "Binary size:"
    ls -lh examples/hello.bin
    echo
else
    echo "❌ Compilation failed"
fi

echo "🎉 LAML Test Suite Complete!"
echo "============================================"
echo "LAML is now ready for development!"
