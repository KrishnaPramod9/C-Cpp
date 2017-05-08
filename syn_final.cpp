#include <assert.h>
#include <algorithm>
#include <ctype.h>
#include <fstream>
#include <iostream>
#include<list>
#include <map>
#include <string>
#include <vector>

struct evl_token   // This is Structure for Evaluate Token 
{
	enum token_type{ NAME, NUMBER, SiNGLE };    
	token_type type;
	std::string str;
	int LineNo;
}; 

typedef std::list<evl_token> evl_tokens;

struct evl_wire
{
	std::string name;
	int width;
}; 

typedef std::list<evl_wire> evl_wires;   

struct evl_statement
{
	enum statement_type { MODULE, WiRE, COMPONENT, ENDMODULE };   
	statement_type type;
	evl_tokens tokens;
};

typedef std::list<evl_statement> evl_statements;

struct evl_pin
{
	std::string name;
	int bus_msb, bus_lsb;
}; 
typedef std::list<evl_pin> evl_pins;

struct evl_component
{
	int NoPins;
	std::string type, name;
	evl_pins pins;

}; // structure for evl_component

typedef std::list<evl_component>evl_components;

struct evl_module
{
	std::string name;
};
typedef std::list<evl_module>evl_modules;

bool pin_valid(evl_wires &wires, evl_pin &p) // Pin Validation 
{
	int valid = 0;
	for (evl_wires::const_iterator it = wires.begin(); it != wires.end(); ++it) {
		if ((it->name) == (p.name)) {
			valid = 1;
		}
	}
	if (!valid)
		return false;
	else
		return true;
}

bool pin_valid_bus(evl_wires &wires, evl_pin &p)
{
	int valid = 0;
	for (evl_wires::const_iterator it = wires.begin(); it != wires.end(); ++it) {
		if ((it->name) == (p.name)) {
			valid = 1;
			if (it->width > 1) {
				p.bus_msb = it->width - 1;
//				p.bus_lsb = 0;
			}
		}
	}
	if (!valid)
		return false;
	else
		return true;
}

bool tokens_extract_line(std::string line,    // Extracting Tokens line by line 
	int LineNo, evl_tokens &tokens)
{
	for (size_t i = 0; i < line.size();)
	{
		if (line[i] == '/')
		{
			++i;
			if ((i == line.size()) || (line[i] != '/'))
			{
				std::cerr << "LiNE " << LineNo
					<< ": a single / is not allowed" << std::endl;
				return false;
			}
			break; 
		}
		else if (isspace(line[i]))   
		{
			++i;
		}
		else if ((line[i] == '(') || (line[i] == ')')
			|| (line[i] == '[') || (line[i] == ']')
			|| (line[i] == ':') || (line[i] == ';')
			|| (line[i] == ','))
		{

			evl_token token;
			token.LineNo = LineNo;
			token.type = evl_token::SiNGLE;
			token.str = std::string(1, line[i]);
			tokens.push_back(token);
			++i;
		}
		else if (isalpha(line[i]) || (line[i] == '_')
			|| (line[i] == '\\') || (line[i] == '.'))
		{
			size_t name_begin = i;
			for (++i; i < line.size(); ++i)
			{
				if (!(isalpha(line[i]) || isdigit(line[i])
					|| (line[i] == '_') || (line[i] == '\\')
					|| (line[i] == '.')))
				{
					break; 	
				}
			}
			evl_token token;
			token.LineNo = LineNo;
			token.type = evl_token::NAME;
			token.str = line.substr(name_begin, i - name_begin);
			tokens.push_back(token);

		}
		else if (isdigit(line[i]))
		{
			size_t number_begin = i;
			for (++i; i<line.size(); ++i)
			{
				if (!isdigit(line[i]))
				{
					break; 	
				}
			}
			evl_token token;
			token.LineNo = LineNo;
			token.type = evl_token::NUMBER;
			token.str = line.substr(number_begin, i - number_begin);
			tokens.push_back(token);
		}
		else
		{
			std::cerr << "LiNE " << LineNo
				<< ": invalid character" << std::endl;
			return false;
		}
	}
	return true; 
}

