//
//  compile.h
//  brainfuck
//
//  Created by Félix on 2015-04-16.
//  Copyright (c) 2015 Félix Cloutier. All rights reserved.
//

#ifndef __brainfuck__compile__
#define __brainfuck__compile__

#include "parse.h"
#include <string>

int compile_program(brainfuck::scope& program, const std::string& out_file);

#endif /* defined(__brainfuck__compile__) */
