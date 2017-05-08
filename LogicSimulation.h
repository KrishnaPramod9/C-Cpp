#include <assert.h>
#include <fstream>
#include <iostream>
#include <map>
#include <list>
#include <string>

#include "Lexical.h"
#include "Netlist.h"
#include "Syntactic.h"
#include "Type_Conv.h"

int main(int argc, char *argv[]) 
{
  if (argc < 2) 
  {
    std::cerr << "You should provide a file name." << std::endl;
    return -1;
  }
  	evl_wires wires;
	evl_components components;
	evl_ports ports;
	evl_modules_map modules_map;
	//evl_modules modules;

	if (!parse_evl_file(argv[1], wires, components, ports))
		return false;
/*// creating an output file
 std::string output_file_name = argv[1];
 // naming the output file
 output_file_name += ".sim_out.evl_output";
 std::ofstream output_file(output_file_name.c_str());
 if (!output_file) 
 {
  std::cerr << "I can't write " << output_file_name << "." << std::endl;
  return -1;
 }*/

  
	netlist netlist_1;

	if (!netlist_1.create(wires,components))
		return -1;

	std::cout << argv[1] << ": Info" << ": Ports " << ports.size() 
	  << ", Components " << netlist_1.components_count << ", Assigns 0" << std::endl;

	std::cout << argv[1] << ": Info: build module " << ": " << components.size() 
	  << " gates, " << netlist_1.nets_list.size() << " nets, " << netlist_1.pin_count << " pins." << std::endl;

if(!netlist_1.simulation_run (components, argv[1], 1000))
	return -1;
}

#include <iostream>
#include <string>
#include <list>
#include <assert.h>

#include "Lexical.h"

bool is_character_a_space(char ch)
{
	return (ch == ' ') || (ch == '\t')|| (ch == '\r') || (ch == '\n');
}

bool is_character_a_letter(char ch)
{
	return (((ch >= 'a') && (ch <= 'z'))       // a to z
				|| ((ch >= 'A') && (ch <= 'Z')));    // A to Z
}

bool is_character_a_digit(char ch)
{
	return ((ch >= '0') && (ch <= '9'));
}

// Extracting tokens from a line.
bool extract_tokens_from_line(std::string line, int line_no, evl_tokens &tokens) 
{ // use reference to modify it
  for (size_t i = 0; i < line.size();) 
  {
    // Skip the Comments
    if (line[i] == '/') 
	{ 
      ++i;
      if ((i == line.size()) || (line[i] != '/')) 
	  {
        std::cerr << "LINE " << line_no << ": a single / is not allowed" << std::endl;
        return false;
      }
      break; // skip the rest of the line by exiting the loop
    }
    
    // skip spaces
    else if (is_character_a_space(line[i])) 
	{
      ++i; // skip this space character
      continue; // skip the rest of the iteration
    } 
    // a SINGLE token

    else if ((line[i] == '(') || (line[i] == ')') || (line[i] == '=') || (line[i] == '[') || (line[i] == ']') || (line[i] == ':') || (line[i] == ';') || (line[i] == ',')) 
	{
      
	  evl_token token;
      token.line_no = line_no;
	  token.type = evl_token::SINGLE;
      token.str = std::string(1, line[i]);
      tokens.push_back(token);

//	  std::cout << line_no << ": " << token.type <<": " << token.str << std::endl; //Printing all the tokens found on each line

      ++i; // we consumed this character
      continue; // skip the rest of the iteration
    }

	// a NAME token
    else if (is_character_a_letter(line[i]) || (line[i] == '_') || (line[i] == '\\') || (line[i] == '.')) 
	{
      size_t name_begin = i;
      for (++i; i < line.size(); ++i) 
	  {
        if (!(is_character_a_letter(line[i]) || (is_character_a_digit(line[i])) || (line[i] == '_') || (line[i] == '\\') || (line[i] == '.'))) 
		{
          break; // [name_begin, i) is the range for the token
        }
      }

      evl_token token;
      token.type = evl_token::NAME;
      token.str = line.substr(name_begin, i-name_begin);
      token.line_no = line_no;
      tokens.push_back(token);

//	  std::cout << line_no << ": " << token.type <<": " << token.str << std::endl; //Printing all the tokens found on each line
	  
    }

	// a NUMBER token
    else if (is_character_a_digit(line[i])) 
	{
      size_t num_begin = i;
      for (++i; i < line.size(); ++i) 
	  {
        if (!(is_character_a_digit(line[i]))) 
		{
          break; // [name_begin, i) is the range for the token
        }
      }

      evl_token token;
      token.type = evl_token::NUMBER;
      token.str = line.substr(num_begin, i-num_begin);
      token.line_no = line_no;
      tokens.push_back(token);	 

//	  std::cout << line_no << ": " << token.type <<": " << token.str << std::endl; //Printing all the tokens found on each line
	   
    }
    else 
	{
      std::cerr << "LINE " << line_no << ": invalid character" << std::endl;
      return false;
    }
  }
  return true; // nothing left
}

// Extracting tokens from a File.
bool extract_tokens_from_file(std::string file_name, evl_tokens &tokens) 
{ // use reference to modify it
  std::ifstream input_file(file_name.c_str());
  
  if (!input_file) 
  {
    std::cerr << "I can't read " << file_name << "." << std::endl;
    return false;
  }
  tokens.clear(); // be defensive, make it empty
  std::string line;

  for (int line_no = 1; std::getline(input_file, line); ++line_no) 
  {
    if (!extract_tokens_from_line(line, line_no, tokens))
	{
      return false;
    }
  }
  return true;
}

void display_tokens(std::ostream &out, const evl_tokens &tokens) 
{
  assert(!tokens.empty());
  for (evl_tokens::const_iterator iterator = tokens.begin(); iterator != tokens.end(); ++iterator) 
  {
    if ((*iterator).type == evl_token::SINGLE) 
	{
      std::cout << "SINGLE " << (*iterator).str << std::endl; //If token type is SINGLE
    }
    else if ((*iterator).type == evl_token::NAME) 
    {
      std::cout << "NAME " << (*iterator).str << std::endl; //If token type is NAME
    }
    else 
    {
      std::cout << "NUMBER " << (*iterator).str << std::endl; // token type is NUMBER
    }
  }
}

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <list>
#include <map>

#include <stddef.h>
#include <assert.h>
#include <time.h>

#include "Lexical.h"
#include "Syntactic.h"
#include "Netlist.h"
#include "Type_Conv.h"

//Gate Functions
bool gate::create(const evl_component &component, std::map<std::string, net *> &netlist_nets) 
{
	// store gate type and name;
	size_t pin_index = 0;

	for (evl_pins::const_iterator iter_p = component.pins.begin(); iter_p != component.pins.end(); ++iter_p)
	{
		create_pin(*iter_p, pin_index, netlist_nets);
		++pin_index;
	}
	return 1;
}

bool gate::create_pin(const evl_pin &p, size_t pin_index, std::map<std::string, net *> &netlist_nets) 
{
	pins_.push_back(new pin);

	return pins_.back()->create(this, pin_index, p, netlist_nets);
}

//Net functions
void net::append_pin(pin *p) 
{
	connections_.push_back(p);
	
}

//Pin Functions
bool pin::create(gate *g, size_t pin_index, const evl_pin &p, std::map<std::string, net *> &netlist_nets) 
{
	//  store g and pin_index;

	if (p.bus_msb == -1) // a 1-bit wire
	{ 
		nets_.push_back(netlist_nets[p.name]); 
	}
	else   // a bus
	{ 
//		size_t index = 0;

		for(int i = p.bus_lsb; i <= p.bus_msb; ++i)
		{
			nets_.push_back(netlist_nets[make_net_name(p.name,i)]);
		}
	}
	return true;
}

