#include <assert.h>
#include <algorithm>
#include <ctype.h>
#include <fstream>
#include <iostream>
#include<list>
#include <map>
#include <string>
#include <vector>

struct evl_token   // Structure Evaluate Token 
{
	enum token_type{ NAME, NUMBER, SINGLE };   // token type defined here... 
	token_type type;
	std::string str;
	int LineNo;
}; // Structure Evl_token

typedef std::list<evl_token> evl_tokens;

struct evl_wire
{
	std::string name;
	int width;
}; // Structure evl_wire

typedef std::list<evl_wire> evl_wires;  // Map Used here 
//typedef std::vector<std::string> names; // vector added
struct evl_statement
{
	enum statement_type { MODULE, WIRE, COMPONENT, ENDMODULE };  // Sequence of token statements type defined here 
	statement_type type;
	evl_tokens tokens;
};//Struct evl_statement

typedef std::list<evl_statement> evl_statements;

struct evl_pin
{
	std::string name;
	int bus_msb, bus_lsb;
}; // struct evl_pin

typedef std::list<evl_pin> evl_pins;

struct evl_component
{
	int NoPins;
	std::string type, name;
	evl_pins pins;

}; // struct evl_component

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

bool extract_tokens_from_line(std::string line,    // Extracting Tokens line by line 
	int LineNo, evl_tokens &tokens)
{
	for (size_t i = 0; i < line.size();)
	{
		if (line[i] == '/')
		{
			++i;
			if ((i == line.size()) || (line[i] != '/'))
			{
				std::cerr << "LINE " << LineNo
					<< ": a single / is not allowed" << std::endl;
				return false;
			}
			break; // skip the rest of the line by exiting the loop
		}
		else if (isspace(line[i]))   // Is space Function is Used here 
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
			token.type = evl_token::SINGLE;
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
					break; 	// [name_begin, i) is the range for the token
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
					break; 	// [number_begin, i) is the range for the token
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
			std::cerr << "LINE " << LineNo
				<< ": invalid character" << std::endl;
			return false;
		}
	}
	return true; // Done here
}

bool extract_tokens_from_file(std::string file_name,  // Extracint Tokens From the File 
	evl_tokens &tokens)
{
	std::ifstream input_file(file_name.c_str());
	if (!input_file)
	{
		std::cerr << "I can't read " << file_name << "." << std::endl;
		return false;
	}

	tokens.clear();

	std::string line;
	for (int LineNo = 1; std::getline(input_file, line); ++LineNo)
	{
		if (!extract_tokens_from_line(line, LineNo, tokens))
		{
			return false;
		}
	}
	return true;
}
/*void display_tokens(std::ostream &out,  // function is Avoided to Avoid Display of Tokens
const evl_tokens &tokens)
{
int count = 1;
for (evl_tokens::const_iterator iter = tokens.begin();
iter != tokens.end(); ++iter, ++count)
{
if ((*iter).type == evl_token::SINGLE)
{
out << "Token " << count;
out << ": SINGLE" << std::endl;
}
else if ((*iter).type == evl_token::NAME)
{
out << "Token " << count;
out << ": NAME" << std::endl;
}
else //if ((*iter).type == evl_token::NUMBER)
{
out << "Token " << count;
out << ": NUMBER" << std::endl;

}
}
}
*/
bool store_tokens_to_file(std::string file_name,  // Store Tokens to Output file with extension .token
	const evl_tokens &tokens)