bool tokens_extract_file(std::string file_name,  // Extracting Tokens From File 
	evl_tokens &tokens)
{
	std::ifstream input_file(file_name.c_str());
	if (!input_file)
	{
		std::cerr << "i can't read " << file_name << "." << std::endl;
		return false;
	}

	tokens.clear();

	std::string line;
	for (int LineNo = 1; std::getline(input_file, line); ++LineNo)
	{
		if (!tokens_extract_line(line, LineNo, tokens))
		{
			return false;
		}
	}
	return true;
}

bool tokens_store_t_file(std::string file_name,  // Store Tokens to Output file with extension .token
	const evl_tokens &tokens)

{
	std::ofstream output_file(file_name.c_str());
	if (!output_file)
	{
		std::cerr << "i can't write into file " << file_name << "."
			<< std::endl;
		return false;
	}
	return true;
}

bool token_is_semicolon(const evl_token &token) // Semicolon Testing or Predication for Statements 
{
	if (token.str == ";")
	{
		return true;
	}
	else
		return false;
}

bool tokens_move_t_statement(evl_tokens &statement_tokens,
	evl_tokens &tokens)
{
	assert(statement_tokens.empty());
	assert(!tokens.empty());

	evl_tokens::iterator next_sc = std::find_if(
		tokens.begin(), tokens.end(), token_is_semicolon);
	if (next_sc == tokens.end())
	{
		std::cerr << "Look for ';' but reach the end of file" <<
			std::endl;
		return false;
	}
	++next_sc;
	statement_tokens.splice(statement_tokens.end(), tokens, tokens.begin(), next_sc);
	return true;
}

bool tokens_group_into_statements(
	evl_statements &statements, evl_tokens &tokens)
{
	assert(statements.empty());
	for (; !tokens.empty();)
	{
		evl_token token = tokens.front();
		if (token.type != evl_token::NAME)
		{
			std::cerr << "Need a NAME token but found '" << token.str
				<< "'on line" << token.LineNo << std::endl;
			return false;
		}
		if (token.str == "module")
		{						
			evl_statement module;
			module.type = evl_statement::MODULE;
			if (!tokens_move_t_statement(module.tokens, tokens))
				return false;
			statements.push_back(module);
		}
		else if (token.str == "endmodule")
		{                 
			evl_statement endmodule;
			endmodule.type = evl_statement::ENDMODULE;
			endmodule.tokens.push_back(token);
			tokens.erase(tokens.begin());
			statements.push_back(endmodule);;
		}
		else if (token.str == "wire")
		{				
			evl_statement wire;
			wire.type = evl_statement::WiRE;
			if (!tokens_move_t_statement(wire.tokens, tokens))
				return false;
			statements.push_back(wire);
		}
		else
			
		{
			evl_statement component;
			component.type = evl_statement::COMPONENT;
			if (!tokens_move_t_statement(component.tokens, tokens))
				return false;
			statements.push_back(component);
		}
	}
	return true;
}