//Netlist Functions
bool netlist::create_nets(const evl_wires &wires) 
{
	
	for (evl_wires::const_iterator iter = wires.begin(); iter != wires.end(); ++iter)
	{
		if (iter->width == 1) 
		{
			nets_list.push_back(iter->name);
		}
		else 
		{
			for (int i = 0; i < iter->width ; ++i) 
			{
				nets_list.push_back(make_net_name(iter->name,i)); 			
			}
		}
	}
	return true;
}

bool netlist::create_gates(const evl_components &components)
{
	for (evl_components::const_iterator iter_c = components.begin(); iter_c != components.end(); ++iter_c)
	{
//		gates_.push_back (new gate);
//		gates_.back()->create(*iter_c, nets_);

		gates_.push_back(new gate);
	}
	return true;
}

void netlist::create_pin(const evl_pin &pin, string_list &comp_pins_) 
{
    if (pin.bus_msb == -1)  // a 1-bit wire
	{
      comp_pins_.push_back(pin.name);
    }
    else if (pin.bus_msb == pin.bus_lsb)  // a 1-bit wire
	{
      comp_pins_.push_back((make_net_name(pin.name,pin.bus_msb)));
    }
    else 
	{
      for (int i = pin.bus_msb; i >= pin.bus_lsb; --i) // a bus
	  {
        comp_pins_.push_back((make_net_name(pin.name,i)));
      }
    }
}

void netlist::create_pins(const evl_pins &pins, string_list &comp_pins_) 
{
  for (evl_pins::const_iterator iter1 = pins.begin(); iter1 != pins.end(); ++iter1) 
  {
	  create_pin(*iter1, comp_pins_);
  }
}

void netlist::create_pins_list(const evl_pins &pins, vector_string &comp_pins_, vector_int &pin_position)
{
  size_t index = 0;

  for (evl_pins::const_iterator iter_p = pins.begin(); iter_p != pins.end(); ++iter_p) 
  {
    if ((*iter_p).bus_msb == -1)		// a 1-bit wire
	{ 
      comp_pins_.push_back((*iter_p).name);
	  pin_position.push_back(index);
    }
    else if ((*iter_p).bus_msb == (*iter_p).bus_lsb)	// a 1-bit wire
	{ 
      comp_pins_.push_back((make_net_name((*iter_p).name,(*iter_p).bus_msb)));
	  pin_position.push_back(index);
    }
    else	// a bus
	{
      for (int i = (*iter_p).bus_lsb; i <= (*iter_p).bus_msb; ++i) 
	  {
        comp_pins_.push_back((make_net_name((*iter_p).name,i)));
	    pin_position.push_back(index);
      }
    }
	index++;
  }
}

bool netlist::create(const evl_wires &wires, const evl_components &components) 
{
	bool ret_value;

	create_nets(wires); 
	create_gates(components); 

	wires_count = 0;

	ret_value = create_connects(components, nets_list, gate_pin_map_list, pins_list_);

	wires_count = nets_list.size();

	components_count = components.size();

   pin_count = 0;

  for (evl_components::const_iterator iter_c = components.begin(); iter_c != components.end(); ++iter_c) 
  {
    pin_count += (*iter_c).pins.size();	     
  }	

	return ret_value;
}

bool netlist::create_connects(const evl_components &components, const string_list &nets_list_, gate_pin_map &gate_pin_map_list, pins_list &pins_list_) 
{
for (string_list::const_iterator iter_n = nets_list_.begin(); iter_n != nets_list_.end(); ++iter_n) 
  {
	string_list evl_pins_n_; 

    for (evl_components::const_iterator iter_c = components.begin(); iter_c != components.end(); ++iter_c) 
	{
		vector_int pin_position;
		vector_string comp_pins_;

		create_pins_list((*iter_c).pins, comp_pins_, pin_position);

		string_2_list predef_gates_type;
	   
	   for (size_t i = 0; i < comp_pins_.size(); ++i) 
	   {
	    if(*iter_n == comp_pins_[i]) 
		{
	 	  gate_pin gate_pin_;
		  gate_pin_.name = (*iter_c).name;
		  gate_pin_.type = (*iter_c).type;

		  gate_pin_.pin_position = pin_position[i];

		  std::ostringstream pin_index_str;

          pin_index_str << pin_position[i];

		  std::string pin_net_map = *iter_n + (*iter_c).name + (*iter_c).type + pin_index_str.str();
		  
		  evl_pins_n_.push_back(pin_net_map);

		  gate_pin_map_list.insert(std::make_pair(pin_net_map, gate_pin_));
		}
      }
    }
    pins_list_.insert(std::make_pair(*iter_n, evl_pins_n_));
  }
  
  for (string_list::const_iterator iter1 = nets_list_.begin(); iter1 != nets_list_.end(); ++iter1) 
  {
	evl_components comp_list;
   
	for (evl_components::const_iterator iter2 = components.begin(); iter2 != components.end(); ++iter2)
	{
      string_list comp_pins_;

      create_pins((*iter2).pins, comp_pins_);
		
	  string_list::const_iterator iter3 = comp_pins_.begin();
	    
		for (; iter3 != comp_pins_.end(); ++iter3) 
		{
	    if(*iter1 == *iter3) 
		{
		  if(!((*iter2).type == "one" || (*iter2).type == "zero" || (*iter2).type == "input" || (*iter2).type == "output")) 
		  { 
		      comp_list.push_back(*iter2);			 
		  }	  
		}
      }
    }
    component_map.insert(std::make_pair(*iter1, comp_list));
  }
  return 1;
}

