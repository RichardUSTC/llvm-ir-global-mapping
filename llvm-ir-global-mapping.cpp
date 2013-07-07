#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/CodeGen/MachineCodeInfo.h"
#include "llvm/Analysis/Verifier.h"
#include <iostream>

using namespace llvm;

/*
** Author: Richard Lee
** Note: this program is only tested with LLVM 3.3.
** LLVM is in development, and the API changes a lot over versions.
** If you compile it with another version, there might be compling errors.
*/
int foo(int x){
    return 2*x;
}

int value = 10;

int main()
{
    // Module Construction
    LLVMContext &context = llvm::getGlobalContext();
    Module *module = new Module("test", context);

    //declare 'value' and 'foo' in module 'test'
    GlobalVariable *v = cast<GlobalVariable>(module->getOrInsertGlobal("value", Type::getInt32Ty(context)));
    // prototype of foo is: int foo(int x)
    Function *f = cast<Function>(module->getOrInsertFunction("foo", Type::getInt32Ty(context),
                                   Type::getInt32Ty(context), NULL));

    //create a LLVM function 'bar'
    Function* bar = cast<Function>(module->getOrInsertFunction("bar", Type::getInt32Ty(context),NULL));

    //basic block construction
    BasicBlock* entry = BasicBlock::Create(context, "entry", bar);
    IRBuilder<> builder(entry);

    //read 'value'
    Value * v_IR = builder.CreateLoad(v);
    //call foo(value)
    Value * ret = builder.CreateCall(f, v_IR);
    //return return value of 'foo'
    builder.CreateRet(ret);

    //now bind global value and global function
    //create execution engine first
    InitializeNativeTarget();
    ExecutionEngine *ee = EngineBuilder(module).setEngineKind(EngineKind::JIT).create();

    //map global variable
    ee->addGlobalMapping(v, &value);

    //map global function
    ee->addGlobalMapping(f, (void *)foo);

    // JIT and run
    void *barAddr = ee->getPointerToFunction(bar);
    typedef int (*FuncType)();
    FuncType barFunc = (FuncType)barAddr;

    std::cout << barFunc() << std::endl;
    return 0;
}