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

#include "synthesized_class.h"

using namespace llvm;
using namespace std;

namespace
{
	constexpr char nl = '\n';
	
	std::string accessModifiers[] = {
		[synthesized_class::am_private] = "private",
		[synthesized_class::am_protected] = "protected",
		[synthesized_class::am_public] = "public",
	};
	
	template<typename T, size_t N>
	constexpr size_t countof(const T (&)[N])
	{
		return N;
	}
}

synthesized_class::synthesized_class(const string& name)
: name(name), constructor("", name)
{
}

void synthesized_class::print_declaration(raw_ostream &os) const
{
	access_modifier lastAccessModifier = am_private;
	os << "class " << name << nl;
	os << '{' << nl;
	for (const field& field : fields)
	{
		if (field.access != lastAccessModifier)
		{
			assert(field.access < countof(accessModifiers));
			os << nl << accessModifiers[field.access] << ':' << nl;
			lastAccessModifier = field.access;
		}
		os << '\t' << field.type << ' ' << field.name << ';' << nl;
	}
	os << nl;
	
	os << "public:" << nl;
	os << '\t' << name << "(llvm::LLVMContext& context, llvm::Module& module);" << nl;
	lastAccessModifier = am_public;
	
	for (const auto& pair : methods)
	{
		if (pair.first != lastAccessModifier)
		{
			assert(pair.first < countof(accessModifiers));
			os << nl << accessModifiers[pair.first] << ':' << nl;
			lastAccessModifier = pair.first;
		}
		os << '\t';
		pair.second.print_declaration(os);
		os << '\n';
	}
	os << "};" << nl;
}

void synthesized_class::print_definition(raw_ostream &os) const
{
	// constructor first
	os << name << "::" << name << "(";
	auto begin = constructor.params_begin();
	for (auto iter = constructor.params_begin(); iter != constructor.params_end(); iter++)
	{
		if (iter != begin)
		{
			os << ", ";
		}
		const auto& param = *iter;
		os << param.type << ' ' << param.name;
	}
	os << ")" << nl;
	
	string initLine;
	raw_string_ostream initLineStream(initLine);
	size_t outCount = 0;
	for (const field& field : fields)
	{
		if (field.initializer.size() > 0)
		{
			initLineStream << (outCount == 0 ? ':' : ',') << ' ' << field.name << '(' << field.initializer << ')';
			outCount++;
		}
	}
	initLineStream.flush();
	if (outCount != 0)
	{
		os << '\t' << initLine << nl;
	}
	
	os << '{' << nl;
	for (auto iter = constructor.lines_begin(); iter != constructor.lines_end(); iter++)
	{
		os << '\t' << *iter << nl;
	}
	os << '}' << nl;
	os << nl;
	
	// and then methods
	for (const auto& pair : methods)
	{
		pair.second.print_definition(os, name);
		os << nl;
	}
}
