#ifndef LLM_DATASET_hpp
#define LLM_DATASET_hpp

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

// <role, content>
using PromptItem = std::pair<std::string, std::string>; // <role, content>

// parse csv
std::vector<std::vector<std::string>> parse_csv(const std::vector<std::string>& lines);
void parse_json(std::string& data, std::vector<std::vector<std::vector<PromptItem>>>& dialogs);
void parse_jsonl(std::string prompt_file, std::vector<std::vector<std::vector<PromptItem>>>& dialogs);

std::string getPPLType(std::string dataset_name);
std::vector<std::string> rowsplit(std::string prompt_file);
std::vector<std::string> plaintext(std::string prompt_file);
std::vector<std::string> wikitext(std::string prompt_file);
std::vector<std::vector<std::vector<PromptItem>>> shareGPT(std::string prompt_file, int sample_size=-1); // -1: no sampling


#endif // LLM_DATASET_hpp