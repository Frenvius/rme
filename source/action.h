//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
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
//////////////////////////////////////////////////////////////////////
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/action.h $
// $Id: action.h 309 2010-02-26 17:38:36Z admin $

#ifndef RME_ACTION_H_
#define RME_ACTION_H_

#include "position.h"

#include <deque>

class Editor;
class Tile;
class House;
class Waypoint;
class Change;
class Action;
class BatchAction;
class ActionQueue;

enum ChangeType {
	CHANGE_NONE,
	CHANGE_TILE,
	CHANGE_MOVE_HOUSE_EXIT,
	CHANGE_MOVE_WAYPOINT,
};

class Change {
private:
	ChangeType type;
	void* data;

	Change();
public:
	Change(Tile* tile);
	static Change* Create(House* house, Position where);
	static Change* Create(Waypoint* wp, Position where);
	~Change();
	void clear();
	
	ChangeType getType() const {return type;}
	void* getData() const {return data;}

	// Get memory footprint
	uint memsize() const;

	friend class Action;
};

typedef std::vector<Change*> ChangeList;

// A dirty list represents a list of all tiles that was changed in an action
class DirtyList {
public:
	DirtyList();
	~DirtyList();

	struct ValueType {
		uint32_t pos;
		uint32_t floors;
	};
	
	uint32_t owner;

protected:
	struct Comparator {
		bool operator()(const ValueType& a, const ValueType& b) const {
			return a.pos < b.pos;
		}
	};
public:

	typedef std::set<ValueType, Comparator> SetType;

	void AddPosition(int x, int y, int z);
	void AddChange(Change* c);
	bool Empty() const {return iset.empty() && ichanges.empty();}
	SetType& GetPosList();
	ChangeList& GetChanges();

protected:
	SetType iset;
	ChangeList ichanges;
};



enum ActionIdentifier {
	ACTION_MOVE,
	ACTION_REMOTE,
	ACTION_SELECT,
	ACTION_DELETE_TILES,
	ACTION_CUT_TILES,
	ACTION_PASTE_TILES,
	ACTION_RANDOMIZE,
	ACTION_BORDERIZE,
	ACTION_DRAW,
	ACTION_SWITCHDOOR,
	ACTION_ROTATE_ITEM,
	ACTION_CHANGE_PROPERTIES,
};

class Action {
public:
	virtual ~Action();

	void addChange(Change* t) {
		changes.push_back(t);
	}

	// Get memory footprint
	size_t approx_memsize() const;
	size_t memsize() const;
	size_t size() const {return changes.size();}
	ActionIdentifier getType() const {return type;}

	void commit(DirtyList* dirty_list);
	bool isCommited() const {return commited;}
	void undo(DirtyList* dirty_list);
	void redo(DirtyList* dirty_list) {commit(dirty_list);}


protected:
	Action(Editor& editor, ActionIdentifier ident);

	bool commited;
	ChangeList changes;
	Editor& editor;
	ActionIdentifier type;

	friend class ActionQueue;
};

typedef std::vector<Action*> ActionVector;

class BatchAction {
public:
	virtual ~BatchAction();

	void resetTimer() {timestamp = 0;}

	// Get memory footprint
	size_t memsize(bool resize = false) const;
	size_t size() const {return batch.size();}
	ActionIdentifier getType() const {return type;}

	virtual void addAction(Action* action);
	virtual void addAndCommitAction(Action* action);


protected:
	BatchAction(Editor& editor, ActionIdentifier ident);

	virtual void commit();
	virtual void undo();
	virtual void redo();

	void merge(BatchAction* other);

	Editor& editor;
	int timestamp;
	uint memory_size;
	ActionIdentifier type;
	ActionVector batch;

	friend class ActionQueue;
};

class ActionQueue {
public:
	ActionQueue(Editor& editor);
	~ActionQueue();
	
	typedef std::deque<BatchAction*> ActionList;

	void resetTimer();
	
	virtual Action* createAction(ActionIdentifier ident);
	virtual Action* createAction(BatchAction* parent);
	virtual BatchAction* createBatch(ActionIdentifier ident);

	void addBatch(BatchAction* action, int stacking_delay = 0);
	void addAction(Action* action, int stacking_delay = 0);

	void undo();
	void redo();
	void clear();

	bool canUndo() {return current > 0;}
	bool canRedo() {return current < actions.size();}
	
protected:
	ActionList actions;
	Editor& editor;
	size_t current;
	size_t memory_size;
};

#endif