bool process_wire_statement(evl_wires &wires, evl_statement &s)
{
	evl_wire ew;
	assert(s.type == evl_statement::WiRE);
	enum state_type {
		iNiT, WiRE, DONE, WiRES, WiRE_NAME,
		BUS, BUS_MSB, BUS_COLON, BUS_LSB, BUS_DONE
	};
	state_type state = iNiT;
	int Bus_Width = 1;
	for (; !s.tokens.empty() && (state != DONE); s.tokens.pop_front())
	{
		evl_token t = s.tokens.front();
		 
		if (state == iNiT)
		{
			if (t.str == "wire") {
				state = WiRE;
			}
			else {
				std::cerr << "Need 'wire' but found '" << t.str
					<< "' on line " << t.LineNo << std::endl;
				return false;
			}
		}
		else if (state == WiRE)
		{
			if (t.type == evl_token::NAME)
			{
				ew.width = Bus_Width;
				ew.name = t.str;
				wires.push_back(ew);
				state = WiRE_NAME;
			}

			else if (t.str == "[")
			{
				state = BUS;
			}

			else {
				std::cerr << "Need NAME but found '" << t.str
					<< "' on line " << t.LineNo << std::endl;
				return false;
			}
		}
		else if (state == WiRES)
		{
			if (t.type == evl_token::NAME)
			{
				//wires.insert(std::make_pair(t.str, Bus_Width));//or-- chabges made
				//nm.push_back(t.str); ----changes made
				ew.width = Bus_Width;
				ew.name = t.str;
				wires.push_back(ew);
				//wires[t.str] = Bus_Width;
				state = WiRE_NAME;
			}

			else
			{
				std::cerr << "Need NAME but found '" << t.str
					<< "' on line " << t.LineNo << std::endl;
				return false;
			}
		}
		else if (state == WiRE_NAME)
		{
			if (t.str == ",")
			{
				state = WiRES;
			}
			else if (t.str == ";")
			{
				state = DONE;
			}
			else
			{
				std::cerr << "Need ',' or ';' but found '" << t.str
					<< "' on line " << t.LineNo << std::endl;
				return false;
			}

		}

		else if (state == BUS)
		{
			if (t.type == evl_token::NUMBER)
			{
				Bus_Width = atoi(t.str.c_str()) + 1; 
				
				state = BUS_MSB;
			}
			else
			{
				std::cerr << "Need NUMBER but found '" << t.str
					<< "' on line " << t.LineNo << std::endl;
				return false;
			}
		}

		else if (state == BUS_MSB)
		{
			if (t.str == ":")
			{
				state = BUS_COLON;
			}
			else
			{
				std::cerr << "Need ':' but found '" << t.str
					<< "' on line " << t.LineNo << std::endl;
				return false;
			}
		}

		else if (state == BUS_COLON)
		{
			if (t.str == "0")
			{
				state = BUS_LSB;
			}
			else
			{
				std::cerr << "Need '0' but found '" << t.str
					<< "' on line " << t.LineNo << std::endl;
				return false;
			}
		}

		else if (state == BUS_LSB)
		{
			if (t.str == "]")
			{
				state = BUS_DONE;
			}
			else
			{
				std::cerr << "Need ']' but found '" << t.str
					<< "' on line " << t.LineNo << std::endl;
				return false;
			}
		}

		else if (state == BUS_DONE)
		{
			if (t.type == evl_token::NAME)
			{
				
				ew.width = Bus_Width;
				ew.name = t.str;
				wires.push_back(ew);
				state = WiRE_NAME;

			}

			else
			{
				std::cerr << "Need NAME but found '" << t.str
					<< "' on line " << t.LineNo << std::endl;
				return false;
			}
		}

		else
		{
			assert(false); 
		}
	}
	if (!s.tokens.empty() || (state != DONE))
	{
		std::cerr << "something wrong with the statement" << std::endl;
		return false;
	}
	return true;
}

void display_wires(std::ostream &out,
	const evl_wires &wires)
{
	out << "wires" << " " << wires.size() << std::endl;
	for (evl_wires::const_iterator it = wires.begin();
		it != wires.end(); ++it)
	{
		out << "  " << "wire " << it->name
			<< " " << it->width << std::endl;
	}

}

