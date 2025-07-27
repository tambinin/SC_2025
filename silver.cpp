#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>

// TODO : tout le monde attend 3

using namespace std;

int	i_abs(int a)
{
	if (a < 0)
		return (-a);
	return (a);
}

int max4index(int a[4])
{
	int max = a[0];
	int	res = 0; 
	for (int i = 0; i < 4; ++i)
	{
		if (max < a[i])
		{
			max = a[i];
			res = i;
		}
	}
	return (res);
}

enum	type
{
	EMPTY,
	LOW,
	HIGH,
	AGENT,
	ME,
	OP
};

class Point
{
	public :
		int x;
		int y;
		Point(void) : x(0), y(0) {};
		Point(int x, int y) : x(x), y(y) {};
		Point(Point const & p) : x(p.x), y(p.y) {};

		Point& operator=(Point const & p)
		{
			this->x = p.x;
			this->y = p.y;
			return (*this);
		};

		bool	operator==(Point const & p)
		{
			return (this->x == p.x && this->y == p.y);
		};

		static int manhDist(Point const & a, Point const & b)
		{
			return (i_abs(a.x - b.x) + i_abs(a.y - b.y));
		}

		static bool isAdj(Point const & a, Point const & b)
		{
			return (i_abs(a.x - b.x) <= 1 && i_abs(a.y - b.y) <= 1);
		}
};

class Agent;

class Map
{
	public :
		int height;
		int width;
		Agent ***grid;
		Map(void);
		~Map(void);
		bool inside(Point const & a) const;
		bool inside(int x, int y) const;
		enum type getVal(Point const & a) const;
		enum type getVal(int x, int y) const;
		void zero(void);
		void update(vector<Agent *> agents, int myID);
		Point findClosestOpTerritory(Point const & me);
};

class Agent
{
    public :
		enum type	_type;
        int			agent_id;		// Unique identifier for this agent
        int			player_id;		// Player id of this agent
        int			shoot_cooldown;	// Number of turns between each of this agent's shots
		int			cooldown;		// Number of turns before this agent can shoot
        int			optimal_range;	// Maximum manhattan distance for greatest damage output
        int			soaking_power;	// Damage output within optimal conditions
        int			splash_bombs;	// Number of splash bombs this can throw this game
		int			wetness;		// Damage (0-100) this agent has taken
		Point		pos;
		Point		assigned_place;
		Point		mustGo;
		int			numberOfWait;

		Agent()
		{
			Point p(-1,-1);
			assigned_place = p;
			Point c(-2,-2);
			mustGo = c;
		}

		void	announce(void) const
		{
			cout << this->agent_id;
		}

		void	move(Point const & pos)
		{
			cout << "; MOVE " << pos.x << " " << pos.y;
		}

		void	throww(Point const & pos)
		{
			cout << "; THROW " << pos.x << " " << pos.y;
		}

		void	shoot(Agent* op) const
		{
			if (op)
			cout << "; SHOOT " << op->agent_id;
			// op.wetness += this->
		}

		void	shoot(Agent & op) const
		{
			cout << "; SHOOT " << op.agent_id;
			// op.wetness += this->
		}

		void	hunker_down(void)
		{
			cout << "; HUNKER_DOWN";
		}

		Agent* closestOp(vector<Agent *> op) const
		{
			int minDist = Point::manhDist(this->pos, op[0]->pos);
			Agent *res = op[0];
			for (int i = 0; i < op.size(); ++i)
			{
				int dist = Point::manhDist(this->pos, op[i]->pos);
				if (dist < minDist)
				{
					minDist = dist;
					res = op[i];
				}
			}
			return (res);
		}

		Agent* optimalAgentShoot(vector<Agent *> op) const
		{
			Agent* target = NULL;
			int maxWet = 0;

			// find the enemy that you can kill inside opt range
			for (int i = 0; i < op.size(); ++i)
			{
				int dist = Point::manhDist(op[i]->pos, this->pos);
				if (dist <= this->optimal_range && 100 - op[i]->wetness <= this->soaking_power && op[i]->wetness >= maxWet)
				{
					target = op[i];
					maxWet = op[i]->wetness;
				}
			}
			if (target)
				return (target);

			// if previous not found, find the ennemy that you can kill inside the optimal range * 2 * 75 / 100 (supposing that ennemy hunk down)
			for (int i = 0; i < op.size(); ++i)
			{
				int dist = Point::manhDist(op[i]->pos, this->pos);
				if (dist <= this->optimal_range * 2 && 100 - op[i]->wetness <= (this->soaking_power / 2 * 75 / 100) && op[i]->wetness >= maxWet)
				{
					target = op[i];
					maxWet = op[i]->wetness;
				}
			}
			if (target)
				return (target);

			// if previous not found, find the ennemy with the highest wetness inside the optimal range
			for (int i = 0; i < op.size(); ++i)
			{
				int dist = Point::manhDist(op[i]->pos, this->pos);
				if (dist <= this->optimal_range && op[i]->wetness >= maxWet)
				{
					target = op[i];
					maxWet = op[i]->wetness;
				}
			}
			if (target)
				return (target);

			// if previous not found, find the ennemy with the highest wetness inside the optimal range * 2
			for (int i = 0; i < op.size(); ++i)
			{
				int dist = Point::manhDist(op[i]->pos, this->pos);
				if (dist <= this->optimal_range * 2 && op[i]->wetness >= maxWet)
				{
					target = op[i];
					maxWet = op[i]->wetness;
				}
			}
			return (target);
		}

		Point	getClosestHighCover(Map const & map) const
		{
			int		min = map.height - 1 + map.width - 1;
			Point	res;
			for (int i = 0; i < map.height; ++i)
			{
				for (int j = 0; j < map.width; ++j)
				{
					Point cover(j, i);
					int dist = Point::manhDist(cover, this->pos);
					if (map.grid[i][j]->_type == HIGH && dist < min)
					{
						min = dist;
						res = cover;
					}
				}
			}
			return (res);
		}

		Agent* getAgentClosestOpLowCover_VersionWood2(Map const & map, vector<Agent*> op) const
		{
			Agent* agent = NULL;
			int closestDist = INT32_MAX;

			for (int i = 0; i < op.size(); ++i)
			{
				Point right(op[i]->pos.x + 1, op[i]->pos.y);
				Point left (op[i]->pos.x - 1, op[i]->pos.y);
				Point vrai;
				if (this->pos.x < op[i]->pos.x)
					vrai = left;
				else
					vrai = right;
				if (map.inside(vrai) && (map.getVal(vrai) == LOW || map.getVal(vrai) == EMPTY || map.getVal(vrai) == AGENT))
				{
					if (Point::manhDist(this->pos, op[i]->pos) < closestDist)
					{
						closestDist = Point::manhDist(this->pos, op[i]->pos);
						agent = op[i];
					}
				}
			}
			return (agent);
		}
};

Map::Map(void) : height(0), width(0), grid(NULL){};
Map::~Map(void)
{
	for (int i = 0; i < height; ++i)
	{
		for (int j = 0; j < width; ++j)
		{
			if (grid[i][j]->_type <= 2)
				delete grid[i][j];
		}
	}
	for (int i = 0; grid && i < height; ++i)
		delete [] grid[i];
	delete [] grid;
}
bool Map::inside(Point const & a) const
{
	if (a.x >= 0 && a.x < width && a.y >= 0 && a.y < height)
		return (true);
	return (false);
}

bool Map::inside(int x, int y) const
{
	if (x >= 0 && x < width && y >= 0 && y < height)
		return (true);
	return (false);
}

enum type Map::getVal(Point const & a) const
{
	return (grid[a.y][a.x]->_type);
}

enum type Map::getVal(int x, int y) const
{
	return (grid[y][x]->_type);
}

