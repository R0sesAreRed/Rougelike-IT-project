#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <random>
#include <windows.h>
#include <conio.h>
#include <string>

using namespace std;



HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE); //do usuniecia

//zmienne globalne:

sf::RenderWindow window(sf::VideoMode(800, 600), "Roguelike v1.0");


const short int rs = 13; //room size
int roomx, roomy;
int hcx = 123, hcy = 5, hrx = 0, hry = 10, chrx = hrx, chry = hry; //hero coordiante x, y , hero room x, y, changed hero room x, y


int aix = 0, aiy = 0, aic;
int behp = 8, bedmg = 10, bespd = 2; //podstawowe statystyki przeciwnik�w

int			help = 0, hero = 4; //wyb�r postaci i obs�uga ekwipunku
bool			change = 0;
short int	menuhandler = 0;

int setrand = time(NULL);
int r(int a, int b) //random
{
	mt19937 generator(setrand);
	uniform_int_distribution<int> dist(a, b);
	setrand++;
	return dist(generator);
}

struct enemy //struktura przeciwnik�w
{
	bool type;
	int hp;
	int dmg;
	int spd;
	int item;
	int exp;
	short color;
	string letter;
	int sprite_no;
	bool stunned;
};
enemy en[500] = {};


class Board
{
public:
	int	walls[130][130] = {};
	int	units[130][130] = {};
	int	items[130][130] = {};
};
Board board;

class Player
{
public:
	int hp, dmg, armor, mr, mana, spd, movesleft, lvlupexp = 300;
	int chp = 1, cmana, cexp = 0, lvl = 1, floor_no = 1; //c - current
	const int bhp = 20, bdmg = 5, barmor = 5, bmr = 5, bmana = 0, bspd = 2, branddmg = 0; //base stats, for balancing purposes, const

	void assign_class_stats(int a, int b, int c, int d, int e, int f)
	{
		hp = bhp + a, dmg = bdmg + b, armor = barmor + c, mr = bmr + d, mana = bmana + e, spd = bspd + f, chp = hp, cmana = mana;
	}
};
Player p;

class Walls //�ciany i pod�oga
{
private:
	int w[rs] = { 4, 4, 4, 4, 0, 0, 0, 0, 4, 4, 4, 4, 4 }; //przej�cie mi�dzy pokojami
	int p0[12][12] = { //pok�j 1 (3-1 - �ciana, 0 - pod�oga)
		{4, 4, 4, 4, 0, 0, 0, 0, 4, 4, 4, 4},
		{4, 4, 4, 0, 0, 0, 0, 0, 0, 4, 4, 4},
		{4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4},
		{4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
		{4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4},
		{4, 4, 4, 0, 0, 0, 0, 0, 0, 4, 4, 4},
		{4, 4, 4, 4, 0, 0, 0, 0, 4, 4, 4, 4}
	};
	int p1[12][12] = { // pok�j 2
		{4, 4, 4, 4, 0, 0, 0, 0, 4, 4, 4, 4},
		{4, 4, 4, 4, 0, 0, 0, 0, 4, 4, 4, 4},
		{4, 4, 4, 4, 0, 0, 0, 0, 4, 4, 4, 4},
		{4, 4, 4, 4, 0, 0, 0, 0, 4, 4, 4, 4},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{4, 4, 4, 4, 0, 0, 0, 0, 4, 4, 4, 4},
		{4, 4, 4, 4, 0, 0, 0, 0, 4, 4, 4, 4},
		{4, 4, 4, 4, 0, 0, 0, 0, 4, 4, 4, 4},
		{4, 4, 4, 4, 0, 0, 0, 0, 4, 4, 4, 4},
	};

public:
	void roomgen(int a, int b, int c[12][12]) //uzupe�nianie segmentu planszy pokojem
	{
		for (int i = 0; i < 12; i++)
		{
			for (int j = 0; j < 12; j++)
			{
				board.walls[a * rs + i][b * rs + j] = c[i][j];
			}
		}
	}

	void wallgenv(int a, int b) //generacja poziomyh przej�� mi�dzy pokojami
	{
		if (b > 0)
		{
			switch (r(0, 3))
			{
			case 0: case 1: case 2:
			{
				for (int i = 0; i < 12; i++)
				{
					board.walls[a * rs + i][b * rs - 1] = w[i];
				}
			}break;
			case 3:
			{
				for (int i = 0; i < 12; i++)
				{
					for (int j = -2; j < 1; j++)
					{
						board.walls[a * rs + i][b * rs + j] = 4;
					}
				}
			}break;
			}
		}
	}

	void wallgenh(int a, int b) //generacja pionowych przej�� im�dzy pokojami
	{
		if (a > 0)
		{
			switch (r(0, 3))
			{
			case 0: case 1: case 2:
			{
				for (int i = 0; i < rs; i++)
				{
					board.walls[a * rs - 1][b * rs + i] = w[i];
				}

			}break;
			case 3:
			{
				for (int i = 0; i < rs; i++)
				{
					for (int j = -2; j < 1; j++)
					{
						board.walls[a * rs + j][b * rs + i] = 4;
					}
				}
			}break;
			}
		}

	}
	void gen()  //generowanie planszy
	{
		for (int i = 0; i < 10; i++)
		{
			for (int j = 0; j < 10; j++)
			{
				switch (r(0, 1))
				{
				case 0:
				{
					roomgen(i, j, p0);
				}break;
				case 1:
				{
					roomgen(i, j, p1);
				}break;
				}
				wallgenh(i, j);
				wallgenv(i, j);
			}
		}
		for (int i = 0; i < 129; i++) //zewnetrzne �ciany
		{
			board.walls[0][i] = 5;
			board.walls[128][i] = 5;
			board.walls[129][i] = 5;
			board.walls[i][0] = 5;
			board.walls[i][128] = 5;
			board.walls[i][129] = 5;
		}
		board.walls[7][123] = 10; //schody
	}
};
Walls walls;

class Units //przeciwnicy
{
public:

	void loottable(int a, int b) //tabele przedmoit�w przeciwnik�w
	{
		switch (a)
		{
		case 0:
		{
			switch (r(0, 2))
			{
			case 0:
			{
				en[b].item = 1;
			}break;
			case 1:
			{
				en[b].item = 2;
			}break;
			default: {};
			}
		}break;
		case 1:
		{
			switch (r(0, 3))
			{
			case 0:
			{
				en[b].item = 1;
			}break;
			case 1:
			{
				en[b].item = 2;
			}break;
			case 2:
			{
				en[b].item = 3;
			}
			default: {};
			}
		}break;
		}
	}

