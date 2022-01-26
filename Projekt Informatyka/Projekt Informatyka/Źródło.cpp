#include <iostream>
#include <ctime>
#include <iomanip>
#include <random>
#include <windows.h>
#include <conio.h>
#include <algorithm>
#include <string>

using namespace std;

HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);

const short int rs = 13; //room size

int hhp, hdmg, harmor, hmr, mana, hspd, movesleft, lvlupexp = 300;
int chp = 1, cmana, cexp = 0, lvl = 1, pietro = 1;
int bhp = 20, bdmg = 5, barmor = 5, bmr = 5, bmana = 0, bspd = 2, branddmg = 0; //base stats, for balancing purposes

int hcx = 123, hcy = 5, hrx = 0, hry = 10, chrx = hrx, chry = hry; //hero coordiante x, y , hero room x, y, changed hero room x, y

int aiused[5] = {}, nr; //AI
int aix = 0, aiy = 0, aic;
int behp = 8, bedmg = 10, bespd = 2; //podstawowe statystyki przeciwników

int		board[130][130] = {}; //plansza
int		items_on_floor[130][130] = {};
int		units[130][130] = {}; //rozstawienie jednostek na planszy

int		minimaptech[10][10] = {}; //minimapa2
char		minimap[10][10]; //minimapa


int			eq[5][4] = { {0, 0, 0, 0 }, {0, 1, 3, 2}, {3, 0, 0, 0}, {1, 0 , 0, 1}, {0, 0, 0, 0} }; //ekwipunek
int			help = 0, hero = 4, eqx = 0, eqy = 0, eqcx = 10, eqcy = 10; //wybór postaci i obs³uga ekwipunku
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

struct enemy //struktura przeciwników
{
	bool type;
	int hp;
	int dmg;
	int spd;
	int item;
	int exp;
	short color;
	string letter;
	string fullname;
	bool stunned;
};
enemy en[500] = {};
struct Item
{
	int hp;
	int dmg;
	int armor;
	int mr;
	int mana;
	int spd;
	bool consumable;
	string disp;
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
		hhp -= hp; hp = 0;
		if (chp > hhp) chp = hhp;
		harmor -= armor; armor = 0;
		hmr -= mr; mr = 0;
		mana -= hmana; hmana = 0;
		if (cmana > mana) cmana = mana;
		hspd -= spd; spd = 0;
		hdmg -= dmg; dmg = 0;
	}
};
tempbuff nextturn = { 0, 0, 0, 0, 0, 0, 0 };
tempbuff nextattack = { 0, 0, 0, 0, 0, 0, 0 };
tempbuff nextroom = { 0, 0, 0, 0, 0, 0, 0 };

Item none{ 0, 0, 0, 0, 0, 0, 0, "[]", 0, 1, };
Item hpdrop{ 15, 0, 0, 0, 0, 0, 1, "<3", 0, 1, };
Item chainmail{ 0, 0, 5, 0, 0, 0, 0,"Cm", 2, 1, "Kolczuga.Zalozenie jej zwieksza pancerz o 5" };
Item hpamulet{ 10, 0, 0, 0, 0, 0, 0, "Ha", 4, 1, "Amulet zdrowia.Zalozenie go zwieksza zdrowie o 10" };
Item items[4] = { none, hpdrop, hpamulet, chainmail };
void dropitem(int a, int b, int d)
{
	int c = 0;
	while (d > 0)
	{
		for (int i = -c; i <= c; i++)
		{
			for (int j = -c; j <= c; j++)
			{
				if (items_on_floor[a + i][b + j] == 0 && board[a + i][b + j] == 0)
				{
					items_on_floor[a + i][b + j] = d;
					d = 0;
				}
			}
		}
		c++;
	}
}


int magicmenu, known_spells = 4, magic = 0, magicfirst;
int magicchoice, how_many_enemies, spellx = 0, spelly = 0, spellhelp, rotation;
bool respawn = 0, stealth = 0;

void kulaognia() { //zakelcia maga
	for (int i = -1; i <= 1; i++)
	{
		for (int j = -1; j <= 1; j++)
		{
			if (units[spellx + i + hrx * rs][spelly + j + hry * rs] == 2)
			{
				chp -= 20;
			}
			else if (en[units[spellx + i + hrx * rs][spelly + j + hry * rs]].hp - 20 <= 0)
			{
				dropitem(hrx * rs + spellx + i, hry * rs + spelly + j, en[units[hrx * rs + spellx + i][hry * rs + spelly + j]].item);
				cexp += en[units[hrx * rs + spellx + i][hry * rs + spelly + j]].exp;
				units[hrx * rs + spellx + i][hry * rs + spelly + j] = 0;
			}
			else if (en[units[spellx + i + hrx * rs][spelly + j + hry * rs]].hp - 20 > 0)
			{
				en[units[spellx + i + hrx * rs][spelly + j + hry * rs]].hp -= 20;
			}
		}
	}
}
void pancerzmaga() {
	nextroom.armor += 5;
	nextroom.mr += 5;
	harmor += 5;
	hmr += 5;
}
void wallhelp(int a, int b)
{
	board[spellx + a + hrx * rs][spelly + b + hry * rs] = 1;
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
	if (en[units[hrx * rs + spellx][hry * rs + spelly]].hp - 2 <= 0)
	{
		dropitem(hrx * rs + spellx, hry * rs + spelly, en[units[hrx * rs + spellx][hry * rs + spelly]].item);
		cexp += en[units[hrx * rs + spellx][hry * rs + spelly]].exp;
		units[hrx * rs + spellx][hry * rs + spelly] = 0;
	}
	else
	{
		en[units[hrx * rs + spellx][hry * rs + spelly]].hp -= 2;
		en[units[hrx * rs + spellx][hry * rs + spelly]].stunned = TRUE;
	}
}
void Teleportacja() {
	if (board[spellx + hrx * rs][spelly + hry * rs] == 1)
	{
		swap(units[spellx + hrx * rs][spelly + hry * rs], units[hcx][hcy]);
		chp = 0;
	}
	else
	{
		swap(units[spellx + hrx * rs][spelly + hry * rs], units[hcx][hcy]);
		hcx = spellx + hrx * rs;
		hcy = spelly + hry * rs;
	}
}