void Map::zero(void)
{
	for (int i = 0; i < height; ++i)
	{
		for (int j = 0; j < width; ++j)
		{
			if (grid[i][j]->_type == AGENT)
			{
				grid[i][j] = new Agent();
				grid[i][j]->_type = EMPTY;
			}
		}
	}
}

void Map::update(vector<Agent *> agents, int myId)
{
	for (int y = 0; y < this->height; ++y)
	{
		for (int x = 0; x < this->width; ++x)
		{
			if (this->grid[y][x]->_type == EMPTY || this->grid[y][x]->_type == OP || this->grid[y][x]->_type == ME)
			{
				Point	cur(x, y);
				int min = this->height + this->width;
				for (int i = 0; i < agents.size(); ++i)
				{
					int dist = Point::manhDist(cur, agents[i]->pos);
					if (dist < min)
					{
						min = dist;
						if (agents[i]->player_id == myId)
							this->grid[y][x]->_type = ME;
						else
							this->grid[y][x]->_type = OP;
					}
					else if (dist == min)
					{
						if (agents[i]->player_id != myId)
							this->grid[y][x]->_type = OP;
					}
				}
			}
		}
	}
}

Point Map::findClosestOpTerritory(Point const & me)
{
	Point res;
	int minDist = this->height + this->width;
	for (int y = 0; y < this->height; ++y)
	{
		for (int x = 0; x < this->width; ++x)
		{
			Point cur(x, y);
			int dist = Point::manhDist(me, cur);
			if (dist < minDist && this->getVal(cur) == OP)
			{
				res = cur;
				minDist = dist;
			}
		}
	}
	return (res);
}

Point	getBestSideCover(Point const & cover, vector<Agent*> & op)
{
	Point	res(cover);
	int		count[4] = {0};

	for (int i = 0; i < op.size(); ++i)
	{
		if (Point::isAdj(cover, op[i]->pos))
			continue;
		//left
		if (op[i]->pos.x < cover.x)
			++count[0];
		//up
		if (op[i]->pos.y > cover.y)
			++count[1];
		//right
		if (op[i]->pos.x > cover.x)
			++count[2];
		// down
		if (op[i]->pos.y < cover.y)
			++count[3];
	}
	int bestSide = max4index(count);
	if (bestSide == 0)
		++res.x;
	else if (bestSide == 1)
		--res.y;
	else if (bestSide == 2)
		--res.x;
	else if (bestSide == 3)
		++res.y;
	
	return (res);
}

bool myAgentsAround(Point const & pos, Map const & map)
{
	Point p(pos.x + 1, pos.y);
	if (map.inside(p) && map.getVal(p) == ME)
		return (true);
	p.x = pos.x - 1;  p.y = pos.y;
	if (map.inside(p) && map.getVal(p) == ME)
		return (true);
	p.x = pos.x;  p.y = pos.y + 1;
	if (map.inside(p) && map.getVal(p) == ME)
		return (true);
	p.x = pos.x;  p.y = pos.y - 1; 
	if (map.inside(p) && map.getVal(p) == ME)
		return (true);
	p.x = pos.x + 1; p.y = pos.y + 1;
	if (map.inside(p) && map.getVal(p) == ME)
		return (true);
	p.x = pos.x - 1;  p.y = pos.y - 1;
	if (map.inside(p) && map.getVal(p) == ME)
		return (true);
	p.x = pos.x + 1;  p.y = pos.y - 1;
	if (map.inside(p) && map.getVal(p) == ME)
		return (true);
	p.x = pos.x - 1;  p.y = pos.y + 1; 
	if (map.inside(p) && map.getVal(p) == ME)
		return (true);
	return (false);

}

vector<pair<Point, bool>> getMoreCenterHighCover(Map const & map)
{
	vector<pair<Point, bool>> res;
	int begin;
	int end;


		begin = map.width / 3;
		end = map.width * 2 / 3;
	for (int y = 0; y < map.height; ++y)
	{
		for (int x = begin; x <= end; ++x)
		{
			Point cur(x, y);
			if (map.getVal(cur) == HIGH)
				res.push_back({cur, false});
		}
	}
	return (res);
}

vector<pair<Point, bool>> getMoreCenterLowCover(Map const & map)
{
	vector<pair<Point, bool>> res;
	int begin;
	int end;


		begin = map.width / 3;
		end = map.width * 2 / 3;
	for (int y = 0; y < map.height; ++y)
	{
		for (int x = begin; x <= end; ++x)
		{
			Point cur(x, y);
			if (map.getVal(cur) == LOW)
				res.push_back({cur, false});
		}
	}
	return (res);
}

vector<Point>getHighCovers(Map const & map)
{
	vector<Point> res;
	for (int y = 0; y < map.height; ++y)
	{
		for (int x = 0; x < map.width; ++x)
		{
			Point cur(x, y);
			if (map.getVal(cur) == HIGH)
				res.push_back(cur);
		}
	}
	return (res);
}

vector<Point>getLowCovers(Map const & map)
{
	vector<Point> res;
	for (int y = 0; y < map.height; ++y)
	{
		for (int x = 0; x < map.width; ++x)
		{
			Point cur(x, y);
			if (map.getVal(cur) == LOW)
				res.push_back(cur);
		}
	}
	return (res);
}


vector<pair<Point, bool>> getCenterHighCover(Map const & map)
{
	vector<pair<Point, bool>> res;
	int begin;
	int end;

	if (map.width <= 16)
	{
		begin = map.width / 4;
		end = map.width * 3 / 4;
	}
	else
	{
		begin = map.width / 3;
		end = map.width * 2 / 3;
	}
	for (int y = 0; y < map.height; ++y)
	{
		for (int x = begin; x <= end; ++x)
		{
			Point cur(x, y);
			if (map.getVal(cur) == HIGH)
				res.push_back({cur, false});
		}
	}
	return (res);
}

vector<pair<Point, bool>> getCenterLowCover(Map const & map)
{
	vector<pair<Point, bool>> res;
	int begin;
	int end;

	if (map.width <= 16)
	{
		begin = map.width / 4;
		end = map.width * 3 / 4;
	}
	else
	{
		begin = map.width / 3;
		end = map.width * 2 / 3;
	}
	for (int y = 0; y < map.height; ++y)
	{
		for (int x = begin; x <= end; ++x)
		{
			Point cur(x, y);
			if (map.getVal(cur) == LOW)
				res.push_back({cur, false});
		}
	}
	return (res);
}

int countOpAroundCase(Point const & _case, Map const & map, int myId)
{
	int count = 0;
	for (int y = _case.y - 1; y <= _case.y + 1; ++y)
	{
		for (int x = _case.x - 1; x <= _case.x + 1; ++x)
		{
			if (map.inside(x,y) && map.getVal(x, y) == AGENT)
			{
				if (map.grid[y][x]->player_id != myId)
					++count;
				else
					--count;
			}
		}
	}
	return (count);
}


// ! use this instead of the upper
float countOpAroundCase_opt(Point const & _case, Map const & map, int myId)
{
	float count = 0;
	for (int y = _case.y - 1; y <= _case.y + 1; ++y)
	{
		for (int x = _case.x - 1; x <= _case.x + 1; ++x)
		{
			if (map.inside(x,y) && map.getVal(x, y) == AGENT)
			{
				if (map.grid[y][x]->player_id != myId)
				{
					Point p(x,y);
					int dist = Point::manhDist(p, _case);
					if (dist == 0)
					{
						count += 1.0;
					}
					else if (dist == 1)
					{
						count += 0.75;
					}
					else
						count += 0.5;
				}
				else
				{
					Point p(x,y);
					int dist = Point::manhDist(p, _case);
					if (dist == 0)
					{
						count -= 1.0;
					}
					else if (dist == 1)
					{
						count -= 0.75;
					}
					else
						count -= 0.5;
				}
			}
		}
	}
	return (count);
}

