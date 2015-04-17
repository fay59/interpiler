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

#include "dump_constant.h"
#include "type_dumper.h"

#include <iomanip>
#include <sstream>
#include <vector>

using namespace std;
using namespace llvm;

namespace
{
	void param_types(raw_ostream& output, const string& varName, const vector<size_t>& indices)
	{
		output << "ArrayRef<Type*> " << varName << " = { ";
		for (size_t index : indices)
		{
			output << "types[" << index << "], ";
		}
		output << "};";
	}
}

raw_ostream& type_dumper::new_line()
{
	ostream.reset(new raw_string_ostream(method.nl()));
	return *ostream;
}

raw_ostream& type_dumper::insert(Type *type)
{
	size_t index = type_indices.size();
	type_indices[type] = index;
	
	resizeLine.clear();
	raw_string_ostream(resizeLine) << "types.resize(" << type_indices.size() << ");";
	
	return new_line() << "types[" << index << "] = ";
}

void type_dumper::make_dump(SequentialType* type, const string& typeName, uint64_t subclassData)
{
	size_t elementIndex = accumulate(type->getElementType());
	insert(type) << typeName << "Type::get(types[" << elementIndex << "], " << subclassData << ");";
}

void type_dumper::make_dump(Type* type, const string& typeMethod)
{
	insert(type) << "Type::get" << typeMethod << "Ty(context);";
}

void type_dumper::make_dump(IntegerType* type)
{
	insert(type) << "IntegerType::get(context, " << type->getBitWidth() << ");";
}

void type_dumper::make_dump(ArrayType* type)
{
	make_dump(type, "Array", type->getNumElements());
}

void type_dumper::make_dump(PointerType* type)
{
	make_dump(type, "Pointer", type->getAddressSpace());
}

void type_dumper::make_dump(VectorType* type)
{
	make_dump(type, "Vector", type->getNumElements());
}

void type_dumper::make_dump(FunctionType* type)
{
	vector<size_t> typeIndices;
	for (auto iter = type->param_begin(); iter != type->param_end(); iter++)
	{
		typeIndices.push_back(accumulate(*iter));
	}
	
	size_t self_index = type_indices.size();
	stringstream ss;
	ss << "func_type_params_" << self_index;
	string typeParamsVar = ss.str();
	param_types(new_line(), typeParamsVar, typeIndices);
	
	size_t returnTypeIndex = accumulate(type->getReturnType());
	insert(type) << "FunctionType::get(types[" << returnTypeIndex << "], " << typeParamsVar << ", " << type->isVarArg() << ");";
}

void type_dumper::make_dump(StructType* type)
{
	size_t self_index = type_indices.size();
	raw_ostream& typeDeclLine = new_line();
	typeDeclLine << "StructType* struct_" << self_index << " = StructType::create(context";
	if (type->hasName())
	{
		typeDeclLine << ", \"";
		typeDeclLine.write_escaped(type->getName());
		typeDeclLine << '"';
	}
	typeDeclLine << ");";
	insert(type) << "struct_" << self_index << ";";
	if (type->hasName())
	{
		raw_ostream& named = new_line();
		named << "struct_types[\"";
		named.write_escaped(type->getName());
		named << "\"] = struct_" << self_index << ";";
	}
	
	vector<size_t> typeIndices;
	for (auto iter = type->element_begin(); iter != type->element_end(); iter++)
	{
		typeIndices.push_back(accumulate(*iter));
	}
	
	stringstream ss;
	ss << "struct_type_params_" << self_index;
	string typeParamsVar = ss.str();
	param_types(new_line(), typeParamsVar, typeIndices);
	new_line() << "struct_" << self_index << "->setBody(" << typeParamsVar << ", " << type->isPacked() << ");";
}

void type_dumper::make_dump(Type* type)
{
	if (type->isVoidTy()) return make_dump(type, "Void");
	if (type->isLabelTy()) return make_dump(type, "Label");
	if (type->isHalfTy()) return make_dump(type, "Half");
	if (type->isFloatTy()) return make_dump(type, "Float");
	if (type->isDoubleTy()) return make_dump(type, "Double");
	if (type->isMetadataTy()) return make_dump(type, "Metadata");
	if (type->isX86_FP80Ty()) return make_dump(type, "X86_FP80");
	if (type->isFP128Ty()) return make_dump(type, "FP128");
	if (type->isPPC_FP128Ty()) return make_dump(type, "PPC_FP128");
	if (type->isX86_MMXTy()) return make_dump(type, "X86_MMX");
	
	if (auto t = dyn_cast<IntegerType>(type)) return make_dump(t);
	if (auto t = dyn_cast<ArrayType>(type)) return make_dump(t);
	if (auto t = dyn_cast<PointerType>(type)) return make_dump(t);
	if (auto t = dyn_cast<VectorType>(type)) return make_dump(t);
	if (auto t = dyn_cast<FunctionType>(type)) return make_dump(t);
	if (auto t = dyn_cast<StructType>(type)) return make_dump(t);
	
	throw invalid_argument("unknown type type");
}

type_dumper::type_dumper(synthesized_class& klass)
: method(klass.new_method(synthesized_class::am_private, "void", "make_types")), resizeLine(method.nl())
{
	method.nl() = "using namespace llvm;";
	klass.new_field(synthesized_class::am_private, "std::unordered_map<std::string, llvm::Type*>", "struct_types");
	klass.new_field(synthesized_class::am_private, "std::vector<llvm::Type*>", "types");
	klass.ctor_nl() = "make_types();";
	
	auto& type_by_name = klass.new_method(synthesized_class::am_public, "llvm::Type*", "type_by_name");
	type_by_name.new_param("const std::string&", "name");
	type_by_name.nl() = "auto iter = struct_types.find(name);";
	type_by_name.nl() = "assert(iter != struct_types.end());";
	type_by_name.nl() = "return iter->second;";
}

size_t type_dumper::accumulate(Type* type)
{
	auto iter = type_indices.find(type);
	if (iter == type_indices.end())
	{
		make_dump(type);
		return type_indices[type];
	}
	return iter->second;
}

size_t type_dumper::index_of(Type *type) const
{
	auto iter = type_indices.find(type);
	if (iter == type_indices.end())
	{
		return npos;
	}
	return iter->second;
}
