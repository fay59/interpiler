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

#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>

#include <unistd.h>

#include "compile.h"
#include "exec.h"
#include "parse.h"
#include "print.h"

using namespace std;

namespace
{
	string program_name;

	enum class mode
	{
		print,
		execute,
		compile,
		dump_ir,
	};

	class options
	{
		fstream maybe_in_file;
		
		options()
		{
			mode = mode::execute;
		}
		
	public:
		mode mode;
		string out_file;
		
		options(const options&) = delete;
		
		options(options&& that)
		: maybe_in_file(move(that.maybe_in_file)), out_file(move(that.out_file))
		{
			mode = that.mode;
		}
		
		static unique_ptr<options> parse(int argc, const char** argv)
		{
			options result;
			
			const char* optionString = "c:epS";
			int c = getopt(argc, const_cast<char**>(argv), optionString);
			while (c != -1)
			{
				switch (c)
				{
					case 'e': result.mode = mode::execute; break;
					case 'p': result.mode = mode::print; break;
					case 'S': result.mode = mode::dump_ir; break;
						
					case 'c':
						result.mode = mode::compile;
						result.out_file = optarg;
						break;
						
					case '?':
						if (optopt != 'h')
						{
							cerr << program_name << ": unknown option -";
							if (isprint(optopt))
							{
								cerr << char(optopt);
							}
							else
							{
								cerr << "\\x" << setfill('0') << setw(2) << hex << optopt;
							}
							cerr << endl;
						}
						return nullptr;
				}
				c = getopt(argc, const_cast<char**>(argv), optionString);
			}
			
			for (int i = optind; i < argc; i++)
			{
				if (result.maybe_in_file.is_open())
				{
					cerr << program_name << ": multiple input files" << endl;
					return nullptr;
				}
				
				result.maybe_in_file.open(argv[i], ios_base::in);
				if (!result.maybe_in_file.is_open())
				{
					cerr << program_name << ": can't open " << argv[i] << endl;
					return nullptr;
				}
			}
			
			return make_unique<options>(move(result));
		}
		
		istream& input()
		{
			return maybe_in_file.is_open() ? maybe_in_file : cin;
		}
	};
	
	int print_program(brainfuck::scope& program)
	{
		brainfuck::print_visitor printer(cout);
		printer.visit(program);
		return 0;
	}
	
	int execute_program(brainfuck::scope& program)
	{
		brainfuck::to_executable_visitor to_sequence;
		to_sequence.visit(program);
		
		auto sequence = to_sequence.code();
		brainfuck::execute(sequence, brainfuck::execute_one);
		return 0;
	}
}

int main(int argc, const char * argv[])
{
	string argv0 = argv[0];
	program_name = argv0.substr(argv0.find_last_of('/') + 1);
	
	if (auto opts = options::parse(argc, argv))
	{
		istream_iterator<char> program_begin(opts->input());
		istream_iterator<char> program_end;
		
		if (auto program = brainfuck::scope::parse(program_begin, program_end))
		{
			try
			{
				switch (opts->mode)
				{
					case mode::compile: return compile_program(*program, opts->out_file);
					case mode::dump_ir: return dump_ir(*program);
					case mode::execute: return execute_program(*program);
					case mode::print: return print_program(*program);
					default: break;
				}
			}
			catch (exception& ex)
			{
				cerr << program_name << ": failed to complete action: " << ex.what() << endl;
			}
		}
		else
		{
			cerr << program_name << ": could not parse program" << endl;
			cerr << "Check that brackets match." << endl;
			return 2;
		}
	}
	
	cerr << "usage: " << program_name << " [-c output_file | -e | -p | -S] [input-file]" << endl;
	cerr << "       " << program_name << " -c <outout file>: compile source to binary" << endl;
	cerr << "       " << program_name << " -S: dump LLVM IR to stdout" << endl;
	cerr << "       " << program_name << " -e: execute source" << endl;
	cerr << "       " << program_name << " -p: print source back (removing no-ops)" << endl;
	cerr << "The last specified mode takes precedence." << endl;
	cerr << "Execution is the default mode." << endl;
	cerr << "Stdin is used if no input file is specified." << endl;
	return 1;
}