bool netlist::structural_semantics (const std::string &type_, const std::string &name_, const evl_pins &pins_, string_2_list &predef_gates_type, string_list &event_scheduler, net_values_map &net_values_map_data)  
{
  if (type_ == "and") 
  {
    if (pins_.size() < 3) 
	{
  	  return semantics_error (type_, name_);
	}  

	evl_pins::const_iterator iter_p = pins_.begin(); // The first pin is an output pin

	predef_gates_type[(*iter_p).name] = "output";

	for (evl_pins::const_iterator iter_p = pins_.begin()++; iter_p != pins_.end(); ++iter_p) // All pins except the first are input pins
	{
	  predef_gates_type[(*iter_p).name] = "input";
    }
  }  
  
  else if (type_ == "or") 
  {
    if (pins_.size() < 3) 
	{
      return semantics_error (type_, name_);
	} 

	evl_pins::const_iterator iter_p = pins_.begin(); // The first pin is an output pin

	predef_gates_type[(*iter_p).name] = "output";

	for (evl_pins::const_iterator iter_p = pins_.begin()++; iter_p != pins_.end(); ++iter_p) // All pins except the first are input pins
	{
	  predef_gates_type[(*iter_p).name] = "input";
    }
  }  
  
  else if (type_ == "xor") 
  {
    if (pins_.size() < 3) 
	{
      return semantics_error (type_, name_);
	}  

	evl_pins::const_iterator iter_p = pins_.begin(); // The first pin is an output pin

	predef_gates_type[(*iter_p).name] = "output";

	for (evl_pins::const_iterator iter_p = pins_.begin()++; iter_p != pins_.end(); ++iter_p) // All pins except the first are input pins
	{
	  predef_gates_type[(*iter_p).name] = "input";
    }
  }  

  else if (type_ == "not") 
  {
    if (pins_.size() != 2) 
	{
      return semantics_error (type_, name_);
	}  
	evl_pins::const_iterator iter_p = pins_.begin(); // The first pin is an output pin

	predef_gates_type[(*iter_p).name] = "output";

	iter_p = pins_.begin()++;	// The second pin is an input pin

	predef_gates_type[(*iter_p).name] = "input";
  }
  
  else if (type_ == "buf") 
  {
    if (pins_.size() != 2) 
	{
      return semantics_error (type_, name_);
	}  
	evl_pins::const_iterator iter_p = pins_.begin(); // The first pin is an output pin

	predef_gates_type[(*iter_p).name] = "output";

	iter_p = pins_.begin()++;	// The second pin is an input pin

	predef_gates_type[(*iter_p).name] = "input";
  }
  
  else if (type_ == "dff") 
  {
    if (pins_.size() != 2) 
	{
      return semantics_error (type_, name_);
	}  

	evl_pins::const_iterator iter_p = pins_.begin(); // The first pin is an output pin

	predef_gates_type[(*iter_p).name] = "output";

	iter_p = pins_.begin()++;	// The second pin is an input pin

	predef_gates_type[(*iter_p).name] = "input";

	string_list component_pins_i;

    create_pin(*iter_p, component_pins_i);

    string_list::iterator iter_sl = component_pins_i.begin();

	event_scheduler.push_back(*iter_sl);
	
	net_values_map_data[(*iter_sl)].push_back(0);
  }
  
  else if (type_ == "one") 
  {
    if (pins_.size() < 1) 
	{
      return semantics_error (type_, name_); 
	}  

	for (evl_pins::const_iterator iter_p = pins_.begin(); iter_p != pins_.end(); ++iter_p) // All pins are output pins
	{
	  predef_gates_type[(*iter_p).name] = "output";
    }
  }  
  
  else if (type_ == "zero") 
  {
    if (pins_.size() < 1) 
	{
      return semantics_error (type_, name_);
	}  
	
	for (evl_pins::const_iterator iter_p = pins_.begin(); iter_p != pins_.end(); ++iter_p)	// All pins are output pins
	{
	  predef_gates_type[(*iter_p).name] = "output";
    }
  }  
  
  else if (type_ == "input") 
  {
    if (pins_.size() < 1) 
	{
      return semantics_error (type_, name_);
	}  

	for (evl_pins::const_iterator iter_p = pins_.begin(); iter_p != pins_.end(); ++iter_p) // All pins are output pins
	{
	  predef_gates_type[(*iter_p).name] = "output";
    }
  }  
 
  else if (type_ == "output") 
  {
    if (pins_.size() < 1) 
	{
      return semantics_error (type_, name_);
	}  
	
	for (evl_pins::const_iterator iter_p = pins_.begin(); iter_p != pins_.end(); ++iter_p)  // All pins are input pins
	{
	  predef_gates_type[(*iter_p).name] = "input";
    }
  }  
  return true;
}

bool netlist::semantics_error (const std::string &type_, const std::string &name_)
{
	std::cerr << "Error: Number of pins for " << type_ << " gate: " << name_ << " is invalid!" << std::endl;

	return false;
}

bool netlist::capture_inputs(std::string &input_file_, string_2_list &input_data_, const evl_components &c, evl_components &comps_cout, net_values_map &net_values_map_data, std::string file_name, string_list &event_scheduler, string_list &event_scheduler_init, int Cycles_num, string_2_list &comp_pins_dir_, string_list &DFF_event_scheduler) 
{
  for (evl_components::const_iterator iter_c = c.begin(); iter_c != c.end(); ++iter_c) 
  {
	   if(!structural_semantics((*iter_c).type, (*iter_c).name, (*iter_c).pins, comp_pins_dir_, DFF_event_scheduler, net_values_map_data))
         
		   return false;

	   if ((*iter_c).type == "one" || (*iter_c).type == "zero") 
	   {
	     
		   if(!capture_one_zero_values(input_file_, input_data_, (*iter_c).type, (*iter_c).name, (*iter_c).pins, net_values_map_data, file_name, event_scheduler, event_scheduler_init, Cycles_num))
           
			   return false;
	   }

	   if ((*iter_c).type == "output") 
	   {
	      comps_cout.push_back(*iter_c);
	   }
  }

  for (evl_components::const_iterator iter_c = c.begin(); iter_c != c.end(); ++iter_c) 
  {
	if ((*iter_c).type == "input") 
	{
      std::string sim_file = file_name+"."+(*iter_c).name;    
	
	  if (!extract_input_from_file(sim_file, (*iter_c).pins, net_values_map_data, event_scheduler, Cycles_num))
      
		  return false;
    }
  }
  
  return true;
}

bool netlist::extract_input_from_file(std::string file_name, const evl_pins &pins, net_values_map &net_values_map_data, string_list &event_scheduler, int Cycles_num) 
{
  std::ifstream input_file(file_name.c_str());

  if (!input_file) 
  {
    std::cerr << "I can't read " << file_name << "." << std::endl;
    return false;
  }
      size_t pin_index = 0;

	  std::map<size_t, std::string> Ref_pins_map; 
	  std::map<size_t, size_t> Size_pins_map; 
	  std::map<size_t, string_list> List_pins_map; 

	  for (evl_pins::const_iterator iter_c = pins.begin(); iter_c != pins.end(); ++iter_c, pin_index++) 
	  {
        string_list component_pins_z;

	    create_pin(*iter_c, component_pins_z);

	    Size_pins_map[pin_index] = component_pins_z.size();

	    Ref_pins_map[pin_index] = (*iter_c).name;

	    for (string_list::const_iterator iter_cp = component_pins_z.begin(); iter_cp != component_pins_z.end(); ++iter_cp) 
		{
		  List_pins_map[pin_index].push_back(*iter_cp);

		  event_scheduler.push_back(*iter_cp);
        }
	  }	  

	  std::string line;

	  std::getline(input_file, line);

	  std::istringstream word(line);

	  std::string str;

	  size_t word_size;

	  getline(word, str , ' ');

	  std::istringstream s_int(str);
	  s_int >> word_size;
  
	  for (; std::getline(input_file, line);) 
	  {
      std::istringstream word(line);

      getline(word, str , ' ');

      std::istringstream s_int(str);
      s_int >> word_size;

      for (size_t i = 0; i < pins.size() ; ++i) 
	  {
        getline(word, str , ' ');

      std::string String_1, String_2;

      if(List_pins_map[i].size() > 1) 
	  {
		String_1 = str;
	    String_2 = "";

	  while(String_1 != "") 
	  { 
        char Str_Char = Str_to_Char(String_1);

        String_2 = String_2 + Hex_to_BinStr(Str_Char);
      }

	  if (String_2.size() < Size_pins_map[i]) 
	  {
	    while(String_2.size() != Size_pins_map[i]) 
		{
          String_2 = "0" + String_2;
        }
	  }	
	  else if (String_2.size() > Size_pins_map[i]) 
	  {
	    std::cerr << "Error: The data length is greater than the expected in input file '" << file_name << "' for '" << Ref_pins_map[i] << "'" << std::endl;
	    std::cerr << "Expected width: " << Size_pins_map[i] << " but found " << String_2.size() << std::endl;
	  }	
	  int index_i = 0;
	  for (string_list::iterator iter3 = List_pins_map[i].begin(); iter3 != List_pins_map[i].end(); ++iter3,index_i++) 
	  {
        for (size_t k = 0; k < word_size ; ++k)		
			net_values_map_data[*iter3].push_back(String_2[index_i]);		
	  }  
	}
    else 
	{
	    char S_C = Str_to_Char(str);

		string_list::iterator iter_sc = List_pins_map[i].begin();

        for (size_t j = 0; j < word_size ; ++j)
          net_values_map_data[*iter_sc].push_back(S_C);
    }		
	 
    }
  }
  
  string_list::iterator iter3 = List_pins_map[0].begin();
  
  for (size_t k = List_pins_map.size(); k <= unsigned(Cycles_num) ; ++k) 
  {
    for (size_t i = 0; i < pins.size() ; ++i) 
	{
	  string_list::iterator iter3 = List_pins_map[i].begin();

	  char prev_val2 = net_values_map_data[*iter3].back();

 	  for (; iter3 != List_pins_map[i].end(); ++iter3)
        net_values_map_data[*iter3].push_back(prev_val2);
     }
  }	 

  return true;
}

