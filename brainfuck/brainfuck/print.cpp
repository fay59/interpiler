// 
// Interpiler - turn an interpreter into a compiler
// Brainfuck example
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

#include "print.h"

using namespace std;

namespace brainfuck
{
	print_visitor::print_visitor(ostream& os)
	: os(os)
	{
	}
	
	void print_visitor::visit(inst &inst)
	{
		os << char(inst.opcode);
	}
	
	void print_visitor::visit(scope &scope)
	{
		for (auto& statement : scope.statements)
		{
			visit(*statement);
		}
	}
	
	void print_visitor::visit(loop& loop)
	{
		os << '[';
		loop.body->visit(*this);
		os << ']';
	}
}