	void gen() //generowanie przeciwnikow
	{
		int a = 3, b, rx, ry, ru; //random x, random y, random unit
		for (int i = 0; i < 10; i++)
		{
			for (int j = 0; j < 10; j++)
			{
				if (!(i == 9 && j == 0))
				{
					b = r(3, 5);
					for (int k = 0; k < b; k++)
					{
						rx = r(1, 10);
						ry = r(1, 10);
						while (board.units[i * rs + rx][j * rs + ry] != 0 || board.walls[i * rs + rx][j * rs + ry] != 0)
						{
							rx = r(1, 10);
							ry = r(1, 10);
						}
						ru = r(3, 6);
						board.units[i * rs + rx][j * rs + ry] = a;

						switch (ru)
						{
						case 3: //Goblin
						{
							en[a].hp = behp - r(2, 4);
							en[a].dmg = bedmg + r(0, 2);
							en[a].spd = bespd + 1;
							en[a].exp = 30;
							en[a].type = 0;
							en[a].letter = " g";
							en[a].sprite_no = 2;
							en[a].color = 10;
							loottable(0, a);
						}break;
						case 4: //Spider
						{
							en[a].hp = behp - r(4, 6);
							en[a].dmg = bedmg + r(0, 1);
							en[a].spd = bespd;
							en[a].exp = 30;
							en[a].type = 0;
							en[a].letter = " S";
							en[a].sprite_no = 3;
							en[a].color = 14;
							loottable(0, a);
						}break;
						case 5: //fiend
						{
							en[a].hp = behp;
							en[a].dmg = bedmg;
							en[a].spd = bespd;
							en[a].exp = 30;
							en[a].type = 0;
							en[a].letter = " F";
							en[a].sprite_no = 4;
							en[a].color = 4;
							loottable(0, a);
						}break;
						case 6: //golem
						{
							en[a].hp = behp + r(5, 7);
							en[a].dmg = bedmg + r(2, 4);
							en[a].spd = bespd - 1;
							en[a].exp = 60;
							en[a].type = 0;
							en[a].letter = " G";
							en[a].sprite_no = 5;
							en[a].color = 8;
							loottable(1, a);
						}break;
						}
						a++;
					}
				}
			}
		}
	}
};
Units units;


class Equipment
{
public:
	int	content[5][4] = { {0, 0, 0, 0 }, {0, 1, 3, 2}, {3, 0, 0, 0}, {1, 0 , 0, 1}, {0, 0, 0, 0} }; //ekwipunek
	int x = 0, y = 0, choicex = 10, choicey = 10;
};
Equipment eq;

class Minimap
{
public:
	int minimaptech[10][10] = {}; //minimapa2
	char map[10][10]; //minimapa
	void mmap() //generowanie minimapy
	{
		for (int i = 0; i < 10; i++)
		{
			for (int j = 0; j < 10; j++)
			{
				minimaptech[i][j] = ((board.walls[i * rs][j * rs + 5]) == 0 ? 0 : 1) + ((board.walls[i * rs + 5][(j * rs) + 11] == 0 ? 0 : 1) * 2) + ((board.walls[(i * rs) + 11][j * rs + 5] == 0 ? 0 : 1) * 4) + ((board.walls[i * rs + 5][j * rs] == 0 ? 0 : 1) * 8);
				switch (minimaptech[i][j])
				{
				case 15: {map[i][j] = ' '; }break;
				case 14: {map[i][j] = 0xC1; }break;
				case 13: {map[i][j] = 0xC3; }break;
				case 12: {map[i][j] = 0xC8; }break;
				case 11: {map[i][j] = 0xC2; }break;
				case 10: {map[i][j] = 0xBA; }break;
				case 9: {map[i][j] = 0xC9; }break;
				case 8: {map[i][j] = 0xCC; }break;
				case 7: {map[i][j] = 0xB4; }break;
				case 6: {map[i][j] = 0xBC; }break;
				case 5: {map[i][j] = 0xCD; }break;
				case 4: {map[i][j] = 0xCA; }break;
				case 3: {map[i][j] = 0xBB; }break;
				case 2: {map[i][j] = 0xB9; }break;
				case 1: {map[i][j] = 0xCB; }break;
				case 0: {map[i][j] = 0xCE; }
				}
			}
		}
	}
};
Minimap minimap;


struct Item
{
	int hp;
	int dmg;
	int armor;
	int mr;
	int mana;
	int spd;
	bool consumable;
	int sprite_no;
	int eqslot;
	short rarity;
	string opis;
	void printopis()
	{
		for (short i = 0; i < 5; i++)
		{
			SetConsoleCursorPosition(handle, { 34, (short)(11 + i) });
			std::cout << setw(35) << " " << endl;
		}
		int c = 0, d = 1;
		SetConsoleCursorPosition(handle, { 34, 11 });
		for (int i = 0; i < opis.length(); i++)
		{
			std::cout << opis.at(i); c++;
			if ((c > 25 && opis.at(i) == ' ') || opis.at(i) == '.')
			{
				SetConsoleCursorPosition(handle, { 34, (short)(11 + d) });
				d++;
				c = 0;
			}
		}
	}
};
struct tempbuff
{
	int hp;
	int dmg;
	int armor;
	int mr;
	int	hmana;
	int spd;
	int randdmg;
	void reset()
	{
		p.hp -= hp; hp = 0;
		if (p.chp > p.hp) p.chp = p.hp;
		p.armor -= armor; armor = 0;
		p.mr -= mr; mr = 0;
		p.mana -= hmana; hmana = 0;
		if (p.cmana > p.mana) p.cmana = p.mana;
		p.spd -= spd; spd = 0;
		p.dmg -= dmg; dmg = 0;
	}
};
tempbuff nextturn = { 0, 0, 0, 0, 0, 0, 0 };
tempbuff nextattack = { 0, 0, 0, 0, 0, 0, 0 };
tempbuff nextroom = { 0, 0, 0, 0, 0, 0, 0 };

Item none{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, };
Item hpdrop{ 15, 0, 0, 0, 0, 0, 1, 6, 0, 1, };
Item chainmail{ 0, 0, 5, 0, 0, 0, 0, 7, 2, 1, "Kolczuga.Zalozenie jej zwieksza pancerz o 5" };
Item hpamulet{ 10, 0, 0, 0, 0, 0, 0, 8, 4, 1, "Amulet zdrowia.Zalozenie go zwieksza zdrowie o 10" };
Item item[4] = { none, hpdrop, hpamulet, chainmail };
void dropitem(int a, int b, int d)
{
	int c = 0;
	while (d > 0)
	{
		for (int i = -c; i <= c; i++)
		{
			for (int j = -c; j <= c; j++)
			{
				if (board.items[a + i][b + j] == 0 && board.walls[a + i][b + j] == 0)
				{
					board.items[a + i][b + j] = d;
					d = 0;
				}
			}
		}
		c++;
	}
}


class TextureType
{
public:
	int type = 1;
	sf::Texture texture;
	TextureType(string _texture) {
		texture.loadFromFile(_texture + ".png");
	}
};

TextureType texturetype[] = {
	TextureType("player"),	//0
	TextureType("wall"),	//1
	TextureType("enemy1"),	//2
	TextureType("enemy2"),	//3
	TextureType("enemy3"),	//4
	TextureType("enemy4"),	//5
	TextureType("item1"),	//6
	TextureType("item2"),	//7
	TextureType("item3"),	//8
	TextureType("item4"),	//9
	TextureType("item5"),	//10
	TextureType("item6"),	//11
	TextureType("item7"),	//12
	TextureType("item8"),	//13
	TextureType("item9"),	//14
	TextureType("item10"),	//15
	TextureType("floor"),	//16
	TextureType("spelloverlay"), //17
	TextureType("emptyspelloverlay") //18
};

class Tile {
public:
	int height = 32;
	int width = 32;

	sf::Sprite sprite;
	void set_sprite(int i) {
		sprite.setTexture(texturetype[i].texture);
	}
	void set_position(int x, int y) {
		sprite.setPosition(x, y);
	}
};



class Engine
{
public:
	int size_x;
	int size_y;
	Tile gameboard[14][14] = {};
	Tile overlayboard[14][14] = {1};
	void set_size() {
		size_x = 14;
		size_y = 14;
	}
	Engine() {
		set_size();
		for (int i = 0; i < size_x; i++) {
			for (int j = 0; j < size_y; j++) {
				gameboard[i][j].set_sprite(1);
				gameboard[i][j].set_position(j * 32, i * 32);
				overlayboard[i][j].set_sprite(18);
				overlayboard[i][j].set_position(j * 32, i * 32);
			}
		}
	}

