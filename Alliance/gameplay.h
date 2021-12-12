#pragma once
#include <SFML\Graphics.hpp>
#include "map.h"
#include <SFML/Network.hpp>
#include <list>
#include <math.h>
#include <vector>

sf::Mutex mtx;

int myteam = 0;

void startthrs();

int winstatys = 0;

void anal();

enum TYPE_MSG {
	MS_SYS_DISCONNECT,
	MS_SYS_GENERKEY,
	MS_SYS_MYTEAM,
	MS_GAM_CREATE,
	MS_GAM_HIT,
	MS_GAM_MOVE,
	MS_GAM_ATTACK,
	MS_GAM_ENTER_MINE,
	MS_GAM_LEAVE_MINE,
	MS_GAM_COLLECT,
	MS_GAM_PUT,
	MS_SYS_NOTHING,
	MS_GAM_NEXTTICK
};

enum TYPE_ARMOR {
	ARMOR_NO,
	ARMOR_LIGHT,
	ARMOR_MEDIUM,
	ARMOR_HEAVY,
	ARMOR_BUILD
};

enum TYPE_ATTACK {
	ATTACK_MELEE,
	ATTACK_RANGE,
	ATTACK_SIEGE,
	ATTACK_MAGIC,
	ATTACK_CHAOS,
	ATTACK_REPAIR
};

sf::Texture textures[3][10], rabTex[3], missles[10];


const sf::Color teamcolors[] = { sf::Color(sf::Color::Green), sf::Color(sf::Color::Red), sf::Color(sf::Color::Blue), sf::Color(200,200,255) };

const int WIN_WIDTH = 1024 - 200;
const int WIN_HEIGHT = 600 - 16;
const int SPRSIZE = 64;

int costGold[20] = { 75,135,130,245,270,230,425,745,280,230, 380,160, 80,340,0,0,0,0,0,0, };
int costWood[20] = { 0,  0, 10, 60, 20, 50,250,200, 70, 90, 205, 60, 20,260,0,0,0,0,0,0, };
int needFood[10] = { 1,  2,  2,  4,  4,  4,  5,  8,  4,  3 };
// from 10 - buildings

class player {
public:
	virtual void sendActivity(int, int, int, TYPE_MSG, int) = 0;
	virtual void getActivity(sf::Packet) = 0;

	virtual ~player() {

	}
	virtual void startgame() = 0;
};

player* output;

float getAttackArmorScale(int attackType, int armorType) {
	// i - attacktype, j - armortype
	float scales[6][5] = {   1,    1,  1.5, 1,  0.7,
						   1.5,    2, 0.75, 1, 0.35,
						   1.5,    1,  0.5, 1,  1.5,
						     1, 1.25, 0.75, 2, 0.35,
						     1,    1,    1, 1,    1,
							 0,    0,    0, 0,   -1,
	};

	return scales[attackType][armorType];
}

class object {
protected:
	int i, j;
	int team;
	float hp;
	int statys;
	bool onGround; // true - ground object; false - flying object;
	TYPE_ARMOR armorType;
	float armor;

public:
	int animstatys;

	virtual void die() = 0;

	int allobjsI, doobjI;

	int getStatys() {
		return statys;
	}

	virtual void gotdmg(int damag) {
		TYPE_ATTACK attackType = static_cast<TYPE_ATTACK>(damag / 1000);
		float attack = damag % 1000;
		float damageScale = 1.f - (armor * 0.06 / (1.f + armor * 0.06));
		hp -= attack * damageScale * getAttackArmorScale(attackType, armorType);
	}

	int width, height;
	int maxhp;

	virtual void draw(sf::RenderWindow* win, int pointx, int pointy) = 0;

	int getTeam() {
		return team;
	}

	int getI() {
		return i;
	}

	int getJ() {
		return j;
	}

	virtual int getHp() {
		return hp;
	}

	virtual bool isTree() {
		return false;
	}

	virtual bool isUnit() {
		return false;
	}

	virtual ~object() {};

	virtual void makeaction(int i, int j, int comm, bool shift) {

	}

	virtual void doSmth() = 0;

	friend class missle;
	friend class arrow;
	friend class light;
	friend class bullet;
	friend class missile;
	friend class Host;
	friend class Client;
};

std::vector<object*> allobjs;

class tree : public object {
public:
	bool isTree() override {
		return true;
	}

	void die() override {
		statys = 9;
		for (int i = 0; i < allobjs.size(); i++) {
			if (allobjs[i] == this) {
				allobjs.erase(allobjs.begin() + i, allobjs.begin() + i + 1);
				break;
			}
		}
		objMap[i][j] = NULL;
		hp = -1;
	}

	void gotdmg(int damag) override {
		hp -= damag;
		if (hp <= 0) die();
	}

	~tree() override {}

	void doSmth() override {}

	void draw(sf::RenderWindow* win, int pointx, int pointy) override {
		int windowx = j * SPRSIZE - pointx;
		int windowy = i * SPRSIZE - pointy;

		if (windowx >= -WIN_WIDTH && windowx <= 128 * SPRSIZE) {
			if (windowy >= -WIN_HEIGHT && windowy <= 128 * SPRSIZE) {
				sf::Sprite spr;
				spr.setPosition(windowx, windowy);
				spr.setTexture(textures[2][0]);
				win->draw(spr);
			}
		}
	}

	tree(int i, int j) {
		this->i = i;
		this->j = j;
		hp = 100;
		team = -1;
		statys = 0;
		onGround = 1;
		animstatys = 0;
		width = 1;
		height = 1;
		allobjs.push_back(this);
		allobjsI = allobjs.size() - 1;
		armor = 0;
		armorType = ARMOR_NO;
	}
};

std::vector<object*> doobjs;

void feeltrees(float heightMap[129][129]) {
	for (int i = 0; i < 129; i++) {
		for (int j = 0; j < 129; j++) {
			if (heightMap[i][j] > forest + 100 && heightMap[i][j] < mount) {
				objMap[i][j] = new tree(i, j);
			}
		}
	}
}

class build : public object {
protected:
	int pointI, pointJ;

	int maxdelay, delay;

	void drawHp(sf::RenderWindow* win, int pointx, int pointy) {
		int windowx = j * SPRSIZE - pointx - (width / 2) * SPRSIZE;
		int windowy = i * SPRSIZE - pointy - (height / 2) * SPRSIZE;

		sf::RectangleShape rec;
		rec.setFillColor(sf::Color(0, 0, 0, 64));
		rec.setPosition(windowx + 4, 4 + windowy);
		rec.setSize(sf::Vector2f(width * 64 - 8, 10));
		win->draw(rec);

		rec.setFillColor(teamcolors[team]);
		rec.setPosition(windowx + 4, 4 + windowy);
		rec.setSize(sf::Vector2f(int(float(hp) / float(maxhp) * ((width * 64) - 8)), 10));

		win->draw(rec);
	}

public:

	int timer;
	int maxtimer;

	virtual int getSizeQ() = 0;

	virtual int getTypeQ(int i) = 0;

	virtual void getComm(int t) = 0;

	virtual int getType() = 0;

	void makeaction(int i, int j, int comm, bool shift) override {
		pointI = i;
		pointJ = j;
	}

	build() {
		allobjs.push_back(this);
		doobjs.push_back(this);
		statys = 1;
		timer = 0;
		maxtimer = 0;
		onGround = 1;
		allobjsI = allobjs.size() - 1;
		doobjI = doobjs.size() - 1;
	}
};

class base;

class unit;

class tea {
public:
	virtual std::vector<base*> getBases() = 0;

	virtual std::vector<build*> getBuilds() = 0;

	virtual std::vector<unit*> getUnits() = 0;

	virtual int getWood() = 0;

	virtual int getGold() = 0;

	virtual int getFood() = 0;

	virtual int getFoodLimit() = 0;

	virtual void sendMessage(int typeOfMessage, void* argument) = 0;
};

tea* teams[4];

class unit : public object {
protected:
	int anim/* 0-63 */, direction;
	int attack, attackrange;
	int wayI, wayJ, nexti, nextj;
	int di, dj;
	std::list<int> typecomm;
	std::list<int> commI;
	std::list<int> commJ;
	int wait = 0;
	float movespeed, attackdelay;
	int attackI, attackJ;
	object* attTarget = NULL;
	int maxattdel;
	bool waitServer = 0;
	TYPE_ATTACK attackType;
public:

	virtual void dealdamage(int attack) {
		output->sendActivity(attTarget->getI(), attTarget->getJ(), team, MS_GAM_HIT, attackType*1000 + attack);
	}

	bool isUnit() override {
		return true;
	}

	void makeaction(int i, int j, int comm, bool shift) override {
		if (statys != -1 && statys <= 9) {
			if (!shift) {
				if (animstatys == 1 && anim > 0) {
					statys = 1;
					wayI = nexti;
					wayJ = nextj;
				}
				else {
					anim = 0;
					statys = 0;
					animstatys = 0;
				}
				while (typecomm.size() > 0) {
					commI.pop_back();
					commJ.pop_back();
					typecomm.pop_back();
				}
			}
		}
		typecomm.push_back(comm);
		commI.push_back(i);
		commJ.push_back(j);
	}

	struct point {
		int i, j;
		point(int i, int j) {
			this->i = i;
			this->j = j;
		}
		point() {

		}
	};

	int getdir(int i, int j) {
		int directs[3][3] = { 8, 1, 2, 7, 9, 3, 6, 5, 4 };
		if (i - this->i == 0) {
			i = 1;
		}
		else
			i = (i - this->i) / abs(i - this->i) + 1;
		if (j - this->j == 0) {
			j = 1;
		}
		else
			j = (j - this->j) / abs(j - this->j) + 1;
		return directs[i][j];
	}

	virtual int getType() = 0;

	void createway(int i, int j, int n, int t) {
		float collMap[129][129];

		for (int i = 0; i < 129; i++) {
			for (int j = 0; j < 129; j++) {
				if (objMap[i][j] == attTarget)
					collMap[i][j] = 0;
				else
					if (objMap[i][j] != NULL) {
						if (objMap[i][j] == attTarget)
							collMap[i][j] = 0;
						else if (!objMap[i][j]->isTree() && !objMap[i][j]->isUnit()) {
							collMap[i][j] = 0xffffffff;
						}
						else {
							if (objMap[i][j]->getStatys() == 0)
								collMap[i][j] = 0xffffffff;
							else if (n == 1) {
								collMap[i][j] = 0xffffffff;
							}
							else collMap[i][j] = 0;
						}
						if (objMap[i][j]->isTree()) {
							if (t == 0)
								collMap[i][j] = 0xffffffff;
							else collMap[i][j] = 0;
						}
					}
					else
						if (heightMap[i][j] <= sand) {
							collMap[i][j] = 0x7fffffff;
							if (objMap[i][j] == NULL) {

							}
						}
						else collMap[i][j] = 0;
			}
		}



		bool f = 0;

		if (collMap[i][j] != 0 || (this->statys == 5 && objMap[i][j] == attTarget)) {
			f = 1;
		}

		collMap[i][j] = 1;
		collMap[this->i][this->j] = 0;
		std::list<point> listOfP;
		listOfP.push_back(point(i, j));

		point head = listOfP.front();

		int di[] = { -1, +1, 0, 0, -1, -1, +1, +1 };
		int dj[] = { 0, 0, -1, +1, -1, +1, -1, +1 };
		float add[] = { 1, 1, 1, 1, 2, 2, 2, 2 };


		while (listOfP.size() > 0 && !(head.i == this->i && head.j == this->j)) {
			head = listOfP.front();
			listOfP.pop_front();

			bool fn = 1;

			if (objMap[head.i][head.j] != NULL) {
				if (objMap[head.i][head.j]->isUnit() && !f) {
					if (objMap[head.i][head.j]->animstatys != 1) {
						fn = 0;
					}
				}
				else if (!f) fn = 0;
				if (objMap[head.i][head.j] == attTarget || (objMap[head.i][head.j]->isTree() && t == 1)) fn = 1;
			}

			if (fn)
				for (int k = 0; k < 8; k++) {
					if (head.i > 0 && head.i < 128 && head.j > 0 && head.j < 128) {
						if (collMap[head.i + di[k]][head.j + dj[k]] == 0 && heightMap[head.i + di[k]][head.j + dj[k]] > sand) {
							collMap[head.i + di[k]][head.j + dj[k]] = collMap[head.i][head.j] + add[k];
							listOfP.push_back(point(head.i + di[k], head.j + dj[k]));
							f = 0;
						}
						else if (f) {
							listOfP.push_back(point(head.i + di[k], head.j + dj[k]));
							collMap[head.i + di[k]][head.j + dj[k]] = collMap[head.i][head.j] + add[k];
						}
					}
				}
		}
		nexti = this->i;
		nextj = this->j;

		for (int i = this->i - 1; i <= this->i + 1; i++) {
			for (int j = this->j - 1; j <= this->j + 1; j++) {
				if (objMap[i][j] != NULL && objMap[i][j] != this) {
					collMap[i][j] += 100 * n;
				}
				if (collMap[i][j] > 0 && collMap[i][j] < collMap[nexti][nextj] && heightMap[i][j] > sand) {
					nexti = i;
					nextj = j;
				}
			}
		}

	}

	point getIJ(int dir) {
		point directs[8] = { point(-1, 0), point(-1, 1), point(0, 1), point(1, 1), point(1, 0), point(1, -1), point(0, -1), point(-1, -1) };
		return directs[dir];
	}

