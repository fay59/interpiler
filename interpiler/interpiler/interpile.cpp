//
//  interpile.cpp
//  interpiler
//
//  Created by Félix on 2015-04-09.
//  Copyright (c) 2015 Félix Cloutier. All rights reserved.
//

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <memory>

#include "type_dumper.h"

using namespace llvm;
using namespace std;

void interpile(LLVMContext& context, unique_ptr<Module> module, ostream& header, ostream& impl);

void interpile(LLVMContext& context, unique_ptr<Module> module, const string& class_name, ostream& header, ostream& impl)
{
	type_dumper types;
	for (const GlobalVariable& var : module->getGlobalList())
	{
		types.accumulate(var.getType());
	}
	
	for (const Function& func : module->getFunctionList())
	{
		types.accumulate(func.getType());
	}
	
	cout << types.get_function_body("make_types");
}
