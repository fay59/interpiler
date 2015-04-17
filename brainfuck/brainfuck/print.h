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

#ifndef __brainfuck__print__
#define __brainfuck__print__

#include <iostream>
#include "parse.h"

namespace brainfuck
{
	class print_visitor : public statement_visitor
	{
		std::ostream& os;
		
	public:
		explicit print_visitor(std::ostream& os);
		
		using statement_visitor::visit;
		virtual void visit(inst& inst) override;
		virtual void visit(scope& scope) override;
		virtual void visit(loop& loop) override;
	};
}

#endif /* defined(__brainfuck__print__) */
