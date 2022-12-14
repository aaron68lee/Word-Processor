#ifndef STUDENTSPELLCHECK_H_
#define STUDENTSPELLCHECK_H_

#include "SpellCheck.h"

#include <string>
#include <vector>

class StudentSpellCheck : public SpellCheck {
public:
	StudentSpellCheck();
	virtual ~StudentSpellCheck();
	bool load(std::string dict_file);
	bool spellCheck(std::string word, int maxSuggestions, std::vector<std::string>& suggestions);
	void spellCheckLine(const std::string& line, std::vector<Position>& problems);

private:
	int maxsize = 30;
	struct Node
	{
		bool endword;
		char letter;
		std::vector<Node*> child;
	};

	Node* root; // root of the current dictionary in use
	void printTree(); // prints out trie for debugging
	void printHelper(Node* cur, std::string path); // helper for printer
	void deleteTree(Node* cur); // helper for destructor
	bool findWord(Node* cur, std::string word);

};

#endif  // STUDENTSPELLCHECK_H_