bool netlist::capture_one_zero_values(std::string &input_file_, string_2_list &evl_input_read, const std::string &type_, const std::string &name_, const evl_pins &pins_, net_values_map &net_values_map_data, std::string file_name, string_list &event_scheduler, string_list &event_scheduler_init, int Cycles_num) 
{
  if (type_ == "one") 
  {
    for (evl_pins::const_iterator iter_p = pins_.begin(); iter_p != pins_.end(); ++iter_p) 
	{
      std::list<std::string> component_pins_z;

	  create_pin(*iter_p, component_pins_z); //extracting pins with "one" input

		for (std::list<std::string>::iterator iter_c = component_pins_z.begin(); iter_c != component_pins_z.end(); ++iter_c) 
		{
  	      event_scheduler_init.push_back(*iter_c);

		  for (int i = 1; i <= Cycles_num + 2; ++i) 
		  {
		    net_values_map_data[(*iter_c)].push_back('1'); 
		  }	
		}
    }
  }	
  
  else if (type_ == "zero") 
  {
    for (evl_pins::const_iterator iter_p = pins_.begin(); iter_p != pins_.end(); ++iter_p) 
	{
      std::list<std::string> component_pins_z;

	  create_pin(*iter_p, component_pins_z); //extracting pins with "zero" input

		for (std::list<std::string>::iterator iter_c = component_pins_z.begin(); iter_c != component_pins_z.end(); ++iter_c) 
		{
  	      event_scheduler_init.push_back(*iter_c);

		  for (int i = 1; i <= Cycles_num + 2; ++i) 
		  {
		    net_values_map_data[(*iter_c)].push_back('0');
		  }	
		}
    }
  }	

  return true;  
}

bool netlist::simulation_run(const evl_components &components, std::string file_name, int Cycles_num) 
{
   string_list event_scheduler, event_scheduler_init, DFF_event_scheduler;

   DFF_scheduler_map DFF_event_scheduler_found, event_scheduler_input;

   net_values_map net_values_map_data, net_values_map_data_temp;

   DFF_values_map DFF_values_map_data;

   string_2_list comp_pins_dir_;  
   string_2_list input_data_;

   size_t word_size = 0;

   std::string input_file_;

   evl_components components_cout;
  
   gate_output_map output_gate_name;

   time_t time_begin, time_end; 

   time(&time_begin);

   if(!capture_inputs(input_file_, evl_input_read, components, components_cout, net_values_map_data, file_name, event_scheduler, event_scheduler_init, Cycles_num, comp_pins_dir_, DFF_event_scheduler))
     
	   return false;
       
 	 for (string_list::iterator iter_sch = DFF_event_scheduler.begin(); iter_sch != DFF_event_scheduler.end(); ++iter_sch) 
	 {		    
       DFF_event_scheduler_found[*iter_sch] = 1;
	 }  
 	 
	 for (string_list::iterator iter_sch = event_scheduler.begin(); iter_sch != event_scheduler.end(); ++iter_sch) 
	 {		    
       event_scheduler_input[*iter_sch] = 1;
	 }  

   string_list event_scheduler_temp, event_scheduler_temp1;

   DFF_values_map input_prev_values;

   if(!output_ports_list(output_gate_name, components_cout, file_name))
    
	  return false;  	

   std::cout << "Starting simulation ..." << std::endl;

	nets_list.sort();

   for (int i = 1; i <  Cycles_num + 1; ++i) 
   { 
	 for (string_list::iterator iter_n = nets_list.begin(); iter_n != nets_list.end(); ++iter_n) 
	 {  
		 
	     if(DFF_values_map_data[*iter_n])
	   		net_values_map_data_temp[*iter_n].push_back(DFF_values_map_data[*iter_n]);
		 
	   else 
	   {	
	     if(net_values_map_data[*iter_n].size() > 0) 
		 {
	       net_values_map_data_temp[*iter_n].push_back(net_values_map_data[*iter_n].front());
	       
		   if(event_scheduler_input[*iter_n]) 
		   {
	         if(net_values_map_data[*iter_n].front() != input_prev_values[*iter_n])
		       
				 event_scheduler_temp1.push_back(*iter_n);
 		     
			 input_prev_values[*iter_n] = net_values_map_data[*iter_n].front();
		   }	 
	     }  
		 else  
		   net_values_map_data_temp[*iter_n].push_back('X');
	   }
	     if(net_values_map_data[*iter_n].size() > 0)
           
			 net_values_map_data[*iter_n].pop_front();
     }  
	 
     if(i == 1) 
	 { 
       event_scheduler_temp = DFF_event_scheduler;

	   event_scheduler_temp.splice(event_scheduler_temp.end(), event_scheduler_init);

	   event_scheduler_temp.splice(event_scheduler_temp.end(), event_scheduler);
	 }
	 else 
	 { 
       event_scheduler_temp = event_scheduler_temp1;
	 }
	   event_scheduler_temp1.clear();

	   repeat: 
	   event_scheduler_temp.unique();

	   string_list::iterator iter = event_scheduler_temp.begin();

	   for (evl_components::iterator iter1 = component_map[*iter].begin(); iter1 != component_map[*iter].end(); ++iter1) 
	   {   
		   evaluate(i, input_file_, word_size, input_data_, (*iter1).type, (*iter1).pins, net_values_map_data_temp, i, event_scheduler_temp, DFF_values_map_data, event_scheduler_temp1, DFF_event_scheduler_found);
	   }
	   	 	     
	     event_scheduler_temp.remove(*iter);
		 
		 if(event_scheduler_temp.size() > 0) 
		 { 
   		   goto repeat;
		 }
		 
         if(!simulation_output_write(output_gate_name, components_cout, net_values_map_data_temp))
		 {   
			 return false;
		 }
		
		if(i % 100 == 0) 
		 { 
			 time(&time_end); 

			 std::cout << "cycle " << i << " ... " << difftime(time_end, time_begin) << " seconds"<< std::endl;
		 }  
  }
  
   time(&time_end); 

  std::cout << "Done in " << difftime(time_end, time_begin) << " seconds"<< std::endl;

  return true;
	
}

bool netlist::output_ports_list (gate_output_map &output_gate_name, const evl_components &c, std::string file_name) 
{

  for (evl_components::const_iterator iter2 = c.begin(); iter2 != c.end(); ++iter2) 
  {
      std::string sim_file = file_name + "." + (*iter2).name;
      
	  // create output file for each output port
      
	  std::ofstream output_file(sim_file.c_str());

	  output_gate_name.insert(std::make_pair((*iter2).name, sim_file));

      if (!output_file) 
	  {
        std::cerr << "I can't write " << sim_file << "." << std::endl;
        return false;
      }

	  std::cout << std::endl;
	  std::cout << "Output is written into file: " << sim_file << "." << std::endl;
	  std::cout << std::endl;
	  
	  output_file << (*iter2).pins.size() << std::endl;

      for (evl_pins::const_iterator iter1 = (*iter2).pins.begin(); iter1 != (*iter2).pins.end(); ++iter1) 
	  {
        string_list component_pins_z;
	    create_pin (*iter1, component_pins_z);
	    output_file << "pin " << component_pins_z.size();

		string_list file_output_temp;
	    
		for (string_list::const_iterator iter3 = component_pins_z.begin(); iter3 != component_pins_z.end(); ++iter3) 
		{
		    file_output_temp.push_front(*iter3);
        }

		for (string_list::iterator iter = file_output_temp.begin() ; iter != file_output_temp.end(); ++iter)
		{
			output_file << " " << *iter;
		}

	    output_file << std::endl;
	  }
   }
     return true;
}