	void doSmth() override {
		if (attackdelay > 0) attackdelay--;
		if (!waitServer)
			switch (statys) {
			case 0:
				if (typecomm.size() > 0) {
					int comi = commI.front();
					int comj = commJ.front();
					int com = typecomm.front();
					commI.pop_front();
					commJ.pop_front();
					typecomm.pop_front();
					attTarget = NULL;
					switch (com) {
					case 0:
						if (objMap[comi][comj] == NULL) {
							wayI = comi;
							wayJ = comj;
							statys = 1;
						}
						else {
							if (objMap[comi][comj]->getTeam() == -1 || objMap[comi][comj]->getTeam() == team) {
								wayI = comi;
								wayJ = comj;
								statys = 1;
							}
							else {
								wayI = comi;
								wayJ = comj;
								attTarget = objMap[comi][comj];
								statys = 5;
							}
							if (objMap[comi][comj]->isTree() && this->getType() == 5) {
								wayI = comi;
								wayJ = comj;
								attTarget = objMap[comi][comj];
								statys = 5;
							}
						}

						break;
					case 1:
						wayI = comi;
						wayJ = comj;
						statys = 1;
						break;
					case 2:
						if (objMap[comi][comj] != NULL && objMap[comi][comj] != this && objMap[comi][comj]->getTeam() != -1) {
							attTarget = objMap[comi][comj];
							wayI = comi;
							wayJ = comj;
							statys = 5;
						}
						else {
							wayI = comi;
							wayJ = comj;
							attackI = comi;
							attackJ = comj;
							statys = 4;
						}
						break;
					case 3:
						wayI = comi;
						wayJ = comj;
						attackI = comi;
						attackJ = comj;
						statys = 3;
						break;
					}
				}
				break;
			case 1: //move
				if (anim == 0) {
					if (i == wayI && wayJ == j) { statys = 0; animstatys = 0; }
					else {
						if (wait > 0) {
							wait--;
							if (wait == 0 && objMap[nexti][nextj] != NULL)
								if (objMap[nexti][nextj]->getStatys() == 0 || (!objMap[nexti][nextj]->isTree() && !objMap[nexti][nextj]->isUnit())) { statys = 0; }
						}
						else {

							if (nexti == wayI && nextj == wayJ) {
								statys = 0;
								animstatys = 0;
								break;
							}
							else {
								if (objMap[nexti][nextj] == NULL || objMap[nexti][nextj] == this)
									createway(wayI, wayJ, 0, 0);
								if (objMap[nexti][nextj] == NULL) {
									objMap[nexti][nextj] = this;
									output->sendActivity(this->i, this->j, team, MS_GAM_MOVE, nexti * 1000 + nextj);
								}

								else {

									int nexti2 = nexti;
									int nextj2 = nextj;

									createway(wayI, wayJ, 1, 0);

									if (objMap[nexti][nextj] == NULL) {
										objMap[nexti][nextj] = this;
										output->sendActivity(this->i, this->j, team, MS_GAM_MOVE, nexti * 1000 + nextj);
									}
									else {

										nexti = nexti2;
										nextj = nextj2;

										wait = 64 * 64 / movespeed;

										animstatys = 0;
									}
								}
							}
						}
					}
				}
				else {
					anim++;

					if (anim >= 64 * 64 / movespeed) {
						objMap[i][j] = NULL;
						i = nexti;
						j = nextj;
						anim = 0;
					}
				}
				break;
			case 3:
				if (attTarget != NULL && (attTarget->getHp() <= 0 || attTarget->getStatys() > 9 || attTarget->getStatys() == -1)) {
					attTarget = NULL;
					if (animstatys != 1) {
						animstatys = 1;
						anim = 0;
					}
				}
				if (animstatys == 5) {
					if (attTarget == NULL) {
						animstatys = 1;
						anim = 0;
					}
					else {
						if (attackdelay == 0)
							anim++;
						direction = getdir(attTarget->getI(), attTarget->getJ());
						if (anim >= 63) {
							dealdamage(attack);
							anim = 0;
							attackdelay = maxattdel;
							if (attTarget->getHp() <= 0 || attTarget->getStatys() > 9 || attTarget->getStatys() == -1) {
								attTarget = NULL;
								animstatys = 1;
								anim = 0;
							}
							else
								if (abs(attTarget->getI() - this->i) <= attackrange + attTarget->height / 2 && abs(attTarget->getJ() - this->j) <= attackrange + attTarget->width / 2) {
									output->sendActivity(this->i, this->j, team, MS_GAM_ATTACK, attTarget->getI() * 1000 + attTarget->getJ());
								}
								else animstatys = 1;
						}
					}
				}
				else
					if (anim == 0) {
						for (int i = this->i - attackrange - 5; i <= this->i + attackrange + 5; i++) {
							for (int j = this->j - attackrange - 5; j <= this->j + attackrange + 5; j++) {
								if (i > 0 && i < 129 && j > 0 && j < 129)
									if (objMap[i][j] != NULL) {
										if (objMap[i][j]->getTeam() != team && objMap[i][j]->getTeam() >= 0) {
											if (attTarget == NULL) {
												attTarget = objMap[i][j];
											}
											else if (pow(objMap[i][j]->getI() - this->i, 2) + pow(objMap[i][j]->getJ() - this->j, 2) < pow(attTarget->getI() - this->i, 2) + pow(attTarget->getJ() - this->j, 2)) {
												attTarget = objMap[i][j];
											}
										}
									}
							}
						}
						if (attTarget != NULL && abs(attTarget->getI() - this->i) <= attackrange + attTarget->height / 2 && abs(attTarget->getJ() - this->j) <= attackrange + attTarget->width / 2) {
							output->sendActivity(this->i, this->j, team, MS_GAM_ATTACK, attTarget->getI() * 1000 + attTarget->getJ());
						}
						else {
							if (attTarget != NULL) { wayI = attTarget->getI(); wayJ = attTarget->getJ(); }
							else { wayI = attackI; wayJ = attackJ; }
							if (wait > 0) {
								wait--;
							}
							else {
								if (objMap[nexti][nextj] == NULL || objMap[nexti][nextj] == this)
									createway(wayI, wayJ, 0, 0);
								if (objMap[nexti][nextj] == NULL) {
									objMap[nexti][nextj] = this;
									output->sendActivity(this->i, this->j, team, MS_GAM_MOVE, nexti * 1000 + nextj);
								}

								else {

									int nexti2 = nexti;
									int nextj2 = nextj;

									createway(wayI, wayJ, 1, 0);

									if (objMap[nexti][nextj] == NULL) {
										objMap[nexti][nextj] = this;
										output->sendActivity(this->i, this->j, team, MS_GAM_MOVE, nexti * 1000 + nextj);
									}
									else {

										nexti = nexti2;
										nextj = nextj2;

										wait = -1 + 64 * 64 / movespeed;

										animstatys = 0;

									}
								}

							}

						}
					}
					else {
						anim++;

						if (anim >= 64 * 64 / movespeed) {
							objMap[i][j] = NULL;
							i = nexti;
							j = nextj;
							anim = 0;
							if (attTarget != NULL && abs(attTarget->getI() - this->i) <= attackrange + attTarget->height / 2 && abs(attTarget->getJ() - this->j) <= attackrange + attTarget->width / 2) {
								output->sendActivity(this->i, this->j, team, MS_GAM_ATTACK, attTarget->getI() * 1000 + attTarget->getJ());
							}
						}
					}
				break;
			case 4:
				if (attTarget != NULL && (attTarget->getHp() <= 0 || attTarget->getStatys() > 9 || attTarget->getStatys() == -1)) {
					attTarget = NULL;
					if (animstatys != 1) {
						animstatys = 1;
						anim = 0;
					}
				}
				if (animstatys == 5) {
					if (attTarget == NULL) {
						animstatys = 1;
						anim = 0;
					}
					else {
						if (attackdelay == 0)
							anim++;
						direction = getdir(attTarget->getI(), attTarget->getJ());
						if (anim >= 63) {
							dealdamage(attack);
							attackdelay = maxattdel;
							anim = 0;
							if (attTarget->getHp() <= 0 || attTarget->getStatys() > 9 || attTarget->getStatys() == -1) {
								attTarget = NULL;
								animstatys = 1;
								anim = 0;
							}
							else
								if (abs(attTarget->getI() - this->i) <= attackrange + attTarget->height / 2 && abs(attTarget->getJ() - this->j) <= attackrange + attTarget->width / 2) {
									output->sendActivity(this->i, this->j, team, MS_GAM_ATTACK, attTarget->getI() * 1000 + attTarget->getJ());
								}
								else animstatys = 1;
						}
					}
				}
				else
					if (anim == 0) {
						for (int i = this->i - attackrange - 5; i <= this->i + attackrange + 5; i++) {
							for (int j = this->j - attackrange - 5; j <= this->j + attackrange + 5; j++) {
								if (i > 0 && i < 129 && j > 0 && j < 129)
									if (objMap[i][j] != NULL) {
										if (objMap[i][j]->getTeam() != team && objMap[i][j]->getTeam() >= 0) {
											if (attTarget == NULL) {
												attTarget = objMap[i][j];
											}
											else if (pow(objMap[i][j]->getI() - this->i, 2) + pow(objMap[i][j]->getJ() - this->j, 2) < pow(attTarget->getI() - this->i, 2) + pow(attTarget->getJ() - this->j, 2)) {
												attTarget = objMap[i][j];
											}
										}
									}
							}
						}
						if (attTarget != NULL && abs(attTarget->getI() - this->i) <= attackrange + attTarget->height / 2 && abs(attTarget->getJ() - this->j) <= attackrange + attTarget->width / 2) {
							output->sendActivity(this->i, this->j, team, MS_GAM_ATTACK, attTarget->getI() * 1000 + attTarget->getJ());
						}
						else {
							if (attTarget != NULL) { wayI = attTarget->getI(); wayJ = attTarget->getJ(); }
							else { wayI = attackI; wayJ = attackJ; }
							if (i == attackI && attackJ == j) { statys = 0; animstatys = 0; }
							else {
								if (wait > 0) {
									wait--;
								}
								else {
									if (nexti == wayI && nextj == wayJ) {
										statys = 0;
										animstatys = 0;
										break;
									}
									else {
										if (objMap[nexti][nextj] == NULL || objMap[nexti][nextj] == this)
											createway(wayI, wayJ, 0, 0);
										if (objMap[nexti][nextj] == NULL) {
											objMap[nexti][nextj] = this;
											output->sendActivity(this->i, this->j, team, MS_GAM_MOVE, nexti * 1000 + nextj);
										}

										else {

											int nexti2 = nexti;
											int nextj2 = nextj;

											createway(wayI, wayJ, 1, 0);

											if (objMap[nexti][nextj] == NULL) {
												objMap[nexti][nextj] = this;
												output->sendActivity(this->i, this->j, team, MS_GAM_MOVE, nexti * 1000 + nextj);
											}
											else {

												nexti = nexti2;
												nextj = nextj2;

												wait = -1 + 64 * 64 / movespeed;

												animstatys = 0;

											}
										}
									}
								}
							}
						}
					}
					else {
						anim++;

						if (anim >= 64 * 64 / movespeed) {
							objMap[i][j] = NULL;
							i = nexti;
							j = nextj;
							anim = 0;
							if (attTarget != NULL && abs(attTarget->getI() - this->i) <= attackrange + attTarget->height / 2 && abs(attTarget->getJ() - this->j) <= attackrange + attTarget->width / 2) {
								output->sendActivity(this->i, this->j, team, MS_GAM_ATTACK, attTarget->getI() * 1000 + attTarget->getJ());
							}
						}
					}
				break;
			case 5:

				if (attTarget == NULL || attTarget->getHp() <= 0 || attTarget->getStatys() > 9 || attTarget->getStatys() == -1) {
					if (typecomm.size() == 0) {
						attTarget = NULL;

						for (int i = this->i - attackrange - 10; i <= this->i + attackrange + 10; i++) {
							for (int j = this->j - attackrange - 10; j <= this->j + attackrange + 10; j++) {
								if (i > 0 && i < 129 && j > 0 && j < 129)
									if (objMap[i][j] != NULL) {
										if (objMap[i][j]->getTeam() != team && objMap[i][j]->getTeam() >= 0) {
											if (attTarget == NULL) {
												attTarget = objMap[i][j];
											}
											else if (pow(objMap[i][j]->getI() - this->i, 2) + pow(objMap[i][j]->getJ() - this->j, 2) < pow(attTarget->getI() - this->i, 2) + pow(attTarget->getJ() - this->j, 2)) {
												attTarget = objMap[i][j];
											}
										}
									}
							}
						}

						if (attTarget == NULL) {

							if (anim > 0 && animstatys == 1) {
								statys = 1;
								wayI = nexti;
								wayJ = nextj;
							}
							else {
								statys = 0;
								animstatys = 0;
								anim = 0;
							}
						}
						else {
							if (animstatys != 1) {
								animstatys = 1;
								anim = 0;
							}
						}
						break;
					}
					else {
						if (anim > 0 && animstatys == 1) {
							statys = 1;
							wayI = nexti;
							wayJ = nextj;
						}
						else {
							statys = 0;
							animstatys = 0;
							anim = 0;
						}
					}
				}
				else if (animstatys == 5) {
					if (attackdelay == 0)
						anim++;
					direction = getdir(attTarget->getI(), attTarget->getJ());
					if (anim >= 63) {
						dealdamage(attack);
						attackdelay = maxattdel;
						anim = 0;
						if (attTarget->getHp() <= 0 || attTarget->getStatys() > 9 || attTarget->getStatys() == -1) {
							if (typecomm.size() == 0) {
								attTarget = NULL;

								for (int i = this->i - attackrange - 10; i <= this->i + attackrange + 10; i++) {
									for (int j = this->j - attackrange - 10; j <= this->j + attackrange + 10; j++) {
										if (i > 0 && i < 129 && j > 0 && j < 129)
											if (objMap[i][j] != NULL) {
												if (objMap[i][j]->getTeam() != team && objMap[i][j]->getTeam() >= 0) {
													if (attTarget == NULL) {
														attTarget = objMap[i][j];
													}
													else if (pow(objMap[i][j]->getI() - this->i, 2) + pow(objMap[i][j]->getJ() - this->j, 2) < pow(attTarget->getI() - this->i, 2) + pow(attTarget->getJ() - this->j, 2)) {
														attTarget = objMap[i][j];
													}
												}
											}
									}
								}

								if (attTarget == NULL) {

									if (anim > 0 && animstatys == 1) {
										statys = 1;
										wayI = nexti;
										wayJ = nextj;
									}
									else {
										statys = 0;
										animstatys = 0;
										anim = 0;
									}
								}
								else {
									if (animstatys != 1) {
										animstatys = 1;
										anim = 0;
									}
								}
								break;
							}
							else {
								if (anim > 0 && animstatys == 1) {
									statys = 1;
									wayI = nexti;
									wayJ = nextj;
								}
								else {
									statys = 0;
									animstatys = 0;
									anim = 0;
								}
							}
						}
						else
							if (abs(attTarget->getI() - this->i) <= attackrange + attTarget->height / 2 && abs(attTarget->getJ() - this->j) <= attackrange + attTarget->width / 2) {
								output->sendActivity(this->i, this->j, team, MS_GAM_ATTACK, attTarget->getI() * 1000 + attTarget->getJ());
							}
							else animstatys = 1;
					}
				}
				else {
					if (anim == 0) {
						if (abs(attTarget->getI() - this->i) <= attackrange + attTarget->height / 2 && abs(attTarget->getJ() - this->j) <= attackrange + attTarget->width / 2) {
							output->sendActivity(i, j, team, MS_GAM_ATTACK, attTarget->getI() * 1000 + attTarget->getJ());
						}
						else
							if (wait > 0) wait--;
							else {
								wayJ = attTarget->getJ();
								wayI = attTarget->getI();
								if (objMap[nexti][nextj] == NULL || objMap[nexti][nextj] == this)
									createway(wayI, wayJ, 0, 0);
								if (objMap[nexti][nextj] == NULL) {
									objMap[nexti][nextj] = this;
									output->sendActivity(this->i, this->j, team, MS_GAM_MOVE, nexti * 1000 + nextj);
								}
								else {
									int nexti2 = nexti;
									int nextj2 = nextj;

									createway(wayI, wayJ, 1, 0);

									if (objMap[nexti][nextj] == NULL) {
										objMap[nexti][nextj] = this;
										output->sendActivity(this->i, this->j, team, MS_GAM_MOVE, nexti * 1000 + nextj);
									}
									else
									{

										nexti = nexti2;
										nextj = nextj2;

										if (!(objMap[nexti][nextj]->getStatys() == 0 || (!objMap[nexti][nextj]->isTree() && !objMap[nexti][nextj]->isUnit()))) { wait = 128 * 64 / movespeed; }
										{
											wait = -1 + 64 * 64 / movespeed;
										}
										animstatys = 0;
									}
								}

							}
					}
					else {
						anim++;

						if (anim >= 64 * 64 / movespeed) {
							objMap[i][j] = NULL;
							i = nexti;
							j = nextj;
							anim = 0;
							if (abs(attTarget->getI() - this->i) <= attackrange + attTarget->height / 2 && abs(attTarget->getJ() - this->j) <= attackrange + attTarget->width / 2) {
								output->sendActivity(this->i, this->j, team, MS_GAM_ATTACK, attTarget->getI() * 1000 + attTarget->getJ());
							}
						}
					}
				}
				break;
			case 9:
				if (attTarget == NULL) {
					statys = 0;
					anim = 0;
					animstatys = 0;
					attTarget = NULL;
					break;
				}
				if (attTarget->getHp() <= 0 || attTarget->getStatys() > 9 || attTarget->getStatys() == -1) {
					statys = 0;
					anim = 0;
					animstatys = 0;
					attTarget = NULL;
					break;
				}
				direction = getdir(attTarget->getI(), attTarget->getJ());
				if (attackdelay <= 0) {
					anim++;
					if (anim >= 63) {
						dealdamage(0);
						attTarget = NULL;
						attackdelay = maxattdel;
						statys = 0;
						anim = 0;
						animstatys = 0;
					}
				}
				break;

			case 10:
				nexti = i;
				nextj = j;
				anim++;
				if (anim >= 48) {
					hp = -1;
					statys = 11;
					die();
				}
				break;
			default:

				break;
			}
	}

	void die() override {
		for (int i = 0; i < allobjs.size(); i++) {
			if (allobjs[i] == this) {
				allobjs.erase(allobjs.begin() + i, allobjs.begin() + i + 1);
				break;
			}
		}
		for (int i = 0; i < doobjs.size(); i++) {
			if (doobjs[i] == this) {
				doobjs.erase(doobjs.begin() + i, doobjs.begin() + i + 1);
				break;
			}
		}
	}

	int getAttack() {
		return attack;
	}

	unit() {
		statys = 0;
		animstatys = 0;
		anim = 0;
		attackdelay = 0;
		direction = 5;
		allobjs.push_back(this);
		doobjs.push_back(this);
		allobjsI = allobjs.size() - 1;
		doobjI = doobjs.size() - 1;
		attackI = 0;
		attackJ = 0;
	}

	friend class missle;
	friend class arrow;
	friend class bullet;
	friend class light;
	friend class missile;
	friend class Host;
	friend class Client;
};

class mine;

class barracks;

class farm;

class factory;

std::vector<object*> allMines;

void createrab(int, int, int);

class missle {
protected:
	float i, j;
	int targi, targj;
	object* attTarget;
	int statys, anim;
	int speed, dmg;
	int team;
public:

	virtual void fly() = 0;

	//float ni = i - targi;//âåêòîð , êîëèíåàðíûé ïðÿìîé, êîòîðàÿ ïåðåñåêàåò ñïðàéò è êóðñîð
	//float nj = j - targj;//îí æå, êîîðäèíàòà y
	//float rotation = (atan2(ni, nj)) * 180 / 3.14159265;

	int getStatys() {
		return statys;
	}

	virtual void draw(sf::RenderWindow* win, int pointx, int pointy) = 0;

	virtual ~missle() {};
};

std::vector<missle*> misl;

class base : public build { //statys - 0 = doNothning; statys - 1 = Building
	struct que {
		int type;
	};
	std::vector<que> queue;

public:

	void getComm(int t) override {
		if (t / 10 == 0) {
			if (queue.size() < 5)
				if (t == 0) {
					if (teams[team]->getWood() >= costWood[t] && teams[team]->getGold() >= costGold[t]) {
						que node;
						node.type = t;
						queue.push_back(node);
					}
				}
		}
		else {
			t %= 10;
			if (t < queue.size()) {
				queue.erase(queue.begin() + t);
				if (t == 0) {
					timer = 0;
				}
			}
		}
	}

	int getSizeQ() override {
		return queue.size();
	}

	int getTypeQ(int i) override {
		if (i >= queue.size()) return -1;
		else return queue[i].type;
	}

	int getType() override {
		return 0;
	}

