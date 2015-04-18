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

#include <llvm/IR/Function.h>

#include "dump_constant.h"
#include "global_dumper.h"

using namespace std;
using namespace llvm;

#define ENUM_STRING(x) [(size_t)x] = #x

namespace
{
	template<typename T, size_t N>
	[[gnu::always_inline]]
	constexpr size_t countof(const T (&)[N]) { return N; }
	
	string threadLocalModes[] = {
		ENUM_STRING(GlobalValue::NotThreadLocal),
		ENUM_STRING(GlobalValue::GeneralDynamicTLSModel),
		ENUM_STRING(GlobalValue::LocalDynamicTLSModel),
		ENUM_STRING(GlobalValue::InitialExecTLSModel),
		ENUM_STRING(GlobalValue::LocalExecTLSModel),
	};
	
	string linkageTypes[] = {
		ENUM_STRING(GlobalValue::ExternalLinkage),
		ENUM_STRING(GlobalValue::AvailableExternallyLinkage),
		ENUM_STRING(GlobalValue::LinkOnceAnyLinkage),
		ENUM_STRING(GlobalValue::LinkOnceODRLinkage),
		ENUM_STRING(GlobalValue::WeakAnyLinkage),
		ENUM_STRING(GlobalValue::WeakODRLinkage),
		ENUM_STRING(GlobalValue::AppendingLinkage),
		ENUM_STRING(GlobalValue::InternalLinkage),
		ENUM_STRING(GlobalValue::PrivateLinkage),
		ENUM_STRING(GlobalValue::ExternalWeakLinkage),
		ENUM_STRING(GlobalValue::CommonLinkage),
	};
}

raw_ostream& global_dumper::nl()
{
	ostream.reset(new raw_string_ostream(method.nl()));
	return *ostream;
}

raw_ostream& global_dumper::on_index(size_t index)
{
	return nl() << "globals[" << index << "]";
}

raw_ostream& global_dumper::insert(GlobalObject* var)
{
	size_t index = var_indices.size();
	var_indices[var] = index;
	
	resizeLine.clear();
	raw_string_ostream(resizeLine) << "globals.resize(" << var_indices.size() << ");";
	return on_index(index) << " = ";
}

void global_dumper::make_global(GlobalVariable *var)
{
	assert((size_t)var->getThreadLocalMode() < countof(threadLocalModes));
	assert((size_t)var->getLinkage() < countof(linkageTypes));
	
	size_t typeIndex = types.accumulate(var->getType()->getPointerElementType());
	size_t varIndex = var_indices.size();
	
	string initializer = "nullptr";
	if (var->hasInitializer())
	{
		string prefix;
		raw_string_ostream(prefix) << "var" << varIndex << '_';
		initializer = dump_constant(method, types, prefix, var->getInitializer());
	}
	
	string varName;
	(raw_string_ostream(varName) << "cv" << var_indices.size());
	auto& declarationLine = nl() << "GlobalVariable* " << varName << " = ";
	declarationLine << "new GlobalVariable("
		<< "module, " // Module&
		<< "types[" << typeIndex << "], " // Type*
		<< var->isConstant() << ", " // bool isConstant
		<< linkageTypes[var->getLinkage()] << ", " // LinkageType
		<< initializer << ", "; // Constant* initializer
	declarationLine << '"';
	declarationLine.write_escaped(var->getName()); // const Twine& name
	declarationLine << "\", ";
	declarationLine << "nullptr, " // GlobalVariable* insertBefore
		<< threadLocalModes[(size_t)var->getThreadLocalMode()] << ", " // TLMode
		<< var->getType()->getAddressSpace() << ", " // addressSpace
		<< var->isExternallyInitialized()
		<< ");";
	
	if (var->hasUnnamedAddr())
	{
		nl() << varName << "->setUnnamedAddr(true);";
	}
	
	insert(var) << varName << ";";
	method.nl();
}

void global_dumper::make_global(Function* fn)
{
	assert((size_t)fn->getThreadLocalMode() < countof(threadLocalModes));
	assert((size_t)fn->getLinkage() < countof(linkageTypes));
	assert(fn->isDeclaration());
	
	size_t typeIndex = types.accumulate(fn->getFunctionType());
	auto& functionDeclarationLine = insert(fn);
	functionDeclarationLine << "Function::Create(cast<FunctionType>(types[" << typeIndex << "]), " << linkageTypes[fn->getLinkage()] << ", \"";
	functionDeclarationLine.write_escaped(fn->getName());
	functionDeclarationLine << "\", &module);";
}

global_dumper::global_dumper(synthesized_class& klass, type_dumper& types)
: types(types), method(klass.new_method(synthesized_class::am_private, "void", "make_globals")), resizeLine(method.nl())
{
	method.nl() = "using namespace llvm;";
	klass.new_field(synthesized_class::am_private, "std::vector<llvm::GlobalValue*>", "globals");
	klass.ctor_nl() = "make_globals();";
}

size_t global_dumper::accumulate(GlobalVariable *variable)
{
	auto iter = var_indices.find(variable);
	if (iter == var_indices.end())
	{
		make_global(variable);
		return var_indices[variable];
	}
	return iter->second;
}

size_t global_dumper::accumulate(Function *fn)
{
	auto iter = var_indices.find(fn);
	if (iter == var_indices.end())
	{
		make_global(fn);
		return var_indices[fn];
	}
	return iter->second;
}