	void set_gameboard() {
		for (int i = 0; i <= rs; i++) {
			for (int j = 0; j <= rs; j++) {
				if (board.walls[roomx + i - 1][roomy + j - 1] != 0) {
					gameboard[i][j].set_sprite(1); //walls
				}
				else if (board.units[roomx + i - 1][roomy + j - 1] > 2) {
					gameboard[i][j].set_sprite(en[board.units[roomx + i - 1][roomy + j - 1]].sprite_no); //units
				}
				else if (board.units[roomx + i - 1][roomy + j - 1] == 2) {
					gameboard[i][j].set_sprite(0); //player
				}
				else if (board.items[roomx + i - 1][roomy + j - 1] != 0) {
					gameboard[i][j].set_sprite(item[board.items[roomx + i - 1][roomy + j - 1]].sprite_no); //items
				}
				else {
					gameboard[i][j].set_sprite(16); //floor
				}
			}
		}
	}
	void set_overlayboard(int i, int j)
	{
		overlayboard[i+1][j+1].set_sprite(17);
	}
	void draw_gameboard(sf::RenderWindow& _window) {
		for (int i = 0; i < 14; i++) {
			for (int j = 0; j < 14; j++) {
				_window.draw(gameboard[j][i].sprite);
			}
		}
	}
	void draw_overlayboard(sf::RenderWindow& _window) {
		for (int i = 0; i < 14; i++) {
			for (int j = 0; j < 14; j++) {
				_window.draw(overlayboard[j][i].sprite);
			}
		}
	}
	void reset_overlayboard()
	{
		for (int i = 0; i <= rs; i++) {
			for (int j = 0; j <= rs; j++) {
				overlayboard[i][j].set_sprite(18);
			}
		}

	}
};


Engine engine;

void floorprint() {
	window.clear();
	engine.set_gameboard();
	engine.draw_gameboard(window);
	window.display();
}
void overlayprint()
{
	window.clear();
	engine.draw_gameboard(window);
	engine.draw_overlayboard(window);
	window.display();
}


int magicmenu, known_spells = 4, magic = 0, magicfirst;
int magicchoice, how_many_enemies, spellx = 0, spelly = 0, spellhelp, rotation;
bool respawn = 0, stealth = 0;

void kulaognia() { //zakelcia maga
	for (int i = -1; i <= 1; i++)
	{
		for (int j = -1; j <= 1; j++)
		{
			board.items[roomx + spellx + i][roomy + spelly + j] = 0;
			if (board.units[spellx + i + roomx][spelly + j + roomy] == 2)
			{
				p.chp -= 20;
			}
			else if (en[board.units[spellx + i + roomx][spelly + j + roomy]].hp - 20 <= 0)
			{
				p.cexp += en[board.units[roomx + spellx + i][roomy + spelly + j]].exp;
				board.units[roomx + spellx + i][roomy + spelly + j] = 0;
			}
			else if (en[board.units[spellx + i + roomx][spelly + j + roomy]].hp - 20 > 0)
			{
				en[board.units[spellx + i + roomx][spelly + j + roomy]].hp -= 20;
			}
		}
	}
}
void pancerzmaga() {
	nextroom.armor += 5;
	nextroom.mr += 5;
	p.armor += 5;
	p.mr += 5;
}
void wallhelp(int a, int b)
{
	board.walls[spellx + a + roomx][spelly + b + roomy] = 4;
}
void kamiennasciana() {
	switch (rotation)
	{
	case 0: case 4:
	{
		wallhelp(0, -1);
		wallhelp(0, 0);
		wallhelp(0, 1);

	}break;
	case 1: case 5:
	{
		wallhelp(-1, 1);
		wallhelp(0, 0);
		wallhelp(1, -1);

	}break;
	case 2: case 6:
	{
		wallhelp(1, 0);
		wallhelp(0, 0);
		wallhelp(-1, 0);

	}break;
	case 3: case 7:
	{
		wallhelp(1, 1);
		wallhelp(0, 0);
		wallhelp(-1, -1);

	}break;
	}
}
void pocisklodu() {
	if (en[board.units[roomx + spellx][roomy + spelly]].hp - 2 <= 0)
	{
		dropitem(roomx + spellx, roomy + spelly, en[board.units[roomx + spellx][roomy + spelly]].item);
		p.cexp += en[board.units[roomx + spellx][roomy + spelly]].exp;
		board.units[roomx + spellx][roomy + spelly] = 0;
	}
	else
	{
		en[board.units[roomx + spellx][roomy + spelly]].hp -= 2;
		en[board.units[roomx + spellx][roomy + spelly]].stunned = TRUE;
	}
}
void teleportacja() {
	if (board.walls[spellx + roomx][spelly + roomy] == 1)
	{
		swap(board.units[spellx + roomx][spelly + roomy], board.units[hcx][hcy]);
		p.chp = 0;
	}
	else
	{
		swap(board.units[spellx + roomx][spelly + roomy], board.units[hcx][hcy]);
		hcx = spellx + roomx;
		hcy = spelly + roomy;
	}
}

void uleczrane() { //zaklecia kalpana
	p.chp = p.chp + 20 > p.hp ? p.hp : p.chp + 20;
}
void rozblyskcienia() {

	for (int i = -1; i <= 1; i++)
	{
		for (int j = -1; j <= 1; j++)
		{
			if (board.units[hcx + i][hcy + j] > 2)
			{
				if (en[board.units[hcx + i][hcy + j]].hp - 5 <= 0)
				{
					dropitem(hcx + i, hcy + j, en[board.units[hcx + i][hcy + j]].item);
					p.cexp += en[board.units[hcx + i][hcy + j]].exp;
					board.units[hcx + i][hcy + j] = 0;
					p.hp++;
				}
				else
				{
					en[board.units[hcx + i][hcy + j]].hp -= 5;
				}
				p.chp = p.chp + 5 > p.hp ? p.hp : p.chp + 5;
			}
		}
	}
}
void wybuchswiatla() {
	for (int i = -1; i <= 1; i++)
	{
		for (int j = -1; j <= 1; j++)
		{
			if (board.units[hcx + i][hcy + j] > 2)
			{
				if (board.walls[hcx + 2 * i][hcy + 2 * j] > 0 || board.units[hcx + 2 * i][hcy + 2 * j] != 0)
				{
					if (en[board.units[hcx + i][hcy + j]].hp - 10 <= 0)
					{
						dropitem(hcx + i, hcy + j, en[board.units[hcx + i][hcy + j]].item);
						p.cexp += en[board.units[hcx + i][hcy + j]].exp;
						board.units[hcx + i][hcy + j] = 0;
					}
					else
					{
						en[board.units[hcx + i][hcy + j]].hp -= 10;
					}
				}
				else if (board.units[hcx + 2 * i][hcy + 2 * j] == 0)
				{
					swap(board.units[hcx + i][hcy + j], board.units[hcx + 2 * i][hcy + 2 * j]);
				}
				en[board.units[hcx + i][hcy + j]].stunned = TRUE;
			}
		}
	}

}
void odrodzenie() {
	respawn = 1;
}
void boskiwyrok() {
	dropitem(roomx + spellx, roomy + spelly, en[board.units[roomx + spellx][roomy + spelly]].item);
	p.cexp += en[board.units[roomx + spellx][roomy + spelly]].exp;
	board.units[roomx + spellx][roomy + spelly] = 0;
}

void cioswplecy() { //zaklecia �otra
	nextattack.dmg += 10;
	p.dmg += 10;
}
void sprint() {
	nextturn.spd += 2;
	p.movesleft += 2;
	p.spd += 2;
}
void ukrycie() {
	stealth = 1;
}
void rzutnozem() {
	if (en[board.units[roomx + spellx][roomy + spelly]].hp - p.dmg <= 0)
	{
		dropitem(roomx + spellx, roomy + spelly, en[board.units[roomx + spellx][roomy + spelly]].item);
		p.cexp += en[board.units[roomx + spellx][roomy + spelly]].exp;
		board.units[roomx + spellx][roomy + spelly] = 0;
	}
	else
	{
		en[board.units[roomx + spellx][roomy + spelly]].hp -= p.dmg;
	}
	nextattack.reset();
}
void improwizacja() {
	switch (r(0, 9))
	{
	case 0: {spellx = r(roomx, roomx + 12); spelly = r(roomx, roomx + 12); kulaognia(); }break;
	case 1: {pancerzmaga(); }break;
	case 2: {spellx = r(roomx, roomx + 12); spelly = r(roomx, roomx + 12); rotation = r(0, 7); kamiennasciana(); }break;
	case 3: {while (board.units[spellx][spelly] < 2) { spellx = r(roomx, roomx + 12); spelly = r(roomx, roomx + 12); }pocisklodu(); }break;
	case 4: {spellx = r(roomx, roomx + 12); spelly = r(roomx, roomx + 12); teleportacja(); }break;
	case 5: {uleczrane(); }break;
	case 6: {rozblyskcienia(); }break;
	case 7: {wybuchswiatla(); }break;
	case 8: {odrodzenie(); }break;
	case 9: {while (board.units[spellx][spelly] < 2) { spellx = r(roomx, roomx + 12); spelly = r(roomx, roomx + 12); }boskiwyrok(); }break;
	}
}


/*void spelldisplay(int a, int b)
{
	if (board.units[spellx + a + roomx][spelly + b + roomy] > 2)
	{
		SetConsoleCursorPosition(handle, { (short)((spelly + 1 + b) * 2), (short)(spellx + 1 + a) });
		SetConsoleTextAttribute(handle, 92);
		std::cout << setw(2) << "E";
	}
	else if (board.units[spellx + a + roomx][spelly + b + roomy] == 2 || board.units[spellx + roomx + a][spelly + roomy + b] == 1)
	{
		SetConsoleCursorPosition(handle, { (short)((spelly + 1 + b) * 2), (short)(spellx + 1 + a) });
		SetConsoleTextAttribute(handle, 95);
		std::cout << setw(2) << 'H';
	}
	else
	{
		SetConsoleCursorPosition(handle, { (short)((spelly + 1 + b) * 2), (short)(spellx + 1 + a) });
		SetConsoleTextAttribute(handle, 80);
		std::cout << setw(2) << "  ";
	}
}*/

//^ DO USUNIECIA


class Zaklecie
{
public:
	string spellname;
	int whencast;
	int manacost;
	string opis;
	void(*castt)();
	void cast()
	{
		castt();
	}
	void printopis() // DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA 
	{
		for (short i = 0; i < 5; i++)
		{
			SetConsoleCursorPosition(handle, { (short)(rs * 2 + 2), (short)(11 + i) });
			std::cout << setw(35) << " " << endl;
		}
		int c = 0, d = 1;
		SetConsoleCursorPosition(handle, { (short)(rs * 2 + 2), 11 });
		for (int i = 0; i < opis.length(); i++)
		{
			std::cout << opis.at(i); c++;
			if (c > 25 && opis.at(i) == ' ')
			{
				SetConsoleCursorPosition(handle, { (short)(rs * 2 + 2), (short)(11 + d) });
				d++;
				c = 0;
			}
		}
	}
	void display(int a, int b)
	{
		switch (a)
		{
		case 2:
		{
			switch (b)
			{
			case 0:
			{
				for (int i = -1; i <= 1; i++)
				{
					for (int j = -1; j <= 1; j++)
					{
						engine.set_overlayboard(spellx + i, spelly + j);
					}
				}
			}break;
			case 2:
			{
				switch (rotation)
				{
				case 0: case 4:
				{
					engine.set_overlayboard(spellx, spelly - 1);
					engine.set_overlayboard(spellx, spelly);
					engine.set_overlayboard(spellx, spelly + 1);

				}break;
				case 1: case 5:
				{
					engine.set_overlayboard(spellx - 1, spelly + 1);
					engine.set_overlayboard(spellx, spelly);
					engine.set_overlayboard(spellx + 1, spelly - 1);

				}break;
				case 2: case 6:
				{
					engine.set_overlayboard(spellx + 1, spelly);
					engine.set_overlayboard(spellx, spelly);
					engine.set_overlayboard(spellx - 1, spelly);

				}break;
				case 3: case 7:
				{
					engine.set_overlayboard(spellx + 1, spelly + 1);
					engine.set_overlayboard(spellx, spelly);
					engine.set_overlayboard(spellx - 1, spelly - 1);

				}break;
				}
			}break;
			case 4:
			{
				engine.set_overlayboard(spellx, spelly);
			}break;
			}
		}
		overlayprint();
		}
	}
	Zaklecie() {}
	Zaklecie(string s, int w, int m, string o, void(*c)()) {
		spellname = s;
		whencast = w;
		manacost = m;
		opis = o;
		castt = c;
	}


};
Zaklecie pliska;
Zaklecie z[3][5] = { {
Zaklecie("Cios w plecy       ", 3, 2, "Zwieksza obrazenia nastepnego ataku lub rzutu nozem", cioswplecy), //3 - natychmiastowe, 2 - namierzane na cel, 1 namierzane na podloge
Zaklecie("Sprint             " , 3, 4, "Zwieksza predkoc ruchu o 2 do nastepnego pokoju", sprint),
Zaklecie("Ukrycie               ", 3, 0, "Przeciwnicy nie widza cie do nastepnego pokoju", ukrycie),
Zaklecie("Rzut nozem           ", 2, 4, "Zadaje obrazenia postaci wybranemu przeciwnikowi w pokoju", rzutnozem),
Zaklecie("Improwizacja          ", 3, 10, "Rzuca losowe zaklecie z innej klasy", improwizacja),
}, {
Zaklecie("Kula ognia        ", 1, 15, "Zadaje duze obrazenia w polu 3x3. Niszczy przedmioty.", kulaognia),
Zaklecie("Pancerz maga        ", 3, 5, "Zwieksza pancerz i odpornosc na magie do nastepnego pokoju", pancerzmaga),
Zaklecie("Kamienna sciana    ", 1, 10, "Tworzy sciane w wybranym miejscu", kamiennasciana),
Zaklecie("Pocisk lodu        ", 2, 5, "Zadaje obazenia na odlegolosc i zamraza przeciwnika na nastepny ruch", pocisklodu),
Zaklecie("Teleportacja        ", 1, 10, "Teleportuje postac na wybrane pole", teleportacja),
}, {
Zaklecie("Ulecz Rane       ", 3, 5, "Przywraca zdrowie", uleczrane),
Zaklecie("Rozblysk cienia     ", 3, 10, "Zadaje obrazenia wszystkim wrogom dookola postaci i zwieksza maksymalne zdrowie o 1 za kazdego zbitego tym zakleciem wroga", rozblyskcienia),
Zaklecie("Wybuch swiatla        " , 3, 10, "Odrzuca wszystkich wrogow dookola bohatera i oglusza ich na nastepna ture", wybuchswiatla),
Zaklecie("Odrodzenie          ", 3, 20, "Zapobiega nastepnemu smiertelnemu ciosowi przeciwnika", odrodzenie),
Zaklecie("Boski Wyrok         ", 2, 20, "Zabija wybranego wroga", boskiwyrok),
} };



//do zastapienia


void deathanim(short a, short b) // animacja przy �mierci lub escape DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA 
{
	for (short i = 26; i >= 0; i -= 2)
	{
		SetConsoleCursorPosition(handle, { i, a });
		Sleep(40 - b);
		cout << setw(2) << "  ";
	}
	for (short i = 0; i <= 26; i += 2)
	{
		SetConsoleCursorPosition(handle, { i, (short)(a + 1) });
		Sleep(37 - b);
		cout << setw(2) << "  ";
	}

}
void ramkaeq() //funkcja wypisuj�ca ramk� DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA 
{
	SetConsoleTextAttribute(handle, 12);
	std::cout << (char)0xC9 << setw(6) << setfill((char)0xCD) << (char)0xCB;
	SetConsoleTextAttribute(handle, 7);
	std::cout << setw(6) << (char)0xCB << setw(6) << (char)0xCB << setw(6) << (char)0xCB << setw(6) << (char)0xBB;
	for (int i = 1; i < 4; i++)
	{
		SetConsoleTextAttribute(handle, 12);
		SetConsoleCursorPosition(handle, { 0, (short)(4 * i) });
		std::cout << (char)0xCC << setw(6) << (char)0xCE;
		SetConsoleTextAttribute(handle, 7);
		std::cout << setw(6) << (char)0xCE << setw(6) << (char)0xCE << setw(6) << (char)0xCE << setw(6) << (char)0xB9;
	}
	SetConsoleCursorPosition(handle, { 0, 16 });
	SetConsoleTextAttribute(handle, 12);
	std::cout << (char)0xC8 << setw(6) << (char)0xCA;
	SetConsoleTextAttribute(handle, 7);
	std::cout << setw(6) << (char)0xCA << setw(6) << (char)0xCA << setw(6) << (char)0xCA << setw(6) << (char)0xBC;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 6; j++)
		{
			if (j == 0 || j == 1) SetConsoleTextAttribute(handle, 12);
			SetConsoleCursorPosition(handle, { (short)(j * 6), (short)(i * 4 + 1) });
			std::cout << (char)0xBA;
			SetConsoleCursorPosition(handle, { (short)(j * 6), (short)(i * 4 + 2) });
			std::cout << (char)0xBA;
			SetConsoleCursorPosition(handle, { (short)(j * 6), (short)(i * 4 + 3) });
			std::cout << (char)0xBA;
			if (j == 0 || j == 1) SetConsoleTextAttribute(handle, 7);
		}
	}
	cout << setfill(' ');
}
void printstats() //DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA 
{
	for (short i = 0; i < 7; i++)
	{
		SetConsoleCursorPosition(handle, { (short)(rs * 2 + 15), i });
		std::cout << setw(23) << " " << endl;
	}
	SetConsoleCursorPosition(handle, { (short)(rs * 2 + 15), 0 });
	SetConsoleTextAttribute(handle, 15);
	std::cout << "exp: " << p.cexp << " / " << p.lvlupexp << "  LVL: " << p.lvl;
	SetConsoleCursorPosition(handle, { (short)(rs * 2 + 15), 1 });
	double ratio = (p.chp * 1.0) / (p.hp * 1.0);
	for (double i = 0.0; i < 10.0; i++)
	{
		if ((i / 10) < ratio)
		{
			SetConsoleTextAttribute(handle, 192);
			std::cout << "  ";
		}
		else
		{
			SetConsoleTextAttribute(handle, 128);
			std::cout << "  ";
		}
	}
	SetConsoleCursorPosition(handle, { (short)(rs * 2 + 15), 2 });
	SetConsoleTextAttribute(handle, 15);
	std::cout << p.chp << " / " << p.hp;
	SetConsoleCursorPosition(handle, { (short)(rs * 2 + 15), 3 });
	SetConsoleTextAttribute(handle, 14);
	std::cout << "Armor: " << p.armor;
	SetConsoleCursorPosition(handle, { (short)(rs * 2 + 15), 4 });
	SetConsoleTextAttribute(handle, 11);
	std::cout << "Magic Res.: " << p.mr;
	SetConsoleCursorPosition(handle, { (short)(rs * 2 + 15), 5 });
	SetConsoleTextAttribute(handle, 4);
	std::cout << "Damage: " << p.dmg;
	SetConsoleCursorPosition(handle, { (short)(rs * 2 + 15), 6 });
	SetConsoleTextAttribute(handle, 8);
	std::cout << "Speed: " << p.movesleft << " / " << p.spd << "   ";
	if (hero != 0)
	{
		SetConsoleCursorPosition(handle, { (short)(rs * 2 + 15), 7 });
		SetConsoleTextAttribute(handle, 9);
		std::cout << "Mana: " << p.cmana << " / " << p.mana << "   ";
	}

}
void spellborderprint() //wypisywanie ramki menu zakl�� DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA DO USUNIECIA 
{
	SetConsoleCursorPosition(handle, { 0, 8 });
	SetConsoleTextAttribute(handle, 11);
	std::cout << (char)0xC9 << setw(27) << setfill((char)0xCD) << (char)0xBB;
	for (short i = 9; i < 14; i++)
	{
		SetConsoleCursorPosition(handle, { 0, i });
		std::cout << (char)0xBA << setw(27) << setfill(' ') << (char)0xBA;
	}
	SetConsoleCursorPosition(handle, { 0, 14 });
	std::cout << (char)0xC8 << setw(27) << setfill((char)0xCD) << (char)0xBC;
	cout << setfill(' ');
}
//do zastapienia