void uleczrane() { //zaklecia kalpana
	chp = chp + 20 > hhp ? hhp : chp + 20;
}
void rozblyskcienia() {

	for (int i = -1; i <= 1; i++)
	{
		for (int j = -1; j <= 1; j++)
		{
			if (units[hcx + i][hcy + j] > 2)
			{
				if (en[units[hcx + i][hcy + j]].hp - 5 <= 0)
				{
					dropitem(hcx + i, hcy + j, en[units[hcx + i][hcy + j]].item);
					cexp += en[units[hcx + i][hcy + j]].exp;
					units[hcx + i][hcy + j] = 0;
					hhp++;
				}
				else
				{
					en[units[hcx + i][hcy + j]].hp -= 5;
				}
				chp = chp + 5 > hhp ? hhp : chp + 5;
			}
		}
	}
}
void wybuchswiatla() {
	for (int i = -1; i <= 1; i++)
	{
		for (int j = -1; j <= 1; j++)
		{
			if (units[hcx + i][hcy + j] > 2)
			{
				if (board[hcx + 2 * i][hcy + 2 * j] == 1 || units[hcx + 2 * i][hcy + 2 * j] != 0)
				{
					if (en[units[hcx + i][hcy + j]].hp - 10 <= 0)
					{
						dropitem(hcx + i, hcy + j, en[units[hcx + i][hcy + j]].item);
						cexp += en[units[hcx + i][hcy + j]].exp;
						units[hcx + i][hcy + j] = 0;
					}
					else
					{
						en[units[hcx + i][hcy + j]].hp -= 10;
						en[units[hcx + i][hcy + j]].stunned = TRUE;
					}
				}
				else if (units[hcx + 2 * i][hcy + 2 * j] == 0)
				{
					en[units[hcx + i][hcy + j]].stunned = TRUE;
					swap(units[hcx + i][hcy + j], units[hcx + 2 * i][hcy + 2 * j]);
				}
			}
		}
	}

}
void Odrodzenie() {
	respawn = 1;
}
void boskiwyrok() {
	dropitem(hrx * rs + spellx, hry * rs + spelly, en[units[hrx * rs + spellx][hry * rs + spelly]].item);
	cexp += en[units[hrx * rs + spellx][hry * rs + spelly]].exp;
	units[hrx * rs + spellx][hry * rs + spelly] = 0;
}

void cioswplecy() { //zaklecia ³otra
	nextattack.dmg += 10;
	hdmg += 10;
}
void Sprint() {
	nextturn.spd += 2;
	movesleft += 2;
	hspd += 2;
}
void Ukrycie() {
	stealth = 1;
}
void rzutnozem() {
	if (en[units[hrx * rs + spellx][hry * rs + spelly]].hp - hdmg <= 0)
	{
		dropitem(hrx * rs + spellx, hry * rs + spelly, en[units[hrx * rs + spellx][hry * rs + spelly]].item);
		cexp += en[units[hrx * rs + spellx][hry * rs + spelly]].exp;
		units[hrx * rs + spellx][hry * rs + spelly] = 0;
	}
	else
	{
		en[units[hrx * rs + spellx][hry * rs + spelly]].hp -= hdmg;
	}
	nextattack.reset();
}
void Improwizacja() {
	switch (r(0, 9))
	{
	case 0: {spellx = r(hrx * rs, hrx * rs + 12); spelly = r(hrx * rs, hrx * rs + 12); kulaognia(); }break;
	case 1: {pancerzmaga(); }break;
	case 2: {spellx = r(hrx * rs, hrx * rs + 12); spelly = r(hrx * rs, hrx * rs + 12); rotation = r(0, 7); kamiennasciana(); }break;
	case 3: {while (units[spellx][spelly] < 2) { spellx = r(hrx * rs, hrx * rs + 12); spelly = r(hrx * rs, hrx * rs + 12); }pocisklodu(); }break;
	case 4: {spellx = r(hrx * rs, hrx * rs + 12); spelly = r(hrx * rs, hrx * rs + 12); Teleportacja(); }break;
	case 5: {uleczrane(); }break;
	case 6: {rozblyskcienia(); }break;
	case 7: {wybuchswiatla(); }break;
	case 8: {Odrodzenie(); }break;
	case 9: {while (units[spellx][spelly] < 2) { spellx = r(hrx * rs, hrx * rs + 12); spelly = r(hrx * rs, hrx * rs + 12); }boskiwyrok(); }break;
	}
}

void spelldisplay(int a, int b)
{
	if (units[spellx + a + hrx * rs][spelly + b + hry * rs] > 2)
	{
		SetConsoleCursorPosition(handle, { (short)((spelly + 1 + b) * 2), (short)(spellx + 1 + a) });
		SetConsoleTextAttribute(handle, 92);
		std::cout << setw(2) << "E";
	}
	else if (units[spellx + a + hrx * rs][spelly + b + hry * rs] == 2 || units[spellx + hrx * rs + a][spelly + hry * rs + b] == 1)
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
}

struct Zaklecie
{
	string spellname;
	int whencast;
	int manacost;
	string opis;
	void printopis()
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
						spelldisplay(i, j);
					}
				}
			}break;
			case 2:
			{
				switch (rotation)
				{
				case 0: case 4:
				{
					spelldisplay(0, -1);
					spelldisplay(0, 0);
					spelldisplay(0, 1);

				}break;
				case 1: case 5:
				{
					spelldisplay(-1, 1);
					spelldisplay(0, 0);
					spelldisplay(1, -1);

				}break;
				case 2: case 6:
				{
					spelldisplay(1, 0);
					spelldisplay(0, 0);
					spelldisplay(-1, 0);

				}break;
				case 3: case 7:
				{
					spelldisplay(1, 1);
					spelldisplay(0, 0);
					spelldisplay(-1, -1);

				}break;
				}
			}break;
			case 4:
			{
				spelldisplay(0, 0);
			}break;
			}
		}
		}
	}
	void spell(int a, int b)
	{
		switch (a)
		{
		case 1:
		{
			switch (b)
			{
			case 0: {cioswplecy(); }break;
			case 1: {Sprint(); }break;
			case 2: {Ukrycie(); }break;
			case 3: {rzutnozem(); }break;
			case 4: {Improwizacja(); }break;
			}
		}break;
		case 2:
		{
			switch (b)
			{
			case 0: {kulaognia(); }break;
			case 1: {pancerzmaga(); }break;
			case 2: {kamiennasciana(); }break;
			case 3: {pocisklodu(); }break;
			case 4: {Teleportacja(); }break;
			}
		}break;
		case 3:
		{
			switch (b)
			{
			case 0: {uleczrane(); }break;
			case 1: {rozblyskcienia(); }break;
			case 2: {wybuchswiatla(); }break;
			case 3: {Odrodzenie(); }break;
			case 4: {boskiwyrok(); }break;
			}
		}break;
		}
	}
};
Zaklecie cast{ "", 0 };

Zaklecie cios_w_plecy{ "Cios w plecy       ", 3, 2, "Zwieksza obrazenia nastepnego ataku lub rzutu nozem" }; //3 - natychmiastowe, 2 - namierzane na cel, 1 namierzane na podloge
Zaklecie sprint{ "Sprint             " , 3, 4, "Zwieksza predkoc ruchu o 2 do nastepnego pokoju" };
Zaklecie ukrycie{ "Ukrycie          ", 3, 0, "Przeciwnicy nie widza cie do nastepnego pokoju" };
Zaklecie rzut_nozem{ "Rzut nozem           ", 2, 4, "Zadaje obrazenia postaci wybranemu przeciwnikowi w pokoju" };
Zaklecie improwizacja{ "Improwizacja          ", 3, 10, "Rzuca losowe zaklecie z innej klasy" };

