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

#ifndef __interpiler__synthesized_method__
#define __interpiler__synthesized_method__

#include <deque>
#include <llvm/Support/raw_ostream.h>
#include <string>

class synthesized_method
{
public:
	typedef std::deque<std::string> string_vector;
	
	struct arg
	{
		std::string type;
		std::string name;
		std::string default_value;
	};
	
private:
	std::string return_type;
	std::string name;
	std::deque<arg> parameters;
	string_vector code;
	
	template<typename T>
	inline T& new_of(std::deque<T>& vec)
	{
		vec.emplace_back();
		return vec.back();
	}
	
public:
	synthesized_method(const std::string& returnType, const std::string& name);
	
	inline std::deque<arg>::const_iterator params_begin() const { return parameters.begin(); }
	inline std::deque<arg>::const_iterator params_end() const { return parameters.end(); }
	
	inline arg& new_param()
	{
		return new_of(parameters);
	}
	inline void new_param(const std::string& type, const std::string& name, const std::string& default_value = "")
	{
		arg& param = new_param();
		param.type = type;
		param.name = name;
		param.default_value = default_value;
	}
	
	inline string_vector::const_iterator lines_begin() const { return code.begin(); }
	inline string_vector::const_iterator lines_end() const { return code.end(); }
	inline std::string& nl()
	{
		return new_of(code);
	}
	
	void print_declaration(llvm::raw_ostream& os) const;
	void print_definition(llvm::raw_ostream& os, const std::string& ns_prefix = "") const;
};

#endif /* defined(__interpiler__synthesized_method__) */