	void doSmth() override {
		switch (statys) {
		case 0:
			if (queue.size() > 0) {
				if (teams[team]->getFood() + costWood[queue[0].type] <= teams[team]->getFoodLimit())
					timer++;
				if (timer >= maxtimer) {
					int typ = queue[0].type;
					int nI = this->i;
					int nJ = this->j;
					if (typ == 0 && (teams[team]->getWood() >= costWood[typ] && teams[team]->getGold() >= costGold[typ])) {
						int r = 3;
						while (objMap[nI][nJ] != NULL) {
							for (int i = -r; i <= r; i++) {
								if (objMap[this->i + i][this->j - r] == NULL) {
									if (objMap[nI][nJ] != NULL) {
										nI = this->i + i;
										nJ = this->j - r;
									}
									else if (pow(pointI - (this->i + i), 2) + pow(pointJ - (this->j - r), 2) < pow(pointI - nI, 2) + pow(pointJ - nJ, 2)) {
										nI = this->i + i;
										nJ = this->j - r;
									}
								}
								if (objMap[this->i + i][this->j + r] == NULL) {
									if (objMap[nI][nJ] != NULL) {
										nI = this->i + i;
										nJ = this->j + r;
									}
									else if (pow(pointI - (this->i + i), 2) + pow(pointJ - (this->j + r), 2) < pow(pointI - nI, 2) + pow(pointJ - nJ, 2)) {
										nI = this->i + i;
										nJ = this->j + r;
									}
								}
								if (objMap[this->i - r][this->j + i] == NULL) {
									if (objMap[nI][nJ] != NULL) {
										nI = this->i - r;
										nJ = this->j + i;
									}
									else if (pow(pointI - (this->i - r), 2) + pow(pointJ - (this->j + i), 2) < pow(pointI - nI, 2) + pow(pointJ - nJ, 2)) {
										nI = this->i - r;
										nJ = this->j + i;
									}
								}
								if (objMap[this->i + r][this->j + i] == NULL) {
									if (objMap[nI][nJ] != NULL) {
										nI = this->i + r;
										nJ = this->j + i;
									}
									else if (pow(pointI - (this->i + r), 2) + pow(pointJ - (this->j + i), 2) < pow(pointI - nI, 2) + pow(pointJ - nJ, 2)) {
										nI = this->i + r;
										nJ = this->j + i;
									}
								}
							}
							r++;
						}
						createrab(nI, nJ, team);
					}
					timer = 0;
					queue.erase(queue.begin());
				}
			}
			else {
				timer = 0;
			}
		case 1:
			if (hp >= 0) {
				if (hp >= maxhp) {
					statys = 0;
					hp = maxhp;
					armor = 5;
					armorType = ARMOR_BUILD;
				}
			}
			break;
		}
	}

	base(int i, int j, int team) {
		armor = 0;
		armorType = ARMOR_HEAVY;
		maxtimer = 1 * 64;
		teams[team]->sendMessage(0, this);
		this->i = i;
		this->j = j;
		this->team = team;
		width = 5;
		height = 5;
		for (int i = -height / 2; i <= height / 2; i++) {
			for (int j = -width / 2; j <= width / 2; j++) {
				objMap[i + this->i][j + this->j] = this;
			}
		}
		hp = 1;
		maxhp = 1500;
		pointI = this->i;
		pointJ = this->j;
	}

	void draw(sf::RenderWindow* win, int pointx, int pointy) override {
		int windowx = j * SPRSIZE - pointx - (width / 2) * SPRSIZE;
		int windowy = i * SPRSIZE - pointy - (height / 2) * SPRSIZE;

		if (windowx >= -WIN_WIDTH * width && windowx <= 128 * SPRSIZE) {
			if (windowy >= -WIN_HEIGHT * height && windowy <= 128 * SPRSIZE) {
				sf::Sprite spr;
				spr.setPosition(windowx, windowy);

				spr.setTexture(textures[1][0]);
				if (statys == 1) {
					spr.setTextureRect(sf::IntRect(0, 64 * height, 64 * width, 64 * height));
				}
				else spr.setTextureRect(sf::IntRect(0, 0, 64 * width, 64 * height));

				win->draw(spr);

				build::drawHp(win, pointx, pointy);
			}
		}
	}

	void die() override {
		teams[team]->sendMessage(1, this);
		statys = 10;
		for (int i = 0; i < allobjs.size(); i++) {
			if (allobjs[i] == this) {
				allobjs.erase(allobjs.begin() + i, allobjs.begin() + i + 1);
				break;
			}
		}
		for (int i = 0; i < doobjs.size(); i++) {
			if (doobjs[i] == this) {
				doobjs.erase(doobjs.begin() + i, doobjs.begin() + i + 1);
				break;
			}
		}
		for (int i = -height / 2; i <= height / 2; i++) {
			for (int j = -width / 2; j <= width / 2; j++) {
				objMap[i + this->i][j + this->j] = NULL;
			}
		}
		hp = -1;
	}

	~base() override {};
};

class team : public tea {
protected:
	int wood, gold, foodcurrent, foodlimit;

	std::vector<base*> teambases;
	std::vector<build*> teambuilds;
	std::vector<unit*> teamunits;

public:

	std::vector<base*> getBases() override {
		return teambases;
	}

	std::vector<build*> getBuilds() override {
		return teambuilds;
	}

	std::vector<unit*> getUnits() override {
		return teamunits;
	}

	int getWood() override {
		return wood;
	}
	int getGold() override {
		return gold;
	}
	int getFood() override {
		return foodcurrent;
	}
	int getFoodLimit() override {
		return foodlimit;
	}
	void sendMessage(int typeOfMessage, void* argument)  override {
		switch (typeOfMessage) {
		case 0:
		{
			build* bui = (build*)argument;
			if (bui->getType() == 0) {
				teambases.push_back((base*)bui);
			}
			else {
				teambuilds.push_back(bui);
				switch (bui->getType()) {
				case 2:
					foodlimit += 5;
					break;
				}
			}
			wood -= costWood[bui->getType() + 10];
			gold -= costGold[bui->getType() + 10];
			break;
		}
		case 1:
		{
			build* bui = (build*)argument;
			switch (bui->getType()) {
			case 0:

				break;
			case 1:

				break;
			case 2:
				foodlimit -= 5;
				break;
			}
			if (bui->getType() == 0) {
				for (int i = 0; i < teambases.size(); i++) {
					if (teambases[i] == (base*)bui) {
						teambases.erase(teambases.begin() + i);
						break;
					}
				}
			}
			else {
				for (int i = 0; i < teambuilds.size(); i++) {
					if (teambuilds[i] == bui) {
						teambuilds.erase(teambuilds.begin() + i);
						break;
					}
				}
			}
			break;
		}
		case 2:
		{
			unit* un = (unit*)argument;
			teamunits.push_back(un);
			wood -= costWood[un->getType()];
			gold -= costGold[un->getType()];
			foodcurrent += needFood[un->getType()];
			break;
		}
		case 3:
		{
			unit* un = (unit*)argument;
			for (int i = 0; i < teamunits.size(); i++) {
				if (teamunits[i] == un) {
					teamunits.erase(teamunits.begin() + i);
					foodcurrent -= needFood[un->getType()];
					break;
				}
			}
			break;
		}
		case 4: // got wood
		{
			int* value = (int*)argument;
			wood += *value;
			break;
		}
		case 5:
		{
			int* value = (int*)argument;
			gold += *value;
			break;
		}
		}

	}

	team() {
		wood = 205 + 0;
		gold = 755 + 50;
		foodcurrent = 0;
		foodlimit = 10;
	}

	~team() {
		for (object* a : teambases) {
			a->die();
		}
		for (object* a : teambuilds) {
			a->die();
		}
		for (object* a : teamunits) {
			a->die();
		}
	}
};

class arrow : public missle {
public:

	arrow(int i, int j, int team, object* attTarget, int speed, int dmg) {
		this->i = i * 64 + 32;
		this->j = j * 64 + 32;
		this->attTarget = attTarget;
		targi = attTarget->getI() * 64;
		targj = attTarget->getJ() * 64;
		statys = 1;
		this->dmg = dmg;
		this->speed = speed;
		anim = 0;
		statys = 0;
		this->team = team;
	}

	void fly() override {
		if (attTarget != NULL) {
			if (attTarget->statys >= 10 || attTarget->hp <= 0) {
				attTarget = NULL;
			}
			else if (attTarget->isUnit()) {
				unit* un = (unit*)attTarget;

				targi = (un->i * 64 + (un->nexti - un->i) * un->anim);
				targj = (un->j * 64 + (un->nextj - un->j) * un->anim);
			}
		}
		if (pow(i - targi - 32, 2) + pow(j - targj - 32, 2) <= speed * speed) {
			statys = 10;
			if (attTarget != NULL)
				output->sendActivity(attTarget->getI(), attTarget->getJ(), team, MS_GAM_HIT, dmg);
		}
		else {
			float rotation = atan2(-i + targi + 32, -j + targj + 32);
			i += sin(rotation) * speed;
			j += cos(rotation) * speed;
		}

	}

	void draw(sf::RenderWindow* win, int pointx, int pointy) override {
		int windowx = j - pointx;
		int windowy = i - pointy;
		if (windowx >= -WIN_WIDTH && windowx <= 128 * SPRSIZE) {
			if (windowy >= -WIN_HEIGHT && windowy <= 128 * SPRSIZE) {
				sf::Sprite spr;
				spr.setPosition(windowx, windowy);
				spr.setTexture(missles[0]);
				spr.setOrigin(missles[0].getSize().x / 2, missles[0].getSize().y / 2);
				spr.setRotation(atan2(-i + targi + 32, -j + targj + 32) * 180.0 / 3.14);
				win->draw(spr);
			}
		}
	}

	~arrow() override {  };
};

class light : public missle {
public:

	light(int i, int j, int team, object* attTarget, int speed, int dmg) {
		this->i = i * 64 + 32;
		this->j = j * 64 + 32;
		this->attTarget = attTarget;
		targi = attTarget->getI() * 64;
		targj = attTarget->j * 64;
		statys = 1;
		this->dmg = dmg;
		this->speed = speed;
		anim = 0;
		statys = 0;
		this->team = team;
	}

	void fly() override {
		switch (statys) {
		case 0:
			if (attTarget != NULL) {
				if (attTarget->statys >= 10 || attTarget->hp <= 0) {
					attTarget = NULL;
				}
				else if (attTarget->isUnit()) {
					unit* un = (unit*)attTarget;

					targi = (un->i * 64 + (un->nexti - un->i) * un->anim);
					targj = (un->j * 64 + (un->nextj - un->j) * un->anim);
				}
			}
			if (pow(i - targi - 32, 2) + pow(j - targj - 32, 2) <= speed * speed) {
				statys = 9;
				if (attTarget != NULL) {
					output->sendActivity(attTarget->getI(), attTarget->getJ(), team, MS_GAM_HIT, dmg);
				}

			}
			else {
				float rotation = atan2(-i + targi + 32, -j + targj + 32);
				i += sin(rotation) * speed;
				j += cos(rotation) * speed;
			}
			break;
		case 9:
			anim++;
			if (anim > 8 * 5 - 1) statys = 10;
			break;
		}
	}

	void draw(sf::RenderWindow* win, int pointx, int pointy) override {
		int windowx = j - pointx;
		int windowy = i - pointy;
		if (windowx >= -WIN_WIDTH && windowx <= 128 * SPRSIZE) {
			if (windowy >= -WIN_HEIGHT && windowy <= 128 * SPRSIZE) {
				sf::Sprite spr;
				spr.setPosition(windowx, windowy);
				spr.setTexture(missles[1]);
				spr.setColor(teamcolors[team]);
				if (statys == 9) {
					spr.setTextureRect(sf::IntRect(0, 32 + (anim / 8) * 32, 32, 32));
				}
				else {
					spr.setTextureRect(sf::IntRect(0, 0, 32, 32));
					spr.setOrigin(missles[1].getSize().x / 2, missles[1].getSize().y / (2 * 6));
					spr.setRotation(atan2(-i + targi + 32, -j + targj + 32) * 180.0 / 3.14);
				}
				win->draw(spr);
			}
		}
	}

	~light() override {  };
};

class missile : public missle {
public:

	missile(int i, int j, int team, object* attTarget, int speed, int dmg) {
		this->i = i * 64 + 32;
		this->j = j * 64 + 32;
		this->attTarget = attTarget;
		targi = attTarget->getI() * 64;
		targj = attTarget->j * 64;
		statys = 1;
		this->dmg = dmg;
		this->speed = speed;
		anim = 0;
		statys = 0;
		this->team = team;
	}

	void fly() override {
		anim++;
		if (anim > 63) anim = 0;
		if (attTarget != NULL) {
			if (attTarget->statys >= 10 || attTarget->hp <= 0) {
				attTarget = NULL;
			}
			else if (attTarget->isUnit()) {
				unit* un = (unit*)attTarget;

				targi = (un->i * 64 + (un->nexti - un->i) * un->anim);
				targj = (un->j * 64 + (un->nextj - un->j) * un->anim);
			}
		}
		if (pow(i - targi - 32, 2) + pow(j - targj - 32, 2) <= speed * speed) {
			statys = 10;
			if (objMap[static_cast<int>(this->i / 64)][static_cast<int>(this->j / 64)] != NULL)
			if (!objMap[static_cast<int>(this->i / 64)][static_cast<int>(this->j / 64)]->isTree()) {
				if (!objMap[static_cast<int>(this->i / 64)][static_cast<int>(this->j / 64)]->isUnit()) {
					if (static_cast<build*>(objMap[static_cast<int>(this->i / 64)][static_cast<int>(this->j / 64)])->getType() != 2) {
						output->sendActivity(this->i / 64, this->j / 64, team, MS_GAM_HIT, dmg);
						return;
					}
				}
			}
			for (int i = (this->i / 64) - 1; i <= (this->i / 64) + 1; i++) {
				for (int j = (this->j / 64) - 1; j <= (this->j / 64) + 1; j++) {
					if (objMap[i][j] != NULL && i >= 0 && i < 128 && j >= 0 && j < 128) {
							if (i == this->i && j == this->j)
								output->sendActivity(i, j, team, MS_GAM_HIT, dmg);
							else if (!(!objMap[i][j]->isTree() && objMap[i][j]->getTeam() == -1))
									output->sendActivity(i, j, team, MS_GAM_HIT, dmg / 2);
						
					}
				}
			}
		}
		else {
			float rotation = atan2(-i + targi + 32, -j + targj + 32);
			i += sin(rotation) * speed;
			j += cos(rotation) * speed;
		}

	}

	void draw(sf::RenderWindow* win, int pointx, int pointy) override {
		int windowx = j - pointx;
		int windowy = i - pointy;
		if (windowx >= -WIN_WIDTH && windowx <= 128 * SPRSIZE) {
			if (windowy >= -WIN_HEIGHT && windowy <= 128 * SPRSIZE) {
				sf::Sprite spr;
				spr.setPosition(windowx, windowy);
				spr.setTexture(missles[2]);
				spr.setTextureRect(sf::IntRect(0, (anim / 16) * 32, 32, 32));
				spr.setOrigin(missles[2].getSize().x / 2, missles[2].getSize().y / (2 * 4));
				win->draw(spr);
			}
		}
	}

	~missile() override {  };
};

class bullet : public missle {
public:

	bullet(int i, int j, int team, object* attTarget, int speed, int dmg) {
		this->i = i * 64 + 32;
		this->j = j * 64 + 32;
		this->attTarget = attTarget;
		targi = attTarget->getI() * 64;
		targj = attTarget->j * 64;
		statys = 1;
		this->dmg = dmg;
		this->speed = speed;
		anim = 0;
		statys = 0;
		this->team = team;
	}

	void fly() override {
		if (attTarget != NULL) {
			if (attTarget->statys >= 10 || attTarget->hp <= 0) {
				attTarget = NULL;
			}
			else if (attTarget->isUnit()) {
				unit* un = (unit*)attTarget;

				targi = (un->i * 64 + (un->nexti - un->i) * un->anim);
				targj = (un->j * 64 + (un->nextj - un->j) * un->anim);
			}
		}
		if (pow(i - targi - 32, 2) + pow(j - targj - 32, 2) <= speed * speed) {
			statys = 10;
			if (attTarget != NULL) {
				output->sendActivity(attTarget->getI(), attTarget->getJ(), team, MS_GAM_HIT, dmg);
			}

		}
		else {
			float rotation = atan2(-i + targi + 32, -j + targj + 32);
			i += sin(rotation) * speed;
			j += cos(rotation) * speed;
		}

	}

	void draw(sf::RenderWindow* win, int pointx, int pointy) override {
		int windowx = j - pointx;
		int windowy = i - pointy;
		if (windowx >= -WIN_WIDTH && windowx <= 128 * SPRSIZE) {
			if (windowy >= -WIN_HEIGHT && windowy <= 128 * SPRSIZE) {
				sf::Sprite spr;
				spr.setPosition(windowx, windowy);
				spr.setTexture(missles[3]);
				spr.setOrigin(missles[3].getSize().x / 2, missles[3].getSize().y / 2);
				win->draw(spr);
			}
		}
	}

	~bullet() override {  };
};

class archer :public unit {
public:
	void die() override {
		teams[team]->sendMessage(3, this);
		unit::die();
	}

	void dealdamage(int attack) override {
		misl.push_back(new arrow(i, j, team, attTarget, 8, attackType * 1000 + attack));
	}

	void gotdmg(int damag) override {
		TYPE_ATTACK attackType = static_cast<TYPE_ATTACK>(damag / 1000);
		float attack = damag % 1000;
		float damageScale = 1.f - (armor * 0.06 / (1.f + armor * 0.06));
		hp -= attack * damageScale * getAttackArmorScale(attackType, armorType);
		if (hp <= 0) {
			hp = 1;
			objMap[this->i][this->j] = NULL;
			if (animstatys == 1)
				objMap[nexti][nextj] = NULL;
			animstatys = 9;
			statys = 10;
			anim = 0;
			teams[team]->sendMessage(3, this);
		}
	}

	int getType() override {
		return 2;
	}

	archer(int i, int j, int team) {
		this->i = i;
		this->j = j;
		this->team = team;
		teams[team]->sendMessage(2, this);
		nexti = i;
		nextj = j;
		attackrange = 4;
		hp = 245;
		maxhp = hp;
		width = 1;
		height = 1;
		onGround = 1;
		nexti = i;
		nextj = j;
		movespeed = 64;
		attack = 26;
		maxattdel = 96;
		armor = 6;
		armorType = ARMOR_MEDIUM;
		attackType = ATTACK_RANGE;
	}