Zaklecie kula_ognia{ "Kula ognia        ", 1, 15, "Zadaje duze obrazenia w polu 3x3" };
Zaklecie pancerz_maga{ "Pancerz maga        ", 3, 5, "Zwieksza pancerz i odpornosc na magie do nastepnego pokoju" };
Zaklecie kamienna_sciana{ "Kamienna sciana    ", 1, 10, "Tworzy sciane w wybranym miejscu" };
Zaklecie pocisk_lodu{ "Pocisk lodu        ", 2, 5, "Zadaje obazenia na odlegolosc i zamraza przeciwnika na nastepny ruch" };
Zaklecie teleportacja{ "Teleportacja        ", 1, 10, "Teleportuje postac na wybrane pole" };

Zaklecie ulecz_rane{ "Ulecz Rane       ", 3, 5, "Przywraca zdrowie" };
Zaklecie rozblysk_cienia{ "Rozblysk cienia     ", 3, 10, "Zadaje obrazenia wszystkim wrogom dookola postaci i zwieksza maksymalne zdrowie o 1 za kazdego zbitego tym zakleciem wroga" };
Zaklecie wybuch_swiatla{ "Wybuch swiatla        " , 3, 10, "Odrzuca wszystkich wrogow dookola bohatera i oglusza ich na nastepna ture" };
Zaklecie odrodzenie{ "Odrodzenie          ", 3, 20, "Zapobiega nastepnemu smiertelnemu ciosowi przeciwnika" };
Zaklecie boski_wyrok{ "Boski Wyrok         ", 2, 20, "Zabija wybranego wroga" };
Zaklecie z[4][5] = { {}, {cios_w_plecy, sprint, ukrycie, rzut_nozem, improwizacja}, {kula_ognia, pancerz_maga, kamienna_sciana, pocisk_lodu, teleportacja}, {ulecz_rane, rozblysk_cienia, wybuch_swiatla, odrodzenie , boski_wyrok} };


int w[rs] = { 4, 4, 4, 4, 0, 0, 0, 0, 4, 4, 4, 4, 4 }; //przejœcie miêdzy pokojami
int p0[12][12] = { //pokój 1 (3-1 - œciana, 0 - pod³oga)
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
int p1[12][12] = { // pokój 2
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

void roomgen(int a, int b, int c[12][12]) //uzupe³nianie segmentu planszy pokojem
{
	for (int i = 0; i < 12; i++)
	{
		for (int j = 0; j < 12; j++)
		{
			board[a * rs + i][b * rs + j] = c[i][j];
		}
	}
}
void wallgenv(int a, int b) //generacja poziomyh przejœæ miêdzy pokojami
{
	if (b > 0)
	{
		switch (r(0, 3))
		{
		case 0: case 1: case 2:
		{
			for (int i = 0; i < 12; i++)
			{
				board[a * rs + i][b * rs - 1] = w[i];
			}
		}break;
		case 3:
		{
			for (int i = 0; i < 12; i++)
			{
				for (int j = -2; j < 1; j++)
				{
					board[a * rs + i][b * rs + j] = 4;
				}
			}
		}break;
		}
	}
}
void wallgenh(int a, int b) //generacja pionowych przejœæ imêdzy pokojami
{
	if (a > 0)
	{
		switch (r(0, 3))
		{
		case 0: case 1: case 2:
		{
			for (int i = 0; i < rs; i++)
			{
				board[a * rs - 1][b * rs + i] = w[i];
			}

		}break;
		case 3:
		{
			for (int i = 0; i < rs; i++)
			{
				for (int j = -2; j < 1; j++)
				{
					board[a * rs + j][b * rs + i] = 4;
				}
			}
		}break;
		}
	}

}

void boardgen()  //generowanie planszy
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
	for (int i = 0; i < 129; i++) //zewnetrzne œciany
	{
		board[0][i] = 5;
		board[128][i] = 5;
		board[129][i] = 5;
		board[i][0] = 5;
		board[i][128] = 5;
		board[i][129] = 5;
	}
	board[7][123] = 10;
}

void mmap() //generowanie minimapy
{
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			minimaptech[i][j] = ((board[i * rs][j * rs + 5]) == 0 ? 0 : 1) + ((board[i * rs + 5][(j * rs) + 11] == 0 ? 0 : 1) * 2) + ((board[(i * rs) + 11][j * rs + 5] == 0 ? 0 : 1) * 4) + ((board[i * rs + 5][j * rs] == 0 ? 0 : 1) * 8);
			switch (minimaptech[i][j])
			{
			case 15: {minimap[i][j] = ' '; }break;
			case 14: {minimap[i][j] = 0xC1; }break;
			case 13: {minimap[i][j] = 0xC3; }break;
			case 12: {minimap[i][j] = 0xC8; }break;
			case 11: {minimap[i][j] = 0xC2; }break;
			case 10: {minimap[i][j] = 0xBA; }break;
			case 9: {minimap[i][j] = 0xC9; }break;
			case 8: {minimap[i][j] = 0xCC; }break;
			case 7: {minimap[i][j] = 0xB4; }break;
			case 6: {minimap[i][j] = 0xBC; }break;
			case 5: {minimap[i][j] = 0xCD; }break;
			case 4: {minimap[i][j] = 0xCA; }break;
			case 3: {minimap[i][j] = 0xBB; }break;
			case 2: {minimap[i][j] = 0xB9; }break;
			case 1: {minimap[i][j] = 0xCB; }break;
			case 0: {minimap[i][j] = 0xCE; }
			}
		}
	}
}
void loottable(int a, int b) //tabele przedmoitów przeciwników
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
void enemygen() //generowanie przeciwnikow
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
					while (units[i * rs + rx][j * rs + ry] != 0 || board[i * rs + rx][j * rs + ry] != 0)
					{
						rx = r(1, 10);
						ry = r(1, 10);
					}
					ru = r(3, 6);
					units[i * rs + rx][j * rs + ry] = a;

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
						en[a].fullname = "Goblin";
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
						en[a].fullname = "Spider";
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
						en[a].fullname = "Fiend";
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
						en[a].fullname = "Golem";
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

void secretrooms()
{
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			if (minimap[i][j] == ' ') //pokój bez wejœæ
			{
				for (int k = i * rs; k < i * rs + rs; k++)
				{
					for (int l = j * rs; l < j * rs + rs; l++)
					{
						units[k][l] = 0;
					}
				}
				loottable(1, 0);
				items_on_floor[i * rs + 5][j * rs + 5] = en[0].item;
				loottable(1, 0);
				items_on_floor[i * rs + 4][j * rs + 6] = en[0].item;
				loottable(1, 0);
				items_on_floor[i * rs + 5][j * rs + 4] = en[0].item;
			}
		}
	}
}

void wyborpostaci() //wybór klasy postaci na pocz¹tku gry
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