bool process_component_statement(evl_components &components,
	evl_statement &s, evl_wires &wires)
{
	assert((!(s.type == evl_statement::WiRE))
		&& (!(s.type == evl_statement::MODULE))
		&& (!(s.type == evl_statement::ENDMODULE)));
	enum state_type {
		iNiT, TYPE, NAME, PiNS, PiN_NAME,
		BUS, BUS_MSB, BUS_COLON, BUS_LSB, BUS_DONE, PiNS_DONE, DONE
	};
	state_type state = iNiT;
	evl_component comp;
	evl_pin pin;
	int no_ofpins = 0;

	for (; !s.tokens.empty() && (state != DONE); s.tokens.pop_front())
	{
		evl_token t = s.tokens.front();
		
		if (state == iNiT)
		{

			if (t.type == evl_token::NAME)
			{
				comp.type = t.str;
				state = TYPE;
			}
			else {
				std::cerr << "Need NAME but found '" << t.str
					<< "' on line " << t.LineNo << std::endl;
				return false;
			}
		}

		else if (state == TYPE)
		{
			if (t.type == evl_token::NAME)
			{
				comp.name = t.str;
				state = NAME;
			}
			else if (t.str == "(")
			{
				state = PiNS;
			}
			else {
				std::cerr << "Need NAME or '(' but found '" << t.str
					<< "' on line " << t.LineNo << std::endl;
				return false;
			}
		}

		else if (state == NAME)
		{
			if (t.str == "(")
			{
				state = PiNS;
			}
			else {
				std::cerr << "Need '(' but found '" << t.str
					<< "' on line " << t.LineNo << std::endl;
				return false;
			}
		}

		else if (state == PiNS)
		{
			if (t.type == evl_token::NAME)
			{
				pin.name = t.str;
				pin.bus_msb = -1; pin.bus_lsb = -1;

				state = PiN_NAME;

			}

			else
			{
				std::cerr << "Need NAME but found '" << t.str
					<< "' on line " << t.LineNo << std::endl;
				return false;
			}
		}

		else if (state == PiN_NAME)
		{
			if (t.str == ",")
			{
				if (!pin_valid_bus(wires, pin))
				{
					std::cerr << "The pin is not valid, check it out" << std::endl;
				}
				comp.pins.push_back(pin);
				no_ofpins++;
				state = PiNS;
			}
			else if (t.str == ")")
			{
				if (!pin_valid_bus(wires, pin))
				{
					std::cerr << "The pin is not valid, check it out" << std::endl;
				}
				comp.pins.push_back(pin);
				//no_ofpins++;
				state = PiNS_DONE;
			}
			else if (t.str == "[")
			{
				state = BUS;
			}
			else
			{
				std::cerr << "Need ',' or ')' or '[' but found '" << t.str
					<< "' on line " << t.LineNo << std::endl;
				return false;
			}
		}//

		else if (state == BUS)
		{
			if (t.type == evl_token::NUMBER)
			{
				pin.bus_msb = atoi(t.str.c_str());
				pin.bus_lsb = atoi(t.str.c_str());
				state = BUS_MSB;
			}
			else
			{
				std::cerr << "Need NUMBER but found '" << t.str
					<< "' on line " << t.LineNo << std::endl;
				return false;
			}
		}

		else if (state == BUS_MSB)
		{
			if (t.str == ":")
			{
				state = BUS_COLON;
			}
			else if (t.str == "]")
			{
				state = BUS_DONE;
			}
			else
			{
				std::cerr << "Need ':' or ']' but found '" << t.str
					<< "' on line " << t.LineNo << std::endl;
				return false;
			}
		}

		else if (state == BUS_COLON)
		{
			if (t.type == evl_token::NUMBER)
			{
				pin.bus_lsb = atoi(t.str.c_str());
				state = BUS_LSB;
			}
			else
			{
				std::cerr << "Need NUMBER but found '" << t.str
					<< "' on line " << t.LineNo << std::endl;
				return false;
			}
		}

		else if (state == BUS_LSB)
		{
			if (t.str == "]")
			{
				state = BUS_DONE;
			}
			else
			{
				std::cerr << "Need ']' but found '" << t.str
					<< "' on line " << t.LineNo << std::endl;
				return false;
			}
		}

		else if (state == BUS_DONE)
		{
			if (t.str == ")")
			{

				state = PiNS_DONE;

			}
			else if (t.str == ",")
			{
				state = PiNS;

			}
			else
			{
				std::cerr << "Need ')' or ',' but found '" << t.str
					<< "' on line " << t.LineNo << std::endl;
				return false;
			}
			if (!pin_valid(wires, pin))
			{
				std::cerr << "The pin is not valid, check it out" << std::endl;
			}

			comp.pins.push_back(pin);
			no_ofpins++;
		}
		else if (state == PiNS_DONE)
		{
			if (t.str == ";")
			{
				state = DONE;
			}
			else
			{
				std::cerr << "Need ';' but found '" << t.str
					<< "' on line " << t.LineNo << std::endl;
				return false;
			}

		}
		else if (state == DONE)
		{
			comp.NoPins = no_ofpins;
			return true;
		}
		else
		{
			assert(false); // shouldn't reach here
		}

	}
	components.push_back(comp);

	if (!s.tokens.empty() || (state != DONE))
	{
		std::cerr << "something wrong with the statement" << std::endl;
		return false;
	}
	return true;
}


