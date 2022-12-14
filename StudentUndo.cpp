#include "StudentUndo.h"

Undo* createUndo()
{
	return new StudentUndo;
}

void StudentUndo::submit(const Action action, int row, int col, char ch) 
{
	actionCombo* act = new actionCombo();
	bool noBatch = true;

	if (!actions.empty()) // stack not empty and top action is batchable
	{
		actionCombo* top = actions.top();

		switch (action)
		{

			case Action::DELETE: // I deleted (backspaces and deletes) and want to batch insertions

				if (action == top->a && top->col == col && top->row == row) // batch deletions
				{
					std::string s;
					s += ch + top->s;
					actions.top()->s = s;
					actions.top()->row = row;
					actions.top()->col = col;
					noBatch = false;
				}

				else if (action == top->a && top->col == col + 1 && top->row == row) // batch backspaces
				{
					std::string s;
					s += top->s + ch;
					actions.top()->s = s;
					actions.top()->row = row;
					actions.top()->col = col;
					noBatch = false;
				}
				break;

			case Action::INSERT: // I inserted and want to batch deletions (backspaces)

				if (action == top->a && top->col == col - 1 && top->row == row)
				{
					std::string s;
					s += top->s + ch;
					actions.top()->s = s;
					actions.top()->row = row;
					actions.top()->col = col;
					noBatch = false;
				}
				break;

			default:
				// join or merge lines
				break;
		}
	}

	if (noBatch)
	{
		act->a = action;
		act->row = row;
		act->col = col;
		std::string s;
		s += ch;
		act->s = s;
		actions.push(act); // either batched or new action
	}
}

StudentUndo::Action StudentUndo::get(int &row, int &col, int& count, std::string& text) 
{
	actionCombo* prev;

	if (actions.empty())
		return Action::ERROR;
	else
	{
		prev = actions.top();
	}
	
	switch (prev->a)
	{
		case Action::INSERT: // I just inserted and want to batch deletions

			count = prev->s.size();
			row = prev->row;
			col = prev->col;
			text = "";
			actions.pop();
			break;

		case Action::DELETE: // I just deleted and want to batch insertions

			count = 1;
			row = prev->row;
			col = prev->col;
			text = prev->s;
			actions.pop();
			break;
		
		case Action::JOIN:
		case Action::SPLIT:
			row = prev->row;
			col = prev->col;
			actions.pop();
			count = 1;
			text = "";
			break;
	}

    return prev->a;
}

void StudentUndo::clear() 
{
	while (!actions.empty())
	{
		actions.pop();
	}
}