	void draw(sf::RenderWindow* win, int pointx, int pointy) override {
		int windowx = j * SPRSIZE - pointx;
		int windowy = i * SPRSIZE - pointy;

		if (windowx >= -WIN_WIDTH && windowx <= 128 * SPRSIZE) {
			if (windowy >= -WIN_HEIGHT && windowy <= 128 * SPRSIZE) {
				sf::Sprite spr;
				spr.setPosition(windowx + (nextj - j) * (float)(anim) / (float)(64) * movespeed, windowy + (nexti - i) * (float)(anim) / (float)(64) * movespeed);

				spr.setTexture(textures[0][2]);
				spr.setTextureRect(sf::IntRect((direction - 1) * 64, (animstatys) * 64 + ((anim * int(movespeed) / 64) / 16) * 64, 64, 64));

				win->draw(spr);

				sf::RectangleShape rec;
				rec.setFillColor(sf::Color(0, 0, 0, 64));
				rec.setPosition(4 + windowx + (nextj - j) * (float)(anim) / (float)(64) * movespeed, 4 + windowy + (nexti - i) * (float)(anim) / (float)(64) * movespeed);
				rec.setSize(sf::Vector2f(width * 64 - 8, 10));
				win->draw(rec);

				rec.setFillColor(teamcolors[team]);
				rec.setPosition(4 + windowx + (nextj - j) * (float)(anim) / (float)(64) * movespeed, 4 + windowy + (nexti - i) * (float)(anim) / (float)(64) * movespeed);
				rec.setSize(sf::Vector2f(int(float(hp) / float(maxhp) * (width * 64 - 8)), 10));

				win->draw(rec);
			}
		}
	}

	~archer() override {};
};

class footman :public unit {
public:
	void die() override {
		teams[team]->sendMessage(3, this);
		unit::die();
	}

	void gotdmg(int damag) override {
		TYPE_ATTACK attackType = static_cast<TYPE_ATTACK>(damag / 1000);
		float attack = damag % 1000;
		float damageScale = 1.f - (armor * 0.06 / (1.f + armor * 0.06));
		hp -= attack * damageScale * getAttackArmorScale(attackType, armorType);
		if (hp <= 0) {
			hp = 1;
			objMap[this->i][this->j] = NULL;
			if (animstatys == 1)
				objMap[nexti][nextj] = NULL;
			animstatys = 9;
			statys = 10;
			anim = 0;
			teams[team]->sendMessage(3, this);
		}
	}


	int getType() override {
		return 1;
	}

	footman(int i, int j, int team) {
		this->i = i;
		this->j = j;
		this->team = team;
		teams[team]->sendMessage(2, this);
		attack = 17;
		attackrange = 1;
		hp = 420;
		movespeed = 64;
		maxhp = hp;
		width = 1;
		height = 1;
		onGround = 1;
		nexti = i;
		nextj = j;
		maxattdel = 86;
		armor = 8;
		armorType = ARMOR_MEDIUM;
		attackType = ATTACK_MELEE;
	}

	void draw(sf::RenderWindow* win, int pointx, int pointy) override {
		int windowx = j * SPRSIZE - pointx;
		int windowy = i * SPRSIZE - pointy;

		if (windowx >= -WIN_WIDTH && windowx <= 128 * SPRSIZE) {
			if (windowy >= -WIN_HEIGHT && windowy <= 128 * SPRSIZE) {
				sf::Sprite spr;
				spr.setPosition(windowx + (nextj - j) * (float)(anim) / (float)(64) * movespeed, windowy + (nexti - i) * (float)(anim) / (float)(64) * movespeed);

				spr.setTexture(textures[0][1]);
				spr.setTextureRect(sf::IntRect((direction - 1) * 64, (animstatys) * 64 + ((anim / (64 / int(movespeed))) / 16) * 64, 64, 64));

				win->draw(spr);

				sf::RectangleShape rec;
				rec.setFillColor(sf::Color(0, 0, 0, 64));
				rec.setPosition(4 + windowx + (nextj - j) * (float)(anim) / (float)(64) * movespeed, 4 + windowy + (nexti - i) * (float)(anim) / (float)(64) * movespeed);
				rec.setSize(sf::Vector2f(width * 64 - 8, 10));
				win->draw(rec);

				rec.setFillColor(teamcolors[team]);
				rec.setPosition(4 + windowx + (nextj - j) * (float)(anim) / (float)(64) * movespeed, 4 + windowy + (nexti - i) * (float)(anim) / (float)(64) * movespeed);
				rec.setSize(sf::Vector2f(int(float(hp) / float(maxhp) * (width * 64 - 8)), 10));

				win->draw(rec);
			}
		}
	}

	~footman() override {};
};

class knight :public unit {
public:
	void die() override {
		teams[team]->sendMessage(3, this);
		unit::die();
	}

	void gotdmg(int damag) override {
		TYPE_ATTACK attackType = static_cast<TYPE_ATTACK>(damag / 1000);
		float attack = damag % 1000;
		float damageScale = 1.f - (armor * 0.06 / (1.f + armor * 0.06));
		hp -= attack * damageScale * getAttackArmorScale(attackType, armorType);
		if (hp <= 0) {
			hp = 1;
			objMap[this->i][this->j] = NULL;
			if (animstatys == 1)
				objMap[nexti][nextj] = NULL;
			animstatys = 9;
			statys = 10;
			anim = 0;
			teams[team]->sendMessage(3, this);
		}
	}

	int getType() override {
		return 3;
	}

	knight(int i, int j, int team) {
		this->i = i;
		this->j = j;
		teams[team]->sendMessage(2, this);
		this->team = team;
		attack = 40;
		attackrange = 1;
		hp = 980;
		movespeed = 92;
		maxhp = hp;
		width = 1;
		height = 1;
		onGround = 1;
		nexti = i;
		nextj = j;
		maxattdel = 89;
		armor = 11;
		armorType = ARMOR_HEAVY;
		attackType = ATTACK_MELEE;
	}

	void draw(sf::RenderWindow* win, int pointx, int pointy) override {
		int windowx = j * SPRSIZE - pointx;
		int windowy = i * SPRSIZE - pointy;

		if (windowx >= -WIN_WIDTH && windowx <= 128 * SPRSIZE) {
			if (windowy >= -WIN_HEIGHT && windowy <= 128 * SPRSIZE) {
				sf::Sprite spr;
				spr.setPosition(windowx + (nextj - j) * (float)(anim) / (float)(64) * movespeed, windowy + (nexti - i) * (float)(anim) / (float)(64) * movespeed);

				spr.setTexture(textures[0][3]);
				spr.setTextureRect(sf::IntRect((direction - 1) * 64, (animstatys) * 64 + ((anim * (int(movespeed) / 64)) / 16) * 64, 64, 64));

				win->draw(spr);

				sf::RectangleShape rec;
				rec.setFillColor(sf::Color(0, 0, 0, 64));
				rec.setPosition(4 + windowx + (nextj - j) * (float)(anim) / (float)(64) * movespeed, 4 + windowy + (nexti - i) * (float)(anim) / (float)(64) * movespeed);
				rec.setSize(sf::Vector2f(width * 64 - 8, 10));
				win->draw(rec);

				rec.setFillColor(teamcolors[team]);
				rec.setPosition(4 + windowx + (nextj - j) * (float)(anim) / (float)(64) * movespeed, 4 + windowy + (nexti - i) * (float)(anim) / (float)(64) * movespeed);
				rec.setSize(sf::Vector2f(int(float(hp) / float(maxhp) * (width * 64 - 8)), 10));

				win->draw(rec);
			}
		}
	}

	~knight() override {};
};

class mage :public unit {
public:
	void die() override {
		teams[team]->sendMessage(3, this);
		unit::die();
	}

	void dealdamage(int attack) override {
		misl.push_back(new light(i, j, team, attTarget, 3, attackType * 1000 + attack));
	}

	void gotdmg(int damag) override {
		TYPE_ATTACK attackType = static_cast<TYPE_ATTACK>(damag / 1000);
		float attack = damag % 1000;
		float damageScale = 1.f - (armor * 0.06 / (1.f + armor * 0.06));
		hp -= attack * damageScale * getAttackArmorScale(attackType, armorType);
		if (hp <= 0) {
			hp = 1;
			objMap[this->i][this->j] = NULL;
			if (animstatys == 1)
				objMap[nexti][nextj] = NULL;
			animstatys = 9;
			statys = 10;
			anim = 0;
			teams[team]->sendMessage(3, this);
		}
	}

	int getType() override {
		return 4;
	}

	mage(int i, int j, int team) {
		this->i = i;
		this->j = j;
		this->team = team;
		teams[team]->sendMessage(2, this);
		attack = 26;
		attackrange = 6;
		hp = 630;
		movespeed = 64;
		maxhp = hp;
		width = 1;
		height = 1;
		onGround = 1;
		nexti = i;
		nextj = j;
		maxattdel = 70;
		armor = 3;
		armorType = ARMOR_NO;
		attackType = ATTACK_MAGIC;
	}

	void draw(sf::RenderWindow* win, int pointx, int pointy) override {
		int windowx = j * SPRSIZE - pointx;
		int windowy = i * SPRSIZE - pointy;

		if (windowx >= -WIN_WIDTH && windowx <= 128 * SPRSIZE) {
			if (windowy >= -WIN_HEIGHT && windowy <= 128 * SPRSIZE) {
				sf::Sprite spr;
				spr.setPosition(windowx + (nextj - j) * (float)(anim) / (float)(64) * movespeed, windowy + (nexti - i) * (float)(anim) / (float)(64) * movespeed);

				spr.setTexture(textures[0][4]);
				spr.setTextureRect(sf::IntRect((direction - 1) * 64, (animstatys) * 64 + ((anim / (64 / int(movespeed))) / 16) * 64, 64, 64));

				win->draw(spr);

				sf::RectangleShape rec;
				rec.setFillColor(sf::Color(0, 0, 0, 64));
				rec.setPosition(4 + windowx + (nextj - j) * (float)(anim) / (float)(64) * movespeed, 4 + windowy + (nexti - i) * (float)(anim) / (float)(64) * movespeed);
				rec.setSize(sf::Vector2f(width * 64 - 8, 10));
				win->draw(rec);

				rec.setFillColor(teamcolors[team]);
				rec.setPosition(4 + windowx + (nextj - j) * (float)(anim) / (float)(64) * movespeed, 4 + windowy + (nexti - i) * (float)(anim) / (float)(64) * movespeed);
				rec.setSize(sf::Vector2f(int(float(hp) / float(maxhp) * (width * 64 - 8)), 10));

				win->draw(rec);
			}
		}
	}

	~mage() override {};
};

class catapult :public unit {
public:
	void die() override {
		teams[team]->sendMessage(3, this);
		unit::die();
	}

	void dealdamage(int attack) override {
		misl.push_back(new missile(i, j, team, attTarget, 2, attackType * 1000 + attack));
	}

	void gotdmg(int damag) override {
		TYPE_ATTACK attackType = static_cast<TYPE_ATTACK>(damag / 1000);
		float attack = damag % 1000;
		float damageScale = 1.f - (armor * 0.06 / (1.f + armor * 0.06));
		hp -= attack * damageScale * getAttackArmorScale(attackType, armorType);
		if (hp <= 0) {
			hp = 1;
			objMap[this->i][this->j] = NULL;
			if (animstatys == 1)
				objMap[nexti][nextj] = NULL;
			animstatys = 9;
			statys = 10;
			anim = 0;
			teams[team]->sendMessage(3, this);
		}
	}

	int getType() override {
		return 5;
	}

	catapult(int i, int j, int team) {
		this->i = i;
		this->j = j;
		this->team = team;
		teams[team]->sendMessage(2, this);
		attack = 108;
		attackrange = 12;
		hp = 450;
		movespeed = 48;
		maxhp = hp;
		width = 1;
		height = 1;
		onGround = 1;
		nexti = i;
		nextj = j;
		maxattdel = 288;
		armor = 8;
		armorType = ARMOR_HEAVY;
		attackType = ATTACK_SIEGE;
	}

	void draw(sf::RenderWindow* win, int pointx, int pointy) override {
		int windowx = j * SPRSIZE - pointx;
		int windowy = i * SPRSIZE - pointy;

		if (windowx >= -WIN_WIDTH && windowx <= 128 * SPRSIZE) {
			if (windowy >= -WIN_HEIGHT && windowy <= 128 * SPRSIZE) {
				sf::Sprite spr;
				spr.setTexture(textures[0][5]);
				spr.setPosition(windowx + (nextj - j) * (float)(anim) / (float)(64) * movespeed, windowy + (nexti - i) * (float)(anim) / (float)(64) * movespeed);
				if (animstatys == 5)
					spr.setTextureRect(sf::IntRect((direction - 1) * 64, ((anim / 16) * 64), 64, 64));
				else spr.setTextureRect(sf::IntRect((direction - 1) * 64, 0, 64, 64));
				win->draw(spr);

				sf::RectangleShape rec;
				rec.setFillColor(sf::Color(0, 0, 0, 64));
				rec.setPosition(4 + windowx + (nextj - j) * (float)(anim) / (float)(64) * movespeed, 4 + windowy + (nexti - i) * (float)(anim) / (float)(64) * movespeed);
				rec.setSize(sf::Vector2f(width * 64 - 8, 10));
				win->draw(rec);

				rec.setFillColor(teamcolors[team]);
				rec.setPosition(4 + windowx + (nextj - j) * (float)(anim) / (float)(64) * movespeed, 4 + windowy + (nexti - i) * (float)(anim) / (float)(64) * movespeed);
				rec.setSize(sf::Vector2f(int(float(hp) / float(maxhp) * (width * 64 - 8)), 10));

				win->draw(rec);
			}
		}
	}

	~catapult() override {};
};

class flymachine :public unit {
private:

	int animvint;

public:
	void die() override {
		teams[team]->sendMessage(3, this);
		unit::die();
	}

	void dealdamage(int attack) override {
		misl.push_back(new bullet(i, j, team, attTarget, 10, attackType * 1000 + attack));
	}

	void gotdmg(int damag) override {
		TYPE_ATTACK attackType = static_cast<TYPE_ATTACK>(damag / 1000);
		float attack = damag % 1000;
		float damageScale = 1.f - (armor * 0.06 / (1.f + armor * 0.06));
		hp -= attack * damageScale * getAttackArmorScale(attackType, armorType);
		if (hp <= 0) {
			hp = 1;
			objMap[this->i][this->j] = NULL;
			if (animstatys == 1)
				objMap[nexti][nextj] = NULL;
			animstatys = 9;
			statys = 10;
			anim = 0;
			teams[team]->sendMessage(3, this);
		}
	}

	int getType() override {
		return 9;
	}

	flymachine(int i, int j, int team) {
		this->i = i;
		this->j = j;
		this->team = team;
		teams[team]->sendMessage(2, this);
		attack = 17;
		attackrange = 3;
		hp = 400;
		movespeed = 128;
		maxhp = hp;
		width = 1;
		height = 1;
		onGround = 1;
		nexti = i;
		nextj = j;
		animvint = 0;
		maxattdel = 64;
		armor = 14;
		armorType = ARMOR_MEDIUM;
		attackType = ATTACK_SIEGE;
	}

	void doSmth() override {
		animvint = 1 - animvint;
		unit::doSmth();
	}

	void draw(sf::RenderWindow* win, int pointx, int pointy) override {
		int windowx = j * SPRSIZE - pointx;
		int windowy = i * SPRSIZE - pointy;

		if (windowx >= -WIN_WIDTH && windowx <= 128 * SPRSIZE) {
			if (windowy >= -WIN_HEIGHT && windowy <= 128 * SPRSIZE) {
				sf::Sprite spr;
				spr.setTexture(textures[0][9]);
				spr.setPosition(windowx + (nextj - j) * (float)(anim) / (float)(64) * movespeed, windowy + (nexti - i) * (float)(anim) / (float)(64) * movespeed);
				if (animstatys == 9)
					spr.setTextureRect(sf::IntRect((direction - 1) * 64, ((anim / 16) * 64), 64, 64));
				else spr.setTextureRect(sf::IntRect((direction - 1) * 64, (1 - animvint) * 64, 64, 64));
				win->draw(spr);

				sf::RectangleShape rec;
				rec.setFillColor(sf::Color(0, 0, 0, 64));
				rec.setPosition(4 + windowx + (nextj - j) * (float)(anim) / (float)(64) * movespeed, 4 + windowy + (nexti - i) * (float)(anim) / (float)(64) * movespeed);
				rec.setSize(sf::Vector2f(width * 64 - 8, 10));
				win->draw(rec);

				rec.setFillColor(teamcolors[team]);
				rec.setPosition(4 + windowx + (nextj - j) * (float)(anim) / (float)(64) * movespeed, 4 + windowy + (nexti - i) * (float)(anim) / (float)(64) * movespeed);
				rec.setSize(sf::Vector2f(int(float(hp) / float(maxhp) * (width * 64 - 8)), 10));

				win->draw(rec);
			}
		}
	}

	~flymachine() override {};
};

class dragon :public unit {
private:

	int animvint;

public:
	void die() override {
		teams[team]->sendMessage(3, this);
		unit::die();
	}

	void dealdamage(int attack) override {
		misl.push_back(new missile(i, j, team, attTarget, 2, attackType * 1000 + attack));
	}

	void gotdmg(int damag) override {
		TYPE_ATTACK attackType = static_cast<TYPE_ATTACK>(damag / 1000);
		float attack = damag % 1000;
		float damageScale = 1.f - (armor * 0.06 / (1.f + armor * 0.06));
		hp -= attack * damageScale * getAttackArmorScale(attackType, armorType);
		if (hp <= 0) {
			hp = 1;
			objMap[this->i][this->j] = NULL;
			if (animstatys == 1)
				objMap[nexti][nextj] = NULL;
			animstatys = 9;
			statys = 10;
			anim = 0;
			teams[team]->sendMessage(3, this);
		}
	}

	int getType() override {
		return 7;
	}