void netlist::evaluate(int cycle_num, std::string &input_file_, size_t &word_size, string_2_list &input_data_, const std::string &type_, const evl_pins &pins_, net_values_map &net_values_map_data, int cycle_no, string_list &event_scheduler, DFF_values_map &DFF_values_map_data, string_list &DFF_event_scheduler, DFF_scheduler_map &DFF_event_scheduler_found) 
{
  std::string out_name = "";

  int strobe_, count = 0;

  char net_prev_value = ' ';

  static std::map<std::string, int> DFF_cycle_num;
 
  if (type_ == "and") 
  { 
    strobe_ = 0;
	
    string_list component_pins_i;

	evl_pins::const_iterator iter_p = pins_.begin();

    create_pin(*iter_p, component_pins_i);

    string_list::iterator iter_c = component_pins_i.begin();

	out_name = *iter_c;

    iter_p++;

	net_prev_value = net_values_map_data[out_name].back();

	for (; iter_p != pins_.end(); ++iter_p) 
	{
	  component_pins_i.clear();

      create_pin(*iter_p, component_pins_i);

      string_list::iterator iter_ci = component_pins_i.begin();
      
	  if (net_values_map_data[*iter_ci].back() == '0') 
	  {
        strobe_ = 1;
		break;
      }
    }
	
	if(strobe_ == 1)
	  net_values_map_data[out_name].back() = '0';
	else  
      net_values_map_data[out_name].back() = '1';

	if(net_values_map_data[out_name].back() != net_prev_value)
      event_scheduler.push_back(out_name);
	  
	if(DFF_event_scheduler_found[out_name]) 
	{
	  if(net_values_map_data[out_name].back() != net_prev_value)
	    DFF_event_scheduler.push_back(out_name);
    }
	
  }  
  
  else if (type_ == "or") 
  {
    strobe_ = 0; 

    string_list component_pins_i;
	evl_pins::const_iterator iter_p = pins_.begin();
    
	create_pin(*iter_p, component_pins_i);

    string_list::iterator iter_c = component_pins_i.begin();
	out_name = *iter_c;

    iter_p++;
	net_prev_value = net_values_map_data[out_name].back();
	
	for (; iter_p != pins_.end(); ++iter_p) 
	{
	  component_pins_i.clear();
      create_pin(*iter_p, component_pins_i);

      string_list::iterator iter_ci = component_pins_i.begin();

      if (net_values_map_data[*iter_ci].back() == '1') 
	  {
        strobe_ = 1;
		break;
      }
    }

	if(strobe_ == 1)
	  net_values_map_data[out_name].back() = '1';
	else  
      net_values_map_data[out_name].back() = '0';

	if(net_values_map_data[out_name].back() != net_prev_value)
      event_scheduler.push_back(out_name);

	if(DFF_event_scheduler_found[out_name]) 
	{
	  if(net_values_map_data[out_name].back() != net_prev_value)
	    DFF_event_scheduler.push_back(out_name);
    }		

  }  
  
  else if (type_ == "xor") 
  {
	count = 0;

    string_list component_pins_i;

	evl_pins::const_iterator iter_p = pins_.begin();

    create_pin(*iter_p, component_pins_i);
	
    string_list::iterator iter_ci = component_pins_i.begin();

	out_name = *iter_ci;

    iter_p++;

	net_prev_value = net_values_map_data[out_name].back();
	
	for (; iter_p != pins_.end(); ++iter_p) 
	{
	  component_pins_i.clear();

      create_pin(*iter_p, component_pins_i);

      string_list::iterator iter_c = component_pins_i.begin();

      if (net_values_map_data[*iter_c].back() == '1')
	  {
        count++;
      }
    }

	if(count%2 == 0)
	  net_values_map_data[out_name].back() = '0';
	else  
      net_values_map_data[out_name].back() = '1';

	if(net_values_map_data[out_name].back() != net_prev_value)
      event_scheduler.push_back(out_name);

	if(DFF_event_scheduler_found[out_name]) 
	{
	  if(net_values_map_data[out_name].back() != net_prev_value)
	    DFF_event_scheduler.push_back(out_name);
    }		
  }  
 
  else if (type_ == "not") 
  {
    string_list component_pins_i,component_pins_o; 

	evl_pins::const_iterator iter_p = pins_.begin();

    create_pin(*iter_p, component_pins_i);

	iter_p++;
	    
	create_pin(*iter_p, component_pins_o);

    string_list::iterator iter_ci = component_pins_i.begin();
    string_list::iterator iter_co = component_pins_o.begin();

	out_name = *iter_ci;

	net_prev_value = net_values_map_data[out_name].back();
    
	if (net_values_map_data[*iter_co].back() == '1')
		net_values_map_data[out_name].back() = '0';
	else  
		net_values_map_data[out_name].back() = '1';
		
	if(net_values_map_data[out_name].back() != net_prev_value)
      event_scheduler.push_back(out_name);
 
	if(DFF_event_scheduler_found[out_name]) 
	{
	  if(net_values_map_data[out_name].back() != net_prev_value)
	    DFF_event_scheduler.push_back(out_name);
    }
	 
  }

  else if (type_ == "buf") 
  {
    string_list component_pins_i,component_pins_o;

	evl_pins::const_iterator iter_p = pins_.begin();

    create_pin(*iter_p, component_pins_i);

	++iter_p;
    create_pin(*iter_p, component_pins_o);

    string_list::iterator iter_co = component_pins_o.begin();

   for (string_list::iterator iter_ci = component_pins_i.begin(); iter_ci != component_pins_i.end(); ++iter_ci, ++iter_co) 
   {
	out_name = *iter_ci;
	net_prev_value = net_values_map_data[out_name].back();

    net_values_map_data[out_name].back() = net_values_map_data[*iter_co].back();

	if(net_values_map_data[out_name].back() != net_prev_value)
      event_scheduler.push_back(out_name);
   }
  
	if(DFF_event_scheduler_found[out_name]) 
	{
	  if(net_values_map_data[out_name].back() != net_prev_value)
	    DFF_event_scheduler.push_back(out_name);
    }		
  }
  
  else if (type_ == "dff") 
  {
    string_list component_pins_i,component_pins_o;

	evl_pins::const_iterator iter_p = pins_.begin();

    create_pin(*iter_p, component_pins_i);

	iter_p++;
    create_pin(*iter_p, component_pins_o);

    string_list::iterator iter_ci = component_pins_i.begin();
    string_list::iterator iter_co = component_pins_o.begin();

	out_name = *iter_ci;

    if(cycle_num != DFF_cycle_num[out_name] || !DFF_cycle_num[out_name]) 
	{
	  DFF_cycle_num[out_name] = cycle_num;

      if(DFF_cycle_num[out_name] == 1) 
	  {
  	    DFF_values_map_data[out_name] = '0';
		net_values_map_data[out_name].back() = '0';
	  }	
	  else 
	  {
	    //char net_prev_value = DFF_values_map_data[out_name];

	    net_value_list::iterator iter_nv = net_values_map_data[*iter_co].end();

		--iter_nv;
		--iter_nv;
		
  	    DFF_values_map_data[out_name] = *iter_nv;
		net_values_map_data[out_name].back() = *iter_nv;
	  }
	}
  
	if(DFF_event_scheduler_found[out_name]) 
	{
	  if(net_values_map_data[out_name].back() != net_prev_value)
	    DFF_event_scheduler.push_back(out_name);
    }		
  }
}

