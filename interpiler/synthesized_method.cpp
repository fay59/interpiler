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

#include "synthesized_method.h"

using namespace llvm;
using namespace std;

synthesized_method::synthesized_method(const string& returnType, const string& name)
: return_type(returnType), name(name)
{
}

void synthesized_method::print_declaration(llvm::raw_ostream &os) const
{
	os << return_type << ' ' << name << '(';
	for (size_t i = 0; i < parameters.size(); i++)
	{
		if (i != 0)
		{
			os << ", ";
		}
		const auto& param = parameters[i];
		os << param.type << ' ' << param.name;
		if (param.default_value.size() > 0)
		{
			os << " = " << param.default_value;
		}
	}
	os << ");";
}

void synthesized_method::print_definition(llvm::raw_ostream &os, const std::string& ns_prefix) const
{
	os << return_type << ' ';
	if (ns_prefix.size() > 0)
	{
		os << ns_prefix << "::";
	}
	os << name << '(';
	
	for (size_t i = 0; i < parameters.size(); i++)
	{
		if (i != 0)
		{
			os << ", ";
		}
		const auto& param = parameters[i];
		os << param.type << ' ' << param.name;
	}
	os << ')' << '\n';
	os << '{' << '\n';
	for (const auto& line : code)
	{
		os << '\t' << line << '\n';
	}
	os << '}' << '\n';
}