void wallprint() //wypisywanie œcian
{
	SetConsoleTextAttribute(handle, 112);
	for (int i = -1; i < rs; i++)
	{
		SetConsoleCursorPosition(handle, { 0, (short)(i + 1) });
		for (int j = -1; j < rs; j++)
		{
			if ((hrx * rs + i) < 0 || (hry * rs + j) < 0 || (hrx * rs + i) > 128 || (hry * rs + j) > 128) //poprawne printowanie przypadków skrajnych
			{
				SetConsoleCursorPosition(handle, { (short)(2 * (j + 1)), (short)(i + 1) });
				SetConsoleTextAttribute(handle, 112);
				std::cout << setw(2) << " ";
			}
			else
			{
				SetConsoleCursorPosition(handle, { (short)(2 * (j + 1)), (short)(i + 1) });
				switch (board[hrx * rs + i][hry * rs + j])
				{
				case 1:
				{
					SetConsoleTextAttribute(handle, 15);
					std::cout << (char)0xB0 << (char)0xB0;
				}break;
				case 2:
				{
					SetConsoleTextAttribute(handle, 15);
					std::cout << (char)0xB1 << (char)0xB1;
				}break;
				case 3:
				{
					SetConsoleTextAttribute(handle, 15);
					std::cout << (char)0xB2 << (char)0xB2;
				}break;
				case 4: case 5:
				{
					SetConsoleTextAttribute(handle, 112);
					std::cout << setw(2) << " ";
				}break;
				case 10:
				{
					SetConsoleTextAttribute(handle, 124);
					std::cout << setw(2) << (char)(0xBD);
				}break;
				}
			}
		}
		SetConsoleCursorPosition(handle, { 28, (short)(i + 1) });
		SetConsoleTextAttribute(handle, 0);
		std::cout << "  ";
	}
}
void floorprint() //wypisywanie pod³ogi i przeciwników
{
	for (int i = -1; i < rs; i++)
	{
		SetConsoleCursorPosition(handle, { 0, (short)(i + 1) });
		for (int j = -1; j < rs; j++)
		{
			if (units[hrx * rs + i][hry * rs + j] != 0)
			{
				SetConsoleCursorPosition(handle, { (short)(2 * (j + 1)), (short)(i + 1) });
				if (units[hrx * rs + i][hry * rs + j] == 2)
				{
					SetConsoleTextAttribute(handle, 7);
					std::cout << setw(2) << "H";
				}
				else if (i >= 0 && j >= 0)
				{
					SetConsoleTextAttribute(handle, en[units[hrx * rs + i][hry * rs + j]].color);
					std::cout << setw(2) << en[units[hrx * rs + i][hry * rs + j]].letter;

				}
			}
			else if (items_on_floor[hrx * rs + i][hry * rs + j] != 0)
			{
				SetConsoleCursorPosition(handle, { (short)(2 * (j + 1)), (short)(i + 1) });
				SetConsoleTextAttribute(handle, 12);
				std::cout << setw(2) << items[items_on_floor[hrx * rs + i][hry * rs + j]].disp;
			}
			else if (board[hrx * rs + i][hry * rs + j] == 0)  //printowanie pod³ogi
			{
				SetConsoleCursorPosition(handle, { (short)(2 * (j + 1)), (short)(i + 1) });
				SetConsoleTextAttribute(handle, 0);
				std::cout << setw(2) << " ";
			}
		}
		SetConsoleCursorPosition(handle, { 28, (short)(i + 1) });
		SetConsoleTextAttribute(handle, 0);
		std::cout << "  ";
	}
}

void mapcontrol(int a, int b) //poruszanie sie postaci, zadawanie obra¿eñ, przedmioty, zmiana piêtra
{
	if (board[hcx + a][hcy + b] == 0 && units[hcx + a][hcy + b] == 0) { units[hcx][hcy] = 0; hcy += b; hcx += a; movesleft--; }
	else if (units[hcx + a][hcy + b] != 0)
	{
		en[units[hcx + a][hcy + b]].hp -= hdmg;
		nextattack.reset();
		if (stealth == 1) stealth = 0;
		if (en[units[hcx + a][hcy + b]].hp <= 0)  //przeciwnik ginie
		{
			dropitem(hcx + a, hcy + b, en[units[hcx + a][hcy + b]].item);
			cexp += en[units[hcx + a][hcy + b]].exp; //zdobywanie doswiadczenia za zabijanie przeciwnikow
			units[hcx + a][hcy + b] = 0;
		}
		movesleft--;
	}
	else if (board[hcx + a][hcy + b] > 0 && board[hcx + a][hcy + b] < 5)
	{
		board[hcx + a][hcy + b]--; movesleft--;
		wallprint();
	}
	else if (board[hcx + a][hcy + b] == 10) //zmiana piêtra
	{
		behp = behp * 2;
		bedmg = bedmg * 2;
		bespd++; //zwiêkszenie statystyk przeciwników, poziomu trudnoœci
		swap(units[hcx][hcy], units[123][5]); //reset coordów postaci
		hcx = 123;
		hcy = 5;
		for (int i = 0; i < 130; i++) //czyszczenie tabel
		{
			for (int j = 0; j < 130; j++)
			{
				if (units[i][j] > 2)
					units[i][j] = 0;
				board[i][j] = 0;
				items_on_floor[i][j] = 0;
			}
		}
		boardgen();
		mmap();
		enemygen();
		secretrooms();
		wallprint();
		floorprint();
		pietro++;
	}
	if (items_on_floor[hcx][hcy] > 0) //podnoszenie przedmiotów
	{
		if (items_on_floor[hcx][hcy] == 1) //consumable ZMIENIÆ NA UNIWESALNE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1 
		{
			chp = chp + items[items_on_floor[hcx][hcy]].hp > hhp ? hhp : chp + items[items_on_floor[hcx][hcy]].hp;
			items_on_floor[hcx][hcy] = 0;
		}
		else
		{
			for (int i = 0; i < 4; i++) //umieszczanie przedmiotów w ekwipunku
			{
				for (int j = 1; j < 5; j++)
				{
					if (eq[j][i] == 0)
					{
						eq[j][i] = items_on_floor[hcx][hcy];
						items_on_floor[hcx][hcy] = 0;
					}
				}
			}
		}
	}
}

void ramkaeq() //funkcja wypisuj¹ca ramkê ekwipunku
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