bool netlist::simulation_output_write(gate_output_map &output_gate_name, const evl_components &c, net_values_map &net_values_map_data) 
{ 
  for (evl_components::const_iterator iter2 = c.begin(); iter2 != c.end(); ++iter2) 
  {
      std::string sim_file = output_gate_name[(*iter2).name];
	  
      std::ofstream output_file(sim_file.c_str(), std::ios_base::app);
	  	  
      int space_flag = 0;

      for (evl_pins::const_iterator iter1 = (*iter2).pins.begin(); iter1 != (*iter2).pins.end(); ++iter1) 
	  {
        string_list component_pins_z;

	    create_pin(*iter1, component_pins_z);

	    std::string in, in_hex = "";

		string_list::const_iterator iter3 = component_pins_z.begin();
		
		if(component_pins_z.size() > 1) 
		{
			for (; iter3 != component_pins_z.end(); ++iter3) 
			{
 			   in = in + net_values_map_data[*iter3].back();
			}
		
			if(in.size()%4 != 0) 
			{
			  while(in.size()%4 != 0) 
			  {
				in = "0" + in;
			  }
			}	

			while(in != "") 
			{
			  std::string string_seq = Str_to_CharStr(in);

			  string_seq = string_seq + Str_to_CharStr(in);
			  string_seq = string_seq + Str_to_CharStr(in);
			  string_seq = string_seq + Str_to_CharStr(in);

			  in_hex = in_hex + Bin_to_HexVal(string_seq, true);
			}

			if(space_flag == 0) 
			{
		   output_file <<  in_hex;
    		  //out <<  in_hex;
			  space_flag = 1;
			}
        
			else 
			{
 		   output_file << " " << in_hex;
    		  //out << " " << in_hex;
			}		  
        }
		
		else 
		{
 			char net_data = net_values_map_data[*iter3].back();

			if(space_flag == 0) 
			{
		  output_file <<  net_data;
    		  //out <<  net_data;
			  space_flag = 1;
			}
        
			else 
			{
		  output_file << " " << net_data;
    		  //out << " " << net_data;
			}
			 
		}
		
	  }	
	   output_file << std::endl;
	   //out << std::endl;
  }
  return true;
}  

//Other Functions
std::string make_net_name(std::string wire_name, int i) 
{
  assert(i >= 0);

  std::ostringstream oss;

  oss << wire_name << "[" << i << "]";

  return oss.str();
}

bool parse_evl_file(std::string evl_file,evl_wires &wires, evl_components &components , evl_ports &ports) //, evl_modules &modules
{
	evl_tokens tokens;
	evl_statements statements;
	evl_wires_map wires_map;
	evl_ports_map ports_map;
	evl_modules_map modules_map;
	map_evl_modules_line modules_line_map;
	//evl_modules modules;

	if (!extract_tokens_from_file(evl_file, tokens)) 
	{
		return false;
	}
  
  int token_count = tokens.size();

  if (!move_tokens_to_module(tokens, modules_map, modules_line_map)) 
  {
		 return false;
  }
 
  std::cout << std::endl; //Displaying summary of results
  std::cout << evl_file << ": Info: Total token count = " << token_count << "." << std::endl;
  std::cout << evl_file << ": Info: Module(s) count = " << modules_map.size() << "." << std::endl << std::endl;

  for (evl_modules_map::iterator iter = modules_map.begin(); iter != modules_map.end(); ++iter) 
  {
    statements.clear();
	wires.clear();
	components.clear();
	ports.clear();
	wires_map.clear();
	ports_map.clear();

    if (!group_tokens_into_statements(statements, iter->second)) 
	{
      return false;
    }
    evl_statements statements1(statements.begin(), statements.end());
  
    if (!process_all_statements(statements, wires, components, ports, wires_map, ports_map)) 
	{
      return false;
	}
  }
  return 1;
}

#include <iostream>
#include <string>
#include <list>
#include <assert.h>
#include <map>
#include <algorithm>

#include "Lexical.h"
#include "Syntactic.h"

bool is_endmodule(const evl_token &token) 
{
  return token.str == "endmodule";
}

bool is_module(const evl_token &token) 
{
  return token.str == "module";
}

bool operator<(const evl_wire &x, const evl_wire &y)
{
	return x.name < y.name;
}

bool move_tokens_to_statement(evl_tokens &statement_tokens, evl_tokens &tokens) 
{
  assert(statement_tokens.empty());

  assert(!tokens.empty());

  for (; !tokens.empty();) 
  {
    statement_tokens.push_back(tokens.front());
    tokens.erase(tokens.begin()); // consume one token per iteration
    
	if (statement_tokens.back().str == ";")
      break; // exit if the ending ";" is found
  }
  if (statement_tokens.back().str != ";") 
  {
    std::cerr << "Look for ';' but reach the end of file" << std::endl;
    return false;
  }
  return true;
}

bool group_tokens_into_statements(evl_statements &statements, evl_tokens &tokens) 
{
  assert(statements.empty());

  assert(!tokens.empty());

  for (; !tokens.empty();) 
  {
    // generate one statement per iteration

    evl_token token = tokens.front();
    if (token.type != evl_token::NAME)
	{
      std::cerr << "Need a NAME token but found '" << token.str << "' on line " << token.line_no << "." << std::endl;
      return false;
    }

    evl_statement statement;
	statement.tokens.clear(); // be defensive, make it empty

	if (token.str == "endmodule") // ENDMODULE statement
	{ 
	  statement.type = evl_statement::ENDMODULE;
      statement.tokens.push_back(tokens.front());
      tokens.erase(tokens.begin()); // consume one token per iteration
	  statements.push_back(statement);	
      continue; // skip the rest for the current iteration
   }

    if (token.str == "module") // MODULE statement
	{ 
      statement.type = evl_statement::MODULE;
    }
    else if (token.str == "wire") // WIRE statement
	{ 
      statement.type = evl_statement::WIRE;
    }
    else // COMPONENT statement
	{ 
      statement.type = evl_statement::COMPONENT;
    }
    
	if (!move_tokens_to_statement(statement.tokens, tokens))
	{
      return false;
    } 
	 
    assert(statement.tokens.back().str == ";");
    statements.push_back(statement);	
	
  }
	  
  return true;
}

bool move_tokens_to_module(evl_tokens &tokens, evl_modules_map &modules_map, map_evl_modules_line &modules_line_map) 
{
  assert(!tokens.empty());

  for(;!tokens.empty();)
  {
  evl_tokens::iterator iter_1 = std::find_if(tokens.begin(), tokens.end(), is_module);

  evl_tokens::iterator iter_2 = std::find_if(tokens.begin(), tokens.end(), is_module);
  
  ++iter_2;
  
  if (!((*iter_2).type == evl_token::NAME)) 
  {
     std::cerr << "Looking for module NAME but found '" << (*iter_2).str << "' on line " << (*iter_2).line_no << "." <<std::endl;
     return false;
  }
  
  std::string module_name;

  int module_line_no;
  module_name = (*iter_2).str;
  module_line_no = (*iter_2).line_no;
  
  evl_tokens::iterator next_ = std::find_if(tokens.begin(), tokens.end(), is_endmodule);
  
  if (next_ == tokens.end()) 
  {
    std::cerr << "Looking for 'endmodule' but reached the end of file." << std::endl;
    return false;
  }
  // move tokens within [tokens.begin(), next_] to statement_tokens

  ++next_;
  evl_tokens statement_tokens;
  statement_tokens.clear();
  statement_tokens.splice(statement_tokens.begin(), tokens, iter_1, next_);
  
  evl_modules_map::iterator it = modules_map.find(module_name);

  if (it != modules_map.end()) 
  {
    std::cerr << "Error at line " << module_line_no << ": The module '" << module_name << "' already exists." <<  std::endl;  
	return false;
  }

  modules_map[module_name] = statement_tokens;
  modules_line_map[module_name] = module_line_no;
  }

  return true;
}

