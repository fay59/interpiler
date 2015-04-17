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

#ifndef __interpiler__constant_dumper__
#define __interpiler__constant_dumper__

#include <llvm/IR/Constant.h>
#include <llvm/Support/raw_ostream.h>
#include <string>

#include "type_dumper.h"
#include "synthesized_method.h"

std::string dump_constant(synthesized_method& into, type_dumper& types, const std::string& prefix, llvm::Constant* constant);

llvm::raw_ostream& operator<<(llvm::raw_ostream& into, bool b);

#endif /* defined(__interpiler__constant_dumper__) */
