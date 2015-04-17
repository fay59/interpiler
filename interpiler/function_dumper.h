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

#ifndef __interpiler__function_dumper__
#define __interpiler__function_dumper__

#include <llvm/IR/Function.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <string>
#include <unordered_set>

#include "global_dumper.h"
#include "type_dumper.h"
#include "synthesized_class.h"

class function_dumper
{
	type_dumper& types;
	global_dumper& globals;
	synthesized_class& klass;
	std::unordered_set<llvm::Function*> known_functions;
	
	llvm::LLVMContext& context;
	
	void make_function(llvm::Function* function, synthesized_method& output);
	
public:
	function_dumper(llvm::LLVMContext& context, synthesized_class& klass, type_dumper& types, global_dumper& globals);
	
	void accumulate(llvm::Function* function);
};

#endif /* defined(__interpiler__function_dumper__) */
