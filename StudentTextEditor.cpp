#include "StudentTextEditor.h"
#include "Undo.h"
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>

TextEditor* createTextEditor(Undo* un)
{
	return new StudentTextEditor(un);
}

StudentTextEditor::StudentTextEditor(Undo* undo)
: TextEditor(undo) {
	r = 0;
	c = 0;
	list.push_back("");
	row = list.begin();
}

StudentTextEditor::~StudentTextEditor()
{
// delete list?
}

bool StudentTextEditor::load(std::string file)
{
	std::ifstream read(file);
	std::string s; // temp for each line that we read

	if (!read)
	{
		std::cout << "Failed to open and read file." << std::endl;
		return false;
	}
	else
	{
		reset();

		while (getline(read, s))
		{
			list.push_back(s); // add lines into list
			std::cout << s << std::endl; // display lines as you add them
		}

		r = 0; // move cursor to start
		c = 0;
		row = list.begin();
		return true;
	}
}

bool StudentTextEditor::save(std::string file)
{
	std::ofstream write(file);

	if (!write)
	{
		std::cout << "Failed to open and write file." << std::endl;
		return false;
	}
	else
	{
		for (auto it = list.begin(); it != list.end(); it++)
		{
			write << *it << '\n';
		}
		return true;
	}
}

void StudentTextEditor::reset()
{
	list.clear();
	r = 0;
	c = 0;
	row = list.begin();
}

void StudentTextEditor::move(Dir dir) 
{
	//studentUndo->submit(Undo::Action::ERROR, -1, -1, "");

	switch (dir)
	{
		case Dir::UP:
			if (r != 0)
			{
				int size = (*row).length();
				r--;
				row--;

				if (size > (*row).length() && c > (*row).length()) // moving cursor up when current row longer than previous
					c = (*row).length();
			}	
			break;

		case Dir::DOWN:
			if (r != list.size() - 1)
			{
				int size = (*row).length();
				r++;
				row++;

				if (size > (*row).length() && c > (*row).length()) // moving cursor down when current row longer than next
					c = (*row).length();

			}
			break;

		case Dir::LEFT:
			if (r != 0 || c != 0)
			{
				if (c == 0)
				{
					r--;
					row--;
					c = (*row).length(); // string size of prev row
				}
				else
				{
					c--;
				}
			}
			break;

		case Dir::RIGHT:
			if (r != list.size() - 1 || c != (*row).length())
			{
				if (c == (*row).length())
				{
					r++;
					row++;
					c = 0;
				}
				else
				{
					c++;
				}
			}
			break;

		case Dir::HOME:
			c = 0;
			break;

		case Dir::END:
			c = (*row).length();
			break;
	}
}

void StudentTextEditor::del() 
{
	if ((r != list.size() - 1) || c != (*row).length()) // do nothing if r == last and c == last
	{
		if (c == (*row).length() && list.size() > 1) // if at end of line merge rows
		{
			row++;
			std::string s = *(row); // get contents of next row

			row = list.erase(row); // erase next row
			row--; // back to original row

			*row = *row + s;

			// add JOIN to the undo stack

			getUndo()->submit(Undo::Action::JOIN, r, c, '\0');
		}
		else
		{
			std::string s = *row;

			// ADD deletion to undo stack

			getUndo()->submit(Undo::Action::DELETE, r, c, (*row)[c]);

			*row = s.substr(0, c) + s.substr(c + 1);
		}
	}
}

void StudentTextEditor::backspace() 
{
	if (r != 0 || c != 0) // do nothing if r == 0 and c == 0
	{
		if (c > 0)
		{
			std::string s = *row;
			
			
			// ADD backspace to undo stack

			getUndo()->submit(Undo::Action::DELETE, r, c - 1, s[c - 1]); // backspace gets rid of char behind cursor
			*row = s.substr(0, c - 1) + s.substr(c);

			c--;
		}
		else if (c == 0) // merge rows if cursor at beginning of line
		{
			std::string s = *row;
			row = list.erase(row);
			row--;
			r--;
			c = (*row).length();
			*row = *row + s;

			// add JOIN to undo stack

			getUndo()->submit(Undo::Action::JOIN, r, c, '\0');
		}
	}
}

void StudentTextEditor::insert(char ch)
{
	if (ch == '\t') // user hit tab
	{
		if (list.size() > 0)
		{
			std::string s = *row;

			if (c == s.length())
			{
				*row = s.substr(0, c) + "    ";
			}
			else
			{
				*row = s.substr(0, c) + "    " + s.substr(c) + ""; // insert char
			}
			c += 4;
		}
	}
	else
	{
		if (list.size() > 0)
		{
			std::string s = *row; // gets current row
			if (c == s.length()) // if cursor at end of row
			{
				*row = s.substr(0, c) + ch;
			}
			else
			{
				*row = s.substr(0, c) + ch + s.substr(c) + ""; // insert char in the middle of row
			}
			c++;
		}
	}

	// ADD insertion to undo stack

	getUndo()->submit(Undo::Action::INSERT, r, c, ch);
}