// find the best place to throw a bomb
Point getOptCaseBomb(Agent const & src, Map const & map, int myId)
{
	float maxOp = 0;
	Point pp(-1, -1);
	for (int y = src.pos.y - 4; y <= src.pos.y + 4; ++y)
	{
		for (int x = src.pos.x - 4; x <= src.pos.x + 4; ++x)
		{
			int x_dif = i_abs(src.pos.x - x);
			int y_dif = i_abs(src.pos.y - y);
			Point p(x, y);

			int dist = Point::manhDist(src.pos, p);
			float ops = countOpAroundCase_opt(p, map, myId);
			if (dist <= 4 && (x_dif > 1 || y_dif > 1) && map.inside(x, y) && ops > maxOp)
			{
				pp = p;
				maxOp = ops;
			}
		}
	}
	cerr << "id: " << src.agent_id << ", " << "(" << pp.x << "," << pp.y << "), " << "ops : " << maxOp << endl; 
	return (pp);
}

Point	territoryWithTheMostEnnemy(Map const & map, int myId)
{
	int maxOp = 0;
	Point pp(-1, -1);
	for (int y = 0; y < map.height; ++y)
	{
		for (int x = 0; x < map.width; ++x)
		{
			Point p(x, y);
			int ops = countOpAroundCase(p, map, myId);
			if (ops >= maxOp)
			{
				pp = p;
				maxOp = ops;
			}
		}
	}
	return (pp);
}

int countMyTerritory(Map const & map, int my_Id)
{
	int count = 0;
	for (int y = 0; y < map.height; ++y)
	{
		for (int x = 0; x < map.width; ++x)
		{
			if (map.getVal(x, y) == ME || (map.getVal(x, y) == AGENT && map.grid[y][x]->player_id == my_Id))
				++count;
		}
	}
	return (count);
}

int countOpTerritory(Map const & map, int my_Id)
{
	int count = 0;
	for (int y = 0; y < map.height; ++y)
	{
		for (int x = 0; x < map.width; ++x)
		{
			if (map.getVal(x, y) == OP || (map.getVal(x, y) == AGENT && map.grid[y][x]->player_id != my_Id))
				++count;
		}
	}
	return (count);
}