	dragon(int i, int j, int team) {
		this->i = i;
		this->j = j;
		this->team = team;
		teams[team]->sendMessage(2, this);
		maxattdel = 192;
		attack = 113;
		attackrange = 3;
		hp = 1350;
		movespeed = 48;
		maxhp = hp;
		width = 1;
		height = 1;
		onGround = 1;
		nexti = i;
		nextj = j;
		animvint = 0;
		attackType = ATTACK_CHAOS;
		armor = 7;
		armorType = ARMOR_LIGHT;
	}

	void doSmth() override {
		animvint++;
		if (animvint + 1 > 12 * 4) animvint = 0;
		unit::doSmth();
	}

	void draw(sf::RenderWindow* win, int pointx, int pointy) override {
		int windowx = j * SPRSIZE - pointx;
		int windowy = i * SPRSIZE - pointy;

		if (windowx >= -WIN_WIDTH && windowx <= 128 * SPRSIZE) {
			if (windowy >= -WIN_HEIGHT && windowy <= 128 * SPRSIZE) {
				sf::Sprite spr;
				spr.setTexture(textures[0][7]);
				spr.setPosition(windowx + (nextj - j) * (float)(anim) / (float)(64) * movespeed, windowy + (nexti - i) * (float)(anim) / (float)(64) * movespeed);
				switch (animstatys) {
				case 5:
					spr.setTextureRect(sf::IntRect((direction - 1) * 64, 64 * (1 + anim / 16), 64, 64));
					break;
				case 9:
					spr.setTextureRect(sf::IntRect((direction - 1) * 64, 5 * 64 + (anim / 16) * 64, 64, 64));
					break;
				default:
					spr.setTextureRect(sf::IntRect((direction - 1) * 64, (animvint / 12) * 64, 64, 64));
					break;
				}
				win->draw(spr);

				sf::RectangleShape rec;
				rec.setFillColor(sf::Color(0, 0, 0, 64));
				rec.setPosition(4 + windowx + (nextj - j) * (float)(anim) / (float)(64) * movespeed, 4 + windowy + (nexti - i) * (float)(anim) / (float)(64) * movespeed);
				rec.setSize(sf::Vector2f(width * 64 - 8, 10));
				win->draw(rec);

				rec.setFillColor(teamcolors[team]);
				rec.setPosition(4 + windowx + (nextj - j) * (float)(anim) / (float)(64) * movespeed, 4 + windowy + (nexti - i) * (float)(anim) / (float)(64) * movespeed);
				rec.setSize(sf::Vector2f(int(float(hp) / float(maxhp) * (width * 64 - 8)), 10));

				win->draw(rec);
			}
		}
	}

	~dragon() override {};
};

class earthknight :public unit {
private:

	int healcd;

public:
	void die() override {
		teams[team]->sendMessage(3, this);
		unit::die();
	}

	void gotdmg(int damag) override {
		TYPE_ATTACK attackType = static_cast<TYPE_ATTACK>(damag / 1000);
		float attack = damag % 1000;
		float damageScale = 1.f - (armor * 0.06 / (1.f + armor * 0.06));
		hp -= attack * damageScale * getAttackArmorScale(attackType, armorType);
		if (hp <= 0) {
			hp = 1;
			objMap[this->i][this->j] = NULL;
			if (animstatys == 1)
				objMap[nexti][nextj] = NULL;
			animstatys = 9;
			statys = 10;
			anim = 0;
			teams[team]->sendMessage(3, this);
		}
	}

	int getType() override {
		return 6;
	}

	void doSmth() override {
		healcd++;
		if (healcd > 64) {
			for (int i = this->i - 1; i <= this->i + 1; i++) {
				for (int j = this->j - 1; j <= this->j + 1; j++) {
					if (objMap[i][j] != NULL)
						if (objMap[i][j]->isUnit() && objMap[i][j]->getTeam() == this->team) {
							output->sendActivity(i, j, myteam, MS_GAM_HIT, -2);
						}
				}
			}
			healcd = 0;
		}
		unit::doSmth();
	}

	earthknight(int i, int j, int team) {
		this->i = i;
		this->j = j;
		this->team = team;
		teams[team]->sendMessage(2, this);
		maxattdel = 116;
		attack = 54;
		attackrange = 1;
		hp = 1275;
		movespeed = 96;
		maxhp = hp;
		width = 1;
		height = 1;
		onGround = 1;
		nexti = i;
		nextj = j;
		healcd = 0;
		attackType = ATTACK_MAGIC;
	}

	void draw(sf::RenderWindow* win, int pointx, int pointy) override {
		int windowx = j * SPRSIZE - pointx;
		int windowy = i * SPRSIZE - pointy;

		if (windowx >= -WIN_WIDTH && windowx <= 128 * SPRSIZE) {
			if (windowy >= -WIN_HEIGHT && windowy <= 128 * SPRSIZE) {
				sf::Sprite spr;
				spr.setPosition(windowx + (nextj - j) * (float)(anim) / (float)(64) * movespeed, windowy + (nexti - i) * (float)(anim) / (float)(64) * movespeed);

				spr.setTexture(textures[0][6]);
				spr.setTextureRect(sf::IntRect((direction - 1) * 64, (animstatys) * 64 + ((anim * (int(movespeed) / 64)) / 16) * 64, 64, 64));

				win->draw(spr);

				sf::RectangleShape rec;
				rec.setFillColor(sf::Color(0, 0, 0, 64));
				rec.setPosition(4 + windowx + (nextj - j) * (float)(anim) / (float)(64) * movespeed, 4 + windowy + (nexti - i) * (float)(anim) / (float)(64) * movespeed);
				rec.setSize(sf::Vector2f(width * 64 - 8, 10));
				win->draw(rec);

				rec.setFillColor(teamcolors[team]);
				rec.setPosition(4 + windowx + (nextj - j) * (float)(anim) / (float)(64) * movespeed, 4 + windowy + (nexti - i) * (float)(anim) / (float)(64) * movespeed);
				rec.setSize(sf::Vector2f(int(float(hp) / float(maxhp) * (width * 64 - 8)), 10));

				win->draw(rec);
			}
		}
	}

	~earthknight() override {};
};

class gryphon :public unit {
private:

	int animvint;

public:
	void die() override {
		teams[team]->sendMessage(3, this);
		unit::die();
	}

	void dealdamage(int attack) override {
		misl.push_back(new light(i, j, team, attTarget, 5, attackType * 1000 + attack));
	}

	void gotdmg(int damag) override {
		TYPE_ATTACK attackType = static_cast<TYPE_ATTACK>(damag / 1000);
		float attack = damag % 1000;
		float damageScale = 1.f - (armor * 0.06 / (1.f + armor * 0.06));
		hp -= attack * damageScale * getAttackArmorScale(attackType, armorType);
		if (hp <= 0) {
			hp = 1;
			objMap[this->i][this->j] = NULL;
			if (animstatys == 1)
				objMap[nexti][nextj] = NULL;
			animstatys = 9;
			statys = 10;
			anim = 0;
			teams[team]->sendMessage(3, this);
		}
	}

	int getType() override {
		return 8;
	}

	gryphon(int i, int j, int team) {
		this->i = i;
		this->j = j;
		this->team = team;
		teams[team]->sendMessage(2, this);
		maxattdel = 153;
		attack = 65;
		attackrange = 5;
		hp = 975;
		movespeed = 96;
		maxhp = hp;
		width = 1;
		height = 1;
		onGround = 1;
		nexti = i;
		nextj = j;
		animvint = 0;
		armor = 6;
		armorType = ARMOR_LIGHT;
		attackType = ATTACK_MAGIC;
	}

	void doSmth() override {
		animvint++;
		if (animvint + 1 > 14 * 5) animvint = 0;
		unit::doSmth();
	}

	void draw(sf::RenderWindow* win, int pointx, int pointy) override {
		int windowx = j * SPRSIZE - pointx;
		int windowy = i * SPRSIZE - pointy;

		if (windowx >= -WIN_WIDTH && windowx <= 128 * SPRSIZE) {
			if (windowy >= -WIN_HEIGHT && windowy <= 128 * SPRSIZE) {
				sf::Sprite spr;
				spr.setTexture(textures[0][8]);
				spr.setPosition(windowx + (nextj - j) * (float)(anim) / (float)(64) * movespeed, windowy + (nexti - i) * (float)(anim) / (float)(64) * movespeed);
				switch (animstatys) {
				case 5:
					if (anim < 32)
						spr.setTextureRect(sf::IntRect((direction - 1) * 64, (anim / 16) * 64, 64, 64));
					else spr.setTextureRect(sf::IntRect((direction - 1) * 64, 3 * 64 + (anim / 16) * 64, 64, 64));
					break;
				case 9:
					spr.setTextureRect(sf::IntRect((direction - 1) * 64, 8 * 64 + (anim / 16) * 64, 64, 64));
					break;
				default:
					spr.setTextureRect(sf::IntRect((direction - 1) * 64, (animvint / 14) * 64, 64, 64));
					break;
				}
				win->draw(spr);

				sf::RectangleShape rec;
				rec.setFillColor(sf::Color(0, 0, 0, 64));
				rec.setPosition(4 + windowx + (nextj - j) * (float)(anim) / (float)(64) * movespeed, 4 + windowy + (nexti - i) * (float)(anim) / (float)(64) * movespeed);
				rec.setSize(sf::Vector2f(width * 64 - 8, 10));
				win->draw(rec);

				rec.setFillColor(teamcolors[team]);
				rec.setPosition(4 + windowx + (nextj - j) * (float)(anim) / (float)(64) * movespeed, 4 + windowy + (nexti - i) * (float)(anim) / (float)(64) * movespeed);
				rec.setSize(sf::Vector2f(int(float(hp) / float(maxhp) * (width * 64 - 8)), 10));

				win->draw(rec);
			}
		}
	}

	~gryphon() override {};
};

class barracks :public build {
	struct que {
		int type;
	};
	std::vector<que> queue;

public:
	void getComm(int t) override {
		if (t / 10 == 0) {
			if (queue.size() < 5) {
				if (teams[team]->getWood() >= costWood[t] && teams[team]->getGold() >= costGold[t]) {
					que node;
					node.type = t;
					queue.push_back(node);
				}
			}
		}
		else {
			t %= 10;
			if (t < queue.size()) {
				queue.erase(queue.begin() + t);
				if (t == 0) {
					timer = 0;
				}
			}
		}
	}

	int getSizeQ() override {
		return queue.size();
	}

	int getTypeQ(int i) override {
		if (i >= queue.size()) return -1;
		else return queue[i].type;
	}

	int getType() override {
		return 1;
	}

	void doSmth() override {
		switch (statys) {
		case 0:
			if (queue.size() > 0) {
				if (teams[team]->getFood() + needFood[queue[0].type] <= teams[team]->getFoodLimit())
					timer++;
				if (timer >= maxtimer) {
					int typ = queue[0].type;
					int r = 2;
					int nI = this->i;
					int nJ = this->j;
					while (objMap[nI][nJ] != NULL) {
						for (int i = -r; i <= r; i++) {
							if (objMap[this->i + i][this->j - r] == NULL) {
								if (objMap[nI][nJ] != NULL) {
									nI = this->i + i;
									nJ = this->j - r;
								}
								else if (pow(pointI - (this->i + i), 2) + pow(pointJ - (this->j - r), 2) < pow(pointI - nI, 2) + pow(pointJ - nJ, 2)) {
									nI = this->i + i;
									nJ = this->j - r;
								}
							}
							if (objMap[this->i + i][this->j + r] == NULL) {
								if (objMap[nI][nJ] != NULL) {
									nI = this->i + i;
									nJ = this->j + r;
								}
								else if (pow(pointI - (this->i + i), 2) + pow(pointJ - (this->j + r), 2) < pow(pointI - nI, 2) + pow(pointJ - nJ, 2)) {
									nI = this->i + i;
									nJ = this->j + r;
								}
							}
							if (objMap[this->i - r][this->j + i] == NULL) {
								if (objMap[nI][nJ] != NULL) {
									nI = this->i - r;
									nJ = this->j + i;
								}
								else if (pow(pointI - (this->i - r), 2) + pow(pointJ - (this->j + i), 2) < pow(pointI - nI, 2) + pow(pointJ - nJ, 2)) {
									nI = this->i - r;
									nJ = this->j + i;
								}
							}
							if (objMap[this->i + r][this->j + i] == NULL) {
								if (objMap[nI][nJ] != NULL) {
									nI = this->i + r;
									nJ = this->j + i;
								}
								else if (pow(pointI - (this->i + r), 2) + pow(pointJ - (this->j + i), 2) < pow(pointI - nI, 2) + pow(pointJ - nJ, 2)) {
									nI = this->i + r;
									nJ = this->j + i;
								}
							}
						}
						r++;
					}
					if (teams[team]->getWood() >= costWood[typ] && teams[team]->getGold() >= costGold[typ]) {
						output->sendActivity(nI, nJ, team, MS_GAM_CREATE, typ);
					}
					timer = 0;
					queue.erase(queue.begin());
				}
			}
			else {
				timer = 0;
			}
			break;
		case 1:
			if (hp >= 0) {
				if (hp >= maxhp) {
					statys = 0;
					hp = maxhp;
					armor = 5;
					armorType = ARMOR_BUILD;
				}
			}
			break;
		}
	}

	barracks(int i, int j, int team) {
		armor = 0;
		armorType = ARMOR_HEAVY;
		teams[team]->sendMessage(0, this);
		this->i = i;
		this->j = j;
		this->team = team;
		width = 3;
		height = 3;
		for (int i = -height / 2; i <= height / 2; i++) {
			for (int j = -width / 2; j <= width / 2; j++) {
				objMap[i + this->i][j + this->j] = this;
			}
		}
		hp = 1;
		maxhp = 1400;
		timer = 0;
		maxtimer = 1 * 64;
		pointI = this->i;
		pointJ = this->j;
	}

	void draw(sf::RenderWindow* win, int pointx, int pointy) override {
		int windowx = j * SPRSIZE - pointx - (width / 2) * SPRSIZE;
		int windowy = i * SPRSIZE - pointy - (height / 2) * SPRSIZE;

		if (windowx >= -WIN_WIDTH * width && windowx <= 128 * SPRSIZE) {
			if (windowy >= -WIN_HEIGHT * height && windowy <= 128 * SPRSIZE) {
				sf::Sprite spr;
				spr.setPosition(windowx, windowy);

				spr.setTexture(textures[1][2]);
				if (statys == 1) {
					spr.setTextureRect(sf::IntRect(0, 64 * height, 64 * width, 64 * height));
				}
				else spr.setTextureRect(sf::IntRect(0, 0, 64 * width, 64 * height));

				win->draw(spr);

				build::drawHp(win, pointx, pointy);
			}
		}
	}

	void die() override {
		teams[team]->sendMessage(1, this);
		statys = 10;
		for (int i = 0; i < allobjs.size(); i++) {
			if (allobjs[i] == this) {
				allobjs.erase(allobjs.begin() + i, allobjs.begin() + i + 1);
				break;
			}
		}
		for (int i = 0; i < doobjs.size(); i++) {
			if (doobjs[i] == this) {
				doobjs.erase(doobjs.begin() + i, doobjs.begin() + i + 1);
				break;
			}
		}
		for (int i = -height / 2; i <= height / 2; i++) {
			for (int j = -width / 2; j <= width / 2; j++) {
				objMap[i + this->i][j + this->j] = NULL;
			}
		}
		hp = -1;
	}

	~barracks() override {};
};

class farm :public build {
public:

	int getSizeQ() override {
		return 0;
	}

	int getTypeQ(int i) override {
		return -1;
	}

	int getType() override {
		return 2;
	}

	void getComm(int t) override {}

	void doSmth() override {
		switch (statys) {
		case 1:
			if (hp >= 0) {
				if (hp >= maxhp) {
					statys = 0;
					hp = maxhp;
					teams[team]->sendMessage(0, this);
					armor = 5;
					armorType = ARMOR_BUILD;
				}
			}
			break;
		}
	}

	farm(int i, int j, int team) {
		armor = 0;
		armorType = ARMOR_HEAVY;
		this->i = i;
		this->j = j;
		this->team = team;
		width = 1;
		height = 1;
		for (int i = -height / 2; i <= height / 2; i++) {
			for (int j = -width / 2; j <= width / 2; j++) {
				objMap[i + this->i][j + this->j] = this;
			}
		}
		hp = 1;
		maxhp = 500;
		pointI = this->i;
		pointJ = this->j;
	}

	void draw(sf::RenderWindow* win, int pointx, int pointy) override {
		int windowx = j * SPRSIZE - pointx - (width / 2) * SPRSIZE;
		int windowy = i * SPRSIZE - pointy - (height / 2) * SPRSIZE;

		if (windowx >= -WIN_WIDTH * width && windowx <= 128 * SPRSIZE) {
			if (windowy >= -WIN_HEIGHT * height && windowy <= 128 * SPRSIZE) {
				sf::Sprite spr;
				spr.setPosition(windowx, windowy);

				spr.setTexture(textures[1][3]);
				if (statys == 1) {
					spr.setTextureRect(sf::IntRect(0, 64 * height, 64 * width, 64 * height));
				}
				else spr.setTextureRect(sf::IntRect(0, 0, 64 * width, 64 * height));

				win->draw(spr);

				build::drawHp(win, pointx, pointy);
			}
		}
	}

	void die() override {
		teams[team]->sendMessage(1, this);
		statys = 10;
		for (int i = 0; i < allobjs.size(); i++) {
			if (allobjs[i] == this) {
				allobjs.erase(allobjs.begin() + i, allobjs.begin() + i + 1);
				break;
			}
		}
		for (int i = 0; i < doobjs.size(); i++) {
			if (doobjs[i] == this) {
				doobjs.erase(doobjs.begin() + i, doobjs.begin() + i + 1);
				break;
			}
		}
		for (int i = -height / 2; i <= height / 2; i++) {
			for (int j = -width / 2; j <= width / 2; j++) {
				objMap[i + this->i][j + this->j] = NULL;
			}
		}
		hp = -1;
	}