bool process_wire_statement(evl_wires &wires, evl_statement &s, evl_wires_map &wires_map)
{
  assert(s.type == evl_statement::WIRE);
  
  enum state_type {INIT, WIRE, BUS, BUS_MSB, BUS_COLON, BUS_LSB, BUS_DONE, WIRE_NAME, WIRES, DONE};
  
  state_type state = INIT;
  evl_wire wire;
  int bus_width = 1;

  for (; !s.tokens.empty() && (state != DONE); s.tokens.pop_front()) 
  {
    evl_token t = s.tokens.front();
    
    switch(state) 
	{
      case INIT:
				if (t.str == "wire") 
				{
				  state = WIRE;
				}
				else 
				{
				  std::cerr << "Need 'wire' but found '" << t.str << "' on line " << t.line_no << "." << std::endl;
				  return false;
				}
				break;

      case WIRE:
				if (t.type == evl_token::NAME) 
				{
				  wire.name = t.str; wire.width = 1;
				  wire.line_no = t.line_no;
				  wires.push_back(wire);
		  
				  if (!map_wires(wires, wires_map)) 
				  {
					return false;
				  }
				  state = WIRE_NAME;
				}
				else if (t.str == "[") 
				{
				  state = BUS;
				}
				else 
				{
				  std::cerr << "Need NAME or '[' but found '" << t.str << "' on line " << t.line_no << "." << std::endl;
				  return false;
				}
				break;
		
      case BUS:
				if (t.type == evl_token::NUMBER) 
				{
				  bus_width = atoi(t.str.c_str())+1;
				  state = BUS_MSB;
				}
				else 
				{
				  std::cerr << "Need NUMBER but found '" << t.str << "' on line " << t.line_no << "." << std::endl;
				  return false;
				}
				break;
		
      case BUS_MSB:
				if (t.str == ":") 
				{
				  state = BUS_COLON;
				}
				else 
				{
				  std::cerr << "Need ':' but found '" << t.str << "' on line " << t.line_no << "." << std::endl;
				  return false;
				}	
				break;
		
      case BUS_COLON:
				if (t.str == "0")
				{
				  state = BUS_LSB;
				}
				else 
				{
				  std::cerr << "Need '0' but found '" << t.str << "' on line " << t.line_no << std::endl;
				  return false;
				}	
				break;
		
      case BUS_LSB:
				if (t.str == "]") 
				{
				  state = BUS_DONE;
				}
				else 
				{
				  std::cerr << "Need ']' but found '" << t.str << "' on line " << t.line_no << std::endl;
				  return false;
				}	
				break;
		
      case BUS_DONE:
				if (t.type == evl_token::NAME) 
				{
				  wire.name = t.str; wire.width = bus_width;
				  wire.line_no = t.line_no;
				  wires.push_back(wire);
				  if (!map_wires(wires, wires_map)) 
				  {
					return false;
				  }
				  state = WIRE_NAME;
				}
				else 
				{
				  std::cerr << "Need NAME but found '" << t.str << "' on line " << t.line_no << std::endl;
				  return false;
				}
				break;
		
      case WIRES:
				if (t.type == evl_token::NAME) 
				{
				  wire.name = t.str;
				  wire.line_no = t.line_no;
				  wires.push_back(wire);
		  
				  if (!map_wires(wires, wires_map)) 
				  {
					return false;
				  }
				  state = WIRE_NAME;
				}
				else 
				{
				  std::cerr << "Need NAME but found '" << t.str << "' on line " << t.line_no << "." << std::endl;
				  return false;
				}
				break;
		
      case WIRE_NAME:
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
				  std::cerr << "Need ',' or ';' but found '" << t.str << "' on line " << t.line_no << "." << std::endl;
				  return false;
				}	
				break;
		
      default :	
				assert(false); // shouldn't reach here
				
				return false;
				break;
    }
	
  }	
  if (!s.tokens.empty() || (state != DONE)) 
  {
    std::cerr << "Something wrong with statement." << std::endl;
    return false;
  }
  return true;
}

bool process_component_statement(evl_components &components, evl_statement &s, evl_wires_map &wires_map, evl_ports_map &ports_map) 
{
  assert(s.type == evl_statement::COMPONENT);
  
  enum state_type {INIT, TYPE, NAME, PINS, PIN_NAME, BUS, BUS_MSB, BUS_COLON, BUS_LSB, BUS_DONE, PINS_DONE, DONE};
  
  state_type state = INIT;
  evl_pin pin;
  evl_component comp;   

  for (; !s.tokens.empty() && (state != DONE); s.tokens.pop_front()) 
  {
    evl_token t = s.tokens.front();
    switch(state)
	{

      case INIT:
				if (t.type == evl_token::NAME) 
				{
				  comp.type = t.str;
				  comp.name = "NONE";
				  state = TYPE;
				}
				else 
				{
				  std::cerr << "Need NAME but found '" << t.str << "' on line " << t.line_no << "." << std::endl;
				  return false;
				}
				break;

      case TYPE:
				if (t.type == evl_token::NAME) 
				{
				  comp.name = t.str;
				  state = NAME;
				}
				else if (t.str == "(") 
				{
				  state = PINS;
				}
				else 
				{
				  std::cerr << "Need NAME or '(' but found '" << t.str << "' on line " << t.line_no << "." << std::endl;
				  return false;
				}
				break;
		
      case NAME:
				if (t.str == "(") 
				{
				  state = PINS;
				}
				else 
				{
				  std::cerr << "Need '(' but found '" << t.str << "' on line " << t.line_no << "." << std::endl;
				  return false;
				}
				break;
		
      case PINS:
      // same as the branch for WIRE
				if (t.type == evl_token::NAME)
				{
				  pin.name = t.str; 
				  pin.bus_lsb = -1;
				  pin.bus_msb = -1;
				  state = PIN_NAME;
				}
				else
				{
				  std::cerr << "Need NAME but found '" << t.str << "' on line " << t.line_no << "." << std::endl;
				  return false;
				}
				break;
		
      case PIN_NAME:
				if (t.str == "[") 
				{
				  state = BUS;
				}
				else if (t.str == ",") 
				{
				  if(!match_pins_and_wires(pin, wires_map, ports_map, t)) 
				  {
					return false;
				  }
				  comp.pins.push_back(pin);

				  state = PINS;
				}
				else if (t.str == ")") 
				{
				  if(!match_pins_and_wires(pin, wires_map, ports_map, t)) {
					return false;
				  }
				  comp.pins.push_back(pin);
				  state = PINS_DONE;
				}
				else 
				{
				  std::cerr << "Need '[' or ',' or ')' but found '" << t.str << "' on line " << t.line_no << "." << std::endl;
				  return false;
				}
				break;
		
		
      case BUS:
				if (t.type == evl_token::NUMBER) 
				{
				  pin.bus_msb = atoi(t.str.c_str());
				  state = BUS_MSB;
				}
				else 
				{
				  std::cerr << "Need NUMBER but found '" << t.str << "' on line " << t.line_no << "." << std::endl;
				  return false;
				}
				break;
		
      case BUS_MSB:
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
				  std::cerr << "Need ':' or ']' but found '" << t.str << "' on line " << t.line_no << "." << std::endl;
				  return false;
				}	
				break;
		
      case BUS_COLON:
				if (t.type == evl_token::NUMBER) 
				{
				  pin.bus_lsb = atoi(t.str.c_str());
				  state = BUS_LSB;
				}
				else 
				{
				  std::cerr << "Need NUMBER but found '" << t.str << "' on line " << t.line_no << "." << std::endl;
				  return false;
				}	
				break;
		
      case BUS_LSB:
				if (t.str == "]")
				{
				  state = BUS_DONE;
				}
				else 
				{
				  std::cerr << "Need ']' but found '" << t.str << "' on line " << t.line_no << "." << std::endl;
				  return false;
				}	
				break;
		
      case BUS_DONE:
				if (t.str == ",") 
				{
				  if(!match_pins_and_wires(pin, wires_map, ports_map, t)) 
				  {
					return false;
				  }
				  comp.pins.push_back(pin);
				  state = PINS;
				}
				else if (t.str == ")") 
				{
				  if(!match_pins_and_wires(pin, wires_map, ports_map, t))
				  {
					return false;
				  }
				  comp.pins.push_back(pin);
				  state = PINS_DONE;
				}
				else
				{
				  std::cerr << "Need ',' or ')' but found '" << t.str << "' on line " << t.line_no << "." << std::endl;
				  return false;
				}	
				break;
		
      case PINS_DONE:
				if (t.str == ";") 
				{
				  comp.pin_count = comp.pins.size();
				  components.push_back(comp);
//				  std::cout << "Component - Push Back" << comp.name << " " << " " << comp.pin_count << " " << comp.type << " " << std::endl;
				  state = DONE;
				}
				else 
				{
				  std::cerr << "Need ';' but found '" << t.str << "' on line " << t.line_no << "." << std::endl;
				  return false;
				}	
				break;
		
      default :	
				assert(false); // shouldn't reach here
				return false;
				break;
    
    }
	
  }	
  if (!s.tokens.empty() || (state != DONE)) 
  {
    std::cerr << "Something wrong with the statement." << std::endl;
    return false;
  }
  return true;
}