int main()
{
    int my_id; // Your player id (0 or 1)
    cin >> my_id; cin.ignore();
    int agent_count0; // Total number of agents in the game
    cin >> agent_count0; cin.ignore();

	vector<Agent *> agents;
	vector<Agent *> myAgents;
	vector<Agent *> opAgents;
	int seeed = time(NULL);
	srand(seeed);

    for (int i = 0; i < agent_count0; i++) {
		Agent *a = new Agent();
		a->_type = AGENT;
        cin >> a->agent_id >> a->player_id >> a->shoot_cooldown >> a->optimal_range >> a->soaking_power >> a->splash_bombs; cin.ignore();
		agents.push_back(a);
		if (a->player_id == my_id)
			myAgents.push_back(a);
		else
			opAgents.push_back(a);
    }

	/*----------------------------------------------------------------------*/
	/*                                 MAP                                  */
	/*----------------------------------------------------------------------*/

	Map map;
    cin >> map.width >> map.height; cin.ignore();
    
	map.grid = new Agent**[map.height];
	for (int i = 0; i < map.height; ++i)
	{
		map.grid[i] = new Agent*[map.width];
	}

	for (int i = 0; i < map.height; i++) {
        for (int j = 0; j < map.width; j++) {
            int x; // X coordinate, 0 is left edge
            int y; // Y coordinate, 0 is top edge
            int tile_type;
            cin >> x >> y >> tile_type; cin.ignore();
			map.grid[y][x] = new Agent();
			if (tile_type == 0)
				map.grid[y][x]->_type = EMPTY;
			else if (tile_type == 1)
				map.grid[y][x]->_type = LOW;
			else if (tile_type == 2)
				map.grid[y][x]->_type = HIGH;
        }
    }

    /*-------------------------------------------------------------------------*/
	/*                             BEFORE START                                */
	/*----------------------------------------------0--------------------------*/
	
	
	Agent* myAgentMostBomb = NULL;
	{
		int max = 0;
		for (int i = 0; i < myAgents.size(); ++i)
		{
			if (myAgents[i]->splash_bombs > max)
			{
				myAgentMostBomb = myAgents[i];
				max = myAgents[i]->splash_bombs;
			}
		}
	}

	Agent* myAgentHighestRange = NULL;
	{
		int max = 0;
		for (int i = 0; i < myAgents.size(); ++i)
		{
			if (myAgents[i] != myAgentMostBomb && myAgents[i]->optimal_range > max)
			{
				myAgentHighestRange = myAgents[i];
				max = myAgents[i]->optimal_range;
			}
		}
	}

	if (myAgentMostBomb)
		myAgentMostBomb->numberOfWait = 0;
	if (myAgentHighestRange)
		myAgentHighestRange->numberOfWait = 5;
	for (int i = 0; i < myAgents.size(); ++i)
	{
		if (myAgents[i] != myAgentMostBomb && myAgents[i] != myAgentHighestRange)
		{
			myAgents[i]->numberOfWait = 3;
		}
	}

	vector<Point> highCovers = getHighCovers(map);
	vector<Point> lowCovers = getLowCovers(map);
	vector<pair<Point, int>> mylastPos; // lastPos, agentId


    /*----------------------------------------------------------------------*/
	/*                             GAME LOOP                                */
	/*----------------------------------------------------------------------*/

	int loop_count = 0;
	while (1) {
		
		vector<Agent *> aliveAgents;
		vector<Agent *> myAliveAgents;
		vector<Agent *> opAliveAgents; 
        int agent_count;
        cin >> agent_count; cin.ignore();
        map.zero();
		for (int i = 0; i < agent_count; i++) {
            int agent_id;
            int x;
            int y;
            int cooldown; // Number of turns before this agent can shoot
            int splash_bombs;
            int wetness; // Damage (0-100) this agent has taken
            cin >> agent_id >> x >> y >> cooldown >> splash_bombs >> wetness; cin.ignore();
			for (int j = 0; j < agent_count0; ++j)
			{
				if (agents[j]->agent_id == agent_id)
				{
					Point temp(-2, -2);
					agents[j]->mustGo = temp;
					agents[j]->pos.x = x;
					agents[j]->pos.y = y;
					agents[j]->cooldown = cooldown;
					agents[j]->splash_bombs = splash_bombs;
					agents[j]->wetness = wetness;
					aliveAgents.push_back(agents[j]);
					if (agents[j]->player_id == my_id)
						myAliveAgents.push_back(agents[j]);
					else
						opAliveAgents.push_back(agents[j]);
					if (map.getVal(agents[j]->pos) == EMPTY || map.getVal(agents[j]->pos) == ME || map.getVal(agents[j]->pos) == OP)
						delete map.grid[y][x];
					map.grid[y][x] = agents[j];
					map.update(aliveAgents, my_id);
					break;
				}
				
			}
        }
        int my_agent_count; // Number of alive agents controlled by you
        cin >> my_agent_count; cin.ignore();

		// FIX POS
		if (loop_count == 0)
		{
			bool sideLeft;
			if (myAliveAgents[0]->pos.x < map.width / 2)
				sideLeft = true;
			else
				sideLeft = false;

			if (map.width == 20)
			{
				if (sideLeft)
				{
					vector<Point> fixPos;
					{
						Point z(10, 2);
						Point y(10, 8);
						fixPos.push_back(z);
						fixPos.push_back(y);
					}
					if (myAgentHighestRange)
					{
						for (int i = 0; i < highCovers.size(); ++i)
						{
							if (highCovers[i].x < 9 && highCovers[i].x > 6 && highCovers[i].y > 2 && highCovers[i].y < 7
								&& map.getVal(highCovers[i].x - 1, highCovers[i].y) != LOW && map.getVal(highCovers[i].x - 1, highCovers[i].y) != HIGH)
							{
								Point o(highCovers[i].x - 1, highCovers[i].y);
								myAgentHighestRange->assigned_place = o;
								break;
							}
						}
						for (int i = 0; myAgentHighestRange->assigned_place.x != -1 && i < lowCovers.size(); ++i)
						{
							if (lowCovers[i].x < 9 && lowCovers[i].x > 6 && lowCovers[i].y > 2 && lowCovers[i].y < 7
								&& map.getVal(lowCovers[i].x - 1, lowCovers[i].y) != LOW && map.getVal(lowCovers[i].x - 1, lowCovers[i].y) != HIGH)
							{
								Point o(lowCovers[i].x - 1, lowCovers[i].y);
								myAgentHighestRange->assigned_place = o;
								break;
							}
						}
						if (myAgentHighestRange->assigned_place.x == -1)
						{
							for (int y = 4; y <= 5; ++y)
							{
								for (int x = 6; x <= 7; ++x)
								{
									if (map.getVal(x, y) != LOW && map.getVal(x, y) != HIGH)
									{
										Point o(x, y);
										myAgentHighestRange->assigned_place = o;
									}
								}
							}
							if (myAgentHighestRange->assigned_place.x == -1)
							{
								for (int y = 3; y <= 6; ++y)
								{
									for (int x = 6; x <= 7; ++x)
									{
										if (map.getVal(x, y) != LOW && map.getVal(x, y) != HIGH)
										{
											Point o(x, y);

											myAgentHighestRange->assigned_place = o;
										}
									}
								}
							}
						}
						if (myAgentHighestRange->assigned_place.x == -1)
						{
							Point z(7, 4);
							myAgentHighestRange->assigned_place = z;
						}
					}
					if (myAgentMostBomb)
					{
						Point o(10, 5);
						myAgentMostBomb->assigned_place = o;
					}
					
					for (int j = 0; j < myAliveAgents.size(); ++j)
					{
						int closestDist = 99999999;
						int closestInd = 0;
						if (myAliveAgents[j]->assigned_place.x == -1 && (myAliveAgents[j]->splash_bombs != 2 || agent_count != 10))
						{
							for (int i = 0; i < fixPos.size(); ++i)
							{
								int dist = Point::manhDist(myAliveAgents[j]->pos, fixPos[i]);
								if (dist < closestDist)
								{
									closestInd = i;
									closestDist = dist;
								}
							}
							if (closestDist != 99999999)
							{
								myAliveAgents[j]->assigned_place = fixPos[closestInd];
								fixPos.erase(fixPos.begin() + closestInd);
							}
						}
					}
					if (agent_count == 10)
					{
						for (int j = 0; j < myAliveAgents.size(); ++j)
						{
							if (myAliveAgents[j]->assigned_place.x == -1 && myAliveAgents[j]->splash_bombs == 2)
							{
								Point b(17, 4);
								myAliveAgents[j]->assigned_place = b;
							}
						}
					}
				}
				else
				{
					vector<Point> fixPos;
					{
						Point z(9, 2);
						Point y(9, 8);
						fixPos.push_back(z);
						fixPos.push_back(y);
					}
					if (myAgentHighestRange)
					{
						for (int i = 0; i < highCovers.size(); ++i)
						{
							if (highCovers[i].x < 13 && highCovers[i].x > 10 && highCovers[i].y > 2 && highCovers[i].y < 7
								&& map.getVal(highCovers[i].x + 1, highCovers[i].y) != LOW && map.getVal(highCovers[i].x + 1, highCovers[i].y) != HIGH)
							{
								Point o(highCovers[i].x + 1, highCovers[i].y);
								myAgentHighestRange->assigned_place = o;
								break;
							}
						}
						for (int i = 0; myAgentHighestRange->assigned_place.x != -1 && i < lowCovers.size(); ++i)
						{
							if (lowCovers[i].x < 13 && lowCovers[i].x > 10 && lowCovers[i].y > 2 && lowCovers[i].y < 7
								&& map.getVal(lowCovers[i].x + 1, lowCovers[i].y) != LOW && map.getVal(lowCovers[i].x + 1, lowCovers[i].y) != HIGH)
							{
								Point o(lowCovers[i].x + 1, lowCovers[i].y);
								myAgentHighestRange->assigned_place = o;
								break;
							}
						}
						if (myAgentHighestRange->assigned_place.x == -1)
						{
							for (int y = 4; y <= 5; ++y)
							{
								for (int x = 12; x <= 13; ++x)
								{
									if (map.getVal(x, y) != LOW && map.getVal(x, y) != HIGH)
									{
										Point o(x, y);

										myAgentHighestRange->assigned_place = o;
									}
								}
							}
							if (myAgentHighestRange->assigned_place.x == -1)
							{
								for (int y = 3; y <= 6; ++y)
								{
									for (int x = 12; x <= 13; ++x)
									{
										if (map.getVal(x, y) != LOW && map.getVal(x, y) != HIGH)
										{
											Point o(x, y);

											myAgentHighestRange->assigned_place = o;
										}
									}
								}
							}
						}
						if (myAgentHighestRange->assigned_place.x == -1)
						{
							Point z(12, 4);
							myAgentHighestRange->assigned_place = z;
						}
					}
					if (myAgentMostBomb)
					{
						Point o(9, 5);
						myAgentMostBomb->assigned_place = o;
					}
					
					for (int j = 0; j < myAliveAgents.size(); ++j)
					{
						int closestDist = 99999999;
						int closestInd = 0;
						if (myAliveAgents[j]->assigned_place.x == -1 && (myAliveAgents[j]->splash_bombs != 2 || agent_count != 10))
						{
							for (int i = 0; i < fixPos.size(); ++i)
							{
								int dist = Point::manhDist(myAliveAgents[j]->pos, fixPos[i]);
								if (dist < closestDist)
								{
									closestInd = i;
									closestDist = dist;
								}
							}
							if (closestDist != 99999999)
							{
								myAliveAgents[j]->assigned_place = fixPos[closestInd];
								fixPos.erase(fixPos.begin() + closestInd);
							}
						}
					}
					if (agent_count == 10)
					{
						for (int j = 0; j < myAliveAgents.size(); ++j)
						{
							if (myAliveAgents[j]->assigned_place.x == -1 && myAliveAgents[j]->splash_bombs == 2)
							{
								Point b(2, 4);
								myAliveAgents[j]->assigned_place = b;
							}
						}
					}
				}
			}
			else if (map.width == 18)
			{
				if (sideLeft)
				{
					vector<Point> fixPos;
					{
						Point z(8, 1);
						Point y(8, 7);
						fixPos.push_back(z);
						fixPos.push_back(y);
					}
					if (myAgentHighestRange)
					{
						for (int i = 0; i < highCovers.size(); ++i)
						{
							if (highCovers[i].x == 6 && highCovers[i].y > 1 && highCovers[i].y < 7
								&& map.getVal(highCovers[i].x - 1, highCovers[i].y) != LOW && map.getVal(highCovers[i].x - 1, highCovers[i].y) != HIGH)
							{
								Point o(highCovers[i].x - 1, highCovers[i].y);
								myAgentHighestRange->assigned_place = o;
								break;
							}
						}
						for (int i = 0; myAgentHighestRange->assigned_place.x != -1 && i < lowCovers.size(); ++i)
						{
							if (lowCovers[i].x == 6 && lowCovers[i].y > 1 && lowCovers[i].y < 7
								&& map.getVal(lowCovers[i].x - 1, lowCovers[i].y) != LOW && map.getVal(lowCovers[i].x - 1, lowCovers[i].y) != HIGH)
							{
								Point o(lowCovers[i].x - 1, lowCovers[i].y);
								myAgentHighestRange->assigned_place = o;
								break;
							}
						}
						// ! last modif
						if (myAgentHighestRange->assigned_place.x == -1)
						{
							for (int y = 3; y <= 5; ++y)
							{

								if (map.getVal(5, y) != LOW && map.getVal(5, y) != HIGH)
								{
									Point o(5, y);
									myAgentHighestRange->assigned_place = o;
								}
							}
							if (myAgentHighestRange->assigned_place.x == -1)
							{
								for (int y = 2; y <= 6; ++y)
								{

									if (map.getVal(5, y) != LOW && map.getVal(5, y) != HIGH)
									{
										Point o(5, y);

										myAgentHighestRange->assigned_place = o;
									}
								}
							}
						}
						if (myAgentHighestRange->assigned_place.x == -1)
						{
							Point z(5, 4);
							myAgentHighestRange->assigned_place = z;
						}
					}
					if (myAgentMostBomb)
					{
						Point o(8, 4);
						myAgentMostBomb->assigned_place = o;
					}
					
					for (int j = 0; j < myAliveAgents.size(); ++j)
					{
						int closestDist = 99999999;
						int closestInd = 0;
						if (myAliveAgents[j]->assigned_place.x == -1 && (myAliveAgents[j]->splash_bombs != 2 || agent_count != 10))
						{
							for (int i = 0; i < fixPos.size(); ++i)
							{
								int dist = Point::manhDist(myAliveAgents[j]->pos, fixPos[i]);
								if (dist < closestDist)
								{
									closestInd = i;
									closestDist = dist;
								}
							}
							if (closestDist != 99999999)
							{
								myAliveAgents[j]->assigned_place = fixPos[closestInd];
								fixPos.erase(fixPos.begin() + closestInd);
							}
						}
					}
					if (agent_count == 10)
					{
						for (int j = 0; j < myAliveAgents.size(); ++j)
						{
							if (myAliveAgents[j]->assigned_place.x == -1 && myAliveAgents[j]->splash_bombs == 2)
							{
								Point b(15, 4);
								myAliveAgents[j]->assigned_place = b;
							}
						}
					}
				}
				else
				{
					vector<Point> fixPos;
					{
						Point z(9, 1);
						Point y(9, 7);
						fixPos.push_back(z);
						fixPos.push_back(y);
					}
					if (myAgentHighestRange)
					{
						for (int i = 0; i < highCovers.size(); ++i)
						{
							if (highCovers[i].x == 11 && highCovers[i].y > 1 && highCovers[i].y < 7
								&& map.getVal(highCovers[i].x + 1, highCovers[i].y) != LOW && map.getVal(highCovers[i].x + 1, highCovers[i].y) != HIGH)
							{
								Point o(highCovers[i].x + 1, highCovers[i].y);
								myAgentHighestRange->assigned_place = o;
								break;
							}
						}
						for (int i = 0; myAgentHighestRange->assigned_place.x != -1 && i < lowCovers.size(); ++i)
						{
							if (lowCovers[i].x == 11 && lowCovers[i].y > 1 && lowCovers[i].y < 7
								&& map.getVal(lowCovers[i].x + 1, lowCovers[i].y) != LOW && map.getVal(lowCovers[i].x + 1, lowCovers[i].y) != HIGH)
							{
								Point o(lowCovers[i].x + 1, lowCovers[i].y);
								myAgentHighestRange->assigned_place = o;
								break;
							}
						}
						if (myAgentHighestRange->assigned_place.x == -1)
						{
							for (int y = 3; y <= 5; ++y)
							{
								if (map.getVal(12, y) != LOW && map.getVal(12, y) != HIGH)
								{
									Point o(12, y);
									myAgentHighestRange->assigned_place = o;
								}
							}
							if (myAgentHighestRange->assigned_place.x == -1)
							{
								for (int y = 2; y <= 6; ++y)
								{

									if (map.getVal(12, y) != LOW && map.getVal(12, y) != HIGH)
									{
										Point o(12, y);

										myAgentHighestRange->assigned_place = o;
									}
								}
							}
						}
						if (myAgentHighestRange->assigned_place.x == -1)
						{
							Point z(12, 4);
							myAgentHighestRange->assigned_place = z;
						}
					}
					if (myAgentMostBomb)
					{
						Point o(9, 4);
						myAgentMostBomb->assigned_place = o;
					}
					
					for (int j = 0; j < myAliveAgents.size(); ++j)
					{
						int closestDist = 99999999;
						int closestInd = 0;
						if (myAliveAgents[j]->assigned_place.x == -1 && (myAliveAgents[j]->splash_bombs != 2 || agent_count != 10))
						{
							for (int i = 0; i < fixPos.size(); ++i)
							{
								int dist = Point::manhDist(myAliveAgents[j]->pos, fixPos[i]);
								if (dist < closestDist)
								{
									closestInd = i;
									closestDist = dist;
								}
							}
							if (closestDist != 99999999)
							{
								myAliveAgents[j]->assigned_place = fixPos[closestInd];
								fixPos.erase(fixPos.begin() + closestInd);
							}
						}
					}
					if (agent_count == 10)
					{
						for (int j = 0; j < myAliveAgents.size(); ++j)
						{
							if (myAliveAgents[j]->assigned_place.x == -1 && myAliveAgents[j]->splash_bombs == 2)
							{
								Point b(2, 4);
								myAliveAgents[j]->assigned_place = b;
							}
						}
					}
				}
			}
			else if (map.width == 16)
			{
				if (sideLeft)
				{
					vector<Point> fixPos;
					{
						Point z(7, 1);
						Point y(7, 6);
						fixPos.push_back(z);
						fixPos.push_back(y);
					}
					if (myAgentHighestRange)
					{
						for (int i = 0; i < highCovers.size(); ++i)
						{
							if (highCovers[i].x == 6 && highCovers[i].y > 1 && highCovers[i].y < 6
								&& map.getVal(highCovers[i].x - 1, highCovers[i].y) != LOW && map.getVal(highCovers[i].x - 1, highCovers[i].y) != HIGH)
							{
								Point o(highCovers[i].x - 1, highCovers[i].y);
								myAgentHighestRange->assigned_place = o;
								break;
							}
						}
						for (int i = 0; myAgentHighestRange->assigned_place.x != -1 && i < lowCovers.size(); ++i)
						{
							if (lowCovers[i].x == 6 && lowCovers[i].y > 1 && lowCovers[i].y < 6
								&& map.getVal(lowCovers[i].x - 1, lowCovers[i].y) != LOW && map.getVal(lowCovers[i].x - 1, lowCovers[i].y) != HIGH)
							{
								Point o(lowCovers[i].x - 1, lowCovers[i].y);
								myAgentHighestRange->assigned_place = o;
								break;
							}
						}
						if (myAgentHighestRange->assigned_place.x == -1)
						{
							for (int y = 3; y <= 4; ++y)
							{

								if (map.getVal(5, y) != LOW && map.getVal(5, y) != HIGH)
								{
									Point o(5, y);
									myAgentHighestRange->assigned_place = o;
								}
							}
							if (myAgentHighestRange->assigned_place.x == -1)
							{
								for (int y = 2; y <= 5; ++y)
								{

									if (map.getVal(5, y) != LOW && map.getVal(5, y) != HIGH)
									{
										Point o(5, y);

										myAgentHighestRange->assigned_place = o;
									}
								}
							}
						}
						if (myAgentHighestRange->assigned_place.x == -1)
						{
							Point z(5, 4);
							myAgentHighestRange->assigned_place = z;
						}
					}
					if (myAgentMostBomb)
					{
						Point o(8, 4);
						myAgentMostBomb->assigned_place = o;
					}
					
					for (int j = 0; j < myAliveAgents.size(); ++j)
					{
						int closestDist = 99999999;
						int closestInd = 0;
						if (myAliveAgents[j]->assigned_place.x == -1 && (myAliveAgents[j]->splash_bombs != 2 || agent_count != 10))
						{
							for (int i = 0; i < fixPos.size(); ++i)
							{
								int dist = Point::manhDist(myAliveAgents[j]->pos, fixPos[i]);
								if (dist < closestDist)
								{
									closestInd = i;
									closestDist = dist;
								}
							}
							if (closestDist != 99999999)
							{
								myAliveAgents[j]->assigned_place = fixPos[closestInd];
								fixPos.erase(fixPos.begin() + closestInd);
							}
						}
					}
					if (agent_count == 10)
					{
						for (int j = 0; j < myAliveAgents.size(); ++j)
						{
							if (myAliveAgents[j]->assigned_place.x == -1 && myAliveAgents[j]->splash_bombs == 2)
							{
								Point b(13, 3);
								myAliveAgents[j]->assigned_place = b;
							}
						}
					}
				}
				else
				{
					vector<Point> fixPos;
					{
						Point z(8, 1);
						Point y(8, 6);
						fixPos.push_back(z);
						fixPos.push_back(y);
					}
					if (myAgentHighestRange)
					{
						for (int i = 0; i < highCovers.size(); ++i)
						{
							if (highCovers[i].x == 9 && highCovers[i].y > 1 && highCovers[i].y < 6
								&& map.getVal(highCovers[i].x + 1, highCovers[i].y) != LOW && map.getVal(highCovers[i].x + 1, highCovers[i].y) != HIGH)
							{
								Point o(highCovers[i].x + 1, highCovers[i].y);
								myAgentHighestRange->assigned_place = o;
								break;
							}
						}
						for (int i = 0; myAgentHighestRange->assigned_place.x != -1 && i < lowCovers.size(); ++i)
						{
							if (lowCovers[i].x == 9 && lowCovers[i].y > 1 && lowCovers[i].y < 6
								&& map.getVal(lowCovers[i].x + 1, lowCovers[i].y) != LOW && map.getVal(lowCovers[i].x + 1, lowCovers[i].y) != HIGH)
							{
								Point o(lowCovers[i].x + 1, lowCovers[i].y);
								myAgentHighestRange->assigned_place = o;
								break;
							}
						}
						if (myAgentHighestRange->assigned_place.x == -1)
						{
							for (int y = 3; y <= 4; ++y)
							{

								if (map.getVal(10, y) != LOW && map.getVal(10, y) != HIGH)
								{
									Point o(10, y);
									myAgentHighestRange->assigned_place = o;
								}
							}
							if (myAgentHighestRange->assigned_place.x == -1)
							{
								for (int y = 2; y <= 5; ++y)
								{

									if (map.getVal(10, y) != LOW && map.getVal(10, y) != HIGH)
									{
										Point o(10, y);

										myAgentHighestRange->assigned_place = o;
									}
								}
							}
						}
						if (myAgentHighestRange->assigned_place.x == -1)
						{
							Point z(10, 4);
							myAgentHighestRange->assigned_place = z;
						}
					}
					if (myAgentMostBomb)
					{
						Point o(7, 4);
						myAgentMostBomb->assigned_place = o;
					}
					
					for (int j = 0; j < myAliveAgents.size(); ++j)
					{
						int closestDist = 99999999;
						int closestInd = 0;
						if (myAliveAgents[j]->assigned_place.x == -1 && (myAliveAgents[j]->splash_bombs != 2 || agent_count != 10))
						{
							for (int i = 0; i < fixPos.size(); ++i)
							{
								int dist = Point::manhDist(myAliveAgents[j]->pos, fixPos[i]);
								if (dist < closestDist)
								{
									closestInd = i;
									closestDist = dist;
								}
							}
							if (closestDist != 99999999)
							{
								myAliveAgents[j]->assigned_place = fixPos[closestInd];
								fixPos.erase(fixPos.begin() + closestInd);
							}
						}
					}
					if (agent_count == 10)
					{
						for (int j = 0; j < myAliveAgents.size(); ++j)
						{
							if (myAliveAgents[j]->assigned_place.x == -1 && myAliveAgents[j]->splash_bombs == 2)
							{
								Point b(2, 3);
								myAliveAgents[j]->assigned_place = b;
							}
						}
					}
				}
			}
			else if (map.width == 14)
			{
				if (sideLeft)
				{
					vector<Point> fixPos;
					{
						Point z(6, 0);
						Point y(6, 6);
						fixPos.push_back(z);
						fixPos.push_back(y);
					}
					if (myAgentHighestRange)
					{
						for (int i = 0; i < highCovers.size(); ++i)
						{
							if (highCovers[i].x == 4 && highCovers[i].y > 1 && highCovers[i].y < 5
								&& map.getVal(highCovers[i].x - 1, highCovers[i].y) != LOW && map.getVal(highCovers[i].x - 1, highCovers[i].y) != HIGH)
							{
								Point o(highCovers[i].x - 1, highCovers[i].y);
								myAgentHighestRange->assigned_place = o;
								break;
							}
						}
						for (int i = 0; myAgentHighestRange->assigned_place.x != -1 && i < lowCovers.size(); ++i)
						{
							if (lowCovers[i].x == 4 && lowCovers[i].y > 1 && lowCovers[i].y < 5
								&& map.getVal(lowCovers[i].x - 1, lowCovers[i].y) != LOW && map.getVal(lowCovers[i].x - 1, lowCovers[i].y) != HIGH)
							{
								Point o(lowCovers[i].x - 1, lowCovers[i].y);
								myAgentHighestRange->assigned_place = o;
								break;
							}
						}
						if (myAgentHighestRange->assigned_place.x == -1)
						{
							if (map.getVal(4, 3) != LOW && map.getVal(4, 3) != HIGH)
							{
								Point o(4, 3);
								myAgentHighestRange->assigned_place = o;
							}
							if (myAgentHighestRange->assigned_place.x == -1)
							{
								for (int y = 2; y <= 4; ++y)
								{

									if (map.getVal(4, y) != LOW && map.getVal(4, y) != HIGH)
									{
										Point o(4, y);

										myAgentHighestRange->assigned_place = o;
									}
								}
							}
						}
						if (myAgentHighestRange->assigned_place.x == -1)
						{
							Point z(4, 3);
							myAgentHighestRange->assigned_place = z;
						}
					}
					if (myAgentMostBomb)
					{
						Point o(7, 3);
						myAgentMostBomb->assigned_place = o;
					}
					
					for (int j = 0; j < myAliveAgents.size(); ++j)
					{
						int closestDist = 99999999;
						int closestInd = 0;
						if (myAliveAgents[j]->assigned_place.x == -1 && (myAliveAgents[j]->splash_bombs != 2 || agent_count != 10))
						{
							for (int i = 0; i < fixPos.size(); ++i)
							{
								int dist = Point::manhDist(myAliveAgents[j]->pos, fixPos[i]);
								if (dist < closestDist)
								{
									closestInd = i;
									closestDist = dist;
								}
							}
							if (closestDist != 99999999)
							{
								myAliveAgents[j]->assigned_place = fixPos[closestInd];
								fixPos.erase(fixPos.begin() + closestInd);
							}
						}
					}
					if (agent_count == 10)
					{
						for (int j = 0; j < myAliveAgents.size(); ++j)
						{
							if (myAliveAgents[j]->assigned_place.x == -1 && myAliveAgents[j]->splash_bombs == 2)
							{
								Point b(12, 3);
								myAliveAgents[j]->assigned_place = b;
							}
						}
					}
				}
				else
				{
					vector<Point> fixPos;
					{
						Point z(7, 0);
						Point y(7, 6);
						fixPos.push_back(z);
						fixPos.push_back(y);
					}
					if (myAgentHighestRange)
					{
						for (int i = 0; i < highCovers.size(); ++i)
						{
							if (highCovers[i].x == 9 && highCovers[i].y > 1 && highCovers[i].y < 5
								&& map.getVal(highCovers[i].x + 1, highCovers[i].y) != LOW && map.getVal(highCovers[i].x + 1, highCovers[i].y) != HIGH)
							{
								Point o(highCovers[i].x + 1, highCovers[i].y);
								myAgentHighestRange->assigned_place = o;
								break;
							}
						}
						for (int i = 0; myAgentHighestRange->assigned_place.x != -1 && i < lowCovers.size(); ++i)
						{
							if (lowCovers[i].x == 9 && lowCovers[i].y > 1 && lowCovers[i].y < 5
								&& map.getVal(lowCovers[i].x + 1, lowCovers[i].y) != LOW && map.getVal(lowCovers[i].x + 1, lowCovers[i].y) != HIGH)
							{
								Point o(lowCovers[i].x + 1, lowCovers[i].y);
								myAgentHighestRange->assigned_place = o;
								break;
							}
						}
						if (myAgentHighestRange->assigned_place.x == -1)
						{
							if (map.getVal(9, 3) != LOW && map.getVal(9, 3) != HIGH)
							{
								Point o(9, 3);
								myAgentHighestRange->assigned_place = o;
							}
							if (myAgentHighestRange->assigned_place.x == -1)
							{
								for (int y = 2; y <= 4; ++y)
								{

									if (map.getVal(9, y) != LOW && map.getVal(9, y) != HIGH)
									{
										Point o(9, y);

										myAgentHighestRange->assigned_place = o;
									}
								}
							}
						}
						if (myAgentHighestRange->assigned_place.x == -1)
						{
							Point z(9, 3);
							myAgentHighestRange->assigned_place = z;
						}
					}
					if (myAgentMostBomb)
					{
						Point o(6, 3);
						myAgentMostBomb->assigned_place = o;
					}
					
					for (int j = 0; j < myAliveAgents.size(); ++j)
					{
						int closestDist = 99999999;
						int closestInd = 0;
						if (myAliveAgents[j]->assigned_place.x == -1 && (myAliveAgents[j]->splash_bombs != 2 || agent_count != 10))
						{
							for (int i = 0; i < fixPos.size(); ++i)
							{
								int dist = Point::manhDist(myAliveAgents[j]->pos, fixPos[i]);
								if (dist < closestDist)
								{
									closestInd = i;
									closestDist = dist;
								}
							}
							if (closestDist != 99999999)
							{
								myAliveAgents[j]->assigned_place = fixPos[closestInd];
								fixPos.erase(fixPos.begin() + closestInd);
							}
						}
					}
					if (agent_count == 10)
					{
						for (int j = 0; j < myAliveAgents.size(); ++j)
						{
							if (myAliveAgents[j]->assigned_place.x == -1 && myAliveAgents[j]->splash_bombs == 2)
							{
								Point b(1, 3);
								myAliveAgents[j]->assigned_place = b;
							}
						}
					}
				}
			}
			// ! last modif
			else if (map.width == 12)
			{
				if (sideLeft)
				{
					vector<Point> fixPos;
					{
						Point z(6, 0);
						Point y(6, 5);
						fixPos.push_back(z);
						fixPos.push_back(y);
					}
					if (myAgentHighestRange)
					{
						for (int i = 0; i < highCovers.size(); ++i)
						{
							if (highCovers[i].x == 3 && highCovers[i].y > 1 && highCovers[i].y < 4
								&& map.getVal(highCovers[i].x - 1, highCovers[i].y) != LOW && map.getVal(highCovers[i].x - 1, highCovers[i].y) != HIGH)
							{
								Point o(highCovers[i].x - 1, highCovers[i].y);
								myAgentHighestRange->assigned_place = o;
								break;
							}
						}
						for (int i = 0; myAgentHighestRange->assigned_place.x != -1 && i < lowCovers.size(); ++i)
						{
							if (lowCovers[i].x == 3 && lowCovers[i].y > 1 && lowCovers[i].y < 4
								&& map.getVal(lowCovers[i].x - 1, lowCovers[i].y) != LOW && map.getVal(lowCovers[i].x - 1, lowCovers[i].y) != HIGH)
							{
								Point o(lowCovers[i].x - 1, lowCovers[i].y);
								myAgentHighestRange->assigned_place = o;
								break;
							}
						}
						if (myAgentHighestRange->assigned_place.x == -1)
						{
							if (map.getVal(3, 2) != LOW && map.getVal(3, 2) != HIGH)
							{
								Point o(3, 2);
								myAgentHighestRange->assigned_place = o;
							}
							if (myAgentHighestRange->assigned_place.x == -1)
							{
								if (map.getVal(3, 3) != LOW && map.getVal(3, 3) != HIGH)
								{
									Point o(3, 3);

									myAgentHighestRange->assigned_place = o;
								}
							}
						}
						if (myAgentHighestRange->assigned_place.x == -1)
						{
							Point z(3, 2);
							myAgentHighestRange->assigned_place = z;
						}
					}
					if (myAgentMostBomb)
					{
						Point o(6, 3);
						myAgentMostBomb->assigned_place = o;
					}
					
					for (int j = 0; j < myAliveAgents.size(); ++j)
					{
						int closestDist = 99999999;
						int closestInd = 0;
						if (myAliveAgents[j]->assigned_place.x == -1 && (myAliveAgents[j]->splash_bombs != 2 || agent_count != 10))
						{
							for (int i = 0; i < fixPos.size(); ++i)
							{
								int dist = Point::manhDist(myAliveAgents[j]->pos, fixPos[i]);
								if (dist < closestDist)
								{
									closestInd = i;
									closestDist = dist;
								}
							}
							if (closestDist != 99999999)
							{
								myAliveAgents[j]->assigned_place = fixPos[closestInd];
								fixPos.erase(fixPos.begin() + closestInd);
							}
						}
					}
					if (agent_count == 10)
					{
						for (int j = 0; j < myAliveAgents.size(); ++j)
						{
							if (myAliveAgents[j]->assigned_place.x == -1 && myAliveAgents[j]->splash_bombs == 2)
							{
								Point b(11, 2);
								myAliveAgents[j]->assigned_place = b;
							}
						}
					}
				}
				else
				{
					vector<Point> fixPos;
					{
						Point z(5, 0);
						Point y(5, 5);
						fixPos.push_back(z);
						fixPos.push_back(y);
					}
					if (myAgentHighestRange)
					{
						for (int i = 0; i < highCovers.size(); ++i)
						{
							if (highCovers[i].x == 8 && highCovers[i].y > 1 && highCovers[i].y < 4
								&& map.getVal(highCovers[i].x + 1, highCovers[i].y) != LOW && map.getVal(highCovers[i].x + 1, highCovers[i].y) != HIGH)
							{
								Point o(highCovers[i].x + 1, highCovers[i].y);
								myAgentHighestRange->assigned_place = o;
								break;
							}
						}
						for (int i = 0; myAgentHighestRange->assigned_place.x != -1 && i < lowCovers.size(); ++i)
						{
							if (lowCovers[i].x == 8 && lowCovers[i].y > 1 && lowCovers[i].y < 4
								&& map.getVal(lowCovers[i].x + 1, lowCovers[i].y) != LOW && map.getVal(lowCovers[i].x + 1, lowCovers[i].y) != HIGH)
							{
								Point o(lowCovers[i].x + 1, lowCovers[i].y);
								myAgentHighestRange->assigned_place = o;
								break;
							}
						}
						if (myAgentHighestRange->assigned_place.x == -1)
						{
							if (map.getVal(8, 2) != LOW && map.getVal(8, 2) != HIGH)
							{
								Point o(8, 2);
								myAgentHighestRange->assigned_place = o;
							}
							if (myAgentHighestRange->assigned_place.x == -1)
							{
								if (map.getVal(8, 3) != LOW && map.getVal(8, 3) != HIGH)
								{
									Point o(8, 3);

									myAgentHighestRange->assigned_place = o;
								}
							}
						}
						if (myAgentHighestRange->assigned_place.x == -1)
						{
							Point z(8, 2);
							myAgentHighestRange->assigned_place = z;
						}
					}
					if (myAgentMostBomb)
					{
						Point o(5, 3);
						myAgentMostBomb->assigned_place = o;
					}
					
					for (int j = 0; j < myAliveAgents.size(); ++j)
					{
						int closestDist = 99999999;
						int closestInd = 0;
						if (myAliveAgents[j]->assigned_place.x == -1 && (myAliveAgents[j]->splash_bombs != 2 || agent_count != 10))
						{
							for (int i = 0; i < fixPos.size(); ++i)
							{
								int dist = Point::manhDist(myAliveAgents[j]->pos, fixPos[i]);
								if (dist < closestDist)
								{
									closestInd = i;
									closestDist = dist;
								}
							}
							if (closestDist != 99999999)
							{
								myAliveAgents[j]->assigned_place = fixPos[closestInd];
								fixPos.erase(fixPos.begin() + closestInd);
							}
						}
					}
					if (agent_count == 10)
					{
						for (int j = 0; j < myAliveAgents.size(); ++j)
						{
							if (myAliveAgents[j]->assigned_place.x == -1 && myAliveAgents[j]->splash_bombs == 2)
							{
								Point b(0, 2);
								myAliveAgents[j]->assigned_place = b;
							}
						}
					}
				}
			}
		}
		// ! a supprimer
		// if (loop_count == 0)
		// {
		// 	bool sideLeft;
		// 	if (myAliveAgents[0]->pos.x < map.width / 2)
		// 		sideLeft = true;
		// 	else
		// 		sideLeft = false;
		// 	for (int i = 0; i < myAliveAgents.size(); ++i)
		// 	{
		// 		if (myAliveAgents[i] != myAgentHighestRange && myAliveAgents[i]->assigned_place.x != -1)
		// 		{
		// 			if (sideLeft)
		// 				++myAliveAgents[i]->assigned_place.x;
		// 			else
		// 				--myAliveAgents[i]->assigned_place.x;
		// 		}
		// 	}
		// }
		// identify unique pos, to prevent from blocking
		for (int i = 0; i < myAliveAgents.size(); ++i)
		{
			for (int j = 0; j < mylastPos.size(); ++j)
			{
				if (mylastPos[j].second == myAliveAgents[i]->agent_id && myAliveAgents[i]->pos == mylastPos[j].first && !(myAliveAgents[i]->pos == myAliveAgents[i]->assigned_place))
				{
					int random = rand() % 4;
					if (random == 0 && ++random && map.inside(myAliveAgents[i]->pos.x, myAliveAgents[i]->pos.y + 1)
					&& map.getVal(myAliveAgents[i]->pos.x, myAliveAgents[i]->pos.y + 1) >= ME)
					{
						Point p(myAliveAgents[i]->pos.x, myAliveAgents[i]->pos.y + 1);
						myAliveAgents[i]->mustGo = p;
					}
					else if (random == 1 && ++random && map.inside(myAliveAgents[i]->pos.x, myAliveAgents[i]->pos.y - 1)
					&& map.getVal(myAliveAgents[i]->pos.x, myAliveAgents[i]->pos.y - 1) >= ME)
					{
						Point p(myAliveAgents[i]->pos.x, myAliveAgents[i]->pos.y - 1);
						myAliveAgents[i]->mustGo = p;
					}
					else if (random == 2 && ++random && map.inside(myAliveAgents[i]->pos.x + 1, myAliveAgents[i]->pos.y)
					&& map.getVal(myAliveAgents[i]->pos.x + 1, myAliveAgents[i]->pos.y) >= ME)
					{
						Point p(myAliveAgents[i]->pos.x + 1, myAliveAgents[i]->pos.y);
						myAliveAgents[i]->mustGo = p;
					}
					else if (random == 3 && ++random && map.inside(myAliveAgents[i]->pos.x - 1, myAliveAgents[i]->pos.y)
					&& map.getVal(myAliveAgents[i]->pos.x - 1, myAliveAgents[i]->pos.y) >= ME)
					{
						Point p(myAliveAgents[i]->pos.x - 1, myAliveAgents[i]->pos.y);
						myAliveAgents[i]->mustGo = p;
					}
				}
			}
		}

		mylastPos.clear();

		for (int i = 0; i < myAliveAgents.size(); ++i)
		{
			bool hasShoot = false;
			bool hasThrow = false;
			bool hasHunker = false;

			mylastPos.push_back({myAliveAgents[i]->pos, myAliveAgents[i]->agent_id});
			myAliveAgents[i]->announce();
			
			
			Agent* optOptoShoot = myAliveAgents[i]->optimalAgentShoot(opAliveAgents);
			if ((opAliveAgents.size() == 1 || myAliveAgents.size() == 1) && countMyTerritory(map, my_id) < countOpTerritory(map, my_id))
			{
				Point oo = territoryWithTheMostEnnemy(map, my_id);
				cout << "; MESSAGE " << oo.x << " " << oo.y;
				myAliveAgents[i]->move(oo);
			}
			else if (!hasThrow && myAliveAgents[i]->numberOfWait == 0 && myAliveAgents[i]->mustGo.x != -2)
			{
				cout << "; MESSAGE " << myAliveAgents[i]->mustGo.x << " " << myAliveAgents[i]->mustGo.y;
				myAliveAgents[i]->move(myAliveAgents[i]->mustGo);
			}
			else if (myAliveAgents[i]->numberOfWait == 0)
			{
				cout << "; MESSAGE " << myAliveAgents[i]->assigned_place.x << " " << myAliveAgents[i]->assigned_place.y;

				myAliveAgents[i]->move(myAliveAgents[i]->assigned_place);
			}
			if (/* dist <= 4 && dist > 1 && */ myAliveAgents[i]->splash_bombs > 0)
			{
				// ---------- find the furthest op agent that you can bomb ---------------
				// you can bomb if : - dist <= 6 (if 6 or 5 : bomb the case near it)
				//                   - and there's none of your agent near it
				int max_dist = 0;
				Point bombPos(-1, -1);
				bombPos = getOptCaseBomb(*myAliveAgents[i], map, my_id);
				if (bombPos.x != -1)
				{
					myAliveAgents[i]->throww(bombPos);
					hasThrow = true;
				}
			}
			if (!hasThrow && optOptoShoot && myAliveAgents[i]->cooldown == 0)
			{
				myAliveAgents[i]->shoot(optOptoShoot);
				hasShoot = true;
			}
			if (!hasShoot && !hasThrow)
				myAliveAgents[i]->hunker_down();
			cout << endl;
			if (myAliveAgents[i]->numberOfWait > 0)
				--myAliveAgents[i]->numberOfWait;
		}
		++loop_count;
    }

	/*----------------------------------------------------------------------*/
	/*                             CLEANING                                 */
	/*----------------------------------------------------------------------*/
	
	//agents
	for (int i = 0; i < agent_count0; ++i)
		delete agents[i];

}
