// 
// Interpiler - turn an interpreter into a compiler
// Copyright (C) 2015  Félix Cloutier
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
	
	outputClass.ctor_param("llvm::LLVMContext&", "context");
	outputClass.ctor_param("llvm::Module&", "module");
	outputClass.new_field(synthesized_class::am_private, "llvm::LLVMContext&", "context", "context");
	outputClass.new_field(synthesized_class::am_private, "llvm::Module&", "module", "module");
	outputClass.new_field(synthesized_class::am_public, "llvm::Function*", "function", "nullptr");
	outputClass.new_field(synthesized_class::am_public, "llvm::BasicBlock*", "lastBlock", "nullptr");
	
	auto& startFuncMethod = outputClass.new_method(synthesized_class::am_public, "void", "start_function");
	startFuncMethod.new_param("llvm::FunctionType&", "type");
	startFuncMethod.new_param("const std::string&", "name");
	startFuncMethod.nl() = "assert(function == nullptr && \"unterminated function\");";
	startFuncMethod.nl() = "assert(type.getReturnType()->isVoidTy() && \"created functions must return void\");";
	startFuncMethod.nl() = "function = llvm::Function::Create(&type, llvm::GlobalValue::ExternalLinkage, name, &module);";
	startFuncMethod.nl() = "start_block();";
	
	auto& endFuncMethod = outputClass.new_method(synthesized_class::am_public, "llvm::Function*", "end_function");
	endFuncMethod.nl() = "builder.CreateRetVoid();";
	endFuncMethod.nl() = "auto fn = function;";
	endFuncMethod.nl() = "function = nullptr;";
	endFuncMethod.nl() = "return fn;";
	
	auto& startBlockMethod = outputClass.new_method(synthesized_class::am_public, "llvm::BasicBlock*", "start_block");
	startBlockMethod.new_param("const std::string&", "name", "\"\"");
	startBlockMethod.nl() = "lastBlock = llvm::BasicBlock::Create(context, name, function);";
	startBlockMethod.nl() = "if (builder.GetInsertBlock() != nullptr)";
	startBlockMethod.nl() = "{";
	startBlockMethod.nl() = "\tbuilder.CreateBr(lastBlock);";
	startBlockMethod.nl() = "}";
	startBlockMethod.nl() = "builder.SetInsertPoint(lastBlock);";
	startBlockMethod.nl() = "return lastBlock;";
	
	type_dumper types(outputClass);
	global_dumper globals(outputClass, types);
	function_dumper functions(context, outputClass, types, globals);
	
	for (Function& func : module->getFunctionList())
	{
		functions.accumulate(&func);
	}
	return outputClass;
}
