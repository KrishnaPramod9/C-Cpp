#ifndef LEXICAL_H
#define LEXICAL_H

#include <iostream>
#include <fstream>
#include <string>
#include <list>

struct evl_token 
{
  enum token_type {NAME, NUMBER, SINGLE};
  token_type type;
  std::string str;
  int line_no;
}; // struct evl_token

typedef std::list<evl_token> evl_tokens;

bool is_character_a_space(char ch);

bool is_character_a_letter(char ch);

bool is_character_a_digit(char ch);

bool extract_tokens_from_line(std::string line, int line_no, evl_tokens &tokens);

bool extract_tokens_from_file(std::string file_name, evl_tokens &tokens);

void display_tokens(std::ostream &out, const evl_tokens &tokens);

#endif