	~farm() override {};
};

class factory :public build {
	struct que {
		int type;
	};
	std::vector<que> queue;

public:

	int getSizeQ() override {
		return queue.size();
	}

	int getTypeQ(int i) override {
		if (i >= queue.size()) return -1;
		else return queue[i].type;
	}

	void getComm(int t) override {
		if (t / 10 == 0) {
			if (queue.size() < 5) {
				if (teams[team]->getWood() >= costWood[t] && teams[team]->getGold() >= costGold[t]) {
					que node;
					node.type = t;
					queue.push_back(node);
				}
			}
		}
		else {
			t %= 10;
			if (t < queue.size()) {
				queue.erase(queue.begin() + t);
				if (t == 0) {
					timer = 0;
				}
			}
		}
	}

	int getType() override {
		return 3;
	}

	void doSmth() override {
		switch (statys) {
		case 0:
			if (queue.size() > 0) {
				if (teams[team]->getFood() + needFood[queue[0].type] <= teams[team]->getFoodLimit())
					timer++;
				if (timer >= maxtimer) {
					int typ = queue[0].type;
					int r = 2;
					int nI = this->i;
					int nJ = this->j;
					while (objMap[nI][nJ] != NULL) {
						for (int i = -r; i <= r; i++) {
							if (objMap[this->i + i][this->j - r] == NULL) {
								if (objMap[nI][nJ] != NULL) {
									nI = this->i + i;
									nJ = this->j - r;
								}
								else if (pow(pointI - (this->i + i), 2) + pow(pointJ - (this->j - r), 2) < pow(pointI - nI, 2) + pow(pointJ - nJ, 2)) {
									nI = this->i + i;
									nJ = this->j - r;
								}
							}
							if (objMap[this->i + i][this->j + r] == NULL) {
								if (objMap[nI][nJ] != NULL) {
									nI = this->i + i;
									nJ = this->j + r;
								}
								else if (pow(pointI - (this->i + i), 2) + pow(pointJ - (this->j + r), 2) < pow(pointI - nI, 2) + pow(pointJ - nJ, 2)) {
									nI = this->i + i;
									nJ = this->j + r;
								}
							}
							if (objMap[this->i - r][this->j + i] == NULL) {
								if (objMap[nI][nJ] != NULL) {
									nI = this->i - r;
									nJ = this->j + i;
								}
								else if (pow(pointI - (this->i - r), 2) + pow(pointJ - (this->j + i), 2) < pow(pointI - nI, 2) + pow(pointJ - nJ, 2)) {
									nI = this->i - r;
									nJ = this->j + i;
								}
							}
							if (objMap[this->i + r][this->j + i] == NULL) {
								if (objMap[nI][nJ] != NULL) {
									nI = this->i + r;
									nJ = this->j + i;
								}
								else if (pow(pointI - (this->i + r), 2) + pow(pointJ - (this->j + i), 2) < pow(pointI - nI, 2) + pow(pointJ - nJ, 2)) {
									nI = this->i + r;
									nJ = this->j + i;
								}
							}
						}
						r++;
					}
					if (teams[team]->getWood() >= costWood[typ] && teams[team]->getGold() >= costGold[typ]) {
						output->sendActivity(nI, nJ, team, MS_GAM_CREATE, typ);
					}
					timer = 0;
					queue.erase(queue.begin());
				}
			}
			else {
				timer = 0;
			}
			break;
		case 1:
			if (hp >= 0) {
				if (hp >= maxhp) {
					statys = 0;
					hp = maxhp;
					armor = 5;
					armorType = ARMOR_BUILD;
				}
			}
			break;
		}
	}

	factory(int i, int j, int team) {
		armor = 0;
		armorType = ARMOR_HEAVY;
		teams[team]->sendMessage(0, this);
		this->i = i;
		this->j = j;
		this->team = team;
		width = 3;
		height = 3;
		for (int i = -height / 2; i <= height / 2; i++) {
			for (int j = -width / 2; j <= width / 2; j++) {
				objMap[i + this->i][j + this->j] = this;
			}
		}
		hp = 1;
		maxhp = 1200;
		timer = 0;
		maxtimer = 1 * 64;
		pointI = this->i;
		pointJ = this->j;
	}

	void draw(sf::RenderWindow* win, int pointx, int pointy) override {
		int windowx = j * SPRSIZE - pointx - (width / 2) * SPRSIZE;
		int windowy = i * SPRSIZE - pointy - (height / 2) * SPRSIZE;

		if (windowx >= -WIN_WIDTH * width && windowx <= 128 * SPRSIZE) {
			if (windowy >= -WIN_HEIGHT * height && windowy <= 128 * SPRSIZE) {
				sf::Sprite spr;
				spr.setPosition(windowx, windowy);

				spr.setTexture(textures[1][4]);
				if (statys == 1) {
					spr.setTextureRect(sf::IntRect(0, 64 * height, 64 * width, 64 * height));
				}
				else spr.setTextureRect(sf::IntRect(0, 0, 64 * width, 64 * height));

				win->draw(spr);

				build::drawHp(win, pointx, pointy);
			}
		}
	}

	void die() override {
		teams[team]->sendMessage(1, this);
		statys = 10;
		for (int i = 0; i < allobjs.size(); i++) {
			if (allobjs[i] == this) {
				allobjs.erase(allobjs.begin() + i, allobjs.begin() + i + 1);
				break;
			}
		}
		for (int i = 0; i < doobjs.size(); i++) {
			if (doobjs[i] == this) {
				doobjs.erase(doobjs.begin() + i, doobjs.begin() + i + 1);
				break;
			}
		}
		for (int i = -height / 2; i <= height / 2; i++) {
			for (int j = -width / 2; j <= width / 2; j++) {
				objMap[i + this->i][j + this->j] = NULL;
			}
		}
		hp = -1;
	}

	~factory() override {};
};

class civil :public unit {
	int storage; int typestorage;
	int typebuild;

public:
	int sizes[10][2]; // ?acia?u caaiee, 0 - oe?eia, 1 - aunioa

	void die() override {
		teams[team]->sendMessage(3, this);
		unit::die();
	}

	void gotdmg(int damag) override {
		TYPE_ATTACK attackType = static_cast<TYPE_ATTACK>(damag / 1000);
		float attack = damag % 1000;
		float damageScale = 1.f - (armor * 0.06 / (1.f + armor * 0.06));
		hp -= attack * damageScale * getAttackArmorScale(attackType, armorType);
		if (hp <= 0) {
			hp = 1;
			objMap[this->i][this->j] = NULL;
			if (animstatys == 1)
				objMap[nexti][nextj] = NULL;
			animstatys = 9;
			statys = 10;
			anim = 0;
			teams[team]->sendMessage(3, this);
		}
	}

	int getType() override {
		return 0;
	}

	civil(int i, int j, int team) {
		this->i = i;
		this->j = j;
		this->team = team;
		teams[team]->sendMessage(2, this);
		nexti = i;
		nextj = j;
		attack = 6;
		attackrange = 1;
		hp = 220;
		maxhp = hp;
		width = 1;
		height = 1;
		onGround = 1;
		nexti = i;
		nextj = j;
		movespeed = 64;
		storage = 0;
		typestorage = 0;
		sizes[0][0] = 5;
		sizes[0][1] = 5;
		sizes[1][0] = 3;
		sizes[1][1] = 3;
		sizes[2][0] = 1;
		sizes[2][1] = 1;
		sizes[3][0] = 3;
		sizes[3][1] = 3;
		typebuild = -1;
		maxattdel = 0;
		armor = 0;
		armorType = ARMOR_MEDIUM;
		attackType = ATTACK_CHAOS;
	}