void display_components(std::ostream &out,
	const evl_components &comps)
{
	out << "components" << " " << comps.size() << std::endl;
	for (evl_components::const_iterator it = comps.begin();
		it != comps.end(); ++it)
	{
		if (it->name.empty() )
		{
			out << "  " << "component " << it->type << " " <<  it->pins.size() << std::endl;
		}
		else
		{
			out << "  " << "component " << it->type << " " << it->name << " " << it->pins.size() << std::endl;
		}
		for (evl_pins::const_iterator iter = it->pins.begin();
			iter != it->pins.end(); ++iter)
		{
			if (iter->bus_lsb == -1)
			{
				out << "    pin" << " " << iter->name << " " << std::endl;
			}
			else 
				out << "    pin" << " " << iter->name << " " << iter->bus_lsb << " " << std::endl;
			//<< iter->bus_lsb ;
		}
	}
}

bool module_process_statement(evl_modules &modules,
	evl_statement &s)
{
	assert(s.type == evl_statement::MODULE);
	evl_module module;
	for (; !s.tokens.empty(); s.tokens.pop_front())
	{
		evl_token t = s.tokens.front();
		if (t.type == evl_token::NAME)
		{
			module.name = t.str;
		}
		else
		{
			break;
		}

	}
	modules.push_back(module);
	return true;
}

void modules_display(std::ostream &out,
	const evl_modules &modules,
	const evl_wires &wires, const evl_components &comps)
{

	for (evl_modules::const_iterator it = modules.begin();
		it != modules.end(); ++it)
	{
		out << "module" << " " << it->name << " " << std::endl;
	}

}

void count_tokens_by_types(const evl_tokens &tokens)
{	// How many tokens are there with each type?
	typedef std::map<evl_token::token_type, int> token_type_table;
	token_type_table type_counts;
	for (evl_tokens::const_iterator it = tokens.begin();
		it != tokens.end(); ++it)
	{

		token_type_table::iterator map_it = type_counts.find(it->type);
		if (map_it == type_counts.end())
		{
			type_counts.insert(std::make_pair(it->type, 1));
		}
		else
		{
			++map_it->second;
		}
		
	}
	for (token_type_table::iterator map_it = type_counts.begin();
		map_it != type_counts.end(); ++map_it)
	{
		std::cout << "There are " << map_it->second <<
			" tokens of type " << map_it->first << std::endl;
	}

}
int main(int argc, char *argv[])
{
	if (argc < 2)    
	{
		std::cerr << "You should provide a file name." << std::endl;
		return -1;
	}
	std::string evl_file = argv[1];
	evl_tokens tokens;
	if (!tokens_extract_file(evl_file, tokens))  
	{
		return -1;
	}
	

	if (!tokens_store_t_file(evl_file + ".tokens", tokens))  // Calling Store_tokens to file Function 
	{
		return -1;
	}

	evl_statements statements;
	if (!tokens_group_into_statements(statements, tokens))
	{
		return -1;
	}
	
	evl_wires wires;
	evl_components components;
	evl_modules modules;
	std::ofstream output_file((evl_file + ".syntax").c_str());    

	for (evl_statements::iterator it = statements.begin();
		it != statements.end(); ++it)
	{

		if ((*it).type == evl_statement::MODULE)
		{
			if (!module_process_statement(modules, (*it)))
			{
				return -1;
			}
		}
		else if ((*it).type == evl_statement::WiRE)
		{
			if (!process_wire_statement(wires, (*it)))
			{
				return -1;
			}
		}

		else if ((*it).type == evl_statement::COMPONENT)
		{
			if (!process_component_statement(components, *it, wires))
			{
				return -1;

			}
		}

		else
		{
			break; 

		}
	}

	modules_display(std::cout, modules, wires, components);
	modules_display(output_file, modules, wires, components);
	display_wires(std::cout, wires);
	display_wires(output_file, wires);
	display_components(std::cout, components);
	display_components(output_file, components);
	return 0;

	
}