void secretrooms()
{
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			if (minimap.map[i][j] == ' ') //pok�j bez wej��
			{
				for (int k = i * rs; k < i * rs + rs; k++)
				{
					for (int l = j * rs; l < j * rs + rs; l++)
					{
						board.units[k][l] = 0;
					}
				}
				units.loottable(1, 0);
				board.items[i * rs + 5][j * rs + 5] = en[0].item;
				units.loottable(1, 0);
				board.items[i * rs + 4][j * rs + 6] = en[0].item;
				units.loottable(1, 0);
				board.items[i * rs + 5][j * rs + 4] = en[0].item;
			}
		}
	}
}

void mapcontrol(int a, int b) //poruszanie sie postaci, zadawanie obra�e�, przedmioty, zmiana pi�tra
{
	if (board.walls[hcx + a][hcy + b] == 0 && board.units[hcx + a][hcy + b] == 0) { board.units[hcx][hcy] = 0; hcy += b; hcx += a; p.movesleft--; }
	else if (board.units[hcx + a][hcy + b] != 0)
	{
		en[board.units[hcx + a][hcy + b]].hp -= p.dmg;
		nextattack.reset();
		if (stealth == 1) stealth = 0;
		if (en[board.units[hcx + a][hcy + b]].hp <= 0)  //przeciwnik ginie
		{
			dropitem(hcx + a, hcy + b, en[board.units[hcx + a][hcy + b]].item);
			p.cexp += en[board.units[hcx + a][hcy + b]].exp; //zdobywanie doswiadczenia za zabijanie przeciwnikow
			board.units[hcx + a][hcy + b] = 0;
		}
		p.movesleft--;
	}
	else if (board.walls[hcx + a][hcy + b] > 0 && board.walls[hcx + a][hcy + b] < 5)
	{
		board.walls[hcx + a][hcy + b]--; p.movesleft--;
		floorprint();
	}
	else if (board.walls[hcx + a][hcy + b] == 10) //zmiana pi�tra
	{
		behp = behp * 2;
		bedmg = bedmg * 2;
		bespd++; //zwi�kszenie statystyk przeciwnik�w, poziomu trudno�ci
		swap(board.units[hcx][hcy], board.units[123][5]); //reset coord�w postaci
		hcx = 123;
		hcy = 5;
		for (int i = 0; i < 130; i++) //czyszczenie tabel
		{
			for (int j = 0; j < 130; j++)
			{
				if (board.units[i][j] > 2)
					board.units[i][j] = 0;
				board.walls[i][j] = 0;
				board.items[i][j] = 0;
			}
		}
		walls.gen();
		minimap.mmap();
		units.gen();
		secretrooms();
		floorprint();
		p.floor_no++;
	}
	if (board.items[hcx][hcy] > 0) //podnoszenie przedmiot�w
	{
		if (item[board.items[hcx][hcy]].consumable == 1) //consumable
		{
			p.chp = p.chp + item[board.items[hcx][hcy]].hp > p.hp ? p.hp : p.chp + item[board.items[hcx][hcy]].hp;
			board.items[hcx][hcy] = 0;
		}
		else
		{
			for (int i = 0; i < 4; i++) //umieszczanie przedmiot�w w ekwipunku
			{
				for (int j = 1; j < 5; j++)
				{
					if (eq.content[j][i] == 0)
					{
						eq.content[j][i] = board.items[hcx][hcy];
						board.items[hcx][hcy] = 0;
					}
				}
			}
		}
	}
}