void printstats()
{
	for (short i = 0; i < 7; i++)
	{
		SetConsoleCursorPosition(handle, { (short)(rs * 2 + 15), i });
		std::cout << setw(23) << " " << endl;
	}
	SetConsoleCursorPosition(handle, { (short)(rs * 2 + 15), 0 });
	SetConsoleTextAttribute(handle, 15);
	std::cout << "exp: " << cexp << " / " << lvlupexp << "  LVL: " << lvl;
	SetConsoleCursorPosition(handle, { (short)(rs * 2 + 15), 1 });
	double ratio = (chp * 1.0) / (hhp * 1.0);
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
	std::cout << chp << " / " << hhp;
	SetConsoleCursorPosition(handle, { (short)(rs * 2 + 15), 3 });
	SetConsoleTextAttribute(handle, 14);
	std::cout << "Armor: " << harmor;
	SetConsoleCursorPosition(handle, { (short)(rs * 2 + 15), 4 });
	SetConsoleTextAttribute(handle, 11);
	std::cout << "Magic Res.: " << hmr;
	SetConsoleCursorPosition(handle, { (short)(rs * 2 + 15), 5 });
	SetConsoleTextAttribute(handle, 4);
	std::cout << "Damage: " << hdmg;
	SetConsoleCursorPosition(handle, { (short)(rs * 2 + 15), 6 });
	SetConsoleTextAttribute(handle, 8);
	std::cout << "Speed: " << movesleft << " / " << hspd << "   ";
	if (hero != 0)
	{
		SetConsoleCursorPosition(handle, { (short)(rs * 2 + 15), 7 });
		SetConsoleTextAttribute(handle, 9);
		std::cout << "Mana: " << cmana << " / " << mana << "   ";
	}

}

void enemyattack(int i, int j, int k, int c) //funkcja ataku przeciwnika
{
	if (en[units[i][j]].type == 0)
	{
		chp -= ((en[c].dmg - harmor) > 0 ? (en[c].dmg - harmor) : 0);
	}
	else
	{
		chp -= ((en[c].dmg - hmr) > 0 ? (en[c].dmg - hmr) : 0);
	}
	k = en[c].spd + 1;
}