	void doSmth() override {
		if (!waitServer)
			switch (statys) {
			case 0:
				if (typecomm.size() > 0) {
					int comi = commI.front();
					int comj = commJ.front();
					int com = typecomm.front();
					commI.pop_front();
					commJ.pop_front();
					typecomm.pop_front();
					attTarget = NULL;
					switch (com) {
					case 0:
						if (objMap[comi][comj] == NULL) {
							wayI = comi;
							wayJ = comj;
							statys = 1;
						}
						else {
							if (objMap[comi][comj]->isTree()) {
								statys = 6;
							}
							else if (objMap[comi][comj]->getTeam() == -1) {
								statys = 7;
							}
							else {
								if (objMap[comi][comj]->getTeam() == team) {
									if (!objMap[comi][comj]->isUnit() && objMap[comi][comj]->getHp() < objMap[comi][comj]->maxhp) {
										attTarget = objMap[comi][comj];
										statys = 2;
									}
									else {
										wayI = comi;
										wayJ = comj;
										statys = 1;
									}
								}
								else {
									wayI = comi;
									wayJ = comj;
									attTarget = objMap[comi][comj];
									statys = 5;
								}
							}
						}
						break;
					case 1:
						wayI = comi;
						wayJ = comj;
						statys = 1;
						break;
					case 2:
						if (objMap[comi][comj] != NULL && objMap[comi][comj] != this && objMap[comi][comj]->getTeam() != -1) {
							attTarget = objMap[comi][comj];
							wayI = comi;
							wayJ = comj;
							statys = 5;
						}
						else {
							wayI = comi;
							wayJ = comj;
							attackI = comi;
							attackJ = comj;
							statys = 4;
						}
						break;
					case 3: {
						wayI = comi;
						wayJ = comj;
						attackI = comi;
						attackJ = comj;
						statys = 3;
						break;
					}
					case 4:
						if (teams[team]->getWood() < teams[team]->getGold()) {
							statys = 6;
						}
						else statys = 7;
						break;
					default:
						wayI = comi;
						wayJ = comj;
						statys = 8;
						typebuild = com % 10;
						break;
					}
				}
				break;
			case 1: //move
				if (anim == 0) {
					if (i == wayI && wayJ == j) { statys = 0; animstatys = 0; }
					else {
						if (wait > 0) {
							wait--;
							if (wait == 0 && objMap[nexti][nextj] != NULL)
								if (objMap[nexti][nextj]->getStatys() == 0 || (!objMap[nexti][nextj]->isTree() && !objMap[nexti][nextj]->isUnit())) { statys = 0; }
						}
						else {

							if (nexti == wayI && nextj == wayJ) {
								statys = 0;
								animstatys = 0;
								break;
							}
							else {
								if (objMap[nexti][nextj] == NULL || objMap[nexti][nextj] == this)
									createway(wayI, wayJ, 0, 0);
								if (objMap[nexti][nextj] == NULL) {
									objMap[nexti][nextj] = this;
									output->sendActivity(this->i, this->j, team, MS_GAM_MOVE, nexti * 1000 + nextj);
								}

								else {

									int nexti2 = nexti;
									int nextj2 = nextj;

									createway(wayI, wayJ, 1, 0);

									if (objMap[nexti][nextj] == NULL) {
										objMap[nexti][nextj] = this;
										output->sendActivity(this->i, this->j, team, MS_GAM_MOVE, nexti * 1000 + nextj);
									}
									else {

										nexti = nexti2;
										nextj = nextj2;

										wait = 64 * 64 / movespeed;

										animstatys = 0;
									}
								}
							}
						}
					}
				}
				else {
					anim++;

					if (anim >= 64 * 64 / movespeed) {
						objMap[i][j] = NULL;
						i = nexti;
						j = nextj;
						anim = 0;
					}
				}
				break;
			case 2:
				if (attTarget->getHp() >= attTarget->maxhp || attTarget->getStatys() > 9) {

					attTarget = NULL;
					if (anim > 0 && animstatys == 1) {
						statys = 1;
						wayI = nexti;
						wayJ = nextj;
					}
					else {
						statys = 0;
						animstatys = 0;
						anim = 0;
					}
					break;
				}
				else if (animstatys == 5) {
					anim++;
					if (anim % 5 == 0) {
						output->sendActivity(attTarget->getI(), attTarget->getJ(), team, MS_GAM_HIT, -3);
					}
					direction = getdir(attTarget->getI(), attTarget->getJ());
					if (anim >= 63) {
						anim = 0;
						if (attTarget->getHp() >= attTarget->maxhp || attTarget->getStatys() > 9) {
							attTarget = NULL;
							if (anim > 0 && animstatys == 1) {
								statys = 1;
								wayI = nexti;
								wayJ = nextj;
							}
							else {
								statys = 0;
								animstatys = 0;
								anim = 0;
							}
							break;
						}
						else
							if (abs(attTarget->getI() - this->i) <= attackrange + attTarget->height / 2 && abs(attTarget->getJ() - this->j) <= attackrange + attTarget->width / 2) {
								output->sendActivity(this->i, this->j, team, MS_GAM_ATTACK, attTarget->getI() * 1000 + attTarget->getJ());
							}
							else animstatys = 1;
					}
				}
				else {
					if (anim == 0) {
						if (abs(attTarget->getI() - this->i) <= attackrange + attTarget->height / 2 && abs(attTarget->getJ() - this->j) <= attackrange + attTarget->width / 2) {
							animstatys = 5;
							nexti = i;
							nextj = j;
						}
						else
							if (wait > 0) wait--;
							else {
								wayJ = attTarget->getJ();
								wayI = attTarget->getI();
								if (objMap[nexti][nextj] == NULL || objMap[nexti][nextj] == this)
									createway(wayI, wayJ, 0, 0);
								if (objMap[nexti][nextj] == NULL) {
									objMap[nexti][nextj] = this;
									output->sendActivity(this->i, this->j, team, MS_GAM_MOVE, nexti * 1000 + nextj);
								}
								else {
									int nexti2 = nexti;
									int nextj2 = nextj;

									createway(wayI, wayJ, 1, 0);

									if (objMap[nexti][nextj] == NULL) {
										objMap[nexti][nextj] = this;
										output->sendActivity(this->i, this->j, team, MS_GAM_MOVE, nexti * 1000 + nextj);
									}
									else
									{

										nexti = nexti2;
										nextj = nextj2;

										if (!(objMap[nexti][nextj]->getStatys() == 0 || (!objMap[nexti][nextj]->isTree() && !objMap[nexti][nextj]->isUnit()))) { wait = 128 * 64 / movespeed; }
										{
											wait = 64 * 64 / movespeed;
										}
										animstatys = 0;
									}
								}

							}
					}
					else {
						anim++;

						if (anim >= 64 * 64 / movespeed) {
							objMap[i][j] = NULL;
							i = nexti;
							j = nextj;
							anim = 0;
							if (abs(attTarget->getI() - this->i) <= attackrange + attTarget->height / 2 && abs(attTarget->getJ() - this->j) <= attackrange + attTarget->width / 2) {
								output->sendActivity(this->i, this->j, team, MS_GAM_ATTACK, attTarget->getI() * 1000 + attTarget->getJ());
							}
						}
					}
				}
				break;
			case 3:
				if (attTarget != NULL && (attTarget->getHp() <= 0 || attTarget->getStatys() > 9 || attTarget->getStatys() == -1)) {
					attTarget = NULL;
					if (animstatys != 1) {
						animstatys = 1;
						anim = 0;
					}
				}
				if (animstatys == 5) {
					if (attTarget == NULL) {
						animstatys = 1;
						anim = 0;
					}
					else {
						anim++;
						direction = getdir(attTarget->getI(), attTarget->getJ());
						if (anim >= 63) {
							output->sendActivity(attTarget->getI(), attTarget->getJ(), team, MS_GAM_HIT, attack);
							anim = 0;
							if (attTarget->getHp() <= 0 || attTarget->getStatys() > 9 || attTarget->getStatys() == -1) {
								attTarget = NULL;
								animstatys = 1;
								anim = 0;
							}
							else
								if (abs(attTarget->getI() - this->i) <= attackrange + attTarget->height / 2 && abs(attTarget->getJ() - this->j) <= attackrange + attTarget->width / 2) {
									output->sendActivity(this->i, this->j, team, MS_GAM_ATTACK, attTarget->getI() * 1000 + attTarget->getJ());
								}
								else animstatys = 1;
						}
					}
				}
				else
					if (anim == 0) {
						for (int i = this->i - attackrange - 5; i <= this->i + attackrange + 5; i++) {
							for (int j = this->j - attackrange - 5; j <= this->j + attackrange + 5; j++) {
								if (i > 0 && i < 129 && j > 0 && j < 129)
									if (objMap[i][j] != NULL) {
										if (objMap[i][j]->getTeam() != team && objMap[i][j]->getTeam() >= 0) {
											if (attTarget == NULL) {
												attTarget = objMap[i][j];
											}
											else if (pow(objMap[i][j]->getI() - this->i, 2) + pow(objMap[i][j]->getJ() - this->j, 2) < pow(attTarget->getI() - this->i, 2) + pow(attTarget->getJ() - this->j, 2)) {
												attTarget = objMap[i][j];
											}
										}
									}
							}
						}
						if (attTarget != NULL && abs(attTarget->getI() - this->i) <= attackrange + attTarget->height / 2 && abs(attTarget->getJ() - this->j) <= attackrange + attTarget->width / 2) {
							output->sendActivity(this->i, this->j, team, MS_GAM_ATTACK, attTarget->getI() * 1000 + attTarget->getJ());
						}
						else {
							if (attTarget != NULL) { wayI = attTarget->getI(); wayJ = attTarget->getJ(); }
							else { wayI = attackI; wayJ = attackJ; }
							if (wait > 0) {
								wait--;
							}
							else {
								if (objMap[nexti][nextj] == NULL || objMap[nexti][nextj] == this)
									createway(wayI, wayJ, 0, 0);
								if (objMap[nexti][nextj] == NULL) {
									objMap[nexti][nextj] = this;
									output->sendActivity(this->i, this->j, team, MS_GAM_MOVE, nexti * 1000 + nextj);
								}

								else {

									int nexti2 = nexti;
									int nextj2 = nextj;

									createway(wayI, wayJ, 1, 0);

									if (objMap[nexti][nextj] == NULL) {
										objMap[nexti][nextj] = this;
										output->sendActivity(this->i, this->j, team, MS_GAM_MOVE, nexti * 1000 + nextj);
									}
									else {

										nexti = nexti2;
										nextj = nextj2;

										wait = -1 + 64 * 64 / movespeed;

										animstatys = 0;

									}
								}

							}

						}
					}
					else {
						anim++;

						if (anim >= 64 * 64 / movespeed) {
							objMap[i][j] = NULL;
							i = nexti;
							j = nextj;
							anim = 0;
							if (attTarget != NULL && abs(attTarget->getI() - this->i) <= attackrange + attTarget->height / 2 && abs(attTarget->getJ() - this->j) <= attackrange + attTarget->width / 2) {
								output->sendActivity(this->i, this->j, team, MS_GAM_ATTACK, attTarget->getI() * 1000 + attTarget->getJ());
							}
						}
					}
				break;
			case 4:
				if (attTarget != NULL && (attTarget->getHp() <= 0 || attTarget->getStatys() > 9 || attTarget->getStatys() == -1)) {
					attTarget = NULL;
					if (animstatys != 1) {
						animstatys = 1;
						anim = 0;
					}
					break;
				}
				if (animstatys == 5) {
					if (attTarget == NULL) {
						animstatys = 1;
						anim = 0;
					}
					else {
						anim++;
						direction = getdir(attTarget->getI(), attTarget->getJ());
						if (anim >= 63) {
							output->sendActivity(attTarget->getI(), attTarget->getJ(), team, MS_GAM_HIT, attack);
							anim = 0;
							if (attTarget->getHp() <= 0 || attTarget->getStatys() > 8 || attTarget->getStatys() == -1) {
								attTarget = NULL;
								animstatys = 1;
								anim = 0;
							}
							else
								if (abs(attTarget->getI() - this->i) <= attackrange + attTarget->height / 2 && abs(attTarget->getJ() - this->j) <= attackrange + attTarget->width / 2) {
									output->sendActivity(this->i, this->j, team, MS_GAM_ATTACK, attTarget->getI() * 1000 + attTarget->getJ());
								}
								else animstatys = 1;
						}
					}
				}
				else
					if (anim == 0) {
						for (int i = this->i - attackrange - 5; i <= this->i + attackrange + 5; i++) {
							for (int j = this->j - attackrange - 5; j <= this->j + attackrange + 5; j++) {
								if (i > 0 && i < 129 && j > 0 && j < 129)
									if (objMap[i][j] != NULL) {
										if (objMap[i][j]->getTeam() != team && objMap[i][j]->getTeam() >= 0) {
											if (attTarget == NULL) {
												attTarget = objMap[i][j];
											}
											else if (pow(objMap[i][j]->getI() - this->i, 2) + pow(objMap[i][j]->getJ() - this->j, 2) < pow(attTarget->getI() - this->i, 2) + pow(attTarget->getJ() - this->j, 2)) {
												attTarget = objMap[i][j];
											}
										}
									}
							}
						}
						if (attTarget != NULL && abs(attTarget->getI() - this->i) <= attackrange + attTarget->height / 2 && abs(attTarget->getJ() - this->j) <= attackrange + attTarget->width / 2) {
							output->sendActivity(this->i, this->j, team, MS_GAM_ATTACK, attTarget->getI() * 1000 + attTarget->getJ());
						}
						else {
							if (attTarget != NULL) { wayI = attTarget->getI(); wayJ = attTarget->getJ(); }
							else { wayI = attackI; wayJ = attackJ; }
							if (i == attackI && attackJ == j) { statys = 0; animstatys = 0; }
							else {
								if (wait > 0) {
									wait--;
								}
								else {
									if (nexti == wayI && nextj == wayJ) {
										statys = 0;
										animstatys = 0;
										break;
									}
									else {
										if (objMap[nexti][nextj] == NULL || objMap[nexti][nextj] == this)
											createway(wayI, wayJ, 0, 0);
										if (objMap[nexti][nextj] == NULL) {
											objMap[nexti][nextj] = this;
											output->sendActivity(this->i, this->j, team, MS_GAM_MOVE, nexti * 1000 + nextj);
										}

										else {

											int nexti2 = nexti;
											int nextj2 = nextj;

											createway(wayI, wayJ, 1, 0);

											if (objMap[nexti][nextj] == NULL) {
												objMap[nexti][nextj] = this;
												output->sendActivity(this->i, this->j, team, MS_GAM_MOVE, nexti * 1000 + nextj);
											}
											else {

												nexti = nexti2;
												nextj = nextj2;

												wait = -1 + 64 * 64 / movespeed;

												animstatys = 0;

											}
										}
									}
								}
							}
						}
					}
					else {
						anim++;

						if (anim >= 64 * 64 / movespeed) {
							objMap[i][j] = NULL;
							i = nexti;
							j = nextj;
							anim = 0;
							if (attTarget != NULL && abs(attTarget->getI() - this->i) <= attackrange + attTarget->height / 2 && abs(attTarget->getJ() - this->j) <= attackrange + attTarget->width / 2) {
								output->sendActivity(this->i, this->j, team, MS_GAM_ATTACK, attTarget->getI() * 1000 + attTarget->getJ());
							}
						}
					}
				break;
			case 5:
				if (attTarget == NULL || attTarget->getHp() <= 0 || attTarget->getStatys() > 9 || attTarget->getStatys() == -1) {
					if (typecomm.size() == 0) {
						attTarget = NULL;

						for (int i = this->i - attackrange - 10; i <= this->i + attackrange + 10; i++) {
							for (int j = this->j - attackrange - 10; j <= this->j + attackrange + 10; j++) {
								if (i > 0 && i < 129 && j > 0 && j < 129)
									if (objMap[i][j] != NULL) {
										if (objMap[i][j]->getTeam() != team && objMap[i][j]->getTeam() >= 0) {
											if (attTarget == NULL) {
												attTarget = objMap[i][j];
											}
											else if (pow(objMap[i][j]->getI() - this->i, 2) + pow(objMap[i][j]->getJ() - this->j, 2) < pow(attTarget->getI() - this->i, 2) + pow(attTarget->getJ() - this->j, 2)) {
												attTarget = objMap[i][j];
											}
										}
									}
							}
						}

						if (attTarget == NULL) {

							if (anim > 0 && animstatys == 1) {
								statys = 1;
								wayI = nexti;
								wayJ = nextj;
							}
							else {
								statys = 0;
								animstatys = 0;
								anim = 0;
							}
						}
						else {
							if (animstatys != 1) {
								animstatys = 1;
								anim = 0;
							}
						}
						break;
					}
					else {
						if (anim > 0 && animstatys == 1) {
							statys = 1;
							wayI = nexti;
							wayJ = nextj;
						}
						else {
							statys = 0;
							animstatys = 0;
							anim = 0;
						}
					}
					break;
				}
				else if (animstatys == 5) {
					anim++;
					direction = getdir(attTarget->getI(), attTarget->getJ());
					if (anim >= 63) {
						output->sendActivity(attTarget->getI(), attTarget->getJ(), team, MS_GAM_HIT, attack);
						anim = 0;
						if (attTarget == NULL || attTarget->getHp() <= 0 || attTarget->getStatys() > 9 || attTarget->getStatys() == -1) {
							if (typecomm.size() == 0) {
								attTarget = NULL;

								for (int i = this->i - attackrange - 10; i <= this->i + attackrange + 10; i++) {
									for (int j = this->j - attackrange - 10; j <= this->j + attackrange + 10; j++) {
										if (i > 0 && i < 129 && j > 0 && j < 129)
											if (objMap[i][j] != NULL) {
												if (objMap[i][j]->getTeam() != team && objMap[i][j]->getTeam() >= 0) {
													if (attTarget == NULL) {
														attTarget = objMap[i][j];
													}
													else if (pow(objMap[i][j]->getI() - this->i, 2) + pow(objMap[i][j]->getJ() - this->j, 2) < pow(attTarget->getI() - this->i, 2) + pow(attTarget->getJ() - this->j, 2)) {
														attTarget = objMap[i][j];
													}
												}
											}
									}
								}

								if (attTarget == NULL) {

									if (anim > 0 && animstatys == 1) {
										statys = 1;
										wayI = nexti;
										wayJ = nextj;
									}
									else {
										statys = 0;
										animstatys = 0;
										anim = 0;
									}
								}
								else {
									if (animstatys != 1) {
										animstatys = 1;
										anim = 0;
									}
								}
								break;
							}
							else {
								if (anim > 0 && animstatys == 1) {
									statys = 1;
									wayI = nexti;
									wayJ = nextj;
								}
								else {
									statys = 0;
									animstatys = 0;
									anim = 0;
								}
							}
							break;
						}
						else
							if (abs(attTarget->getI() - this->i) <= attackrange + attTarget->height / 2 && abs(attTarget->getJ() - this->j) <= attackrange + attTarget->width / 2) {
								output->sendActivity(this->i, this->j, team, MS_GAM_ATTACK, attTarget->getI() * 1000 + attTarget->getJ());
							}
							else animstatys = 1;
					}
				}
				else {
					if (anim == 0) {
						if (abs(attTarget->getI() - this->i) <= attackrange + attTarget->height / 2 && abs(attTarget->getJ() - this->j) <= attackrange + attTarget->width / 2) {
							output->sendActivity(this->i, this->j, team, MS_GAM_ATTACK, attTarget->getI() * 1000 + attTarget->getJ());
						}
						else
							if (wait > 0) wait--;
							else {
								wayJ = attTarget->getJ();
								wayI = attTarget->getI();
								if (objMap[nexti][nextj] == NULL || objMap[nexti][nextj] == this)
									createway(wayI, wayJ, 0, 0);
								if (objMap[nexti][nextj] == NULL) {
									objMap[nexti][nextj] = this;
									output->sendActivity(this->i, this->j, team, MS_GAM_MOVE, nexti * 1000 + nextj);
								}
								else {
									int nexti2 = nexti;
									int nextj2 = nextj;

									createway(wayI, wayJ, 1, 0);

									if (objMap[nexti][nextj] == NULL) {
										objMap[nexti][nextj] = this;
										output->sendActivity(this->i, this->j, team, MS_GAM_MOVE, nexti * 1000 + nextj);
									}
									else
									{

										nexti = nexti2;
										nextj = nextj2;

										if (!(objMap[nexti][nextj]->getStatys() == 0 || (!objMap[nexti][nextj]->isTree() && !objMap[nexti][nextj]->isUnit()))) { wait = 64 * 64 / movespeed; }
										{
											wait = 64 * 64 / movespeed;
										}
										animstatys = 0;
									}
								}

							}
					}
					else {
						anim++;

						if (anim >= 64 * 64 / movespeed) {
							objMap[i][j] = NULL;
							i = nexti;
							j = nextj;
							anim = 0;
							if (abs(attTarget->getI() - this->i) <= attackrange + attTarget->height / 2 && abs(attTarget->getJ() - this->j) <= attackrange + attTarget->width / 2) {
								output->sendActivity(this->i, this->j, team, MS_GAM_ATTACK, attTarget->getI() * 1000 + attTarget->getJ());
							}
						}
					}
				}
				break;
			case 6:
				if (typestorage != 2 || storage < 7) {
					if (attTarget == NULL || (attTarget->getHp() <= 0 || attTarget->getStatys() > 9)) {

						attTarget = NULL;

						for (int i = this->i - attackrange - 10; i <= this->i + attackrange + 10; i++) {
							for (int j = this->j - attackrange - 10; j <= this->j + attackrange + 10; j++) {
								if (i > 0 && i < 129 && j > 0 && j < 129)
									if (objMap[i][j] != NULL) {
										if (objMap[i][j]->isTree()) {
											if (attTarget == NULL) {
												attTarget = objMap[i][j];
											}
											else if (pow(objMap[i][j]->getI() - this->i, 2) + pow(objMap[i][j]->getJ() - this->j, 2) < pow(attTarget->getI() - this->i, 2) + pow(attTarget->getJ() - this->j, 2)) {
												attTarget = objMap[i][j];
											}
										}
									}
							}
						}

						if (attTarget == NULL) {

							if (anim > 0 && animstatys == 1) {
								statys = 1;
								wayI = nexti;
								wayJ = nextj;
							}
							else {
								statys = 0;
								animstatys = 0;
								anim = 0;
							}
						}
						else {
							if (animstatys != 1) {
								anim = 0;
								animstatys = 1;
							}
						}
					}
					else if (animstatys == 5) {
						anim++;
						direction = getdir(attTarget->getI(), attTarget->getJ());
						if (anim >= 63) {
							anim = 0;
							output->sendActivity(attTarget->getI(), attTarget->getJ(), team, MS_GAM_HIT, 1);
							output->sendActivity(this->i, this->j, this->team, MS_GAM_COLLECT, 200 + 1);
							storage++;
							if (storage >= 7) attTarget = NULL;
							else if (attTarget->getHp() <= 0 || attTarget->getStatys() > 9) {
								attTarget = NULL;

								for (int i = this->i - attackrange - 10; i <= this->i + attackrange + 10; i++) {
									for (int j = this->j - attackrange - 10; j <= this->j + attackrange + 10; j++) {
										if (i > 0 && i < 129 && j > 0 && j < 129)
											if (objMap[i][j] != NULL) {
												if (objMap[i][j]->isTree()) {
													if (attTarget == NULL) {
														attTarget = objMap[i][j];
													}
													else if (pow(objMap[i][j]->getI() - this->i, 2) + pow(objMap[i][j]->getJ() - this->j, 2) < pow(attTarget->getI() - this->i, 2) + pow(attTarget->getJ() - this->j, 2)) {
														attTarget = objMap[i][j];
													}
												}
											}
									}
								}

								if (attTarget == NULL) {

									if (anim > 0 && animstatys == 1) {
										statys = 1;
										wayI = nexti;
										wayJ = nextj;
									}
									else {
										statys = 0;
										animstatys = 0;
										anim = 0;
									}
								}
								else {
									if (animstatys != 1) {
										anim = 0;
										animstatys = 1;
									}
								}
								break;
							}
							else
								if (abs(attTarget->getI() - this->i) <= attackrange + attTarget->height / 2 && abs(attTarget->getJ() - this->j) <= attackrange + attTarget->width / 2) {
									output->sendActivity(this->i, this->j, team, MS_GAM_ATTACK, attTarget->getI() * 1000 + attTarget->getJ());
								}
								else animstatys = 1;
							storage--;
						}
					}
					else {
						if (anim == 0) {
							if (abs(attTarget->getI() - this->i) <= attackrange + attTarget->height / 2 && abs(attTarget->getJ() - this->j) <= attackrange + attTarget->width / 2) {
								output->sendActivity(this->i, this->j, team, MS_GAM_ATTACK, attTarget->getI() * 1000 + attTarget->getJ());
							}
							else
								if (wait > 0) wait--;
								else {
									wayJ = attTarget->getJ();
									wayI = attTarget->getI();
									if (objMap[nexti][nextj] == NULL || objMap[nexti][nextj] == this)
										createway(wayI, wayJ, 0, 1);
									if (objMap[nexti][nextj] == NULL) {
										objMap[nexti][nextj] = this;
										output->sendActivity(this->i, this->j, team, MS_GAM_MOVE, nexti * 1000 + nextj);
									}
									else {
										int nexti2 = nexti;
										int nextj2 = nextj;

										createway(wayI, wayJ, 1, 1);

										if (objMap[nexti][nextj] == NULL) {
											objMap[nexti][nextj] = this;
											output->sendActivity(this->i, this->j, team, MS_GAM_MOVE, nexti * 1000 + nextj);
										}
										else
										{
											if (objMap[nexti][nextj]->isTree()) {
												attTarget = objMap[nexti][nextj];
												break;
											}
											else {
												nexti = nexti2;
												nextj = nextj2;

												if (!(objMap[nexti][nextj]->getStatys() == 0 || (!objMap[nexti][nextj]->isTree() && !objMap[nexti][nextj]->isUnit()))) { wait = 128 * 64 / movespeed; }
												{
													wait = 64 * 64 / movespeed;
												}
												animstatys = 0;
											}
										}
									}

								}
						}
						else {
							anim++;

							if (anim >= 64 * 64 / movespeed) {
								objMap[i][j] = NULL;
								i = nexti;
								j = nextj;
								anim = 0;
								if (abs(attTarget->getI() - this->i) <= attackrange + attTarget->height / 2 && abs(attTarget->getJ() - this->j) <= attackrange + attTarget->width / 2) {
									output->sendActivity(this->i, this->j, team, MS_GAM_ATTACK, attTarget->getI() * 1000 + attTarget->getJ());
								}
							}
						}
					}
				}
				else {
					if (attTarget == NULL || (attTarget->getHp() <= 0 || attTarget->getStatys() > 9)) {
						std::vector<base*> units(teams[this->team]->getBases());
						for (auto a : units) {
							if (a->getStatys() == 0)
								if (attTarget == NULL) attTarget = a;
								else if (pow(a->getI() - this->i, 2) + pow(a->getJ() - this->j, 2) < pow(attTarget->getI() - this->i, 2) + pow(attTarget->getJ() - this->j, 2))
									attTarget = a;
						}
					}
					if (anim == 0) {
						if (attTarget == NULL) {
							statys = 0;
							animstatys = 0;
							break;
						}
						if (abs(attTarget->getI() - this->i) <= 1 + attTarget->height / 2 && abs(attTarget->getJ() - this->j) <= 1 + attTarget->width / 2) {
							output->sendActivity(this->i, this->j, this->team, MS_GAM_PUT, 0);
							attTarget = NULL;
							break;
						}
						else {
							if (wait > 0) {
								wait--;
							}
							else {
								wayJ = attTarget->getJ();
								wayI = attTarget->getI();
								if (objMap[nexti][nextj] == NULL || objMap[nexti][nextj] == this)
									createway(wayI, wayJ, 0, 1);
								if (objMap[nexti][nextj] == NULL) {
									objMap[nexti][nextj] = this;
									output->sendActivity(this->i, this->j, team, MS_GAM_MOVE, nexti * 1000 + nextj);
								}

								else {

									int nexti2 = nexti;
									int nextj2 = nextj;

									createway(wayI, wayJ, 1, 1);

									if (objMap[nexti][nextj] == NULL) {
										objMap[nexti][nextj] = this;
										output->sendActivity(this->i, this->j, team, MS_GAM_MOVE, nexti * 1000 + nextj);
									}
									else {

										nexti = nexti2;
										nextj = nextj2;

										wait = 64 * 64 / movespeed;

										animstatys = 0;
									}
								}
							}
						}
					}
					else {
						anim++;

						if (anim >= 64 * 64 / movespeed) {
							objMap[i][j] = NULL;
							i = nexti;
							j = nextj;
							anim = 0;
						}
					}
				}
				break;
			case 7:
				if (typestorage != 1 || storage < 8) {
					if (attTarget == NULL || attTarget->getHp() <= 0 || attTarget->getStatys() > 9) {
						attTarget = NULL;
						if (animstatys != 1) {
							anim = 0; animstatys = 0;
						}
						for (int i = this->i - attackrange - 10; i <= this->i + attackrange + 10; i++) {
							for (int j = this->j - attackrange - 10; j <= this->j + attackrange + 10; j++) {
								if (i > 0 && i < 129 && j > 0 && j < 129)
									if (objMap[i][j] != NULL) {
										if (!objMap[i][j]->isTree() && !objMap[i][j]->isUnit())
										{
											build* bd = (build*)objMap[i][j];
											if (bd->getType() == -1)
											{
												if (attTarget == NULL) {
													attTarget = objMap[i][j];
												}
												else if (pow(objMap[i][j]->getI() - this->i, 2) + pow(objMap[i][j]->getJ() - this->j, 2) < pow(attTarget->getI() - this->i, 2) + pow(attTarget->getJ() - this->j, 2)) {
													attTarget = objMap[i][j];
												}
											}
										}
									}
							}
						}
					}
					if (animstatys == 5) {
						
						if (attTarget == NULL || attTarget->getHp() <= 0 || attTarget->getStatys() > 9) {
							attTarget = NULL;
							anim = 0;
							break;
						}

						anim++;
						direction = getdir(attTarget->getI(), attTarget->getJ());
						if (anim >= 63) {
							anim = 0;
							if (attTarget == NULL || attTarget->getHp() <= 0 || attTarget->getStatys() > 9) {
								attTarget = NULL;
								anim = 0;
								break;
							}
							output->sendActivity(attTarget->getI(), attTarget->getJ(), team, MS_GAM_HIT, attack);
							storage += 
								4;
							output->sendActivity(this->i, this->j, this->team, MS_GAM_COLLECT, 100 + 4);
							if (storage > 7) attTarget = NULL;
							else output->sendActivity(this->i, this->j, team, MS_GAM_ATTACK, attTarget->getI() * 1000 + attTarget->getJ());
							storage -= 4;
							
						}
					}
					else {
						if (anim == 0) {

							attTarget = NULL;
							for (int i = this->i - attackrange - 10; i <= this->i + attackrange + 10; i++) {
								for (int j = this->j - attackrange - 10; j <= this->j + attackrange + 10; j++) {
									if (i > 0 && i < 129 && j > 0 && j < 129)
										if (objMap[i][j] != NULL) {
											if (!objMap[i][j]->isTree() && !objMap[i][j]->isUnit())
											{
												build* bd = (build*)objMap[i][j];
												if (bd->getType() == -1)
												{
													if (attTarget == NULL) {
														attTarget = objMap[i][j];
													}
													else if (pow(objMap[i][j]->getI() - this->i, 2) + pow(objMap[i][j]->getJ() - this->j, 2) < pow(attTarget->getI() - this->i, 2) + pow(attTarget->getJ() - this->j, 2)) {
														attTarget = objMap[i][j];
													}
												}
											}
										}
								}
							}

							if (attTarget == NULL) {
								statys = 0;
								break;
							}

							if (abs(attTarget->getI() - this->i) <= attackrange + attTarget->height / 2 && abs(attTarget->getJ() - this->j) <= attackrange + attTarget->width / 2) {
								output->sendActivity(this->i, this->j, team, MS_GAM_ATTACK, attTarget->getI() * 1000 + attTarget->getJ());
							}
							else
								if (wait > 0) wait--;
								else {
									wayJ = attTarget->getJ();
									wayI = attTarget->getI();
									if (objMap[nexti][nextj] == NULL || objMap[nexti][nextj] == this)
										createway(wayI, wayJ, 0, 1);
									if (objMap[nexti][nextj] == NULL) {
										objMap[nexti][nextj] = this;
										output->sendActivity(this->i, this->j, team, MS_GAM_MOVE, nexti * 1000 + nextj);
									}
									else {
										int nexti2 = nexti;
										int nextj2 = nextj;

										createway(wayI, wayJ, 1, 1);

										if (objMap[nexti][nextj] == NULL) {
											objMap[nexti][nextj] = this;
											output->sendActivity(this->i, this->j, team, MS_GAM_MOVE, nexti * 1000 + nextj);
										}
										else
										{
											if (objMap[nexti][nextj]->isTree()) {
												attTarget = objMap[nexti][nextj];
												break;
											}
											else {
												nexti = nexti2;
												nextj = nextj2;

												if (!(objMap[nexti][nextj]->getStatys() == 0 || (!objMap[nexti][nextj]->isTree() && !objMap[nexti][nextj]->isUnit()))) { wait = 128 * 64 / movespeed; }
												{
													wait = 64 * 64 / movespeed;
												}
												animstatys = 0;
											}
										}
									}
								}
						}
						else {
							anim++;

							if (anim >= 64 * 64 / movespeed) {
								objMap[i][j] = NULL;
								i = nexti;
								j = nextj;
								anim = 0;
								if (attTarget != NULL)
								if (abs(attTarget->getI() - this->i) <= attackrange + attTarget->height / 2 && abs(attTarget->getJ() - this->j) <= attackrange + attTarget->width / 2) {
									output->sendActivity(this->i, this->j, team, MS_GAM_ATTACK, attTarget->getI() * 1000 + attTarget->getJ());
								}
							}
						}
					}
				}
				else {
				if (storage > 8) storage--;
					if (attTarget == NULL || (attTarget->getHp() <= 0 || attTarget->getStatys() > 9)) {
						std::vector<base*> units(teams[this->team]->getBases());
						for (auto a : units) {
							if (a->getStatys() == 0)
								if (attTarget == NULL) attTarget = a;
								else if (pow(a->getI() - this->i, 2) + pow(a->getJ() - this->j, 2) < pow(attTarget->getI() - this->i, 2) + pow(attTarget->getJ() - this->j, 2))
									attTarget = a;
						}
					}
					if (anim == 0) {
						if (attTarget == NULL) {
							statys = 0;
							animstatys = 0;
							break;
						}
						if (abs(attTarget->getI() - this->i) <= 1 + attTarget->height / 2 && abs(attTarget->getJ() - this->j) <= 1 + attTarget->width / 2) {
							output->sendActivity(this->i, this->j, this->team, MS_GAM_PUT, 0);
							attTarget = NULL;
							break;
						}
						else {
							if (wait > 0) {
								wait--;
							}
							else {
								wayJ = attTarget->getJ();
								wayI = attTarget->getI();
								if (objMap[nexti][nextj] == NULL || objMap[nexti][nextj] == this)
									createway(wayI, wayJ, 0, 0);
								if (objMap[nexti][nextj] == NULL) {
									objMap[nexti][nextj] = this;
									output->sendActivity(this->i, this->j, team, MS_GAM_MOVE, nexti * 1000 + nextj);
								}

								else {

									int nexti2 = nexti;
									int nextj2 = nextj;

									createway(wayI, wayJ, 1, 0);

									if (objMap[nexti][nextj] == NULL) {
										objMap[nexti][nextj] = this;
										output->sendActivity(this->i, this->j, team, MS_GAM_MOVE, nexti * 1000 + nextj);
									}
									else {

										nexti = nexti2;
										nextj = nextj2;

										wait = 64 * 64 / movespeed;

										animstatys = 0;
									}
								}
							}
						}
					}
					else {
						anim++;

						if (anim >= 64 * 64 / movespeed) {
							objMap[i][j] = NULL;
							i = nexti;
							j = nextj;
							anim = 0;
						}
					}
				}
				break;
			case 8:
				if (anim == 0) {
					if (abs(wayI - this->i) <= sizes[typebuild][1] / 2 + 1 && abs(wayJ - this->j) <= sizes[typebuild][0] / 2 + 1) {
						if (objMap[wayI][wayJ] == NULL /*&& teams[team].getGold() >= 400*/) {
							//switch (typebuild) {
							//case 0:
							if (teams[team]->getWood() >= costWood[typebuild + 10] && teams[team]->getGold() >= costGold[typebuild + 10]) {
								output->sendActivity(wayI, wayJ, team, MS_GAM_CREATE, 10 + typebuild);
								output->sendActivity(this->i, this->j, team, MS_GAM_ATTACK, wayI * 1000 + wayJ);
								statys = 2;
							}
							//	else { statys = 0; animstatys = 0; }
							//	break;
							//case 1: // barracks
							//	if (teams[team]->getWood() >= costWood[typebuild + 10] && teams[team]->getGold() >= costGold[typebuild + 10]) {
							//		objMap[wayI][wayJ] = new barracks(wayI, wayJ, team);
							//		attTarget = objMap[wayI][wayJ];
							//		statys = 2;
							//	}
							//	else { statys = 0; animstatys = 0; }
							//	break;
							//case 2: //farm
							//	if (teams[team]->getWood() >= costWood[typebuild + 10] && teams[team]->getGold() >= costGold[typebuild + 10]) {
							//		objMap[wayI][wayJ] = new farm(wayI, wayJ, team);
							//		attTarget = objMap[wayI][wayJ];
							//		statys = 2;

							//	}
							//	else { statys = 0; animstatys = 0; }
							//	break;
							//case 3: // factory
							//	if (teams[team]->getWood() >= costWood[typebuild + 10] && teams[team]->getGold() >= costGold[typebuild + 10]) {
							//		objMap[wayI][wayJ] = new factory(wayI, wayJ, team);
							//		attTarget = objMap[wayI][wayJ];
							//		statys = 2;
							//	}
							//	else { statys = 0; animstatys = 0; }
							//	break;
							//}

						}
						else { statys = 0; animstatys = 0; }
						break;
					}
					else {
						if (wait > 0) {
							wait--;
						}
						else {
							if (objMap[nexti][nextj] == NULL || objMap[nexti][nextj] == this)
								createway(wayI, wayJ, 0, 0);
							if (objMap[nexti][nextj] == NULL) {
								objMap[nexti][nextj] = this;
								output->sendActivity(this->i, this->j, team, MS_GAM_MOVE, nexti * 1000 + nextj);
							}

							else {

								int nexti2 = nexti;
								int nextj2 = nextj;

								createway(wayI, wayJ, 1, 0);

								if (objMap[nexti][nextj] == NULL) {
									objMap[nexti][nextj] = this;
									output->sendActivity(this->i, this->j, team, MS_GAM_MOVE, nexti * 1000 + nextj);
								}
								else {

									nexti = nexti2;
									nextj = nextj2;

									wait = 64 * 64 / movespeed;

									animstatys = 0;
								}
							}
						}

					}
				}
				else {
					anim++;

					if (anim >= 64 * 64 / movespeed) {
						objMap[i][j] = NULL;
						i = nexti;
						j = nextj;
						anim = 0;
					}
				}
				break;

			case 9:
				if (attTarget == NULL) {
					statys = 0;
					anim = 0;
					animstatys = 0;
					attTarget = NULL;
					break;
				}
				if (attTarget->getHp() <= 0 || attTarget->getStatys() > 9 || attTarget->getStatys() == -1) {
					statys = 0;
					anim = 0;
					animstatys = 0;
					attTarget = NULL;
					break;
				}
				anim++;
				if (attTarget->getHp() <= 0 || attTarget->getStatys() > 9 || attTarget->getStatys() == -1) {
					statys = 0;
					anim = 0;
					animstatys = 0;
					attTarget = NULL;
					break;
				}
				direction = getdir(attTarget->getI(), attTarget->getJ());
				if (anim >= 63)
					if (!(attTarget->getHp() <= 0 || attTarget->getStatys() > 9 || attTarget->getStatys() == -1)) {
						attTarget->gotdmg(0);
						attTarget = NULL;
						statys = 0;
						anim = 0;
						animstatys = 0;
						break;
					}
					else {
						statys = 0;
						anim = 0;
						animstatys = 0;
						attTarget = NULL;
						break;
					}
				break;

			case 10:
				nexti = i;
				nextj = j;
				anim++;
				if (anim >= 48) {
					hp = -1;
					statys = 11;
					die();
				}
				break;
			}
	}