class AI
{
private:
	static const int max_no_of_enemies = 5;
public:
	int aiused[max_no_of_enemies] = {}, nr; //AI

	void enemyattack(int i, int j, int k, int c) //funkcja ataku przeciwnika
	{
		if (en[board.units[i][j]].type == 0)
		{
			p.chp -= ((en[c].dmg - p.armor) > 0 ? (en[c].dmg - p.armor) : 0);
		}
		else
		{
			p.chp -= ((en[c].dmg - p.mr) > 0 ? (en[c].dmg - p.mr) : 0);
		}
		k = en[c].spd + 1;
	}

	void AII() //AI przeciwnik�w
	{
		nr = 0;
		for (int i = 0; i < max_no_of_enemies; i++)
		{
			aiused[i] = 0;
		}
		for (int i = roomx; i < roomx + 12; i++)
		{
			for (int j = roomy; j < roomy + 12; j++)
			{
				if (board.units[i][j] > 2) //je�eli jednostka istnieje i nie jest gracze, oraz nie zostala juz poruszona
				{
					int c = board.units[i][j];
					if (en[c].stunned == TRUE) //je�eli jednostka nie ma na sobie efktu stunned
					{
						en[c].stunned = FALSE;
						aiused[nr] = c; nr++;
					}
					else if (board.units[i][j] != aiused[0] && board.units[i][j] != aiused[1] && board.units[i][j] != aiused[2] && board.units[i][j] != aiused[3] && board.units[i][j] != aiused[4])
					{
						int dx = 0, dy = 0;
						aiused[nr] = c; nr++;
						for (int k = 0; k < en[c].spd; k++)
						{
							Sleep(50);
							floorprint();
							if (abs((i + dx) - hcx) >= abs((j + dy) - hcy)) //je�eli jednosta jest dalej w poziomie niz w pionie
							{
								if ((i + dx) - hcx < 0) //je�eli jenostka jest na lewo od gracza
								{
									if (board.units[i + 1 + dx][j + dy] == 0 && board.walls[i + 1 + dx][j + dy] == 0)
									{
										swap(board.units[i + 1 + dx][j + dy], board.units[i + dx][j + dy]);
										dx++;
									}
									else if (board.units[i + 1 + dx][j + dy] == 2) //atak
									{
										enemyattack(i, j, k, c);
									}
									else if ((j + dy) - hcy < 0)
									{
										if (board.units[i + dx][j + 1 + dy] == 0 && board.walls[i + dx][j + 1 + dy] == 0)
										{
											swap(board.units[i + dx][j + 1 + dy], board.units[i + dx][j + dy]);
											dy++;
										}
										else if (board.units[i + dx][j - 1 + dy] == 0 && board.walls[i + dx][j - 1 + dy] == 0)
										{
											swap(board.units[i + dx][j - 1 + dy], board.units[i + dx][j + dy]);
											dy--;
										}
									}
									else if ((j + dy) - hcy > 0)
									{
										if (board.units[i + dx][j - 1 + dy] == 0 && board.walls[i + dx][j - 1 + dy] == 0)
										{
											swap(board.units[i + dx][j - 1 + dy], board.units[i + dx][j + dy]);
											dy--;
										}
										else if (board.units[i + dx][j + 1 + dy] == 0 && board.walls[i + dx][j + 1 + dy] == 0)
										{
											swap(board.units[i + dx][j + 1 + dy], board.units[i + dx][j + dy]);
											dy++;
										}
									}
									else
									{
										switch (r(0, 1))
										{
										case 0:
										{
											if (board.units[i + dx][j + 1 + dy] == 0 && board.walls[i + dx][j + 1 + dy] == 0)
											{
												swap(board.units[i + dx][j + 1 + dy], board.units[i + dx][j + dy]);
												dy++;
											}
											else if (board.units[i + dx][j - 1 + dy] == 0 && board.walls[i + dx][j - 1 + dy] == 0)
											{
												swap(board.units[i + dx][j - 1 + dy], board.units[i + dx][j + dy]);
												dy--;
											}
										}
										case 1:
										{
											if (board.units[i + dx][j - 1 + dy] == 0 && board.walls[i + dx][j - 1 + dy] == 0)
											{
												swap(board.units[i + dx][j - 1 + dy], board.units[i + dx][j + dy]);
												dy--;
											}
											else if (board.units[i + dx][j + 1 + dy] == 0 && board.walls[i + dx][j + 1 + dy] == 0)
											{
												swap(board.units[i + dx][j + 1 + dy], board.units[i + dx][j + dy]);
												dy++;
											}
										}
										}
									}
								}
								else if ((i + dx) - hcx > 0) //je�eli jednosta jest na prawo od gracza
								{
									if (board.units[i - 1 + dx][j + dy] == 0 && board.walls[i - 1 + dx][j + dy] == 0)
									{
										swap(board.units[i - 1 + dx][j + dy], board.units[i + dx][j + dy]);
										dx++;
									}
									else if (board.units[i - 1 + dx][j + dy] == 2) //je�eli jednostka s�siaduje z bohaterem, atakuj
									{
										enemyattack(i, j, k, c);
									}
									else if ((j + dy) - hcy < 0) //je�eli droga na prz�d jest zablokowana, sprawd� czy jdnostka jest nad...
									{
										if (board.units[i + dx][j + 1 + dy] == 0 && board.walls[i + dx][j + 1 + dy] == 0) //czy ruch w d� jest moliwy
										{
											swap(board.units[i + dx][j + 1 + dy], board.units[i + dx][j + dy]);
											dy++;
										}
										else if (board.units[i + dx][j - 1 + dy] == 0 && board.walls[i + dx][j - 1 + dy] == 0) //czy r�ch w g�re jest mozliwy
										{
											swap(board.units[i + dx][j - 1 + dy], board.units[i + dx][j + dy]);
											dy--;
										}
									}
									else if ((j + dy) - hcy > 0) //...czy pod graczem
									{
										if (board.units[i + dx][j - 1 + dy] == 0 && board.walls[i + dx][j - 1 + dy] == 0) //czy ruch w g�re jest mo�liwy
										{
											swap(board.units[i + dx][j - 1 + dy], board.units[i + dx][j + dy]);
											dy--;
										}
										else if (board.units[i + dx][j + 1 + dy] == 0 && board.walls[i + dx][j + 1 + dy] == 0) //czy roch w d� jest mo�liwy
										{
											swap(board.units[i + dx][j + 1 + dy], board.units[i + dx][j + dy]);
											dy++;
										}
									}
									else //jezeli jednoska jest na wysokosci gracza, ale nie moze isc naprz�d, losowo wybiera kolejno�� ruch�w
									{
										switch (r(0, 1))
										{
										case 0:
										{
											if (board.units[i + dx][j + 1 + dy] == 0 && board.walls[i + dx][j + 1 + dy] == 0)
											{
												swap(board.units[i + dx][j + 1 + dy], board.units[i + dx][j + dy]);
												dy++;
											}
											else if (board.units[i + dx][j - 1 + dy] == 0 && board.walls[i + dx][j - 1 + dy] == 0)
											{
												swap(board.units[i + dx][j - 1 + dy], board.units[i + dx][j + dy]);
												dy--;
											}
										}
										case 1:
										{
											if (board.units[i + dx][j - 1 + dy] == 0 && board.walls[i + dx][j - 1 + dy] == 0)
											{
												swap(board.units[i + dx][j - 1 + dy], board.units[i + dx][j + dy]);
												dy--;
											}
											else if (board.units[i + dx][j + 1 + dy] == 0 && board.walls[i + dx][j + 1 + dy] == 0)
											{
												swap(board.units[i + dx][j + 1 + dy], board.units[i + dx][j + dy]);
												dy++;
											}
										}
										}
									}
								}
							}
							else if (abs(i - hcx) < abs(j - hcy)) //je�eli jedsnota jest dalej w pionie niz w poziomie
							{
								if ((j + dy) - hcy < 0) //jezeli jednostka jest nad graczem
								{
									if (board.units[i + dx][j + 1 + dy] == 0 && board.walls[i + dx][j + 1 + dy] == 0)
									{
										swap(board.units[i + dx][j + 1 + dy], board.units[i + dx][j + dy]);
										dy++;
									}
									else if (board.units[i + dx][j + 1 + dy] == 2)
									{
										enemyattack(i, j, k, c);
									}
									else if ((i + dx) - hcx < 0)
									{
										if (board.units[i + 1 + dx][j + dy] == 0 && board.walls[i + 1 + dx][j + dy] == 0)
										{
											swap(board.units[i + 1 + dx][j + dy], board.units[i + dx][j + dy]);
											dx++;
										}
										else if (board.units[i - 1 + dx][j + dy] == 0 && board.walls[i - 1 + dx][j + dy] == 0)
										{
											swap(board.units[i - 1 + dx][j + dy], board.units[i + dx][j + dy]);
											dx--;
										}
									}
									else if ((i + dx) - hcx > 0)
									{
										if (board.units[i - 1 + dx][j + dy] == 0 && board.walls[i - 1 + dx][j + dy] == 0)
										{
											swap(board.units[i - 1 + dx][j + dy], board.units[i + dx][j + dy]);
											dx--;
										}
										else if (board.units[i + 1 + dx][j + dy] == 0 && board.walls[i + 1 + dx][j + dy] == 0)
										{
											swap(board.units[i + 1 + dx][j + dy], board.units[i + dx][j + dy]);
											dx++;
										}
									}
									else
									{
										switch (r(0, 1))
										{
										case 0:
										{
											if (board.units[i + 1 + dx][j + dy] == 0 && board.walls[i + 1 + dx][j + dy] == 0)
											{
												swap(board.units[i + 1 + dx][j + dy], board.units[i + dx][j + dy]);
												dx++;
											}
											else if (board.units[i - 1 + dx][j + dy] == 0 && board.walls[i - 1 + dx][j + dy] == 0)
											{
												swap(board.units[i - 1 + dx][j + dy], board.units[i + dx][j + dy]);
												dx--;
											}
										}
										case 1:
										{
											if (board.units[i - 1 + dx][j + dy] == 0 && board.walls[i - 1 + dx][j + dy] == 0)
											{
												swap(board.units[i - 1 + dx][j + dy], board.units[i + dx][j + dy]);
												dx--;
											}
											else if (board.units[i + 1 + dx][j + dy] == 0 && board.walls[i + 1 + dx][j + dy] == 0)
											{
												swap(board.units[i + 1 + dx][j + dy], board.units[i + dx][j + dy]);
												dx++;
											}
										}
										}
									}
								}
								else if ((j + dy) - hcy > 0) //jezeli jedsnotka jest pod graczem
								{
									if (board.units[i + dx][j - 1 + dy] == 0 && board.walls[i + dx][j - 1 + dy] == 0)
									{
										swap(board.units[i + dx][j - 1 + dy], board.units[i + dx][j + dy]);
										dy++;
									}
									else if (board.units[i + dx][j - 1 + dy] == 2)
									{
										enemyattack(i, j, k, c);
									}
									else if ((i + dx) - hcx < 0)
									{
										if (board.units[i + 1 + dx][j + dy] == 0 && board.walls[i + 1 + dx][j + dy] == 0)
										{
											swap(board.units[i + 1 + dx][j + dy], board.units[i + dx][j + dy]);
											dx++;
										}
										else if (board.units[i - 1 + dx][j + dy] == 0 && board.walls[i - 1 + dx][j + dy] == 0)
										{
											swap(board.units[i - 1 + dx][j + dy], board.units[i + dx][j + dy]);
											dx--;
										}
									}
									else if ((i + dx) - hcx > 0)
									{
										if (board.units[i - 1 + dx][j + dy] == 0 && board.walls[i - 1 + dx][j + dy] == 0)
										{
											swap(board.units[i - 1 + dx][j + dy], board.units[i + dx][j + dy]);
											dx--;
										}
										else if (board.units[i + 1 + dx][j + dy] == 0 && board.walls[i + 1 + dx][j + dy] == 0)
										{
											swap(board.units[i + 1 + dx][j + dy], board.units[i + dx][j + dy]);
											dx++;
										}
									}
									else
									{
										switch (r(0, 1))
										{
										case 0:
										{
											if (board.units[i + 1 + dx][j + dy] == 0 && board.walls[i + 1 + dx][j + dy] == 0)
											{
												swap(board.units[i + 1 + dx][j + dy], board.units[i + dx][j + dy]);
												dx++;
											}
											else if (board.units[i - 1 + dx][j + dy] == 0 && board.walls[i - 1 + dx][j + dy] == 0)
											{
												swap(board.units[i - 1 + dx][j + dy], board.units[i + dx][j + dy]);
												dx--;
											}
										}
										case 1:
										{
											if (board.units[i - 1 + dx][j + dy] == 0 && board.walls[i - 1 + dx][j + dy] == 0)
											{
												swap(board.units[i - 1 + dx][j + dy], board.units[i + dx][j + dy]);
												dx--;
											}
											else if (board.units[i + 1 + dx][j + dy] == 0 && board.walls[i + 1 + dx][j + dy] == 0)
											{
												swap(board.units[i + 1 + dx][j + dy], board.units[i + dx][j + dy]);
												dx++;
											}
										}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
};
AI ai;




int main()
{
	
	/*auto image = sf::Image{};
	image.loadFromFile("test.png");

	window.setIcon(image.getSize().x, image.getSize().y, image.getPixelsPtr());*/

	char inp = ':)';
	char eqinp = ':)';
	walls.gen();
	minimap.mmap();
	units.gen();
	secretrooms();
	help = 0;
	// Start the game loop
	do
	{
		roomx = hrx * rs;
		roomy = hry * rs; //
		switch (menuhandler)
		{
		case 0: //wyb�r postaci
		{
			SetConsoleCursorPosition(handle, { 1, 7 });
			switch (inp)
			{
			case 75: case 'a': case 'A': //left
			{
				if (help > 0) help--;
			}break;
			case 77: case 'd': case 'D': //right
			{
				if (help < 3) help++;
			}break;
			case 13:
			{
				hero = help;
				change = 1;
				menuhandler = 1;
			}break;
			}
			if (inp != 13)
			{
				if (help == 0) std::cout << ">";
				else std::cout << " ";
				std::cout << "Fighter  ";
				if (help == 1) std::cout << ">";
				else std::cout << " ";
				std::cout << "Rogue  ";
				if (help == 2) std::cout << ">";
				else std::cout << " ";
				std::cout << "Mage  ";
				if (help == 3) std::cout << ">";
				else std::cout << " ";
				std::cout << "Cleric  ";
			}
			else //przypisanie statystyk postaci
			{
				switch (hero)
				{
				case 0:
				{
					p.assign_class_stats(20, 0, 3, 3, 0, 0);
				}break;
				case 1:
				{
					p.assign_class_stats(5, 10, 1, 1, 10, 3);
				}break;
				case 2:
				{
					p.assign_class_stats(0, 10, 0, 0, 30, 1);
				}break;
				case 3:
				{
					p.assign_class_stats(10, 5, 2, 2, 20, 1);
				}break;
				default: {}
				}
			}
			if (change == 0)
			{
				inp = _getch();
			}

		}break;
		case 1: //mapa
		{
			change = 0;
			if (p.movesleft > 0)
			{
				switch (inp)
				{
				case 75: case 'a': case 'A': //left
				{
					mapcontrol(0, -1);
				}break;
				case 77: case 'd': case 'D': //right
				{
					mapcontrol(0, 1);
				}break;
				case 72: case 'w': case 'W': //up
				{
					mapcontrol(-1, 0);
				}break;
				case 80: case 's': case 'S': //down
				{
					mapcontrol(1, 0);
				}break;
				case 'e': case 'E': //ekwipunek;
				{
					menuhandler = 2;
					inp = ':)';
					help = 0;
					change = 1;
				}break;
				case 'm': //zakl�cia
				{
					if (hero != 0) //jeżeli nie jest wojownikiem
					{
						inp = ' ';
						magic = 1;
						magicfirst = 0;
						spellborderprint();
						while (magic == 1) 
						{
							switch (inp)
							{
							case 72: case 'w': case 'W': {if (magicmenu > 0) magicmenu--; }break;
							case 80: case 's': case 'S': {if (magicmenu < known_spells) magicmenu++; }break;
							case 13:
							{
								inp = ' ';
								if (p.cmana >= z[hero - 1][magicmenu].manacost)
								{
									p.cmana -= z[hero - 1][magicmenu].manacost;
									if (z[hero - 1][magicmenu].whencast == 3) //instant cast
									{
										z[hero - 1][magicmenu].cast();
										magic = 0;
									}
									else if (z[hero - 1][magicmenu].whencast == 1) //wyb�r miejsca
									{
										magic = 2;
										spellx = hcx - roomx;
										spelly = hcy - roomy;
										while (magic == 2)
										{
											engine.reset_overlayboard();
											switch (inp)
											{
											case 75: case 'a': case 'A': //left
											{
												if (spelly >= 0) spelly--;

											}break;
											case 77: case 'd': case 'D': //right
											{
												if (spelly < 12) spelly++;
											}break;
											case 72: case 'w': case 'W': //up
											{
												if (spellx >= 0) spellx--;
											}break;
											case 80: case 's': case 'S': //down
											{
												if (spellx < 12) spellx++;
											}break;
											case 13: //zatwierdzenie
											{
												z[hero - 1][magicmenu].cast();
												magic = 0;
											}break;
											case 'R': case 'r': //obracanie (wyko�ystywanie w jedym zakl�ciu)
											{
												if (rotation < 8)rotation++;
												else rotation = 0;
											}break;
											case 'm': //anulowanie
											{
												p.cmana += z[hero - 1][magicmenu].manacost;
												magic = 0;
											}
											}
											pliska.display(hero, magicmenu);
											
											if (magic != 0)
											{
												inp = _getch();
											}
											else
											{
												floorprint();
											}
										}
									}
									else if (z[hero - 1][magicmenu].whencast == 2) //wyb�r przeciwnika
									{
										how_many_enemies = 0;
										magic = 2;
										magicchoice = 0;
										for (int i = rs - 1; i >= -1; i--) //funkcja zliczaj�ca przeciwnikow w pokoju i wybieraj�ca pierwszego z nich
										{
											for (int j = rs - 1; j >= -1; j--)
											{
												if (board.units[roomx + i][roomy + j] > 2)
												{
													how_many_enemies++;
													spellx = i;
													spelly = j;
												}
											}
										}
										if (how_many_enemies == 0)
										{
											magic = 0;
										}
										engine.overlayboard[spellx + 1][spelly + 1].set_sprite(17);
										overlayprint();
										while (magic == 2)
										{
											engine.reset_overlayboard();
											switch (inp)
											{
											case 75: case 'a': case 'A': {if (magicchoice == 0) { magicchoice = how_many_enemies - 1; }
												   else { magicchoice--; }
											}break;
											case 77: case 'd': case 'D': {if (magicchoice >= how_many_enemies - 1) { magicchoice = 0; }
												   else { magicchoice++; }
											}break;
											case 13:
											{
												z[hero - 1][magicmenu].cast();
												magic = 0;
											}break;
											case 'm':
											{
												p.cmana += z[hero - 1][magicmenu].manacost;
												magic = 0;
											}break;
											}
											if (magic != 0)
											{
												spellhelp = 0;
												for (int i = -1; i <= rs; i++) //funkcja pozwalaj�ca wybrac przeciwnika
												{
													for (int j = -1; j <= rs; j++)
													{
														if (board.units[roomx + i][roomy + j] > 2)
														{
															spellhelp++;
															if (spellhelp == magicchoice + 1)
															{
																spellx = i;
																spelly = j;
															}
														}
													}
												}
												engine.overlayboard[spellx + 1][spelly + 1].set_sprite(17);
												overlayprint();
												inp = _getch();
											}
											else
											{
												floorprint();
											}
										}
									}
									change = 0;
								}
								else
								{
									SetConsoleCursorPosition(handle, { 32, 11 });
									std::cout << "Brak many                         ";
								}
							}break;
							case 'M': case 'm':
							{
								magic = 0;
								inp = ' ';
							}break;
							}
							if (magicmenu < magicfirst) magicfirst--;
							else if (magicmenu > magicfirst + 2) magicfirst++;
							if (magic != 0)
							{
								z[hero - 1][magicmenu].printopis();
								SetConsoleCursorPosition(handle, { 4, 9 });
								if (magicmenu == magicfirst) { std::cout << ">"; }
								else { std::cout << " "; }
								std::cout << z[hero - 1][magicfirst].spellname;
								SetConsoleCursorPosition(handle, { 4, 11 });
								if (magicmenu == magicfirst + 1) { std::cout << ">"; }
								else { std::cout << " "; }
								std::cout << z[hero - 1][magicfirst + 1].spellname;
								SetConsoleCursorPosition(handle, { 4, 13 });
								if (magicmenu == magicfirst + 2) { std::cout << ">"; }
								else { std::cout << " "; }
								std::cout << z[hero - 1][magicfirst + 2].spellname; //wypisywanie list zaklec
							}
							if (change == 0)
							{
								inp = _getch();
							}
						}
					}
					else
					{
						SetConsoleCursorPosition(handle, { 32, 11 });
						std::cout << "Brak dostepnych zaklec";
					}
				}break;
				} //koniec zakl��
				if (hrx != chrx || hry != chry) //zmiana pokoju
				{
					chrx = hrx;
					chry = hry;
					nextroom.reset();
					if (stealth == true) stealth = false; //reset ukrycia �otra (ZAKL�CIA ZAKL�CIA ZAKL�CIA ZAKL�CIA ZAKL�CIA ZAKL�CIA ZAKL�CIA ZAKL�CIA )
					SetConsoleCursorPosition(handle, { (short)((rs + 2) * 2), 0 });
					SetConsoleTextAttribute(handle, 7);
					for (int i = 0; i < 10; i++) //wypisywanie minimapy
					{
						for (int j = 0; j < 10; j++)
						{
							if (i == hrx && j == hry)
							{
								SetConsoleTextAttribute(handle, 12);
								std::cout << minimap.map[i][j];
								SetConsoleTextAttribute(handle, 7);
							}
							else
							{
								std::cout << minimap.map[i][j];
							}
						}
						SetConsoleCursorPosition(handle, { (short)((rs + 2) * 2), (short)(i + 1) });
					}
				}
			}
			else
			{
				if (stealth == false)
				{
					ai.AII();
				}
				
				p.movesleft = p.spd;
				inp = ':)';
				nextturn.reset(); //reset statystyk
			}
			if (p.cexp >= p.lvlupexp) //zdobywanie poziomow
			{
				p.lvl++;
				p.cexp -= p.lvlupexp;
				p.lvlupexp *= 2;
				p.hp += 5;
				p.armor += 1;
				p.mr += 1;
				p.dmg += 2;
				p.mana += (p.mana > 0 ? 5 : 0);
			}

			if (change == 0)
			{
				hrx = hcx / rs; //hero room x = hero coordinate x / room size
				hry = hcy / rs;
				board.units[hcx][hcy] = 2;
				printstats();
				floorprint();						//printownie pod�ogi(co ruch)
				SetConsoleCursorPosition(handle, { 0, 16 });
				SetConsoleTextAttribute(handle, 7);
				SetConsoleCursorPosition(handle, { 0, 0 });
				if (p.movesleft > 0)
				{
					inp = _getch();
				}
			}
			if (respawn == 1 && p.chp <= 0) //zaklecie kapalana (ZAKL�CIA ZAKL�CIA ZAKL�CIA ZAKL�CIA ZAKL�CIA ZAKL�CIA ZAKL�CIA ZAKL�CIA )
			{
				respawn = 0; p.chp = 10;
			}
		}break;
		case 2: //ekwipunek
		{
			SetConsoleCursorPosition(handle, { 0, 0 });
			SetConsoleTextAttribute(handle, 7);
			if (change == 1) //ramka do ekwipunku
			{
				ramkaeq(); //moze do zmiany ????
			}
			change = 0;
			switch (inp)
			{
			case 75: case 'a': case 'A': //left
			{
				if (eq.x > 0) eq.x--;
			}break;
			case 77: case 'd': case 'D': //right
			{
				if (eq.x < 4) eq.x++;
			}break;
			case 72: case 'w': case 'W': //up
			{
				if (eq.y > 0) eq.y--;
			}break;
			case 80: case 's': case 'S': //down
			{
				if (eq.y < 3) eq.y++;
			}break;
			case 'e': case 'E': //mapa
			{
				inp = ':)';
				menuhandler = 1;
				change = 1;
				cout << setfill(' ');
				floorprint();
			}break;
			case 13: //akcje zwiazane z przedmiotami
			{
				if (help < 3 && eq.content[eq.x][eq.y] != 0)
				{
					while (inp != '0')
					{
						if (item[eq.content[eq.x][eq.y]].consumable == 1)
						{
							switch (eqinp)
							{
							case 72: case 'w': case 'W': {if (help > 0) help--; }break;
							case 80: case 's': case 'S': {if (help < 3) help++; }break;
							case 13:
							{
								switch (help)
								{
								case 0: //uzycie przedmiotu (DODA� RESET STATYSTYK, NIEKT�RE ITEMY MOG� MIEC DZIA�ANIE TYMCZASOWE)
								{
									p.chp = p.chp + item[eq.content[eq.x][eq.y]].hp > p.hp ? p.hp : p.chp + item[eq.content[eq.x][eq.y]].hp;
									p.armor += item[eq.content[eq.x][eq.y]].armor;
									p.mr += item[eq.content[eq.x][eq.y]].mr;
									p.cmana = p.cmana + item[eq.content[eq.x][eq.y]].mana > p.mana ? p.mana : p.cmana + item[eq.content[eq.x][eq.y]].mana;
									p.spd += item[eq.content[eq.x][eq.y]].spd;
									p.dmg += item[eq.content[eq.x][eq.y]].dmg;
									eq.content[eq.x][eq.y] = 0;
									inp = '0';
								}break;
								case 1:	//przenoszenie
								{
									eq.choicey = eq.y;
									eq.choicex = eq.x;
									help = 3;
									inp = '0';
								} break;
								case 2: //wyrzucanie
								{
									dropitem(hcx, hcy, eq.content[eq.x][eq.y]);
									eq.content[eq.x][eq.y] = 0;
									inp = '0';
								} break;
								case 3: //anulowanie 								
								{
									inp = '0';
								} break;
								}
								SetConsoleCursorPosition(handle, { 33, 0 });
								std::cout << "                      ";
								SetConsoleCursorPosition(handle, { 33, 1 });
								std::cout << "                      ";
								SetConsoleCursorPosition(handle, { 33, 2 });
								std::cout << "                      ";
								SetConsoleCursorPosition(handle, { 33, 3 });
								std::cout << "                      ";
								SetConsoleCursorPosition(handle, { 33, 4 });
								std::cout << "							";
							}break;
							}
						}
						else
						{
							switch (eqinp)
							{
							case 72: case 'w': case 'W': {if (help > 0) help--; }break;
							case 80: case 's': case 'S': {if (help < 2) help++; }break;
							case 13:
							{
								switch (help)
								{
								case 0:	//przenoszenie
								{
									eq.choicey = eq.y;
									eq.choicex = eq.x;
									help = 3;
									inp = '0';
								} break;
								case 1: //wyrzucanie
								{
									dropitem(hcx, hcy, eq.content[eq.x][eq.y]);
									eq.content[eq.x][eq.y] = 0;
									inp = '0';
								} break;
								case 2: //anulowanie 								
								{
									inp = '0';
								} break;
								}
								SetConsoleCursorPosition(handle, { 33, 0 });
								std::cout << "                      ";
								SetConsoleCursorPosition(handle, { 33, 1 });
								std::cout << "                      ";
								SetConsoleCursorPosition(handle, { 33, 2 });
								std::cout << "                      ";
								SetConsoleCursorPosition(handle, { 33, 3 });
								std::cout << "                      ";
								SetConsoleCursorPosition(handle, { 33, 4 });
								std::cout << "							";
							}break;
							}
						}
						if (eqinp != 13 && help != 3) //printowanie wyboru akcji przedmiotu (przenies, wyrzuc, uzyj, anuluj)
						{
							short int a = 0;
							if (item[eq.content[eq.x][eq.y]].consumable == 1) 
							{
								SetConsoleCursorPosition(handle, { 33, a });
								if (help == a) { std::cout << ">"; }
								else { std::cout << " "; }
								std::cout << "Uzyj"; a++;
							}
							SetConsoleCursorPosition(handle, { 33, a });
							if (help == a) { std::cout << ">"; }
							else { std::cout << " "; }
							std::cout << "Przenies"; a++;
							SetConsoleCursorPosition(handle, { 33, a });
							if (help == a) { std::cout << ">"; }
							else { std::cout << " "; }
							std::cout << "Wyrzuc"; a++;
							SetConsoleCursorPosition(handle, { 33, a });
							if (help == a) { std::cout << ">"; }
							else { std::cout << " "; }
							std::cout << "Anuluj"; a++;
						}
						eqinp = _getch();
					}
				}
				else if (help == 3)
				{
					if (eq.x == 0)
					{
						if (item[eq.content[eq.choicex][eq.choicey]].eqslot == eq.y + 1) //zak�adanie przedmiot�w
						{
							p.hp = p.hp - item[eq.content[eq.x][eq.y]].hp + item[eq.content[eq.choicex][eq.choicey]].hp;
							p.armor = p.armor - item[eq.content[eq.x][eq.y]].armor + item[eq.content[eq.choicex][eq.choicey]].armor;
							p.mr = p.mr - item[eq.content[eq.x][eq.y]].mr + item[eq.content[eq.choicex][eq.choicey]].mr;
							p.mana = p.mana - item[eq.content[eq.x][eq.y]].mana + item[eq.content[eq.choicex][eq.choicey]].mana;
							p.spd = p.spd - item[eq.content[eq.x][eq.y]].spd + item[eq.content[eq.choicex][eq.choicey]].spd;
							p.dmg = p.dmg - item[eq.content[eq.x][eq.y]].dmg + item[eq.content[eq.choicex][eq.choicey]].dmg;
							swap(eq.content[eq.x][eq.y], eq.content[eq.choicex][eq.choicey]);
						}
						else //pr�ba za�o�enie przedmiotu na z�e miejsce
						{
							SetConsoleCursorPosition(handle, { 33, 4 });
							std::cout << "You cant wear that there!";
							eq.x = eq.choicex; eq.y = eq.choicey;
						}
					}
					else
					{
						swap(eq.content[eq.x][eq.y], eq.content[eq.choicex][eq.choicey]);
						help = 0;
					}
					help = 0;
					eq.choicex = 10;
					eq.choicey = 10;
				}
				else
				{
					help = 0;
				}
			}break;
			}
			if (change == 0) //wypisywanie ekwipunku
			{
				for (int i = 0; i < 5; i++)
				{
					for (int j = 0; j < 4; j++)
					{
						if (i == eq.x && j == eq.y) SetConsoleTextAttribute(handle, 12); //kursor ekwipunku
						else if (i == 0 && j == item[eq.content[eq.x][eq.y]].eqslot - 1 && help != 3)  SetConsoleTextAttribute(handle, 9); //podswietlanie miejsca w kt�rym mo�na za�ozyc przedmiot pod kursorem
						else if (i == 0 && j == item[eq.content[eq.choicex][eq.choicey]].eqslot - 1)  SetConsoleTextAttribute(handle, 9); //podswietlanie miejsca w ktorym mozna zalozyc przenoszony przedmiot 
						else if (i == eq.choicex && j == eq.choicey) SetConsoleTextAttribute(handle, 13); //podswietlanie wybranego przedmiotu
						else SetConsoleTextAttribute(handle, 7); //normalny kolor
						SetConsoleCursorPosition(handle, { (short)(i * 6 + 1), (short)(j * 4 + 2) });
						std::cout << item[eq.content[i][j]].sprite_no;
						if ((i == eq.x && j == eq.y) || (i == eq.choicex && j == eq.choicey)) SetConsoleTextAttribute(handle, 7);
					}
				}
			}
			item[eq.content[eq.x][eq.y]].printopis();
			if (change == 0)
			{
				inp = _getch();
			}

		}break;
		}
	} while (inp != 27 && p.chp > 0);
	SetConsoleCursorPosition(handle, { 14, 28 }); //animacja �mierci i zako�czenie gry
	SetConsoleTextAttribute(handle, 240);
	deathanim(0, 0);
	deathanim(2, 6);
	deathanim(4, 12);
	deathanim(6, 18);
	deathanim(8, 24);
	deathanim(10, 27);
	deathanim(12, 30);
	SetConsoleCursorPosition(handle, { 9, 6 });
	SetConsoleTextAttribute(handle, 252);
	std::cout << "GAME OVER";
	SetConsoleTextAttribute(handle, 0);
	SetConsoleCursorPosition(handle, { 0, 20 });
	//std::system("pause");

}



//dodatkowe pi�tra, z�oto
