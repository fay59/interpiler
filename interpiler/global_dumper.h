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

#ifndef __interpiler__global_dumper__
#define __interpiler__global_dumper__

#include <cstdint>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <unordered_map>
#include <string>

#include "synthesized_method.h"
#include "type_dumper.h"

class global_dumper
{
	type_dumper& types;
	synthesized_method& method;
	std::string& resizeLine;
	std::unordered_map<llvm::GlobalObject*, size_t> var_indices;
	
	std::unique_ptr<llvm::raw_ostream> ostream;
	llvm::raw_ostream& nl();
	llvm::raw_ostream& on_index(size_t index);
	llvm::raw_ostream& insert(llvm::GlobalObject* var);
	void make_global(llvm::GlobalVariable* var);
	void make_global(llvm::Function* fn);
	
public:
	explicit global_dumper(synthesized_class& klass, type_dumper& types);
	
	size_t accumulate(llvm::GlobalVariable* variable);
	size_t accumulate(llvm::Function* func);
};

#endif /* defined(__interpiler__global_dumper__) */