void AII() //AI przeciwników
{
	nr = 0;
	aiused[0] = 0;
	aiused[1] = 0;
	aiused[2] = 0;
	aiused[3] = 0;
	aiused[4] = 0;
	for (int i = hrx * rs; i < hrx * rs + 12; i++)
	{
		for (int j = hry * rs; j < hry * rs + 12; j++)
		{
			if (units[i][j] > 2) //je¿eli jednostka istnieje i nie jest gracze, oraz nie zostala juz poruszona
			{
				int c = units[i][j];
				if (en[c].stunned == TRUE) //je¿eli jednostka nie ma na sobie efktu stunned
				{
					en[c].stunned = FALSE;
					aiused[nr] = c; nr++;
				}
				else if (units[i][j] != aiused[0] && units[i][j] != aiused[1] && units[i][j] != aiused[2] && units[i][j] != aiused[3] && units[i][j] != aiused[4])
				{
					int dx = 0, dy = 0;
					aiused[nr] = c; nr++;
					for (int k = 0; k < en[c].spd; k++)
					{
						/*SetConsoleTextAttribute(handle, 15);
						system("pause");
						floorprint();*/
						Sleep(50);
						floorprint();
						if (abs((i + dx) - hcx) >= abs((j + dy) - hcy)) //je¿eli jednosta jest dalej w poziomie niz w pionie
						{
							if ((i + dx) - hcx < 0) //je¿eli jenostka jest na lewo od gracza
							{
								if (units[i + 1 + dx][j + dy] == 0 && board[i + 1 + dx][j + dy] == 0)
								{
									swap(units[i + 1 + dx][j + dy], units[i + dx][j + dy]);
									dx++;
								}
								else if (units[i + 1 + dx][j + dy] == 2) //atak
								{
									enemyattack(i, j, k, c);
								}
								else if ((j + dy) - hcy < 0)
								{
									if (units[i + dx][j + 1 + dy] == 0 && board[i + dx][j + 1 + dy] == 0)
									{
										swap(units[i + dx][j + 1 + dy], units[i + dx][j + dy]);
										dy++;
									}
									else if (units[i + dx][j - 1 + dy] == 0 && board[i + dx][j - 1 + dy] == 0)
									{
										swap(units[i + dx][j - 1 + dy], units[i + dx][j + dy]);
										dy--;
									}
								}
								else if ((j + dy) - hcy > 0)
								{
									if (units[i + dx][j - 1 + dy] == 0 && board[i + dx][j - 1 + dy] == 0)
									{
										swap(units[i + dx][j - 1 + dy], units[i + dx][j + dy]);
										dy--;
									}
									else if (units[i + dx][j + 1 + dy] == 0 && board[i + dx][j + 1 + dy] == 0)
									{
										swap(units[i + dx][j + 1 + dy], units[i + dx][j + dy]);
										dy++;
									}
								}
								else
								{
									switch (r(0, 1))
									{
									case 0:
									{
										if (units[i + dx][j + 1 + dy] == 0 && board[i + dx][j + 1 + dy] == 0)
										{
											swap(units[i + dx][j + 1 + dy], units[i + dx][j + dy]);
											dy++;
										}
										else if (units[i + dx][j - 1 + dy] == 0 && board[i + dx][j - 1 + dy] == 0)
										{
											swap(units[i + dx][j - 1 + dy], units[i + dx][j + dy]);
											dy--;
										}
									}
									case 1:
									{
										if (units[i + dx][j - 1 + dy] == 0 && board[i + dx][j - 1 + dy] == 0)
										{
											swap(units[i + dx][j - 1 + dy], units[i + dx][j + dy]);
											dy--;
										}
										else if (units[i + dx][j + 1 + dy] == 0 && board[i + dx][j + 1 + dy] == 0)
										{
											swap(units[i + dx][j + 1 + dy], units[i + dx][j + dy]);
											dy++;
										}
									}
									}
								}
							}
							else if ((i + dx) - hcx > 0) //je¿eli jednosta jest na prawo od gracza
							{
								if (units[i - 1 + dx][j + dy] == 0 && board[i - 1 + dx][j + dy] == 0)
								{
									swap(units[i - 1 + dx][j + dy], units[i + dx][j + dy]);
									dx++;
								}
								else if (units[i - 1 + dx][j + dy] == 2) //je¿eli jednostka s¹siaduje z bohaterem, atakuj
								{
									enemyattack(i, j, k, c);
								}
								else if ((j + dy) - hcy < 0) //je¿eli droga na przód jest zablokowana, sprawdŸ czy jdnostka jest nad...
								{
									if (units[i + dx][j + 1 + dy] == 0 && board[i + dx][j + 1 + dy] == 0) //czy ruch w dó³ jest moliwy
									{
										swap(units[i + dx][j + 1 + dy], units[i + dx][j + dy]);
										dy++;
									}
									else if (units[i + dx][j - 1 + dy] == 0 && board[i + dx][j - 1 + dy] == 0) //czy róch w góre jest mozliwy
									{
										swap(units[i + dx][j - 1 + dy], units[i + dx][j + dy]);
										dy--;
									}
								}
								else if ((j + dy) - hcy > 0) //...czy pod graczem
								{
									if (units[i + dx][j - 1 + dy] == 0 && board[i + dx][j - 1 + dy] == 0) //czy ruch w góre jest mo¿liwy
									{
										swap(units[i + dx][j - 1 + dy], units[i + dx][j + dy]);
										dy--;
									}
									else if (units[i + dx][j + 1 + dy] == 0 && board[i + dx][j + 1 + dy] == 0) //czy roch w dó³ jest mo¿liwy
									{
										swap(units[i + dx][j + 1 + dy], units[i + dx][j + dy]);
										dy++;
									}
								}
								else //jezeli jednoska jest na wysokosci gracza, ale nie moze isc naprzód, losowo wybiera kolejnoœæ ruchów
								{
									switch (r(0, 1))
									{
									case 0:
									{
										if (units[i + dx][j + 1 + dy] == 0 && board[i + dx][j + 1 + dy] == 0)
										{
											swap(units[i + dx][j + 1 + dy], units[i + dx][j + dy]);
											dy++;
										}
										else if (units[i + dx][j - 1 + dy] == 0 && board[i + dx][j - 1 + dy] == 0)
										{
											swap(units[i + dx][j - 1 + dy], units[i + dx][j + dy]);
											dy--;
										}
									}
									case 1:
									{
										if (units[i + dx][j - 1 + dy] == 0 && board[i + dx][j - 1 + dy] == 0)
										{
											swap(units[i + dx][j - 1 + dy], units[i + dx][j + dy]);
											dy--;
										}
										else if (units[i + dx][j + 1 + dy] == 0 && board[i + dx][j + 1 + dy] == 0)
										{
											swap(units[i + dx][j + 1 + dy], units[i + dx][j + dy]);
											dy++;
										}
									}
									}
								}
							}
						}
						else if (abs(i - hcx) < abs(j - hcy)) //je¿eli jedsnota jest dalej w pionie niz w poziomie
						{
							if ((j + dy) - hcy < 0) //jezeli jednostka jest nad graczem
							{
								if (units[i + dx][j + 1 + dy] == 0 && board[i + dx][j + 1 + dy] == 0)
								{
									swap(units[i + dx][j + 1 + dy], units[i + dx][j + dy]);
									dy++;
								}
								else if (units[i + dx][j + 1 + dy] == 2)
								{
									enemyattack(i, j, k, c);
								}
								else if ((i + dx) - hcx < 0)
								{
									if (units[i + 1 + dx][j + dy] == 0 && board[i + 1 + dx][j + dy] == 0)
									{
										swap(units[i + 1 + dx][j + dy], units[i + dx][j + dy]);
										dx++;
									}
									else if (units[i - 1 + dx][j + dy] == 0 && board[i - 1 + dx][j + dy] == 0)
									{
										swap(units[i - 1 + dx][j + dy], units[i + dx][j + dy]);
										dx--;
									}
								}
								else if ((i + dx) - hcx > 0)
								{
									if (units[i - 1 + dx][j + dy] == 0 && board[i - 1 + dx][j + dy] == 0)
									{
										swap(units[i - 1 + dx][j + dy], units[i + dx][j + dy]);
										dx--;
									}
									else if (units[i + 1 + dx][j + dy] == 0 && board[i + 1 + dx][j + dy] == 0)
									{
										swap(units[i + 1 + dx][j + dy], units[i + dx][j + dy]);
										dx++;
									}
								}
								else
								{
									switch (r(0, 1))
									{
									case 0:
									{
										if (units[i + 1 + dx][j + dy] == 0 && board[i + 1 + dx][j + dy] == 0)
										{
											swap(units[i + 1 + dx][j + dy], units[i + dx][j + dy]);
											dx++;
										}
										else if (units[i - 1 + dx][j + dy] == 0 && board[i - 1 + dx][j + dy] == 0)
										{
											swap(units[i - 1 + dx][j + dy], units[i + dx][j + dy]);
											dx--;
										}
									}
									case 1:
									{
										if (units[i - 1 + dx][j + dy] == 0 && board[i - 1 + dx][j + dy] == 0)
										{
											swap(units[i - 1 + dx][j + dy], units[i + dx][j + dy]);
											dx--;
										}
										else if (units[i + 1 + dx][j + dy] == 0 && board[i + 1 + dx][j + dy] == 0)
										{
											swap(units[i + 1 + dx][j + dy], units[i + dx][j + dy]);
											dx++;
										}
									}
									}
								}
							}
							else if ((j + dy) - hcy > 0) //jezeli jedsnotka jest pod graczem
							{
								if (units[i + dx][j - 1 + dy] == 0 && board[i + dx][j - 1 + dy] == 0)
								{
									swap(units[i + dx][j - 1 + dy], units[i + dx][j + dy]);
									dy++;
								}
								else if (units[i + dx][j - 1 + dy] == 2)
								{
									enemyattack(i, j, k, c);
								}
								else if ((i + dx) - hcx < 0)
								{
									if (units[i + 1 + dx][j + dy] == 0 && board[i + 1 + dx][j + dy] == 0)
									{
										swap(units[i + 1 + dx][j + dy], units[i + dx][j + dy]);
										dx++;
									}
									else if (units[i - 1 + dx][j + dy] == 0 && board[i - 1 + dx][j + dy] == 0)
									{
										swap(units[i - 1 + dx][j + dy], units[i + dx][j + dy]);
										dx--;
									}
								}
								else if ((i + dx) - hcx > 0)
								{
									if (units[i - 1 + dx][j + dy] == 0 && board[i - 1 + dx][j + dy] == 0)
									{
										swap(units[i - 1 + dx][j + dy], units[i + dx][j + dy]);
										dx--;
									}
									else if (units[i + 1 + dx][j + dy] == 0 && board[i + 1 + dx][j + dy] == 0)
									{
										swap(units[i + 1 + dx][j + dy], units[i + dx][j + dy]);
										dx++;
									}
								}
								else
								{
									switch (r(0, 1))
									{
									case 0:
									{
										if (units[i + 1 + dx][j + dy] == 0 && board[i + 1 + dx][j + dy] == 0)
										{
											swap(units[i + 1 + dx][j + dy], units[i + dx][j + dy]);
											dx++;
										}
										else if (units[i - 1 + dx][j + dy] == 0 && board[i - 1 + dx][j + dy] == 0)
										{
											swap(units[i - 1 + dx][j + dy], units[i + dx][j + dy]);
											dx--;
										}
									}
									case 1:
									{
										if (units[i - 1 + dx][j + dy] == 0 && board[i - 1 + dx][j + dy] == 0)
										{
											swap(units[i - 1 + dx][j + dy], units[i + dx][j + dy]);
											dx--;
										}
										else if (units[i + 1 + dx][j + dy] == 0 && board[i + 1 + dx][j + dy] == 0)
										{
											swap(units[i + 1 + dx][j + dy], units[i + dx][j + dy]);
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

void deathanim(short a, short b) // animacja przy œmierci lub escape
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

void spellborderprint() //wypisywanie ramki menu zaklêæ
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
void cleanup()
{
	SetConsoleCursorPosition(handle, { 0, 14 });
	std::cout << "                                                                  ";
	SetConsoleCursorPosition(handle, { 0, 15 });
	std::cout << "                                                                  ";
	SetConsoleCursorPosition(handle, { 0, 16 });
	std::cout << "                                                                  " << setfill(' ');
	for (short i = 0; i < 5; i++)
	{
		SetConsoleCursorPosition(handle, { (short)(rs * 2 + 2), (short)(11 + i) });
		std::cout << setw(35) << " " << endl;
	}

}

int main()
{
	char inp = ':)';
	char eqinp = ':)';
	boardgen();
	mmap();
	enemygen();
	secretrooms();
	help = 0;
	do
	{
		if (change == 1)
		{
			SetConsoleCursorPosition(handle, { 0, 0 });
			for (int i = 0; i < 14; i++)
				std::cout << setw(50) << " " << endl;
		}
		switch (menuhandler)
		{
		case 0: //wybór postaci
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
				wyborpostaci();
			}
			else //przypisanie statystyk postaci
			{
				switch (hero)
				{
				case 0:
				{
					hhp = bhp + 20; hdmg = bdmg; harmor = barmor + 3; hmr = bmr + 3; mana = bmana; hspd = bspd, chp = hhp, cmana = mana;
				}break;
				case 1:
				{
					hhp = bhp + 5; hdmg = bdmg + 10; harmor = barmor + 1; hmr = bmr + 1; mana = bmana + 10; hspd = bspd + 3, chp = hhp, cmana = mana;
				}break;
				case 2:
				{
					hhp = bhp; hdmg = bdmg + 10; harmor = barmor; hmr = bmr; mana = bmana + 30; hspd = bspd + 1, chp = hhp, cmana = mana;
				}break;
				case 3:
				{
					hhp = bhp + 10; hdmg = bdmg + 5; harmor = barmor + 2; hmr = bmr = 2; mana = bmana + 20; hspd = bspd + 1, chp = hhp, cmana = mana;
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
			if (change == 1 || (hrx != chrx || hry != chry))
			{
				wallprint();
			}
			change = 0;
			if (movesleft > 0)
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
				case 'm': //zaklêcia
				{
					if (hero != 0)
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
								if (cmana >= z[hero][magicmenu].manacost)
								{
									cmana -= z[hero][magicmenu].manacost;
									if (z[hero][magicmenu].whencast == 3)
									{
										cast.spell(hero, magicmenu);
										magic = 0;
										floorprint();
										wallprint();
										cleanup();

									}
									else if (z[hero][magicmenu].whencast == 1) //wybór miejsca
									{
										magic = 2;
										spellx = hcx - (hrx * rs);
										spelly = hcy - (hry * rs);
										floorprint();
										wallprint();
										cleanup();
										while (magic == 2)
										{
											floorprint();
											wallprint();
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
												floorprint();
												wallprint();
												cleanup();
												cast.spell(hero, magicmenu);
												magic = 0;
											}break;
											case 'R': case 'r': //obracanie (wyko¿ystywanie w jedym zaklêciu)
											{
												if (rotation < 8)rotation++;
												else rotation = 0;
											}break;
											case 'm': //anulowanie
											{
												cmana += z[hero][magicmenu].manacost;
												magic = 0;
												floorprint();
												wallprint();
												cleanup();
											}
											}
											if (magic != 0)
											{
												cast.display(hero, magicmenu);
												inp = _getch();
											}
										}
									}
									else if (z[hero][magicmenu].whencast == 2) //wybór przeciwnika
									{
										floorprint();
										wallprint();
										cleanup();
										how_many_enemies = 0;
										magic = 2;
										magicchoice = 0;
										for (int i = rs - 1; i >= -1; i--) //funkcja zliczaj¹ca przeciwnikow w pokoju i wybieraj¹ca pierwszego z nich
										{
											for (int j = rs - 1; j >= -1; j--)
											{
												if (units[hrx * rs + i][hry * rs + j] > 2)
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
										SetConsoleCursorPosition(handle, { (short)((spelly + 1) * 2), (short)(spellx + 1) });
										SetConsoleTextAttribute(handle, (en[units[hrx * rs + spellx][hry * rs + spelly]].color + 80));
										std::cout << setw(2) << en[units[hrx * rs + spellx][hry * rs + spelly]].letter;
										SetConsoleTextAttribute(handle, 15);
										while (magic == 2)
										{
											floorprint();
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
												cast.spell(hero, magicmenu);
												change = 1;
												magic = 0;
												floorprint();
												wallprint();
												cleanup();
											}break;
											case 'm':
											{
												cmana += z[hero][magicmenu].manacost;
												magic = 0;
												floorprint();
												wallprint();
												cleanup();
											}break;
											}
											if (change == 0)
											{
												spellhelp = 0;
												for (int i = -1; i <= rs; i++) //funkcja pozwalaj¹ca wybrac przeciwnika
												{
													for (int j = -1; j <= rs; j++)
													{
														if (units[hrx * rs + i][hry * rs + j] > 2)
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
												SetConsoleCursorPosition(handle, { (short)((spelly + 1) * 2), (short)(spellx + 1) });
												SetConsoleTextAttribute(handle, (en[units[hrx * rs + spellx][hry * rs + spelly]].color + 80));
												std::cout << setw(2) << en[units[hrx * rs + spellx][hry * rs + spelly]].letter;
												SetConsoleTextAttribute(handle, 15);
												inp = _getch();
											}
										}
									}
									change = 0;
									floorprint();
									wallprint();
									cleanup();
								}
								else
								{
									SetConsoleCursorPosition(handle, { 32, 11 });
									std::cout << "Brak many                         ";
								}
							}break;
							case 'M': case 'm':
							{
								floorprint();
								wallprint();
								cleanup();
								magic = 0;
								inp = ' ';
							}break;
							}
							if (magicmenu < magicfirst) magicfirst--;
							else if (magicmenu > magicfirst + 2) magicfirst++;
							if (magic != 0)
							{
								z[hero][magicmenu].printopis();
								SetConsoleCursorPosition(handle, { 4, 9 });
								if (magicmenu == magicfirst) { std::cout << ">"; }
								else { std::cout << " "; }
								std::cout << z[hero][magicfirst].spellname;
								SetConsoleCursorPosition(handle, { 4, 11 });
								if (magicmenu == magicfirst + 1) { std::cout << ">"; }
								else { std::cout << " "; }
								std::cout << z[hero][magicfirst + 1].spellname;
								SetConsoleCursorPosition(handle, { 4, 13 });
								if (magicmenu == magicfirst + 2) { std::cout << ">"; }
								else { std::cout << " "; }
								std::cout << z[hero][magicfirst + 2].spellname;
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
				} //koniec zaklêæ
				if (hrx != chrx || hry != chry) //zmiana pokoju
				{
					chrx = hrx;
					chry = hry;
					nextroom.reset();
					if (stealth == true) stealth = false; //reset ukrycia ³otra (ZAKLÊCIA ZAKLÊCIA ZAKLÊCIA ZAKLÊCIA ZAKLÊCIA ZAKLÊCIA ZAKLÊCIA ZAKLÊCIA )
					SetConsoleCursorPosition(handle, { (short)((rs + 2) * 2), 0 });
					SetConsoleTextAttribute(handle, 7);
					for (int i = 0; i < 10; i++) //wypisywanie minimapy
					{
						for (int j = 0; j < 10; j++)
						{
							if (i == hrx && j == hry)
							{
								SetConsoleTextAttribute(handle, 12);
								std::cout << minimap[i][j];
								SetConsoleTextAttribute(handle, 7);
							}
							else
							{
								std::cout << minimap[i][j];
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
					AII();
				}
				floorprint();
				movesleft = hspd;
				inp = ':)';
				nextturn.reset(); //reset statystyk
			}
			if (cexp >= lvlupexp) //zdobywanie poziomow
			{
				lvl++;
				cexp -= lvlupexp;
				lvlupexp *= 2;
				hhp += 5;
				harmor += 1;
				hmr += 1;
				hdmg += 2;
				mana += (mana > 0 ? 5 : 0);
			}

			if (change == 0)
			{
				hrx = hcx / rs; //hero room x = hero coordinate x / room size
				hry = hcy / rs;
				units[hcx][hcy] = 2;
				printstats();
				floorprint();						//printownie pod³ogi(co ruch)
				SetConsoleCursorPosition(handle, { 0, 16 });
				SetConsoleTextAttribute(handle, 7);
				SetConsoleCursorPosition(handle, { 0, 0 });
				if (movesleft > 0)
				{
					inp = _getch();
				}
			}
			if (respawn == 1 && chp <= 0) //zaklecie kapalana (ZAKLÊCIA ZAKLÊCIA ZAKLÊCIA ZAKLÊCIA ZAKLÊCIA ZAKLÊCIA ZAKLÊCIA ZAKLÊCIA )
			{
				respawn = 0; chp = 10;
			}
		}break;
		case 2: //ekwipunek
		{
			SetConsoleCursorPosition(handle, { 0, 0 });
			SetConsoleTextAttribute(handle, 7);
			if (change == 1) //ramka do ekwipunku
			{
				ramkaeq();
			}
			change = 0;
			switch (inp)
			{
			case 75: case 'a': case 'A': //left
			{
				if (eqx > 0) eqx--;
			}break;
			case 77: case 'd': case 'D': //right
			{
				if (eqx < 4) eqx++;
			}break;
			case 72: case 'w': case 'W': //up
			{
				if (eqy > 0) eqy--;
			}break;
			case 80: case 's': case 'S': //down
			{
				if (eqy < 3) eqy++;
			}break;
			case 'e': case 'E': //mapa
			{
				inp = ':)';
				menuhandler = 1;
				change = 1;
				cout << setfill(' ');
				floorprint();
				wallprint();
				cleanup();
			}break;
			case 13: //akcje zwiazane z przedmiotami
			{
				if (help < 3 && eq[eqx][eqy] != 0)
				{
					while (inp != '0')
					{
						if (items[eq[eqx][eqy]].consumable == 1)
						{
							switch (eqinp)
							{
							case 72: case 'w': case 'W': {if (help > 0) help--; }break;
							case 80: case 's': case 'S': {if (help < 3) help++; }break;
							case 13:
							{
								switch (help)
								{
								case 0: //uzycie przedmiotu DODAÆ RESET STATYSTYK!!!!!!!
								{
									chp = chp + items[eq[eqx][eqy]].hp > hhp ? hhp : chp + items[eq[eqx][eqy]].hp;
									harmor += items[eq[eqx][eqy]].armor;
									hmr += items[eq[eqx][eqy]].mr;
									cmana = cmana + items[eq[eqx][eqy]].mana > mana ? mana : cmana + items[eq[eqx][eqy]].mana;
									hspd += items[eq[eqx][eqy]].spd;
									hdmg += items[eq[eqx][eqy]].dmg;
									eq[eqx][eqy] = 0;
									inp = '0';
								}break;
								case 1:	//przenoszenie
								{
									eqcy = eqy;
									eqcx = eqx;
									help = 3;
									inp = '0';
								} break;
								case 2: //wyrzucanie
								{
									dropitem(hcx, hcy, eq[eqx][eqy]);
									eq[eqx][eqy] = 0;
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
									eqcy = eqy;
									eqcx = eqx;
									help = 3;
									inp = '0';
								} break;
								case 1: //wyrzucanie
								{
									dropitem(hcx, hcy, eq[eqx][eqy]);
									eq[eqx][eqy] = 0;
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
						if (eqinp != 13 && help != 3) //printowanie wyboru akcji
						{
							short int a = 0;
							if (items[eq[eqx][eqy]].consumable == 1)
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
					if (eqx == 0)
					{
						if (items[eq[eqcx][eqcy]].eqslot == eqy + 1) //zak³adanie przedmiotów
						{
							hhp = hhp - items[eq[eqx][eqy]].hp + items[eq[eqcx][eqcy]].hp;
							harmor = harmor - items[eq[eqx][eqy]].armor + items[eq[eqcx][eqcy]].armor;
							hmr = hmr - items[eq[eqx][eqy]].mr + items[eq[eqcx][eqcy]].mr;
							mana = mana - items[eq[eqx][eqy]].mana + items[eq[eqcx][eqcy]].mana;
							hspd = hspd - items[eq[eqx][eqy]].spd + items[eq[eqcx][eqcy]].spd;
							hdmg = hdmg - items[eq[eqx][eqy]].dmg + items[eq[eqcx][eqcy]].dmg;
							swap(eq[eqx][eqy], eq[eqcx][eqcy]);
						}
						else //próba za³o¿enie przedmiotu na z³e miejsce
						{
							SetConsoleCursorPosition(handle, { 33, 4 });
							std::cout << "You cant wear that there!";
							eqx = eqcx; eqy = eqcy;
						}
					}
					else
					{
						swap(eq[eqx][eqy], eq[eqcx][eqcy]);
						help = 0;
					}
					help = 0;
					eqcx = 10;
					eqcy = 10;
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
						if (i == eqx && j == eqy) SetConsoleTextAttribute(handle, 12);
						else if (i == 0 && j == items[eq[eqx][eqy]].eqslot - 1)  SetConsoleTextAttribute(handle, 9);
						else if (i == eqcx && j == eqcy) SetConsoleTextAttribute(handle, 13);
						else SetConsoleTextAttribute(handle, 7);
						SetConsoleCursorPosition(handle, { (short)(i * 6 + 1), (short)(j * 4 + 2) });
						std::cout << items[eq[i][j]].disp;
						if ((i == eqx && j == eqy) || (i == eqcx && j == eqcy)) SetConsoleTextAttribute(handle, 7);
					}
				}
			}
			items[eq[eqx][eqy]].printopis();
			if (change == 0)
			{
				inp = _getch();
			}

		}break;
		}
	} while (inp != 27 && chp > 0);
	SetConsoleCursorPosition(handle, { 14, 28 }); //animacja œmierci i zakoñczenie gry
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
	std::system("pause");

}



//dodatkowe piêtra, kopanie(secret rooms)