bool process_all_statements(evl_statements &statements, evl_wires &wires, evl_components &components, evl_ports &ports, evl_wires_map &wires_map, evl_ports_map &ports_map) {
  int count = 1;

  for (evl_statements::iterator iter = statements.begin(); iter != statements.end(); ++iter, ++count) 
  {
    if ((*iter).type == evl_statement::WIRE) 
	{
      if (!process_wire_statement(wires, *iter, wires_map)) 
	  {
        return false;
      }
    }
    else if ((*iter).type == evl_statement::COMPONENT) 
	{
      if (!process_component_statement(components,*iter, wires_map, ports_map)) 
	  {
        return false;
      } 
    }
    else if ((*iter).type == evl_statement::ENDMODULE) 
	{
  	  for (evl_tokens::iterator iter1 = (*iter).tokens.begin(); iter1 != (*iter).tokens.end(); ++iter1) 
	  {
        if ((*iter1).str != "endmodule") 
		{
          std::cerr << "Need 'endmodule' but found '" << (*iter1).str << "' on line " << (*iter1).line_no << "." << std::endl;
          return false;
		}
      } 
    }
  }
  return true;
}  

bool map_wires(evl_wires &wires, evl_wires_map &wires_map) 
{
    evl_wires_map::iterator it = wires_map.find(wires.back().name);
    
	if (it != wires_map.end()) 
	{
      std::cerr << "Error at line " << wires.back().line_no << ": The wire '" << wires.back().name << "' already exists." <<  std::endl;  
	  return false;
	}
    wires_map[wires.back().name] = wires.back().width;

    return true;    
}

bool map_ports(evl_ports &ports, evl_ports_map &ports_map) 
{
    evl_ports_map::iterator it = ports_map.find(ports.back().name);

    if (it != ports_map.end()) 
	{
      std::cerr << "Error at line " << ports.back().line_no << ": The port '" << ports.back().name << "' already exists." <<  std::endl;  
	  return false;
	}
    ports_map[ports.back().name] = ports.back().width;
    return true;    
}

bool match_pins_and_wires(evl_pin &pin, evl_wires_map &wires_map, evl_ports_map &ports_map, const evl_token &t) 
{
    evl_wires_map::iterator iter_3 = wires_map.find(pin.name);
    evl_ports_map::iterator iter_4 = ports_map.find(pin.name);

	if (iter_3 != wires_map.end()) 
	{ 
      if (wires_map[pin.name] != 1 && pin.bus_msb == -1 && pin.bus_lsb == -1)  
	  { 
        pin.bus_msb = wires_map[pin.name] - 1;
        pin.bus_lsb = 0;
	  }
	  else if (pin.bus_msb != pin.bus_lsb && pin.bus_lsb == -1)  
	  { 
	    if (!((wires_map[pin.name] > pin.bus_msb) && (pin.bus_msb >= 0))) 
		{
          std::cerr << "The wire '" << pin.name << "' does not satisfy the condition [width > msb >= 0] on line " << t.line_no << "." << std::endl;
          return false;
	    }
	    else if (((wires_map[pin.name] == 1) && (pin.bus_msb >= 0))) 
		{
          std::cerr << "The wire '" << pin.name << "' is not a bus on line " << t.line_no << "." << std::endl;
          return false;
	    }
	    pin.bus_lsb = pin.bus_msb;
	  }
	  else if (wires_map[pin.name] == 1 && pin.bus_msb == -1 && pin.bus_lsb == -1)  
	  { 
		;
	  }
	  else 
	  { 
	    if (!((wires_map[pin.name] > pin.bus_msb) && (pin.bus_msb >= pin.bus_lsb) && (pin.bus_lsb >= 0))) 
		{
          std::cerr << "The wire '" << pin.name << "' does not satisfy the condition [width > msb >= lsb >= 0] on line " << t.line_no << "." << std::endl;
          return false;
	    }
	  }
	} 
    else if(iter_4 != ports_map.end())
	{ 
      if (ports_map[pin.name] != 1 && pin.bus_msb == -1 && pin.bus_lsb == -1)  
	  { 
        pin.bus_msb = ports_map[pin.name] - 1;
        pin.bus_lsb = 0;
	  }
	  else if (pin.bus_msb != pin.bus_lsb && pin.bus_lsb == -1)  
	  { 
	    if (!((ports_map[pin.name] > pin.bus_msb) && (pin.bus_msb >= 0))) 
		{
          std::cerr << "The port '" << pin.name << "' does not satisfy the condition [width > msb >= 0] on line " << t.line_no << "." << std::endl;
          return false;
	    }
	    else if (((ports_map[pin.name] == 1) && (pin.bus_msb >= 0))) 
		{
          std::cerr << "The port '" << pin.name << "' is not a bus on line " << t.line_no << "." << std::endl;
          return false;
	    }
	    pin.bus_lsb = pin.bus_msb;
	  }
	  else if (ports_map[pin.name] == 1 && pin.bus_msb == -1 && pin.bus_lsb == -1)  
	  { 
		;
	  }
	  else 
	  { 
	    if (!((ports_map[pin.name] > pin.bus_msb) && (pin.bus_msb >= pin.bus_lsb) && (pin.bus_lsb >= 0))) 
		{
          std::cerr << "The port '" << pin.name << "' does not satisfy the condition [width > msb >= lsb >= 0] on line: " << t.line_no << "." << std::endl;
          return false;
	    }
	  }
	} 
	
    return true;    
}

void display_syntactic(std::ostream &out, const evl_statements &statements, evl_wires &wires, const evl_components &components) 
{
  for (evl_statements::const_iterator iter = statements.begin(); iter != statements.end(); ++iter) 
  {
    if ((*iter).type == evl_statement::MODULE) 
	{
	  for (evl_tokens::const_iterator iter1 = (*iter).tokens.begin(); iter1 != (*iter).tokens.end(); ++iter1) 
	  {
        if ((*iter1).str != ";") 
		{
          out << (*iter1).str << " ";
	    }	
	  }
      out << wires.size() << " " << components.size() <<  std::endl;	  
    }
  }	
  wires.sort();

  for (evl_wires::iterator iter1 = wires.begin(); iter1 != wires.end(); ++iter1) 
  {
    out << "wire " << (*iter1).name << " " << (*iter1).width <<  std::endl;
  }	
      
  for (evl_components::const_iterator iter1 = components.begin(); iter1 != components.end(); ++iter1) 
  {
    out << "component " << (*iter1).type << " " << (*iter1).name << " " << (*iter1).pin_count <<  std::endl;
	
	for (evl_pins::const_iterator iter2 = (*iter1).pins.begin(); iter2 != (*iter1).pins.end(); ++iter2) 
	{
      out << "pin " << (*iter2).name << " " << (*iter2).bus_msb << " " << (*iter2).bus_lsb <<  std::endl;
    }	
  }	
}


