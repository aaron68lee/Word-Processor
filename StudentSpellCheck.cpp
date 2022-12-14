#include "StudentSpellCheck.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <cctype>
#include <algorithm>

SpellCheck* createSpellCheck()
{
	return new StudentSpellCheck;
}

StudentSpellCheck::StudentSpellCheck() // constructor
{
	root = nullptr;
}

void StudentSpellCheck::printTree() // prints the tree using Pre-Order Traversal
{
	printHelper(root, "");
}

void StudentSpellCheck::printHelper(Node* cur, std::string path) // print helper
{
	if (cur == nullptr)
		return;

	for (int i = 0; i < cur->child.size(); i++) // print each child of this cur node (recursively)
	{
		path += '\t';
		std::cout << path << cur->letter;
		printHelper(cur->child[i], path);
	}
}

StudentSpellCheck::~StudentSpellCheck() // deletes the trie using Post-Order traversal
{
	deleteTree(root);
}

void StudentSpellCheck::deleteTree(Node* cur) // delete helper for destructor 
{
	if (cur == nullptr)
		return;

	for (int i = 0; i < cur->child.size(); i++) // delete each child of this node recursively then delete this node
	{
		deleteTree(cur->child[i]);
		delete cur;
	}
}

bool StudentSpellCheck::findWord(Node* cur, std::string word) // returns whether word is in tree given starting node
{
	Node* temp = cur;
	bool found = false;

	for (int i = 0; i < word.length(); i++) // interate through all letters in word
	{
		found = false;
		for (int j = 0; j < temp->child.size(); j++) // iterate through all children
		{
			if (temp->child[j]->letter == word[i])
			{
				temp = temp->child[j]; // move down tree if found matching letter
				found = true;
				break;
			}
		}
		if (!found) // if no children match this letter, word is invalid
			return false;
	}

	return temp->endword; // return if this last letter is actually the end of a valid word
}

bool StudentSpellCheck::load(std::string dictionaryFile) // construct trie from a dictionary file
{
	
	std::ifstream read(dictionaryFile);
	std::string s; // temp for each line that we read

	if (!read)
	{
		std::cout << "Failed to open and read dictionary." << std::endl;
		return false;
	}
	else
	{
		if (root == nullptr) // assume dict has at least 1 word when loaded
		{
			root = new Node();
			root->letter = '\0';
			root->endword = false; 
		}

		Node* temp = root;

		while (getline(read, s)) // add each word in dict file into trie
		{
			temp = root;
			bool childFound = false;

			for (int i = 0; i < s.length(); i++) // iterate through each letter in line s
			{
				int k = 0;
				childFound = false;

				for (auto it = temp->child.begin(); it != temp->child.end(); it++)
				{
					if (s[i] == (*it)->letter) // found node with same letter 
					{
						temp = temp->child[k]; // move down tree to find insertion point
						childFound = true;
						break;
					}
					k++; // for debugging
				}

				if (!childFound) // make a new node for this letter child if none previously existed
				{
					Node* p = new Node();
					p->endword = false;
					p->letter = s[i];

					temp->child.push_back(p);
					temp = p;
				}
				else
				{
					// already at correct Node
				}
			}

			temp->endword = true; // this is a valid word
		}
		return true;
	}
	
}

bool StudentSpellCheck::spellCheck(std::string word, int max_suggestions, std::vector<std::string>& suggestions) 
{
	Node* temp = root;
	int max = max_suggestions;
	std::string altWord = "";
	std::string mword = word;
	int loc = -2;

	if (findWord(root, mword))
	{
		return true;
	}

	for (int i = 0; i < word.length(); i++) // change entire parameter word to lowercase
	{
		mword[i] = tolower(word[i]);
	}

	suggestions.clear(); // clear suggestions for new ones

	for (int i = 0; i < mword.length() && loc != -1; i++) // iterate through each letter in word
	{
		loc = -1;
		for (int j = 0; j < temp->child.size(); j++) // iterate through all children of temp
		{
			if (temp->child[j]->letter != mword[i]) // child is alternative letter 
			{
				if (i != mword.length() - 1)
				{
					if (findWord(temp->child[j], mword.substr(i + 1)) && max > 0) // rest of word can be found in tree
					{
						altWord = mword.substr(0, i) + temp->child[j]->letter + mword.substr(i + 1); // add correct word to suggestions
						suggestions.push_back(altWord);
						max--;
					}
				}
				else
				{
					if (temp->child[j]->endword && max > 0) // this is end of a word up to this point in the path of the tree
					{
						altWord = mword.substr(0, i) + temp->child[j]->letter;
						suggestions.push_back(altWord);
						max--;
					}
				}
			}
			else
			{
				loc = j; // save loc when current letter matches child
			}
		}

		// move down to next letter

		if (loc != -1)
		{
			temp = temp->child[loc]; // temp is now i-th letter of word
		}
	}

	return false;
}

void StudentSpellCheck::spellCheckLine(const std::string& line, std::vector<SpellCheck::Position>& problems) 
{
	std::string word = "";
	int startPos = -1;
	int size = line.length();

	for (int i = 0; i < size + 1; i++)
	{
		if ((line[i] >= 'a' && line[i] <= 'z') || (line[i] >= 'A' && line[i] <= 'Z') || line[i] == 39) // is letter or apostrophe 
		{
			if (word == "")
				startPos = i; // mark starting position of word
			word += tolower(line[i]);
		}
		else
		{
			if (word != "" && !findWord(root, word)) // check to see if word is in dictionary tree
			{
				Position p;
				p.start = startPos;
				p.end = i;

				problems.push_back(p); // add mispelled word to problems
			}
			word = ""; // reset word
		} 

		if (word.length() == size && !findWord(root, word)) // user hit enter
		{
			Position p;
			p.start = startPos;
			p.end = i;

			problems.push_back(p);
			word = "";
		}
	}
}