void StudentTextEditor::enter() 
{
	// add enter onto Undo stack

	getUndo()->submit(Undo::Action::SPLIT, r, c, '\0');

	std::string first = (*row).substr(0, c);
	std::string second;

	if (c == (*row).length())
	{
		second = "";
	}
	else
	{
		second = (*row).substr(c);
	}

	(*row) = first;
	row++;

	list.insert(row, second);
	r++;
	row--;
	c = 0;
}

void StudentTextEditor::getPos(int& row, int& col) const
{
	row = r;
	col = c;
}

int StudentTextEditor::getLines(int startRow, int numRows, std::vector<std::string>& lines) const 
{
	lines.clear();
	int s = list.size();
	int max;

	if (startRow < 0 || numRows < 0)
	{
		return -1;
	}

	max = std::min(numRows, s);

	std::list<std::string>::iterator it;
	it = row;

	if (r > startRow) // moving row iterator to startrow
	{
		for (int i = r; i > startRow; i--)
			it--;
	}
	else
	{
		for (int i = r; i < startRow; i++)
			it++;
	}
	

	for (int i = startRow; i <= startRow + max && it != list.end(); i++) // add numRows to the vector
	{
		lines.push_back(*it);
		it++;
	}
	
	return max;
}

void StudentTextEditor::undo() 
{
	int mr = r;
	int mc = c; // temp copy of current row and col
	int prevR; // previous row before row iterator moves
	int count = 0;
	std::string text = "";
	std::string first = ""; 
	std::string second = "";

	Undo::Action undotype = getUndo()->get(mr, mc, count, text);

	switch (undotype)
	{
		case Undo::Action::INSERT: // undo an insertion (delete char batch)

			prevR = r;
			r = mr;
			c = mc;

			// move row iterator to correct row for insertion of string

			if (r > prevR) // moving row iterator to its new row
			{
				for (int i = prevR; i < r; i++)
					row++;
			}
			else
			{
				for (int i = prevR; i > r; i--)
					row--;
			}

			for (int i = 0; i < count; i++) // do count number of deletions
			{
				// start backspace

				if (r != 0 || c != 0) // do nothing if r == 0 and c == 0
				{
					if (c > 0)
					{
						std::string s = *row;
						*row = s.substr(0, c - 1) + s.substr(c);

						c--;
					}
					else if (c == 0) // merge rows if cursor at beginning of line
					{
						std::string s = *row;
						row = list.erase(row);
						row--;
						r--;
						c = (*row).length();
						*row = *row + s;
					}
				}

				// end backspace
			}
			break;

		case Undo::Action::DELETE: // undo a deletion (insert string)

			prevR = r;
			r = mr;
			c = mc;

			// move row iterator to correct row for insertion of string

			if (r > prevR) // moving row iterator to its new row
			{
				for (int i = prevR; i < r; i++)
					row++;
			}
			else
			{
				for (int i = prevR; i > r; i--)
					row--;
			}

			for (int i = 0; i < text.size(); i++) // insert the string
			{
				// start insert

				if (text[i] == '\t') // user hit tab
				{
					if (list.size() > 0)
					{
						std::string s = *row;

						if (c == s.length())
						{
							*row = s.substr(0, c) + "    ";
						}
						else
						{
							*row = s.substr(0, c) + "    " + s.substr(c) + ""; // insert char
						}
						c += 4;
					}
				}
				else
				{
					if (list.size() > 0)
					{
						std::string s = *row; // gets current row
						if (c == s.length()) // if cursor at end of row
						{
							*row = s.substr(0, c) + text[i];
						}
						else
						{
							*row = s.substr(0, c) + text[i] + s.substr(c) + ""; // insert char in the middle of row
						}
						c++;
					}
				}

				// end insert

				c--; // move cursor back one space
			}
			//c -= text.size(); // move cursor back to beginning of string
			break;

		case Undo::Action::SPLIT: // undo an enter (merge lines)

			prevR = r;
			r = mr;
			c = mc;

			// move row iterator to correct row for insertion of string

			if (r > prevR) // moving row iterator to its new row
			{
				for (int i = prevR; i < r; i++)
					row++;
			}
			else
			{
				for (int i = prevR; i > r; i--)
					row--;
			}

			// moving r to r-undo special case for enter

			r++;
			row++;

			// start backspace

			if (r != 0 || c != 0) // do nothing if r == 0 and c == 0
			{
				std::string s = *row;
				row = list.erase(row);
				row--;
				r--;
				c = (*row).length();
				*row = *row + s;
			}

			// end backspace
			break;
		case Undo::Action::JOIN: // undo line merge (press enter)

			prevR = r;
			r = mr;
			c = mc;

			// move row iterator to correct row for insertion of string

			if (r > prevR) // moving row iterator to its new row
			{
				for (int i = prevR; i < r; i++)
					row++;
			}
			else
			{
				for (int i = prevR; i > r; i--)
					row--;
			}

			// start enter

			first = (*row).substr(0, c);

			if (c == (*row).length())
			{
				second = "";
			}
			else
			{
				second = (*row).substr(c);
			}

			(*row) = first;
			row++;

			list.insert(row, second);
			r++;
			row--;
			c = 0;

			// end enter
			r--;
			row--;
			c = (*row).length();
			break;
		case Undo::Action::ERROR:
			break;
	}
}
