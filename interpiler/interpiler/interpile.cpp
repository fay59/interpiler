//
//  interpile.cpp
//  interpiler
//
//  Created by Félix on 2015-04-09.
//  Copyright (c) 2015 Félix Cloutier. All rights reserved.
//

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_os_ostream.h>

#include <iostream>
#include <memory>

#include "global_dumper.h"
#include "type_dumper.h"
#include "function_dumper.h"
#include "synthesized_class.h"

using namespace llvm;
using namespace std;

synthesized_class interpile(LLVMContext& context, unique_ptr<Module> module, const string& class_name);

synthesized_class interpile(LLVMContext& context, unique_ptr<Module> module, const string& class_name)
{
	synthesized_class outputClass(class_name);
	
	outputClass.ctor_param() = "llvm::LLVMContext& context";
	outputClass.ctor_param() = "llvm::Module& module";
	outputClass.new_field(synthesized_class::am_private, "llvm::LLVMContext&", "context", "context");
	outputClass.new_field(synthesized_class::am_private, "llvm::Module&", "module", "module");
	outputClass.new_field(synthesized_class::am_public, "llvm::Function*", "function", "nullptr");
	outputClass.new_field(synthesized_class::am_public, "llvm::BasicBlock*", "lastBlock", "nullptr");
	
	auto& startFuncMethod = outputClass.new_method(synthesized_class::am_public, "void", "start_function");
	startFuncMethod.new_param() = "llvm::FunctionType* type";
	startFuncMethod.new_param() = "const std::string& name";
	startFuncMethod.nl() = "assert(function == nullptr && \"unterminated function\");";
	startFuncMethod.nl() = "function = Function::Create(type, GlobalValue::ExternalLinkage, name, &module);";
	startFuncMethod.nl() = "lastBlock = BasicBlock::Create(context, \"\", function);";
	
	auto& endFuncMethod = outputClass.new_method(synthesized_class::am_public, "llvm::Function*", "end_function");
	endFuncMethod.nl() = "builder.CreateRetVoid();";
	endFuncMethod.nl() = "Function* fn = function;";
	endFuncMethod.nl() = "function = nullptr;";
	endFuncMethod.nl() = "return fn;";
	
	type_dumper types(outputClass);
	global_dumper globals(outputClass, types);
	function_dumper functions(context, outputClass, types, globals);
	
	for (Function& func : module->getFunctionList())
	{
		functions.accumulate(&func);
	}
	return outputClass;
}
