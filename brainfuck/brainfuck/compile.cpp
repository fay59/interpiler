//
//  compile.cpp
//  brainfuck
//
//  Created by Félix on 2015-04-16.
//  Copyright (c) 2015 Félix Cloutier. All rights reserved.
//

#include <iostream>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/raw_os_ostream.h>

#include "compile.h"
#include "exec.h"
#include "irgen.h"

using namespace brainfuck;
using namespace llvm;
using namespace std;

namespace
{
	unique_ptr<Module> compile(LLVMContext& context, scope& program)
	{
		to_executable_visitor to_sequence;
		to_sequence.visit(program);
		auto sequence = to_sequence.code();
		
		Type* int32Ty = Type::getInt32Ty(context);
		Type* int64Ty = Type::getInt64Ty(context);
		
		auto module = make_unique<Module>("a.out", context);
		auto dataLayout = DataLayout(module.get());
		auto mainType = FunctionType::get(int32Ty, ArrayRef<llvm::Type*>(), false);
		auto bfMainType = FunctionType::get(Type::getVoidTy(context), ArrayRef<llvm::Type*>(), false);
		
		ir_generator irgen(context, *module);
		irgen.start_function(*bfMainType, "bf_main");
		
		Type* bfStateType = irgen.type_by_name("struct.brainfuck::state");
		Value* stateAddress = irgen.builder.CreateAlloca(bfStateType);
		irgen.builder.CreateMemSet(stateAddress, ConstantInt::get(Type::getInt8Ty(context), 0), dataLayout.getTypeAllocSize(bfStateType), dataLayout.getABITypeAlignment(bfStateType));
		
		vector<BasicBlock*> blockByStatement;
		execute(sequence, [&irgen, &blockByStatement, stateAddress, int64Ty](state*, executable_statement statement)
		{
			uint64_t coerced = *reinterpret_cast<const uint64_t*>(&statement);
			Value* v = ConstantInt::get(int64Ty, coerced);
			
			string blockName;
			raw_string_ostream(blockName) << "stmt" << blockByStatement.size();
			blockByStatement.push_back(irgen.start_block(blockName));
			switch (statement.opcode)
			{
				case opcode::dec_ptr: irgen.dec_ptr(stateAddress, v); break;
				case opcode::dec_value: irgen.dec_value(stateAddress, v); break;
				case opcode::inc_ptr: irgen.inc_ptr(stateAddress, v); break;
				case opcode::inc_value: irgen.inc_value(stateAddress, v); break;
				case opcode::input: irgen.input(stateAddress, v); break;
				case opcode::loop_enter: irgen.loop_enter(stateAddress, v); break;
				case opcode::loop_exit: irgen.loop_exit(stateAddress, v); break;
				case opcode::output: irgen.output(stateAddress, v); break;
			}
		});
		
		blockByStatement.push_back(irgen.start_block());
		Function* bfMain = irgen.end_function();
		
		// Replace calls to go_to with branches. This would be better done with a custom pass, but what the hell.
		for (BasicBlock& bb : bfMain->getBasicBlockList())
		{
			Instruction* deleteFrom = nullptr;
			BasicBlock* jumpTarget = nullptr;
			for (Instruction& i : bb.getInstList())
			{
				if (CallInst* call = dyn_cast<CallInst>(&i))
				{
					// Assume no indirect calls
					if (call->getCalledFunction()->getName() == "go_to")
					{
						// Assume no indirect jumps
						uint64_t target = cast<ConstantInt>(call->getOperand(1))->getValue().getLimitedValue();
						jumpTarget = blockByStatement[target];
						assert(jumpTarget);
						deleteFrom = call;
						break;
					}
				}
			}
			
			if (deleteFrom != nullptr)
			{
				// erase everything from the jump to the end of the block since it's unreachable
				auto iter = deleteFrom->eraseFromParent();
				while (iter != bb.end())
				{
					iter = iter->eraseFromParent();
				}
				
				// terminate with jump
				irgen.builder.SetInsertPoint(&bb);
				irgen.builder.CreateBr(jumpTarget);
			}
		}
		
		// call from dummy ABI-compliant main
		Function* abiMain = Function::Create(mainType, GlobalValue::ExternalLinkage, "main", module.get());
		BasicBlock* abiBB = BasicBlock::Create(context, "", abiMain);
		CallInst::Create(bfMain, ArrayRef<Value*>(), "", abiBB);
		ReturnInst::Create(context, ConstantInt::get(int32Ty, 0), abiBB);
		
		return module;
	}
	
	string shell_escape(const string& that)
	{
		string result;
		raw_string_ostream resultStream(result);
		
		resultStream << '\'';
		string::size_type lastIndex = 0;
		string::size_type quote = that.find_first_of('\'');
		while (lastIndex != string::npos)
		{
			resultStream << that.substr(lastIndex, quote - lastIndex);
			if (quote != string::npos)
			{
				// well that's one ugly escape sequence
				resultStream << "'\\''";
				quote++;
			}
			lastIndex = quote;
			quote = that.find_first_of('\'', lastIndex);
		}
		resultStream << '\'';
		resultStream.flush();
		return result;
	}
	
	struct delete_file
	{
		string path;
		
		delete_file(const string& path) : path(path)
		{
		}
		
		~delete_file()
		{
			sys::fs::remove(path);
		}
	};
}

int compile_program(scope& program, const std::string& out_file)
{
	// cheap trick: shove IR into temp file, run clang on it
	int fd;
	SmallVector<char, 40> path;
	if (error_code error = sys::fs::createTemporaryFile("bf", "ll", fd, path))
	{
		cerr << "can't create temporary file " << path.data() << ": " << error.message() << endl;
		return 3;
	}
	
	delete_file deleteLater(path.data());
	
	raw_fd_ostream output(fd, true);
	
	LLVMContext context;
	auto module = compile(context, program);
	module->print(output, nullptr);
	
	string command;
	raw_string_ostream(command) << "clang -O3 -o " << shell_escape(out_file) << " " << shell_escape(path.data());
	cout << command << endl;
	return system(command.c_str());
}

int dump_ir(scope& program)
{
	LLVMContext context;
	auto module = compile(context, program);
	
	// TODO: run optimization passes
	// (right now this is a little awkward as LLVM is in the middle of transitioning its pass management system)
	raw_os_ostream rout(cout);
	module->print(rout, nullptr);
	return 0;
}