{
	std::ofstream output_file(file_name.c_str());
	if (!output_file)
	{
		std::cerr << "I can't write into file " << file_name << "."
			<< std::endl;
		return false;
	}
	//		display_tokens(output_file, tokens);
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

bool move_tokens_to_statement(evl_tokens &statement_tokens,
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
	//statement_tokens.splice(statement_tokens.begin(), tokens, tokens.begin(), next_sc);
	statement_tokens.splice(statement_tokens.end(), tokens, tokens.begin(), next_sc);



	return true;
}

bool group_tokens_into_statements(
	evl_statements &statements, evl_tokens &tokens)
{
	assert(statements.empty());
	for (; !tokens.empty();)
	{
		// Generate one token per iteration
		evl_token token = tokens.front();
		if (token.type != evl_token::NAME)
		{
			std::cerr << "Need a NAME token but found '" << token.str
				<< "'on line" << token.LineNo << std::endl;
			return false;
		}
		if (token.str == "module")
		{						// MODULE statement
			evl_statement module;
			module.type = evl_statement::MODULE;
			if (!move_tokens_to_statement(module.tokens, tokens))
				return false;
			statements.push_back(module);
		}
		else if (token.str == "endmodule")
		{                 // ENDMODULE statement
			evl_statement endmodule;

			endmodule.type = evl_statement::ENDMODULE;
			endmodule.tokens.push_back(token);

			tokens.erase(tokens.begin());
			statements.push_back(endmodule);;
		}
		else if (token.str == "wire")
		{				// WIRE statement

			evl_statement wire;
			wire.type = evl_statement::WIRE;
			if (!move_tokens_to_statement(wire.tokens, tokens))
				return false;
			statements.push_back(wire);
		}
		else
			//		COMPONENT statement
		{
			evl_statement component;
			component.type = evl_statement::COMPONENT;
			if (!move_tokens_to_statement(component.tokens, tokens))
				return false;
			statements.push_back(component);
		}
	}
	return true;
}
/*
void display_statements(std::ostream &out,const evl_statements &statements)
{
int count = 1;
for (evl_statements::const_iterator iter = statements.begin();
iter != statements.end(); ++iter, ++count) //right
{
if ((*iter).type == evl_statement::ENDMODULE)
{
out << "Statement " << count;
out << ": ENDMODULE" << std::endl;
}
else if ((*iter).type == evl_statement::MODULE)
{
out << "Statement " << count;
out << ": MODULE" << std::endl;
}
else if ((*iter).type == evl_statement::WIRE)
{
out << "Statement " << count;
out << ": Wire" << std::endl;
}
else //Remaining Component Module
{
out << "Statement " << count;
out << ": COMPONENT" << std::endl;

}
}
}*/

/*bool store_statements_to_file(std::string file_name,
const evl_statements &statements)
{

std::ofstream output_file(file_name.c_str());
if (!output_file)
{
std::cerr << "I can't write into file " << file_name << "." << std::endl;
return false;
}
display_statements(output_file, statements);
return true;
}*/

/*bool findval(const evl_wires &one, const evl_wires &two)
{
//compares the first value with the second value.
return one.name < two.name;
}*/

bool process_wire_statement(evl_wires &wires, evl_statement &s)
{
	evl_wire ew;
	//names nm;
	assert(s.type == evl_statement::WIRE);
	enum state_type {
		INIT, WIRE, DONE, WIRES, WIRE_NAME,
		BUS, BUS_MSB, BUS_COLON, BUS_LSB, BUS_DONE
	};
	state_type state = INIT;
	int Bus_Width = 1;
	for (; !s.tokens.empty() && (state != DONE); s.tokens.pop_front())
	{
		evl_token t = s.tokens.front();
		//  Consume one token at a time and Start with INIT State 
		if (state == INIT)
		{
			if (t.str == "wire") {
				state = WIRE;
			}
			else {
				std::cerr << "Need 'wire' but found '" << t.str
					<< "' on line " << t.LineNo << std::endl;
				return false;
			}
		}
		else if (state == WIRE)
		{
			if (t.type == evl_token::NAME)
			{
				ew.width = Bus_Width;
				ew.name = t.str;
				wires.push_back(ew);
				//	wires.insert(std::make_pair(t.str, Bus_Width));
				//		nm.push_back(t.str);
				state = WIRE_NAME;
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
		else if (state == WIRES)
		{
			if (t.type == evl_token::NAME)
			{
				/*evl_wires::iterator it =);
				if (it != wires.end()) {
				std::cerr << "Wire '" << t.str << "' on line "
				<< t.LineNo << "is already defined" << std::endl;
				return false;
				}*/
				//wires.insert(std::make_pair(t.str, Bus_Width));//or-- chabges made
				//nm.push_back(t.str); ----changes made
				ew.width = Bus_Width;
				ew.name = t.str;
				wires.push_back(ew);
				//wires[t.str] = Bus_Width;
				state = WIRE_NAME;
			}

			else
			{
				std::cerr << "Need NAME but found '" << t.str
					<< "' on line " << t.LineNo << std::endl;
				return false;
			}
		}
		else if (state == WIRE_NAME)
		{
			if (t.str == ",")
			{
				state = WIRES;
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
				Bus_Width = atoi(t.str.c_str()) + 1; //changed
				//Bus_Width = atoi(t.str.c_str());

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
				//wires.insert(std::make_pair(t.str, Bus_Width)); --- changes made
				//nm.push_back(t.str); ---changes made
				ew.width = Bus_Width;
				ew.name = t.str;
				wires.push_back(ew);
				state = WIRE_NAME;

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
			assert(false); // An Error if occurs here
		}
	}
	if (!s.tokens.empty() || (state != DONE))
	{
		std::cerr << "something wrong with the statement" << std::endl;
		return false;
	}
	return true;
}



bool process_component_statement(evl_components &components,
	evl_statement &s, evl_wires &wires)
{
	assert((!(s.type == evl_statement::WIRE))
		&& (!(s.type == evl_statement::MODULE))
		&& (!(s.type == evl_statement::ENDMODULE)));
	enum state_type {
		INIT, TYPE, NAME, PINS, PIN_NAME,
		BUS, BUS_MSB, BUS_COLON, BUS_LSB, BUS_DONE, PINS_DONE, DONE
	};
	state_type state = INIT;
	evl_component comp;
	evl_pin pin;
	int no_ofpins = 0;

	for (; !s.tokens.empty() && (state != DONE); s.tokens.pop_front())
	{
		evl_token t = s.tokens.front();
		//  Starts computation with INIT state 

		if (state == INIT)
		{

			if (t.type == evl_token::NAME)
			{
				comp.type = t.str;
				//comp.name = "NONE";
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
				state = PINS;
			}
			else {
				std::cerr << "Need NAME or '(' but found '" << t.str<< "' on line " << t.LineNo << std::endl;
				return false;
			}
		}

		else if (state == NAME)
		{
			if (t.str == "(")
			{
				state = PINS;
			}
			else {
				std::cerr << "Need '(' but found '" << t.str
					<< "' on line " << t.LineNo << std::endl;
				return false;
			}
		}

		else if (state == PINS)
		{
			if (t.type == evl_token::NAME)
			{
				pin.name = t.str;
				pin.bus_msb = -1; pin.bus_lsb = -1;

				state = PIN_NAME;

			}

			else
			{
				std::cerr << "Need NAME but found '" << t.str
					<< "' on line " << t.LineNo << std::endl;
				return false;
			}
		}

		else if (state == PIN_NAME)
		{
			if (t.str == ",")
			{
				if (!pin_valid_bus(wires, pin))
				{
					std::cerr << "The pin is not valid, check it out" << std::endl;
				}
				comp.pins.push_back(pin);
				no_ofpins++;
				state = PINS;
			}
			else if (t.str == ")")
			{
				if (!pin_valid_bus(wires, pin))
				{
					std::cerr << "The pin is not valid, check it out" << std::endl;
				}
				comp.pins.push_back(pin);
				//no_ofpins++;
				state = PINS_DONE;
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

				state = PINS_DONE;

			}
			else if (t.str == ",")
			{
				state = PINS;

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
		else if (state == PINS_DONE)
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

void display(std::ostream &out, const evl_modules &modules, const evl_wires &wires, const evl_components &comps, evl_pins  &pins)
{
	//display modules
	
	for (evl_modules::const_iterator it = modules.begin();
		it != modules.end(); ++it)
	{
		out << "module" << " " << it->name << " " << std::endl;
	}
	// display wires
	int width = 0;
	int pos = 0;
	std::string pin_nm;
	//int cnt = 0;
	int pos1 = 0;
	std::string comp_nm;
	std::string comp_type;
	for (evl_wires::const_iterator it = wires.begin();it != wires.end(); ++it)
	{
		//out << "  " << "wire " << it->name	<< " " << it->width << std::endl;
		width += it->width;
	}
	out << "nets" << " " << width << std::endl;
	for (evl_wires::const_iterator it = wires.begin(); it != wires.end(); ++it)
	{
			//pos1 = 0;
			for (int j = 0; j <= it->width; j++)
			{

				pin_nm = it->name;
				//			std::cout << pin_nm<< std::endl;

				for (evl_components::const_iterator it1 = comps.begin(); it1 != comps.end(); ++it1)
				{
					comp_nm = it1->name;
					comp_type = it1->type;
					pos1 = 0;
					if (it1->pins.end()->name == pin_nm)
					{
						std::cout << "func1in " << it1->pins.begin()->name << std::endl;
						pos1 += 1;
					}	std::cout << "func1 out " << it1->pins.begin()->name << std::endl;

				//out << " " << comp_type << " " << comp_nm << " " << pos << std::endl;//<< " " << iter->bus_msb << " " << iter->bus_lsb << std::endl;
				}
				out << "  " << "net " << it->name << "[" << j << "]" << " " << pos1 << std::endl;
				for (evl_components::const_iterator it = comps.begin(); it != comps.end(); ++it)
				{
					if (it->name.empty())
					{
						//	out << "  " << "component " << it->type << " " << it->pins.size() << std::endl;
						comp_nm = it->name;
						comp_type = it->type;
						//std::cout << comp_nm << " " << comp_type;
					}
					else
					{
						//out << "  " << "component " << it->type << " " << it->name << " " << it->pins.size() << std::endl;
						comp_nm = it->name;
						comp_type = it->type;
						//std::cout << comp_nm << " " << comp_type;

					}
					for (evl_pins::const_iterator iter = it->pins.begin(); iter != it->pins.end(); ++iter)
					{
						
						pos = 0;
						if (iter->name == pin_nm)
						{

							pos += 1;
						}
					}
					out << " " << comp_type << " " << comp_nm << " " << pos << std::endl;
					//<< " " << iter->bus_msb << " " << iter->bus_lsb << std::endl;
				}

			}
		
			//width += it->width;
	}



	// display components
	//out << "components" << " " << comps.size() << std::endl;
	/*for (evl_components::const_iterator it = comps.begin();	it != comps.end(); ++it)
	{
		if (it->name.empty())
		{
			out << "  " << "component " << it->type << " " << it->pins.size() << std::endl;
		}
		else
		{
			out << "  " << "component " << it->type << " " << it->name << " " << it->pins.size() << std::endl;
		}
		for (evl_pins::const_iterator iter = it->pins.begin();
			iter != it->pins.end(); ++iter)
		{
			if (iter->bus_msb == -1)
			{
				out << "    pin" << " " << iter->name << " " << std::endl;
			}
			else if (iter->bus_lsb == -1)
			{
				out << "    pin" << " " << iter->name << " " << iter->bus_msb << " " << std::endl;

			}
			else
			{
				out << "    pin" << " " << iter->name << " " << iter->bus_msb << " " << iter->bus_lsb << std::endl;
			}

		}
	}*/

}

bool process_module_statement(evl_modules &modules,
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
		//++type_counts[it->type];// increase the count of it->type by 1
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
	if (argc < 2)   // Provide a Input File here 
	{
		std::cerr << "You should provide a file name." << std::endl;
		return -1;
	}
	std::string evl_file = argv[1];
	evl_tokens tokens;
	if (!extract_tokens_from_file(evl_file, tokens))  // Extract token From the file given into tokens
	{
		return -1;
	}
	//	display_tokens(std::cout, tokens);

	if (!store_tokens_to_file(evl_file + ".tokens", tokens))  // Calling Store_tokens to file Function 
	{
		return -1;
	}

	// count_tokens_by_types(tokens) ;   // Used for Counting the number of types of token

	evl_statements statements;
	if (!group_tokens_into_statements(statements, tokens))
	{
		return -1;
	}
	//	display_statements(std::cout, statements);
	/*if (!store_statements_to_file(evl_file + ".statements", statements))  // Storing the statements here
	{
	return -1;
	}*/

	evl_wires wires;
	evl_components components;
	evl_modules modules;
	evl_pins pins;
	std::ofstream output_file((evl_file + ".netlist").c_str());    // Output Requirement With Syntax extension 

	for (evl_statements::iterator it = statements.begin();
		it != statements.end(); ++it)
	{

		if ((*it).type == evl_statement::MODULE)
		{
			if (!process_module_statement(modules, (*it)))
			{
				return -1;
			}
		}
		else if ((*it).type == evl_statement::WIRE)
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
			break;//End of the Module 

		}
	}

	//	display_modules(std::cout, modules, wires, components);
//	display_modules(output_file, modules, wires, components);
	//display_wires(std::cout, wires);
	//display_wires(output_file, wires,components);
	//	display_components(std::cout, components);
	//display_components(output_file, components);
	display(output_file, modules, wires, components,pins);
	return 0;

	//end of file 
}