	void draw(sf::RenderWindow* win, int pointx, int pointy) override {
		if (statys != -1) {
			int windowx = j * SPRSIZE - pointx;
			int windowy = i * SPRSIZE - pointy;

			if (windowx >= -WIN_WIDTH && windowx <= 128 * SPRSIZE) {
				if (windowy >= -WIN_HEIGHT && windowy <= 128 * SPRSIZE) {
					sf::Sprite* spr = new sf::Sprite;
					spr->setTexture(rabTex[typestorage]);
					spr->setPosition(windowx + (nextj - j) * (float)(anim) / (float)(64) * movespeed, windowy + (nexti - i) * (float)(anim) / (float)(64) * movespeed);
					spr->setTextureRect(sf::IntRect(((direction - 1)) * 64, (animstatys) * 64 + ((anim / (64 / int(movespeed))) / 16) * 64, 64, 64));
					win->draw(*spr);
					delete spr;

					sf::RectangleShape rec;
					rec.setFillColor(sf::Color(0, 0, 0, 64));
					rec.setPosition(4 + windowx + (nextj - j) * (float)(anim) / (float)(64) * movespeed, 4 + windowy + (nexti - i) * (float)(anim) / (float)(64) * movespeed);
					rec.setSize(sf::Vector2f(width * 64 - 8, 10));
					win->draw(rec);

					rec.setFillColor(teamcolors[team]);
					rec.setPosition(4 + windowx + (nextj - j) * (float)(anim) / (float)(64) * movespeed, 4 + windowy + (nexti - i) * (float)(anim) / (float)(64) * movespeed);
					rec.setSize(sf::Vector2f(int(float(hp) / float(maxhp) * (width * 64 - 8)), 10));

					win->draw(rec);
				}
			}
		}
	}

	~civil() override {};

	friend class mine;
	friend class Host;
	friend class Client;

};

void createrab(int i, int j, int team) {
	output->sendActivity(i, j, team, MS_GAM_CREATE, 0 + 0);
}

class mine : public build {
public:
	int getTypeQ(int) override {
		return -1;
	}

	int getSizeQ() override {
		return 0;
	}

	void getComm(int t) override {}

	int getType() override {
		return -1;
	}

	mine(int i, int j) {
		this->i = i;
		this->j = j;
		this->team = -1;
		hp = 75 * 8 * 6;
		width = 1;
		height = 1;
		for (int i = -height / 2; i <= height / 2; i++) {
			for (int j = -width / 2; j <= width / 2; j++) {
				objMap[i + this->i][j + this->j] = this;
			}
		}
		maxhp = hp;
		pointI = this->i;
		pointJ = this->j;
		statys = 0;
		timer = 0;
		allMines.push_back(this);
		armor = 0;
		armorType = ARMOR_HEAVY;
	}

	void doSmth() override {};

	void draw(sf::RenderWindow* win, int pointx, int pointy) override {
		int windowx = j * SPRSIZE - pointx - (width / 2) * SPRSIZE;
		int windowy = i * SPRSIZE - pointy - (height / 2) * SPRSIZE;

		if (windowx >= -WIN_WIDTH * width && windowx <= 128 * SPRSIZE) {
			if (windowy >= -WIN_HEIGHT * height && windowy <= 128 * SPRSIZE) {
				sf::Sprite spr;
				spr.setPosition(windowx, windowy);

				int recty = -1 + maxhp / hp;
				if (recty > 3) recty = 3;

				spr.setTexture(textures[1][1]);
				spr.setTextureRect(sf::IntRect((j+i) % 3 * 64, recty * 64, 64, 64));

				win->draw(spr);

				//build::drawHp(win, pointx, pointy);
			}
		}
	}

	void die() override {
		statys = 11;
		for (int i = 0; i < allobjs.size(); i++) {
			if (allobjs[i] == this) {
				allobjs.erase(allobjs.begin() + i, allobjs.begin() + i + 1);
				break;
			}
		}
		for (int i = 0; i < doobjs.size(); i++) {
			if (doobjs[i] == this) {
				doobjs.erase(doobjs.begin() + i, doobjs.begin() + i + 1);
				break;
			}
		}
		for (int i = -height / 2; i <= height / 2; i++) {
			for (int j = -width / 2; j <= width / 2; j++) {
				objMap[i + this->i][j + this->j] = NULL;
			}
		}
		hp = -1;
	}

	~mine() override {};
};