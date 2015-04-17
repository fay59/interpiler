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

#ifndef interpiler_output_model_h
#define interpiler_output_model_h

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/GlobalVariable.h>

class output_model
{
	llvm::LLVMContext& context;
	llvm::Module& module;
	std::vector<llvm::Type*> types;
	std::vector<llvm::GlobalVariable*> globals;
	
	llvm::Function* currentFunction;
	llvm::BasicBlock* lastBlock;
};

#endif
