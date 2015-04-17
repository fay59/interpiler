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

// build module with:
// clang++ --std=gnu++14 -stdlib=libc++ -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.10.sdk -I/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include/c++/v1 -DMAYBE_NORETURN='[[gnu::noreturn]]' -O3 -S -emit-llvm -o brainfuck.S execute_one.cpp

#include <cstdio>

#include "exec.h"

using namespace brainfuck;

namespace
{
	template<typename T, size_t N>
	constexpr size_t countof(T (&)[N])
	{
		return N;
	}
}

BF_OPCODE(dec_ptr)
{
	state->index = (state->index - 1) % countof(state->memory);
}

BF_OPCODE(dec_value)
{
	state->memory[state->index]--;
}

BF_OPCODE(inc_ptr)
{
	state->index = (state->index + 1) % countof(state->memory);
}

BF_OPCODE(inc_value)
{
	state->memory[state->index]++;
}

BF_OPCODE(input)
{
	state->memory[state->index] = getchar();
}

BF_OPCODE(output)
{
	putchar(state->memory[state->index]);
}

BF_OPCODE(loop_enter)
{
	if (state->memory[state->index] == 0)
	{
		go_to(state, statement.data);
	}
}

BF_OPCODE(loop_exit)
{
	if (state->memory[state->index] != 0)
	{
		go_to(state, statement.data);
	}
}
