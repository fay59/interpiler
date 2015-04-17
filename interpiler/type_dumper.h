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

#ifndef __interpiler__type_dumper__
#define __interpiler__type_dumper__

#include <llvm/IR/DerivedTypes.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <unordered_map>

#include "synthesized_class.h"
#include "synthesized_method.h"

class type_dumper
{
	synthesized_method& method;
	std::string& resizeLine;
	std::unordered_map<llvm::Type*, size_t> type_indices;

	std::unique_ptr<llvm::raw_ostream> ostream;
	llvm::raw_ostream& new_line();
	llvm::raw_ostream& insert(llvm::Type* type);
	
	void make_dump(llvm::Type* type);
	void make_dump(llvm::SequentialType* type, const std::string& typeName, uint64_t subclassData);
	void make_dump(llvm::Type* type, const std::string& typeMethod);
	void make_dump(llvm::IntegerType* type);
	void make_dump(llvm::ArrayType* type);
	void make_dump(llvm::PointerType* type);
	void make_dump(llvm::VectorType* type);
	void make_dump(llvm::StructType* type);
	void make_dump(llvm::FunctionType* type);
	
public:
	static constexpr size_t npos = ~0;
	
	explicit type_dumper(synthesized_class& klass);
	
	size_t accumulate(llvm::Type* type);
	size_t index_of(llvm::Type* type) const;
};

#endif /* defined(__interpiler__type_dumper__) */
