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

#include "parse.h"

using namespace std;

namespace brainfuck
{
	inst::inst(enum opcode o)
	: opcode(o)
	{
	}
	
	void inst::visit(statement_visitor &visitor)
	{
		visitor.visit(*this);
	}
	
	scope::scope(vector<unique_ptr<statement>> statements)
	: statements(move(statements))
	{
	}
	
	void scope::visit(statement_visitor &visitor)
	{
		visitor.visit(*this);
	}
	
	loop::loop(unique_ptr<scope> scope)
	: body(move(scope))
	{
	}
	
	void loop::visit(statement_visitor &visitor)
	{
		visitor.visit(*this);
	}
	
	void statement_visitor::visit(statement &statement)
	{
		statement.visit(*this);
	}
}
