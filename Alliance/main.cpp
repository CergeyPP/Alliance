#include <SFML/Graphics.hpp>
#include <iostream>
#include "render.h"
#include <vector>
#include <list>
#include <string>
#include <sstream>

#pragma warning(disable: 4996)

using namespace std;

sf::Font font;

bool ready = 1;

sf::Texture buttonTex[6];

sf::Texture interfacesprs[20];

sf::Texture resourse[3];

int frScreentoGameX(int screenX, int pointx) {
	return (pointx + screenX) / 64;
}

int frScreentoGameY(int screenY, int pointy) {
	return (pointy + screenY) / 64;
}

int starti = 0; int startj = 0;
bool press = 0;

sf::Clock clck;

std::list<int> doI, doJ, doCM;

int endi, endj;

bool pressed = 0;

int interfacestatys;

void gameplay() {
	while (win->isOpen()) {
		sf::Vector2i pos = sf::Mouse::getPosition(*win);

		/*if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
			int i = frScreentoGameY(pos.y, pointy);
			int j = frScreentoGameX(pos.x, pointx);

			if (objMap[i][j] == NULL)
				objMap[i][j] = new base(i, j, 0);
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::G)) {
			int i = frScreentoGameY(pos.y, pointy);
			int j = frScreentoGameX(pos.x, pointx);

			if (objMap[i][j] == NULL)
				objMap[i][j] = new mage(i, j, 1);
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
			int i = frScreentoGameY(pos.y, pointy);
			int j = frScreentoGameX(pos.x, pointx);

			if (objMap[i][j] == NULL)
				objMap[i][j] = new archer(i, j, 1);
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::K)) {
			int i = frScreentoGameY(pos.y, pointy);
			int j = frScreentoGameX(pos.x, pointx);

			if (objMap[i][j] == NULL)
				objMap[i][j] = new knight(i, j, 1);
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::C)) {
			int i = frScreentoGameY(pos.y, pointy);
			int j = frScreentoGameX(pos.x, pointx);

			if (objMap[i][j] == NULL && teams[0]->getFood() + needFood[0] <= teams[0]->getFoodLimit())
				objMap[i][j] = new civil(i, j, 0);
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::V)) {
			int i = frScreentoGameY(pos.y, pointy);
			int j = frScreentoGameX(pos.x, pointx);

			if (objMap[i][j] == NULL && teams[1]->getFood() + needFood[1] <= teams[1]->getFoodLimit())
				objMap[i][j] = new civil(i, j, 1);
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::F)) {
			int i = frScreentoGameY(pos.y, pointy);
			int j = frScreentoGameX(pos.x, pointx);

			if (objMap[i][j] == NULL)
				objMap[i][j] = new footman(i, j, 1);
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::I)) {
			int i = frScreentoGameY(pos.y, pointy);
			int j = frScreentoGameX(pos.x, pointx);

			if (objMap[i][j] == NULL)
				objMap[i][j] = new mine(i, j);
		}*/

		if (interfacestatys >= 10) press = 0;

		if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !press && !pressed) {
			if (pos.y >= 0 && pos.y < WIN_HEIGHT && pos.x >= 0 && pos.x < WIN_WIDTH) {
				starti = pos.y + pointy;
				startj = pos.x + pointx;
				press = 1;
			}
		}
		if (!sf::Mouse::isButtonPressed(sf::Mouse::Left) && press) {
			if (pos.y >= 0 && pos.y < WIN_HEIGHT && pos.x >= 0 && pos.x < WIN_WIDTH) {
				starti = frScreentoGameY(starti - pointy, pointy);
				startj = frScreentoGameX(startj - pointx, pointx);
				endi = frScreentoGameY(pos.y, pointy);
				endj = frScreentoGameX(pos.x, pointx);
				if (starti > endi) {
					int c = starti;
					starti = endi;
					endi = c;
				}
				if (startj > endj) {
					int c = startj;
					startj = endj;
					endj = c;
				}
				press = 0;
				interfacestatys = 0;
				while (selectedObj.size() > 0)
					selectedObj.pop_back();
				if (starti == endi && startj == endj) {
					if (objMap[endi][endj] != NULL)
						if (!objMap[endi][endj]->isTree() && objMap[endi][endj]->getTeam() == myteam)
							selectedObj.push_back(objMap[endi][endj]);
				}
				else {
					for (int i = starti; i <= endi; i++) {
						for (int j = startj; j <= endj; j++) {
							if (i >= 0 && i < 129 && j >= 0 && j < 129)
								if (objMap[i][j] != NULL && objMap[i][j]->isUnit() && objMap[i][j]->getTeam() == myteam) {
									if (objMap[i][j]->getI() == i && objMap[i][j]->getJ() == j)
										selectedObj.push_back(objMap[i][j]);
								}
						}
					}
				}
			}
		}
		//WIN_WIDTH + (200 - 128) / 2, WIN_HEIGHT - 128

	}
}

float tim = 0;

void anal() {

	int i = 0;

	if (doingobj.size() > 0 && doI.size() > 0 && doJ.size() > 0 && doCM.size() > 0) {
		if (doingobj.front() != NULL)
			doingobj.front()->makeaction(doI.front(), doJ.front(), doCM.front(), sf::Keyboard::isKeyPressed(sf::Keyboard::LShift));
		doingobj.pop_front();
		doI.pop_front();
		doJ.pop_front();
		doCM.pop_front();
	}

	ready = 0;

	while (i < doobjs.size()) {
		if (doobjs[i]->getHp() > 0) {
			doobjs[i]->doSmth();
			if (doobjs[i]->getHp() <= 0) {
				deadlst.push_back(doobjs[i]);
				deadlst.back()->die();
				i--;
			}
			i++;
		}
		else {
			deadlst.push_back(doobjs[i]);
			deadlst.back()->die();
		}

	}

	i = 0;

	while (i < misl.size()) {
		if (misl[i]->getStatys() != 10) {
			misl[i]->fly();
			if (misl[i]->getStatys() != 10) {
				i++;
			}
			else misl.erase(misl.begin() + i);
		}
		else {
			misl.erase(misl.begin() + i);
		}
	}

	while (deadlst.size() > 50 && ready) {
		delete deadlst.back();
		deadlst.pop_back();
	}

	ready = 1;
}

void analizeObjects() {
	while (win->isOpen()) {

		float time = clck.getElapsedTime().asMicroseconds(); //дать прошедшее время в микросекундах
		clck.restart(); //перезагружает время

		float ti = time / 2000;
		
		 sf::Vector2i pos = sf::Mouse::getPosition(*win);

		if (/*(pos.x < 35 && pos.x >= 0) || */sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) pointx -= ti;
		if (/*(pos.x > win->getSize().x - 35) ||*/sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) pointx += ti;
		if (/*(pos.y < 35 && pos.y >= 0) ||*/ sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) pointy -= ti;
		if (/*(pos.y > win->getSize().y - 35) || */sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) pointy += ti;

		if (pointx < 0) pointx = 0;
		if (pointy < 0) pointy = 0;
		if (pointx > 129 * 64 - WIN_WIDTH) pointx = 129 * 64 - WIN_WIDTH;
		if (pointy > 129 * 64 - WIN_HEIGHT) pointy = 129 * 64 - WIN_HEIGHT;

		tim += time / 1500;

		if (tim >= 5) {
			isAnalizyng = 1;
			tim -= 5;
			mtx.lock();
			anal();
			mtx.unlock();
			isAnalizyng = 0;
		}

	}
}

sf::IntRect selectGrass(int i, int j) {
	if (i == 0 || j == 0 || i == 128 || j == 128) return sf::IntRect(0, 768, 64, 64);

	int di[] = {  0, -1,  1,  0,};
	int dj[] = { -1,  0,  0,  1,};

	int count = 0;

	for (int k = 0; k < 4; k++) {
		if (heightMap[i + di[k]][j + dj[k]] > forest) {
			count += pow(2, k);
		}
	}

	switch (count) {
	/*case 1 + 4 + 2:
		if (heightMap[i+1][j-1] > forest) return sf::IntRect(64 * ((i + j) % 2), 11*64, 64, 64);
		return sf::IntRect(64 * ((i+j) % 3), 0, 64, 64);
		break;
	case 1 + 2:
		return sf::IntRect(64 * ((i + j) % 2), 64, 64, 64);
		break;
	case 1 + 4 + 8:
		return sf::IntRect(64 * ((i + j) % 3), 4 * 64, 64, 64);
		break;
	case 2 + 4 + 8:
		return sf::IntRect(64 * ((i + j) % 3), 2 * 64, 64, 64);
		break;
	case 1 + 2 + 8:
		return sf::IntRect(64 * ((i + j) % 3), 3 * 64, 64, 64);
		break;
	case 1 + 4:
		return sf::IntRect(64 * ((i + j) % 2), 5 * 64, 64, 64);
		break;
	case 2 + 8:
		return sf::IntRect(0, 6 * 64, 64, 64);
		break;
	case 8 + 4:
		return sf::IntRect(0, 7 * 64, 64, 64);
		break;*/
	default:
		return sf::IntRect(64 * /*((i + j) % 2)*/ 0, /*12*/12 * 64, 64, 64);
		break;
	/*default:
		return sf::IntRect(64 * ((i + j) % 2) + 256, 0, 64, 64);
		break;*/
	}
}

void draw(int pointx, int pointy) {

	win->clear();

	for (int i = 0; i < WIN_HEIGHT / SPRSIZE + 2; i++) {
		for (int j = 0; j < WIN_WIDTH / SPRSIZE + 2; j++) {
			if (heightMap[(pointy / SPRSIZE) + i][(pointx / SPRSIZE) + j] <= sand) {
				sf::Sprite shape;
				shape.setTexture(landspace[0]);
				shape.setPosition(j * SPRSIZE - (pointx % SPRSIZE), i * SPRSIZE - (pointy % SPRSIZE));

				shape.setColor(sf::Color(255, 255, 255));

				/*if (objMap[(pointy / SPRSIZE) + i][(pointx / SPRSIZE) + j] != NULL) {
					shape.setColor(sf::Color(0, 255, 0));
				}*/
				win->draw(shape);
			}
			else
				if (heightMap[(pointy / SPRSIZE) + i][(pointx / SPRSIZE) + j] <= forest) {
					sf::Sprite shape;
					shape.setTexture(landspace[1]);
					shape.setPosition(j * SPRSIZE - (pointx % SPRSIZE), i * SPRSIZE - (pointy % SPRSIZE));

					shape.setColor(sf::Color(255, 255, 255));
					/*if (objMap[(pointy / SPRSIZE) + i][(pointx / SPRSIZE) + j] != NULL) {
						shape.setColor(sf::Color(0, 255, 0));
					}*/
					win->draw(shape);
				}
				else
					if (heightMap[(pointy / SPRSIZE) + i][(pointx / SPRSIZE) + j] <= mount) {
						sf::Sprite shape;
						shape.setTexture(landspace[2]);
						shape.setPosition(j * SPRSIZE - (pointx % SPRSIZE), i * SPRSIZE - (pointy % SPRSIZE));

						shape.setColor(sf::Color(255, 255, 255));
						/*if (objMap[(pointy / SPRSIZE) + i][(pointx / SPRSIZE) + j] != NULL) {
							shape.setColor(sf::Color(0, 255, 0));
						}*/

						shape.setTextureRect(selectGrass((pointy / SPRSIZE) + i, (pointx / SPRSIZE) + j));
						win->draw(shape);
					}
					else {
						sf::RectangleShape shape;
						shape.setFillColor(sf::Color(heightMap[(pointy / SPRSIZE) + i][(pointx / SPRSIZE) + j] / 4, heightMap[(pointy / SPRSIZE) + i][(pointx / SPRSIZE) + j] / 4, heightMap[(pointy / SPRSIZE) + i][(pointx / SPRSIZE) + j] / 4));
						shape.setSize(sf::Vector2f(SPRSIZE, SPRSIZE));
						/*if (objMap[(pointy / SPRSIZE) + i][(pointx / SPRSIZE) + j] != NULL) {
							shape.setFillColor(sf::Color(0, 255, 0));
						}*/
						shape.setPosition(j * SPRSIZE - (pointx % SPRSIZE), i * SPRSIZE - (pointy % SPRSIZE));
						win->draw(shape);
					}
		}
	}

	int k = 0;

	mtx.lock();
	while (k < selectedObj.size()) {
		if (selectedObj[k] != NULL) {
			if (selectedObj[k]->getHp() > 0 && selectedObj[k]->getStatys() < 10) {
				sf::RectangleShape rect;
				rect.setPosition((selectedObj[k]->getJ() - selectedObj[k]->width / 2) * 64 - pointx, (selectedObj[k]->getI() - selectedObj[k]->height / 2) * 64 - pointy);
				rect.setSize(sf::Vector2f(selectedObj[k]->width * 64, selectedObj[k]->height * 64));

				rect.setFillColor(sf::Color(255, 0, 0, 0));
				rect.setOutlineColor(sf::Color(255, 0, 0));
				rect.setOutlineThickness(1);
				win->draw(rect);
			}
			else {
				selectedObj[k] = NULL;
			}
		}
		k++;
	}

	k = 0;

	while (k < selectedObj.size()) {
		if (selectedObj[k] == NULL) {
			selectedObj.erase(selectedObj.begin() + k);
		}
		else k++;
	}
	mtx.unlock();

	int i = 0;

	while (i < allobjs.size()) {
		if (allobjs[i]->getHp() > 0) {
			allobjs[i]->draw(win, pointx, pointy);
			i++;
		}
	}

	i = 0;

	while (i < misl.size()) {
		misl[i]->draw(win, pointx, pointy);
		i++;
	}
	if (press) {
		sf::Vector2i pos = sf::Mouse::getPosition(*win);
		sf::RectangleShape rect;
		rect.setFillColor(sf::Color(255, 0, 0, 0));
		rect.setOutlineColor(sf::Color(255, 0, 0));
		rect.setOutlineThickness(1);
		rect.setPosition(startj - pointx, starti - pointy);
		rect.setSize(sf::Vector2f(pos.x - startj + pointx, pos.y - starti + pointy));
		win->draw(rect);
	}
}

bool waitforclick = 0;

sf::Text settxt(string s, int charsize, int x, int y, sf::Color clr) {
	sf::Text txt;
	txt.setFont(font);
	txt.setString(s);
	txt.setCharacterSize(charsize);
	txt.setFillColor(clr);
	txt.setPosition(x, y);
	return txt;
}

void drawInterFace() {
	sf::Vector2i pos = sf::Mouse::getPosition(*win);
	if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && pos.x > WIN_WIDTH + (200 - 128) / 2 && pos.x < WIN_WIDTH + (200 - 128) / 2 + 129 && pos.y > WIN_HEIGHT - 128 && pos.y < WIN_HEIGHT + 1) {
		pointx = (pos.x - (WIN_WIDTH + (200 - 128) / 2)) * 64 - WIN_WIDTH / 2;
		pointy = (pos.y - WIN_HEIGHT + 128) * 64 - WIN_HEIGHT / 2;
	}
	{
		sf::Sprite spr;
		spr.setTexture(interfacesprs[19]);
		spr.setPosition(0, WIN_HEIGHT);
		win->draw(spr);
	}
	{
		sf::Sprite spr;
		spr.setTexture(interfacesprs[18]);
		spr.setPosition(WIN_WIDTH, 0);
		win->draw(spr);
	}
	{
		sf::Sprite spr;
		spr.setTexture(resourse[0]);
		spr.setPosition(10, WIN_HEIGHT + 1);
		win->draw(spr);
		ostringstream str;
		str << teams[myteam]->getGold();
		win->draw(settxt(str.str(), 14, 29, WIN_HEIGHT + 1,teamcolors[myteam]));
	}
	{
		sf::Sprite spr;
		spr.setTexture(resourse[1]);
		spr.setPosition(10 + 580 / 3, WIN_HEIGHT + 1);
		win->draw(spr);
		ostringstream str;
		str << teams[myteam]->getWood();
		win->draw(settxt(str.str(), 14, 15 + 14 + 580 / 3, WIN_HEIGHT + 1, teamcolors[myteam]));
	}
	{
		sf::Sprite spr;
		spr.setTexture(resourse[2]);
		spr.setPosition(10 + (580 * 2) / 3, WIN_HEIGHT + 1);
		win->draw(spr);
		ostringstream str;
		str << teams[myteam]->getFood() << "/" << teams[myteam]->getFoodLimit();
		win->draw(settxt(str.str(), 14, 15 + 14 + (580 * 2) / 3, WIN_HEIGHT + 1, teamcolors[myteam]));
	}
	if (selectedObj.size() > 0) {
		if (selectedObj.size() == 1) {
			if (selectedObj.front()->isUnit()) {
				unit* n = (unit*)selectedObj.front();
				{
					string s;
					switch (n->getType()) {
					case 0:
						s = "Peasant";
						break;
					case 1:
						s = "Footman";
						break;
					case 2:
						s = "Archer";
						break;
					case 3:
						s = "Knight";
						break;
					case 4:
						s = "Mage";
						break;
					case 5:
						s = "Catapult";
						break;
					case 6:
						s = "Earth Knight";
						break;
					case 7:
						s = "Dragon";
						break;
					case 8:
						s = "Gryphon rider";
						break;
					case 9:
						s = "Helicopter";
						break;
					}
					win->draw(settxt(s, 25, win->getSize().x - 100 - s.length() * 6, 10, sf::Color::White));
				}

				sf::Sprite spr;
				spr.setTexture(interfacesprs[n->getType()]);
				spr.setPosition(win->getSize().x - 100 - 23 * 3, 50);
				spr.setScale(sf::Vector2f(3, 3));
				win->draw(spr);

				sf::RectangleShape rect;
				rect.setFillColor(sf::Color(0, 0, 0, 128));
				rect.setSize(sf::Vector2f(180, 25));
				rect.setPosition(WIN_WIDTH + 10, 184);
				win->draw(rect);

				rect.setFillColor(sf::Color(0, 128, 0));
				rect.setSize(sf::Vector2f(180 * ((float)n->getHp() / (float)n->maxhp), 25));
				rect.setPosition(WIN_WIDTH + 10, 184);
				win->draw(rect);
				{

					{
						ostringstream str;
						str << n->getHp() << "/" << n->maxhp;
						win->draw(settxt(str.str(), 25, win->getSize().x - 100 - str.str().length() * 6, 181, sf::Color::White));
					}
					
					{
						ostringstream str;
						str << "Attack: " << n->getAttack();
						win->draw(settxt(str.str(), 25, win->getSize().x - 100 - str.str().length() * 6, 226, sf::Color::White));
					}
				}

				if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
					interfacestatys = 10;
					waitforclick = 1;
				}
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::M)) {
					interfacestatys = 11;
					waitforclick = 1;
				}
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
					interfacestatys = 12;
					waitforclick = 1;
				}
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
					interfacestatys = 13;
					waitforclick = 1;
				}

				if (interfacestatys < 10 && !waitforclick) {
					if (n->getType() != 0)
						for (int i = 0; i < 4; i++) {
							int butx = WIN_WIDTH + 3 + (i % 4) * 49;
							int buty = 300 + (i / 4) * 40;
							if (pressed && !sf::Mouse::isButtonPressed(sf::Mouse::Left)) {//если отпустили клавишу
								sf::Vector2i pos = sf::Mouse::getPosition(*win);
								if (pos.x >= butx && pos.x <= butx + 46 && pos.y >= buty && pos.y <= buty + 38) {
									interfacestatys = 10 + i;
									if (i == 0) {
										for (int k = 0; k < selectedObj.size(); k++) {
											doingobj.push_back(selectedObj[k]);
											doI.push_back(selectedObj[k]->getI());
											doJ.push_back(selectedObj[k]->getJ());
											doCM.push_back(1);
										}
										interfacestatys = 0;
									}
									waitforclick = 1;
								}
							}
						}
					else if (interfacestatys != 1) {
						for (int i = 0; i < 6; i++) {
							int butx = WIN_WIDTH + 3 + (i % 4) * 49;
							int buty = 300 + (i / 4) * 40;
							if (pressed && !sf::Mouse::isButtonPressed(sf::Mouse::Left)) {//если отпустили клавишу
								sf::Vector2i pos = sf::Mouse::getPosition(*win);
								if (pos.x >= butx && pos.x <= butx + 46 && pos.y >= buty && pos.y <= buty + 38) {
									interfacestatys = 10 + i;
									if (i == 0) {
										for (int k = 0; k < selectedObj.size(); k++) {
											doingobj.push_back(selectedObj[k]);
											doI.push_back(selectedObj[k]->getI());
											doJ.push_back(selectedObj[k]->getJ());
											doCM.push_back(1);
										}
										interfacestatys = 0;
									}
									if (i != 5)
										waitforclick = 1;
									else interfacestatys = 1;
								}
							}
						}
					}
					else {
						for (int i = 0; i < 4; i++) {
							int butx = WIN_WIDTH + 3 + (i % 4) * 49;
							int buty = 300 + (i / 4) * 40;
							if (pressed && !sf::Mouse::isButtonPressed(sf::Mouse::Left)) {//если отпустили клавишу
								sf::Vector2i pos = sf::Mouse::getPosition(*win);
								if (pos.x >= butx && pos.x <= butx + 46 && pos.y >= buty && pos.y <= buty + 38) {
									interfacestatys = 15 + i;
									waitforclick = 1;
								}
							}
						}
					}

				}
				else {
					if (event.type == sf::Event::MouseButtonPressed)//если отпустили клавишу
						if (event.key.code == sf::Mouse::Left && waitforclick) {
							int n = interfacestatys % 10;
							switch (n) {
							case 0:
							{
								for (int k = 0; k < selectedObj.size(); k++) {
									doingobj.push_back(selectedObj[k]);
									doI.push_back(selectedObj[k]->getI());
									doJ.push_back(selectedObj[k]->getJ());
									doCM.push_back(1);
								}
								break;
							}
							case 1:
							{
								sf::Vector2i pos = sf::Mouse::getPosition(*win);
								if (pos.x >= 0 && pos.x < WIN_WIDTH && pos.y >= 0 && pos.y < WIN_HEIGHT) {

									int i = frScreentoGameY(pos.y, pointy);
									int j = frScreentoGameX(pos.x, pointx);

									for (int k = 0; k < selectedObj.size(); k++) {
										doingobj.push_back(selectedObj[k]);
										doI.push_back(i);
										doJ.push_back(j);
										doCM.push_back(1);
									}

								}
								break;
							}
							case 2: {
								sf::Vector2i pos = sf::Mouse::getPosition(*win);
								if (pos.x >= 0 && pos.x < WIN_WIDTH && pos.y >= 0 && pos.y < WIN_HEIGHT) {

									int i = frScreentoGameY(pos.y, pointy);
									int j = frScreentoGameX(pos.x, pointx);

									for (int k = 0; k < selectedObj.size(); k++) {
										doingobj.push_back(selectedObj[k]);
										doI.push_back(i);
										doJ.push_back(j);
										doCM.push_back(2);
									}

								}
								break;
							}
							case 3: {
								sf::Vector2i pos = sf::Mouse::getPosition(*win);
								if (pos.x >= 0 && pos.x < WIN_WIDTH && pos.y >= 0 && pos.y < WIN_HEIGHT) {

									int i = frScreentoGameY(pos.y, pointy);
									int j = frScreentoGameX(pos.x, pointx);

									for (int k = 0; k < selectedObj.size(); k++) {
										doingobj.push_back(selectedObj[k]);
										doI.push_back(i);
										doJ.push_back(j);
										doCM.push_back(3);
									}

								}
								break;
							}
							case 4:
							{
								sf::Vector2i pos = sf::Mouse::getPosition(*win);
								if (pos.x >= 0 && pos.x < WIN_WIDTH && pos.y >= 0 && pos.y < WIN_HEIGHT) {

									int i = frScreentoGameY(pos.y, pointy);
									int j = frScreentoGameX(pos.x, pointx);

									for (int k = 0; k < selectedObj.size(); k++) {
										doingobj.push_back(selectedObj[k]);
										doI.push_back(i);
										doJ.push_back(j);
										doCM.push_back(4);
									}

								}
								break;
							}
							default:
							{
								sf::Vector2i pos = sf::Mouse::getPosition(*win);
								if (pos.x >= 0 && pos.x < WIN_WIDTH && pos.y >= 0 && pos.y < WIN_HEIGHT) {

									int posi = frScreentoGameY(pos.y, pointy);
									int posj = frScreentoGameX(pos.x, pointx);

									civil* un = (civil*)selectedObj[0];

									int typebuild = n - 5;

									bool f = 1;

									for (int i = -(un->sizes[typebuild][0] / 2); i <= (un->sizes[typebuild][0] / 2); i++) {
										for (int j = -(un->sizes[typebuild][1] / 2); j <= (un->sizes[typebuild][1] / 2); j++) {
											sf::RectangleShape shape;
											if (objMap[posi + i][posj + j] != NULL) {
												f = 0;
												break;
												break;
											}
										}
									}

									if (f)
										for (int k = 0; k < selectedObj.size(); k++) {
											doingobj.push_back(selectedObj[k]);
											doI.push_back(posi);
											doJ.push_back(posj);
											doCM.push_back(interfacestatys - 5);
										}

								}
								break;
							}
							}
							interfacestatys = 0;
							waitforclick = 0;
						}
				}
				if (interfacestatys < 10) {
					if (interfacestatys != 1) {
						for (int i = 0; i < 4; i++) {
							sf::Sprite butspr;
							butspr.setTexture(buttonTex[i]);
							int butx = WIN_WIDTH + 3 + (i % 4) * 49;
							int buty = 300 + (i / 4) * 40;
							butspr.setPosition(butx, buty);
							win->draw(butspr);
						}
						if (n->getType() == 0) {
							civil* un = (civil*)n;
							sf::Sprite butspr;
							butspr.setTexture(buttonTex[4]);
							int butx = WIN_WIDTH + 3;
							int buty = 340;
							butspr.setPosition(butx, buty);
							win->draw(butspr);
							butspr.setTexture(buttonTex[5]);
							butx = WIN_WIDTH + 52;
							buty = 340;
							butspr.setPosition(butx, buty);
							win->draw(butspr);
						}
					}
					else {
						for (int i = 0; i < 4; i++) {
							sf::Sprite* spr = new sf::Sprite;
							spr->setTexture(interfacesprs[10 + i]);
							int butx = WIN_WIDTH + 3 + (i % 4) * 49;
							int buty = 300 + (i / 4) * 40;
							spr->setPosition(butx, buty);
							win->draw(*spr);
							delete spr;
							spr = new sf::Sprite;
							spr->setTexture(resourse[0]);
							spr->setScale(sf::Vector2f(0.7, 0.7));
							spr->setPosition(butx, buty);
							win->draw(*spr);
							delete spr;
							{
								ostringstream str;
								str << costGold[10 + i];
								win->draw(settxt(str.str(), 10, butx + 10, buty, sf::Color::White));
							}
							spr = new sf::Sprite;
							spr->setTexture(resourse[1]);
							spr->setPosition(butx, buty + 10);
							spr->setScale(sf::Vector2f(0.7, 0.7));
							win->draw(*spr);
							delete spr;
							{
								ostringstream str;
								str << costWood[10 + i];
								win->draw(settxt(str.str(), 10, butx + 10, buty + 10, sf::Color::White));
							}
						}
					}
				}

				if (n->getType() == 0 && interfacestatys % 10 > 4) {
					sf::Vector2i pos = sf::Mouse::getPosition(*win);
					civil* un = (civil*)n;
					int posi = frScreentoGameY(pos.y, pointy);
					int posj = frScreentoGameX(pos.x, pointx);
					int typebuild = interfacestatys % 10;
					typebuild -= 5;
					for (int i = -(un->sizes[typebuild][0] / 2); i <= (un->sizes[typebuild][0] / 2); i++) {
						for (int j = -(un->sizes[typebuild][1] / 2); j <= (un->sizes[typebuild][1] / 2); j++) {
							sf::RectangleShape shape;
							if (objMap[posi + i][posj + j] == NULL) {
								shape.setFillColor(sf::Color(0, 255, 0, 64));
							}
							else shape.setFillColor(sf::Color(255, 0, 0, 64));
							shape.setPosition((posj + j) * 64 - (int)pointx, (posi + i) * 64 - (int)pointy);
							shape.setSize(sf::Vector2f(64, 64));
							win->draw(shape);
						}
					}
				}
			}
			else {
				build* n = (build*)selectedObj.front();

				sf::Sprite* spr = new sf::Sprite;
				spr->setTexture(interfacesprs[n->getType() + 10]);
				spr->setPosition(win->getSize().x - 100 - 23 * 3, 50);
				spr->setScale(sf::Vector2f(3, 3));
				win->draw(*spr);
				delete spr;
				{

					string s;

					switch (n->getType()) {
					case 0:
						s = "Town Hall";
						break;
					case 1:
						s = "Barracks";
						break;
					case 2:
						s = "Farm";
						break;
					case 3:
						s = "Factory";
						break;
					}
					win->draw(settxt(s, 25, win->getSize().x - 100 - s.length() * 6, 10, sf::Color::White));
				}
				sf::RectangleShape rect;
				rect.setFillColor(sf::Color(0, 0, 0, 128));
				rect.setSize(sf::Vector2f(180, 25));
				rect.setPosition(WIN_WIDTH + 10, 184);
				win->draw(rect);

				rect.setFillColor(sf::Color(0, 128, 0));
				rect.setSize(sf::Vector2f(180 * ((float)n->getHp() / (float)n->maxhp), 25));
				rect.setPosition(WIN_WIDTH + 10, 184);
				win->draw(rect);
				{
					ostringstream str;
					str << n->getHp() << "/" << n->maxhp;
					win->draw(settxt(str.str(), 25, win->getSize().x - 100 - str.str().length() * 6, 181, sf::Color::White));
				}
				if (n->getStatys() == 0) {
					switch (n->getType()) {
					case 0: {
						int butx = WIN_WIDTH + 3;
						int buty = 400;
						if (pressed && !sf::Mouse::isButtonPressed(sf::Mouse::Left)) {//если отпустили клавишу
							sf::Vector2i pos = sf::Mouse::getPosition(*win);
							if (pos.x >= butx && pos.x <= butx + 46 && pos.y >= buty && pos.y <= buty + 38) {
								n->getComm(0);
							}
						}
						spr = new sf::Sprite;
						spr->setTexture(interfacesprs[0]);
						spr->setPosition(butx, buty);
						win->draw(*spr);
						delete spr;
						spr = new sf::Sprite;
						spr->setTexture(resourse[0]);
						spr->setScale(sf::Vector2f(0.7, 0.7));
						spr->setPosition(butx, buty);
						win->draw(*spr);
						delete spr;
						{
							ostringstream str;
							str << costGold[0];
							win->draw(settxt(str.str(), 10, butx + 10, buty, sf::Color::White));
						}
						spr = new sf::Sprite;
						spr->setTexture(resourse[1]);
						spr->setPosition(butx, buty + 10);
						spr->setScale(sf::Vector2f(0.7, 0.7));
						win->draw(*spr);
						delete spr;
						{
							ostringstream str;
							str << costWood[0];
							win->draw(settxt(str.str(), 10, butx + 10, buty + 10, sf::Color::White));
						}
						spr = new sf::Sprite;
						spr->setTexture(resourse[2]);
						spr->setPosition(butx, buty + 20);
						spr->setScale(sf::Vector2f(0.7, 0.7));
						win->draw(*spr);
						delete spr;
						{
							ostringstream str;
							str << needFood[0];
							win->draw(settxt(str.str(), 10, butx + 10, buty + 20, sf::Color::White));
						}
						if (n->getSizeQ() > 0) {
							int i = n->getTypeQ(0);
							{
								sf::RectangleShape* sh = new sf::RectangleShape;
								sh->setFillColor(sf::Color(0, 0, 0, 128));
								sh->setPosition(WIN_WIDTH + 1, 235);
								sh->setSize(sf::Vector2f(win->getSize().x - (WIN_WIDTH + 1) - 1, 38 * 2 + 80));
								win->draw(*sh);
								delete sh;

								spr = new sf::Sprite;
								spr->setTexture(interfacesprs[i]);
								spr->setPosition(WIN_WIDTH + 3, 250);
								spr->setScale(2, 2);
								win->draw(*spr);
								if (pressed && !sf::Mouse::isButtonPressed(sf::Mouse::Left)) {//если отпустили клавишу
									sf::Vector2i pos = sf::Mouse::getPosition(*win);
									if (pos.x >= WIN_WIDTH + 1 && pos.x <= WIN_WIDTH + 1 + 46 * 2 && pos.y >= 235 && pos.y <= 235 + 38 * 2) {
										n->getComm(10);
									}
								}
								delete spr;

								win->draw(settxt("Training...", 20, WIN_WIDTH + 3 + 46 * 2 + 3, 250, sf::Color::White));

								sh = new sf::RectangleShape;
								sh->setFillColor(sf::Color(0, 0, 0, 128));
								sh->setPosition(WIN_WIDTH + 3 + 46 * 2 + 3, 275);
								sh->setSize(sf::Vector2f(win->getSize().x - (WIN_WIDTH + 3 + 46 * 2 + 3) - 3, 20));
								win->draw(*sh);
								delete sh;

								sh = new sf::RectangleShape;
								sh->setFillColor(sf::Color(0, 128, 0));
								sh->setPosition(WIN_WIDTH + 3 + 46 * 2 + 3, 275);
								sh->setSize(sf::Vector2f(float((float)n->timer / (float)n->maxtimer) * (float)(win->getSize().x - (WIN_WIDTH + 3 + 46 * 2 + 3) - 3), 20));
								win->draw(*sh);
								delete sh;

								for (int i = 0; i < 4; i++) {
									sh = new sf::RectangleShape;
									sh->setFillColor(sf::Color(0, 0, 0, 64));
									sh->setSize(sf::Vector2f(50, 42));
									sh->setPosition(WIN_WIDTH + 1 + 50 * i, 250 + 38 * 2 + 18);
									win->draw(*sh);
									delete sh;

									if (n->getTypeQ(i + 1) != -1) {
										spr = new sf::Sprite;
										spr->setTexture(interfacesprs[n->getTypeQ(i + 1)]);
										spr->setPosition(WIN_WIDTH + 3 + 50 * i, 250 + 38 * 2 + 20);
										win->draw(*spr);
										delete spr;
										if (pressed && !sf::Mouse::isButtonPressed(sf::Mouse::Left)) {//если отпустили клавишу
											sf::Vector2i pos = sf::Mouse::getPosition(*win);
											if (pos.x >= WIN_WIDTH + 3 + 50 * i && pos.x <= WIN_WIDTH + 3 + 50 * i + 46 && pos.y >= 250 + 38 * 2 + 20 && pos.y <= 250 + 38 * 2 + 20 + 38) {
												n->getComm(10 + i + 1);
											}
										}
									}
								}
							}
						}
						break;
					}
					case 1: {
						for (int i = 0; i < 4; i++) {
							int butx = WIN_WIDTH + 3 + 49 * i;
							int buty = 400;
							if (pressed && !sf::Mouse::isButtonPressed(sf::Mouse::Left)) {//если отпустили клавишу
								sf::Vector2i pos = sf::Mouse::getPosition(*win);
								if (pos.x >= butx && pos.x <= butx + 46 && pos.y >= buty && pos.y <= buty + 38) {
									n->getComm(i + 1);
								}
							}
							spr = new sf::Sprite;
							spr->setTexture(interfacesprs[i + 1]);
							spr->setPosition(butx, buty);
							win->draw(*spr);
							delete spr;
							spr = new sf::Sprite;
							spr->setTexture(resourse[0]);
							spr->setScale(sf::Vector2f(0.7, 0.7));
							spr->setPosition(butx, buty);
							win->draw(*spr);
							delete spr;
							{
								sf::Text text;
								text.setFont(font);
								ostringstream str;
								str << costGold[i + 1];
								text.setString(str.str());
								text.setCharacterSize(10);
								text.setPosition(butx + 10, buty);
								win->draw(text);
							}
							spr = new sf::Sprite;
							spr->setTexture(resourse[1]);
							spr->setPosition(butx, buty + 10);
							spr->setScale(sf::Vector2f(0.7, 0.7));
							win->draw(*spr);
							delete spr;
							{
								sf::Text text;
								text.setFont(font);
								ostringstream str;
								str << costWood[i + 1];
								text.setString(str.str());
								text.setCharacterSize(10);
								text.setPosition(butx + 10, buty + 10);
								win->draw(text);
							}
							spr = new sf::Sprite;
							spr->setTexture(resourse[2]);
							spr->setPosition(butx, buty + 20);
							spr->setScale(sf::Vector2f(0.7, 0.7));
							win->draw(*spr);
							delete spr;
							{
								sf::Text text;
								text.setFont(font);
								ostringstream str;
								str << needFood[i + 1];
								text.setString(str.str());
								text.setCharacterSize(10);
								text.setPosition(butx + 10, buty + 20);
								win->draw(text);
							}
						}
						if (n->getSizeQ() > 0) {
							int i = n->getTypeQ(0);
							{
								sf::RectangleShape* sh = new sf::RectangleShape;
								sh->setFillColor(sf::Color(0, 0, 0, 128));
								sh->setPosition(WIN_WIDTH + 1, 235);
								sh->setSize(sf::Vector2f(win->getSize().x - (WIN_WIDTH + 1) - 1, 38 * 2 + 80));
								win->draw(*sh);
								delete sh;

								spr = new sf::Sprite;
								spr->setTexture(interfacesprs[i]);
								spr->setPosition(WIN_WIDTH + 3, 250);
								spr->setScale(2, 2);
								win->draw(*spr);
								if (pressed && !sf::Mouse::isButtonPressed(sf::Mouse::Left)) {//если отпустили клавишу
									sf::Vector2i pos = sf::Mouse::getPosition(*win);
									if (pos.x >= WIN_WIDTH + 1 && pos.x <= WIN_WIDTH + 1 + 46 * 2 && pos.y >= 235 && pos.y <= 235 + 38 * 2) {
										n->getComm(10);
									}
								}
								delete spr;

								sf::Text text;
								text.setFont(font);
								text.setString("Training...");
								text.setCharacterSize(20);
								text.setPosition(WIN_WIDTH + 3 + 46 * 2 + 3, 250);
								win->draw(text);

								sh = new sf::RectangleShape;
								sh->setFillColor(sf::Color(0, 0, 0, 128));
								sh->setPosition(WIN_WIDTH + 3 + 46 * 2 + 3, 275);
								sh->setSize(sf::Vector2f(win->getSize().x - (WIN_WIDTH + 3 + 46 * 2 + 3) - 3, 20));
								win->draw(*sh);
								delete sh;

								sh = new sf::RectangleShape;
								sh->setFillColor(sf::Color(0, 128, 0));
								sh->setPosition(WIN_WIDTH + 3 + 46 * 2 + 3, 275);
								sh->setSize(sf::Vector2f(float((float)n->timer / (float)n->maxtimer) * (float)(win->getSize().x - (WIN_WIDTH + 3 + 46 * 2 + 3) - 3), 20));
								win->draw(*sh);
								delete sh;

								for (int i = 0; i < 4; i++) {
									sh = new sf::RectangleShape;
									sh->setFillColor(sf::Color(0, 0, 0, 64));
									sh->setSize(sf::Vector2f(50, 42));
									sh->setPosition(WIN_WIDTH + 1 + 50 * i, 250 + 38 * 2 + 18);
									win->draw(*sh);
									delete sh;

									if (n->getTypeQ(i + 1) != -1) {
										spr = new sf::Sprite;
										spr->setTexture(interfacesprs[n->getTypeQ(i + 1)]);
										spr->setPosition(WIN_WIDTH + 3 + 50 * i, 250 + 38 * 2 + 20);
										win->draw(*spr);
										delete spr;
										if (pressed && !sf::Mouse::isButtonPressed(sf::Mouse::Left)) {//если отпустили клавишу
											sf::Vector2i pos = sf::Mouse::getPosition(*win);
											if (pos.x >= WIN_WIDTH + 3 + 50 * i && pos.x <= WIN_WIDTH + 3 + 50 * i + 46 && pos.y >= 250 + 38 * 2 + 20 && pos.y <= 250 + 38 * 2 + 20 + 38) {
												n->getComm(10 + i + 1);
											}
										}
									}
								}
							}
						}
						break;
					}
					case 3:
						for (int i = 0; i < 2; i++) {
							int butx = WIN_WIDTH + 3 + 49 * i;
							int buty = 400;
							int typ = i + 5;
							if (i == 1) typ += n->getTeam();
							if (pressed && !sf::Mouse::isButtonPressed(sf::Mouse::Left)) {//если отпустили клавишу
								sf::Vector2i pos = sf::Mouse::getPosition(*win);
								if (pos.x >= butx && pos.x <= butx + 46 && pos.y >= buty && pos.y <= buty + 38) {
									n->getComm(typ);
								}
							}
							spr = new sf::Sprite;
							spr->setTexture(interfacesprs[typ]);
							spr->setPosition(butx, buty);
							win->draw(*spr);
							delete spr;
							spr = new sf::Sprite;
							spr->setTexture(resourse[0]);
							spr->setScale(sf::Vector2f(0.7, 0.7));
							spr->setPosition(butx, buty);
							win->draw(*spr);
							delete spr;
							{
								sf::Text text;
								text.setFont(font);
								ostringstream str;
								str << costGold[typ];
								text.setString(str.str());
								text.setCharacterSize(10);
								text.setPosition(butx + 10, buty);
								win->draw(text);
							}
							spr = new sf::Sprite;
							spr->setTexture(resourse[1]);
							spr->setPosition(butx, buty + 10);
							spr->setScale(sf::Vector2f(0.7, 0.7));
							win->draw(*spr);
							delete spr;
							{
								sf::Text text;
								text.setFont(font);
								ostringstream str;
								str << costWood[typ];
								text.setString(str.str());
								text.setCharacterSize(10);
								text.setPosition(butx + 10, buty + 10);
								win->draw(text);
							}
							spr = new sf::Sprite;
							spr->setTexture(resourse[2]);
							spr->setPosition(butx, buty + 20);
							spr->setScale(sf::Vector2f(0.7, 0.7));
							win->draw(*spr);
							delete spr;
							{
								sf::Text text;
								text.setFont(font);
								ostringstream str;
								str << needFood[typ];
								text.setString(str.str());
								text.setCharacterSize(10);
								text.setPosition(butx + 10, buty + 20);
								win->draw(text);
							}
						}
						if (n->getSizeQ() > 0) {
							int i = n->getTypeQ(0);
							{
								sf::RectangleShape* sh = new sf::RectangleShape;
								sh->setFillColor(sf::Color(0, 0, 0, 128));
								sh->setPosition(WIN_WIDTH + 1, 235);
								sh->setSize(sf::Vector2f(win->getSize().x - (WIN_WIDTH + 1) - 1, 38 * 2 + 80));
								win->draw(*sh);
								delete sh;

								spr = new sf::Sprite;
								spr->setTexture(interfacesprs[i]);
								spr->setPosition(WIN_WIDTH + 3, 250);
								spr->setScale(2, 2);
								win->draw(*spr);
								if (pressed && !sf::Mouse::isButtonPressed(sf::Mouse::Left)) {//если отпустили клавишу
									sf::Vector2i pos = sf::Mouse::getPosition(*win);
									if (pos.x >= WIN_WIDTH + 1 && pos.x <= WIN_WIDTH + 1 + 46 * 2 && pos.y >= 235 && pos.y <= 235 + 38 * 2) {
										n->getComm(10);
									}
								}
								delete spr;

								sf::Text text;
								text.setFont(font);
								text.setString("Training...");
								text.setCharacterSize(20);
								text.setPosition(WIN_WIDTH + 3 + 46 * 2 + 3, 250);
								win->draw(text);

								sh = new sf::RectangleShape;
								sh->setFillColor(sf::Color(0, 0, 0, 128));
								sh->setPosition(WIN_WIDTH + 3 + 46 * 2 + 3, 275);
								sh->setSize(sf::Vector2f(win->getSize().x - (WIN_WIDTH + 3 + 46 * 2 + 3) - 3, 20));
								win->draw(*sh);
								delete sh;

								sh = new sf::RectangleShape;
								sh->setFillColor(sf::Color(0, 128, 0));
								sh->setPosition(WIN_WIDTH + 3 + 46 * 2 + 3, 275);
								sh->setSize(sf::Vector2f(float((float)n->timer / (float)n->maxtimer) * (float)(win->getSize().x - (WIN_WIDTH + 3 + 46 * 2 + 3) - 3), 20));
								win->draw(*sh);
								delete sh;

								for (int i = 0; i < 4; i++) {
									sh = new sf::RectangleShape;
									sh->setFillColor(sf::Color(0, 0, 0, 64));
									sh->setSize(sf::Vector2f(50, 42));
									sh->setPosition(WIN_WIDTH + 1 + 50 * i, 250 + 38 * 2 + 18);
									win->draw(*sh);
									delete sh;

									if (n->getTypeQ(i + 1) != -1) {
										spr = new sf::Sprite;
										spr->setTexture(interfacesprs[n->getTypeQ(i + 1)]);
										spr->setPosition(WIN_WIDTH + 3 + 50 * i, 250 + 38 * 2 + 20);
										win->draw(*spr);
										delete spr;
										if (pressed && !sf::Mouse::isButtonPressed(sf::Mouse::Left)) {//если отпустили клавишу
											sf::Vector2i pos = sf::Mouse::getPosition(*win);
											if (pos.x >= WIN_WIDTH + 3 + 50 * i && pos.x <= WIN_WIDTH + 3 + 50 * i + 46 && pos.y >= 250 + 38 * 2 + 20 && pos.y <= 250 + 38 * 2 + 20 + 38) {
												n->getComm(10 + i + 1);
											}
										}
									}
								}
							}
						}
						break;
					}
				}
			}
		}
		else {
			object* newselect = NULL;
			for (int k = 0; k < selectedObj.size(); k++) {
				unit* n = (unit*)selectedObj[k];
				if (k < 8) {
					sf::Sprite spr;
					int butx = WIN_WIDTH + 3 + (k % 4) * 49;
					int buty = 50 + (k / 4) * 40;
					spr.setTexture(interfacesprs[n->getType()]);
					spr.setPosition(butx, buty);
					win->draw(spr);
					if (pressed && !sf::Mouse::isButtonPressed(sf::Mouse::Left)) {//если отпустили клавишу
						sf::Vector2i pos = sf::Mouse::getPosition(*win);
						if (pos.x >= butx && pos.x <= butx + 46 && pos.y >= buty && pos.y <= buty + 38) {
							newselect = selectedObj[k];
						}
					}
				}
			}

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
				interfacestatys = 10;
				waitforclick = 1;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::M)) {
				interfacestatys = 11;
				waitforclick = 1;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
				interfacestatys = 12;
				waitforclick = 1;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
				interfacestatys = 13;
				waitforclick = 1;
			}

			if (interfacestatys < 10 && !waitforclick) {
				for (int i = 0; i < 4; i++) {
					int butx = WIN_WIDTH + 3 + (i % 4) * 49;
					int buty = 300 + (i / 4) * 40;
					if (pressed && !sf::Mouse::isButtonPressed(sf::Mouse::Left)) {//если отпустили клавишу
						sf::Vector2i pos = sf::Mouse::getPosition(*win);
						if (pos.x >= butx && pos.x <= butx + 46 && pos.y >= buty && pos.y <= buty + 38) {
							interfacestatys = 10 + i;
							if (i == 0) {
								for (int k = 0; k < selectedObj.size(); k++) {
									doingobj.push_back(selectedObj[k]);
									doI.push_back(selectedObj[k]->getI());
									doJ.push_back(selectedObj[k]->getJ());
									doCM.push_back(1);
								}
								interfacestatys = 0;
							}
							waitforclick = 1;
							break;
						}
					}
				}
			}
			else {
				if (event.type == sf::Event::MouseButtonPressed)//если отпустили клавишу
					if (event.key.code == sf::Mouse::Left && waitforclick) {
						int n = interfacestatys % 10;
						switch (n) {
						case 0:
						{
							for (int k = 0; k < selectedObj.size(); k++) {
								doingobj.push_back(selectedObj[k]);
								doI.push_back(selectedObj[k]->getI());
								doJ.push_back(selectedObj[k]->getJ());
								doCM.push_back(1);
							}
							break;
						}
						case 1:
						{
							sf::Vector2i pos = sf::Mouse::getPosition(*win);
							if (pos.x >= 0 && pos.x < WIN_WIDTH && pos.y >= 0 && pos.y < WIN_HEIGHT) {

								int i = frScreentoGameY(pos.y, pointy);
								int j = frScreentoGameX(pos.x, pointx);

								for (int k = 0; k < selectedObj.size(); k++) {
									doingobj.push_back(selectedObj[k]);
									doI.push_back(i);
									doJ.push_back(j);
									doCM.push_back(1);
								}

							}
							break;
						}
						case 2: {
							sf::Vector2i pos = sf::Mouse::getPosition(*win);
							if (pos.x >= 0 && pos.x < WIN_WIDTH && pos.y >= 0 && pos.y < WIN_HEIGHT) {

								int i = frScreentoGameY(pos.y, pointy);
								int j = frScreentoGameX(pos.x, pointx);

								for (int k = 0; k < selectedObj.size(); k++) {
									doingobj.push_back(selectedObj[k]);
									doI.push_back(i);
									doJ.push_back(j);
									doCM.push_back(2);
								}

							}
							break;
						}
						case 3:
							sf::Vector2i pos = sf::Mouse::getPosition(*win);
							if (pos.x >= 0 && pos.x < WIN_WIDTH && pos.y >= 0 && pos.y < WIN_HEIGHT) {

								int i = frScreentoGameY(pos.y, pointy);
								int j = frScreentoGameX(pos.x, pointx);

								for (int k = 0; k < selectedObj.size(); k++) {
									doingobj.push_back(selectedObj[k]);
									doI.push_back(i);
									doJ.push_back(j);
									doCM.push_back(3);
								}

							}
							break;
						}

						interfacestatys = 0;
						waitforclick = 0;
					}
			}
			if (interfacestatys < 10) {
				if (interfacestatys != 1) {
					for (int i = 0; i < 4; i++) {
						sf::Sprite butspr;
						butspr.setTexture(buttonTex[i]);
						int butx = WIN_WIDTH + 3 + (i % 4) * 49;
						int buty = 300 + (i / 4) * 40;
						butspr.setPosition(butx, buty);
						win->draw(butspr);
					}
				}
				else {

				}
			}

			if (newselect != NULL) {
				while (selectedObj.size() > 0)
					selectedObj.pop_back();
				selectedObj.push_back(newselect);
			}
		}
	}
	pressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);
}

sf::Thread th[3] = { &gameplay, &drawMap, &analizeObjects };

void startthrs() {
	th[0].launch();
	th[1].launch();
	th[2].launch();
}

int main()
{
	win = new sf::RenderWindow(sf::VideoMode(WIN_WIDTH + 200, WIN_HEIGHT + 16), "Elemental Wars");

	font.loadFromFile("warcraft2/warfont.ttf");

	win->setKeyRepeatEnabled(false);
	//win->setMouseCursorGrabbed(true);

	//CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)gameplay, NULL, NULL, NULL);
	//CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)analizeObjects, NULL, NULL, NULL);
	//CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)drawMap, NULL, NULL, NULL);

	{
		textures[2][0].loadFromFile("warcraft2/tree.png");
		landspace[0].loadFromFile("warcraft2/water.png");
		landspace[1].loadFromFile("warcraft2/sand.png");
		landspace[2].loadFromFile("warcraft2/grass.png");
		textures[0][0].loadFromFile("warcraft2/human/units/peasant.png");
		textures[0][1].loadFromFile("warcraft2/human/units/footman.png");
		textures[0][2].loadFromFile("warcraft2/human/units/elven_archer.png");
		textures[0][3].loadFromFile("warcraft2/human/units/knight.png");
		sf::Image img;
		img.loadFromFile("warcraft2/human/units/mage.png");
		img.createMaskFromColor(sf::Color(255, 255, 255));
		textures[0][4].loadFromImage(img);
		textures[0][5].loadFromFile("warcraft2/human/units/catapult.png");

		textures[0][6].loadFromFile("warcraft2/human/units/dead_knight.png");
		textures[0][7].loadFromFile("warcraft2/human/units/dragon.png");
		textures[0][8].loadFromFile("warcraft2/human/units/gryphon_rider.png");
		textures[0][9].loadFromFile("warcraft2/human/units/gnomish_flying_machine.png");

		textures[1][0].loadFromFile("warcraft2/base.png");
		textures[1][2].loadFromFile("warcraft2/barracks.png");
		textures[1][3].loadFromFile("warcraft2/farm.png");
		textures[1][4].loadFromFile("warcraft2/factory.png");
		textures[1][1].loadFromFile("warcraft2/minerals.png");

		missles[0].loadFromFile("warcraft2/human/arrow.png");
		missles[1].loadFromFile("warcraft2/human/light.png");
		missles[2].loadFromFile("warcraft2/human/missile.png");
		missles[3].loadFromFile("warcraft2/human/bullet.png");

		interfacesprs[19].loadFromFile("warcraft2/ui/orc/800x600/statusline.png");
		interfacesprs[18].loadFromFile("warcraft2/ui/orc/panel_1.png");
		interfacesprs[0].loadFromFile("warcraft2/human/civicon.png");
		interfacesprs[1].loadFromFile("warcraft2/human/footicon.png");
		interfacesprs[2].loadFromFile("warcraft2/human/archericon.png");
		interfacesprs[3].loadFromFile("warcraft2/human/knighticon.png");
		interfacesprs[4].loadFromFile("warcraft2/human/mageicon.png");
		interfacesprs[5].loadFromFile("warcraft2/human/catapulticon.png");

		interfacesprs[9].loadFromFile("warcraft2/human/gyroicon.png");
		interfacesprs[7].loadFromFile("warcraft2/human/dragonicon.png");
		interfacesprs[6].loadFromFile("warcraft2/human/deadknicon.png");
		interfacesprs[8].loadFromFile("warcraft2/human/gryphicon.png");

		interfacesprs[10].loadFromFile("warcraft2/human/baseicon.png");
		interfacesprs[11].loadFromFile("warcraft2/human/barracksicon.png");
		interfacesprs[12].loadFromFile("warcraft2/human/farmicon.png");
		interfacesprs[13].loadFromFile("warcraft2/human/factoryicon.png");

		resourse[0].loadFromFile("warcraft2/human/goldicon.png");
		resourse[1].loadFromFile("warcraft2/human/woodicon.png");
		resourse[2].loadFromFile("warcraft2/human/foodicon.png");

		img.loadFromFile("warcraft2/human/units/peasant.png");
		img.createMaskFromColor(sf::Color(255, 255, 255));
		rabTex[0].loadFromImage(img);
		img.loadFromFile("warcraft2/human/units/peasant_with_gold.png");
		img.createMaskFromColor(sf::Color(255, 255, 255));
		rabTex[1].loadFromImage(img);
		img.loadFromFile("warcraft2/human/units/peasant_with_wood.png");
		img.createMaskFromColor(sf::Color(255, 255, 255));
		rabTex[2].loadFromImage(img);

		buttonTex[0].loadFromFile("Warcraft2/human/stayicon.png");
		buttonTex[1].loadFromFile("Warcraft2/human/walkicon.png");
		buttonTex[2].loadFromFile("Warcraft2/human/attackicon.png");
		buttonTex[3].loadFromFile("Warcraft2/human/defendicon.png");
		buttonTex[4].loadFromFile("Warcraft2/human/earnicon.png");
		buttonTex[5].loadFromFile("Warcraft2/human/buildicon.png");
	}	

	//output->startgame();

	string ipstr;

	interfacestatys = 0;
	while (win->isOpen())
	{
		

		switch (winstatys) {
		case 0:
		{
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
				win->close();
				break;
			}
			sf::Vector2i pos = sf::Mouse::getPosition(*win);

			while (win->pollEvent(event)) {
				if (event.type == sf::Event::Closed) {
					win->close();
				}
				if (event.type == sf::Event::MouseButtonReleased) {
					if (event.key.code == sf::Mouse::Left) {
						if (pos.x > (win->getSize().x - 150) / 2 && pos.x < (win->getSize().x - 150) / 2 + 150 && pos.y > win->getSize().y / 2 - 50 * 2 && pos.y < win->getSize().y / 2 - 50 * 2 + 50) {
							winstatys = 1;
							output = new Host(25656);
							for (int i = 0; i < 4; i++) {
								teams[i] = new team();
							}
						}
						else if (pos.x > (win->getSize().x - 150) / 2 && pos.x < (win->getSize().x - 150) / 2 + 150 && pos.y > win->getSize().y / 2 + 50 * 2 && pos.y < win->getSize().y / 2 + 50 * 2 + 50) {
							for (int i = 0; i < 4; i++) {
								teams[i] = new team();
							}
							output = new Client(ipstr, 25656);

							Client* cl = (Client*)output;

							if (cl->isConnected()) winstatys = 2;
							else {
								winstatys = 0;
								for (int i = 0; i < 4; i++) {
									delete teams[i];
								}
							}
						}
					}
				}
				if (event.type == sf::Event::KeyReleased) {
					if (event.key.code == sf::Keyboard::Num0)
					{
						ipstr += "0";
					}
					if (event.key.code == sf::Keyboard::Num1)
					{
						ipstr += "1";
					}
					if (event.key.code == sf::Keyboard::Num2)
					{
						ipstr += "2";
					}
					if (event.key.code == sf::Keyboard::Num3)
					{
						ipstr += "3";
					}
					if (event.key.code == sf::Keyboard::Num4)
					{
						ipstr += "4";
					}
					if (event.key.code == sf::Keyboard::Num5)
					{
						ipstr += "5";
					}
					if (event.key.code == sf::Keyboard::Num6)
					{
						ipstr += "6";
					}
					if (event.key.code == sf::Keyboard::Num7)
					{
						ipstr += "7";
					}
					if (event.key.code == sf::Keyboard::Num8)
					{
						ipstr += "8";
					}
					if (event.key.code == sf::Keyboard::Num9)
					{
						ipstr += "9";
					}
					if (event.key.code == sf::Keyboard::Period)
					{
						ipstr += ".";
					}
					if (event.key.code == sf::Keyboard::BackSpace)
					{
						if (ipstr.length() > 0)
							ipstr.erase(ipstr.length() - 1);
					}
				}
			}

			win->clear();

			sf::RectangleShape rect;
			rect.setPosition(0, 0);
			rect.setSize(sf::Vector2f(win->getSize().x, win->getSize().y));
			rect.setFillColor(sf::Color(0, 158, 97));
			win->draw(rect);

			rect.setPosition((win->getSize().x - 150) / 2, win->getSize().y / 2 - 50 * 2);
			rect.setSize(sf::Vector2f(150, 50));
			rect.setOutlineColor(sf::Color(0, 138, 77));
			rect.setOutlineThickness(5);
			rect.setFillColor(sf::Color::Transparent);
			win->draw(rect);

			rect.setPosition((win->getSize().x - 150) / 2, win->getSize().y / 2 + 50 * 2);
			rect.setSize(sf::Vector2f(150, 50));
			rect.setOutlineColor(sf::Color(0, 138, 77));
			rect.setOutlineThickness(5);
			rect.setFillColor(sf::Color::Transparent);
			win->draw(rect);

			sf::Text txt;

			txt.setFont(font);
			txt.setString("Host the game");
			if (pos.x > (win->getSize().x - 150) / 2 && pos.x < (win->getSize().x - 150) / 2 + 150 && pos.y > win->getSize().y / 2 - 50 * 2 && pos.y < win->getSize().y / 2 - 50 * 2 + 50)
				txt.setColor(sf::Color::White);
			else
				txt.setColor(sf::Color(0, 68, 27));
			txt.setCharacterSize(23);
			txt.setPosition((win->getSize().x - 150) / 2, win->getSize().y / 2 - 50 * 2 + (50.f / 5));
			win->draw(txt);

			txt.setFont(font);
			txt.setString("Write ip to join the game");
			txt.setColor(sf::Color(0, 68, 27));
			txt.setCharacterSize(23);
			txt.setPosition((win->getSize().x - 250) / 2, win->getSize().y / 2);
			win->draw(txt);

			rect.setPosition((win->getSize().x - 150) / 2, win->getSize().y / 2 + 30);
			rect.setSize(sf::Vector2f(150, 25));
			rect.setOutlineColor(sf::Color(0, 138, 77));
			rect.setOutlineThickness(3);
			rect.setFillColor(sf::Color::Transparent);
			win->draw(rect);

			txt.setFont(font);
			txt.setString(ipstr);
			txt.setColor(sf::Color(0, 68, 27));
			txt.setCharacterSize(20);
			txt.setPosition(rect.getPosition().x, rect.getPosition().y);
			win->draw(txt);

			txt.setFont(font);
			txt.setString("Join the game");
			if (pos.x > (win->getSize().x - 150) / 2 && pos.x < (win->getSize().x - 150) / 2 + 150 && pos.y > win->getSize().y / 2 + 50 * 2 && pos.y < win->getSize().y / 2 + 50 * 2 + 50)
				txt.setColor(sf::Color::White);
			else
				txt.setColor(sf::Color(0, 68, 27));
			txt.setCharacterSize(24);
			txt.setPosition((win->getSize().x - 150) / 2, win->getSize().y / 2 + 50 * 2 + (50.f / 5));
			win->draw(txt);

			win->display();
		}
		break;
		case 1:
		{
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
				win->close();
				break;
			}
			cout << sizeof(barracks) << endl;
			win->clear();
			Host* host = (Host*)output;
			sf::Vector2i pos = sf::Mouse::getPosition(*win);

			while (win->pollEvent(event)) {
				if (event.type == sf::Event::Closed) {
					delete output;
					win->close();
				}
				if (event.type == sf::Event::MouseButtonReleased) {
					if (event.key.code == sf::Mouse::Left) {
						if (pos.x > win->getSize().x - 125 && pos.x < win->getSize().x - 25 && pos.y > 15 && pos.y < 65) {
							output->startgame();
							startthrs();
						}
						if (pos.x > win->getSize().x - 125 && pos.x < win->getSize().x - 25 && pos.y > 80 && pos.y < 130) {
							delete output;
							for (int i = 0; i < 4; i++) {
								delete teams[i];
							}
							winstatys = 0;
						}
					}
				}
			}

			sf::RectangleShape rect;
			rect.setPosition(0, 0);
			rect.setSize(sf::Vector2f(win->getSize().x, win->getSize().y));
			rect.setFillColor(sf::Color(0, 138, 97));
			win->draw(rect);

			rect.setPosition(50, 50);
			rect.setSize(sf::Vector2f(win->getSize().x / 2 + 50, (win->getSize().y + 65) / 6));
			rect.setOutlineColor(sf::Color(0, 68, 27));
			rect.setOutlineThickness(5);
			rect.setFillColor(sf::Color::Transparent);
			win->draw(rect);

			sf::Text txt;
			txt.setFont(font);
			sf::IpAddress ip = sf::IpAddress::getLocalAddress();
			txt.setString("Your address: " + ip.toString());
			txt.setColor(teamcolors[0]);
			txt.setCharacterSize(40);
			txt.setPosition(rect.getPosition().x, rect.getPosition().y);
			win->draw(txt);

			mtx.lock();
			for (int i = 0; i < 3; i++) if (host->clients[i] != NULL) {
				rect.setPosition(50, 50 + (15 + (win->getSize().y + 65) / 6) * (i + 1));
				rect.setSize(sf::Vector2f(win->getSize().x / 2 + 50, (win->getSize().y + 65) / 6));
				rect.setOutlineColor(sf::Color(0, 68, 27));
				rect.setOutlineThickness(5);
				rect.setFillColor(sf::Color::Transparent);
				win->draw(rect);

				sf::Text txt;
				txt.setFont(font);
				sf::IpAddress ip = sf::IpAddress::getPublicAddress();
				txt.setString(host->clients[i]->getRemoteAddress().toString());
				txt.setColor(teamcolors[i + 1]);
				txt.setCharacterSize(40);
				txt.setPosition(rect.getPosition().x, rect.getPosition().y);
				win->draw(txt);
			}
			mtx.unlock();

			rect.setPosition(win->getSize().x - 125, 15);
			rect.setSize(sf::Vector2f(100, 50));
			rect.setOutlineColor(sf::Color(0, 68, 27));
			rect.setOutlineThickness(5);
			rect.setFillColor(sf::Color::Transparent);
			win->draw(rect);

			txt.setFont(font);
			txt.setString("START");
			if (pos.x > rect.getPosition().x&& pos.x < rect.getPosition().x + rect.getSize().x && pos.y > rect.getPosition().y&& pos.y < rect.getPosition().y + rect.getSize().y)
				txt.setColor(sf::Color::White);
			else txt.setColor(sf::Color(0, 68, 27));
			txt.setCharacterSize(35);
			txt.setPosition(rect.getPosition().x, rect.getPosition().y);
			win->draw(txt);

			rect.setPosition(win->getSize().x - 125, 80);
			rect.setSize(sf::Vector2f(100, 50));
			rect.setOutlineColor(sf::Color(0, 68, 27));
			rect.setOutlineThickness(5);
			rect.setFillColor(sf::Color::Transparent);
			win->draw(rect);

			txt.setFont(font);
			txt.setString("LEAVE");
			if (pos.x > rect.getPosition().x&& pos.x < rect.getPosition().x + rect.getSize().x && pos.y > rect.getPosition().y&& pos.y < rect.getPosition().y + rect.getSize().y)
				txt.setColor(sf::Color::White);
			else txt.setColor(sf::Color(0, 68, 27));
			txt.setCharacterSize(35);
			txt.setPosition(rect.getPosition().x, rect.getPosition().y);
			win->draw(txt);

			win->display();
		}
		break;
		case 2:
		{
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
				win->close();
				break;
			}
			win->clear();

			sf::Vector2i pos = sf::Mouse::getPosition(*win);

			while (win->pollEvent(event)) {
				if (event.type == sf::Event::Closed) {
					delete output;
					win->close();
				}
				if (event.type == sf::Event::MouseButtonReleased) {
					if (event.key.code == sf::Mouse::Left) {
						if (pos.x > 3 * win->getSize().x / 4 && pos.x < 3 * win->getSize().x / 4 + 100 && pos.y > win->getSize().y - 50 * 2 && pos.y < win->getSize().y - 50 * 2 + 50)
						{
							winstatys = 0;
							for (int i = 0; i < 4; i++) {
								delete teams[i];
							}
							delete output;
						}
					}
				}
			}

			sf::RectangleShape rect;
			rect.setPosition(0, 0);
			rect.setSize(sf::Vector2f(win->getSize().x, win->getSize().y));
			rect.setFillColor(sf::Color(0, 158, 97));
			win->draw(rect);

			sf::Text txt;
			txt.setFont(font);
			txt.setString("Successfully connected to " + ipstr);
			txt.setColor(sf::Color(0, 68, 27));
			txt.setCharacterSize(40);
			txt.setPosition(100, 100);
			win->draw(txt);

			txt.setFont(font);
			txt.setString("Waiting for host...");
			txt.setColor(sf::Color(0, 68, 27));
			txt.setCharacterSize(40);
			txt.setPosition(100, 150);
			win->draw(txt);

			rect.setPosition(3 * win->getSize().x / 4, win->getSize().y - 50 * 2);
			rect.setSize(sf::Vector2f(100, 50));
			rect.setFillColor(sf::Color::Transparent);
			rect.setOutlineColor(sf::Color(0, 68, 27));
			rect.setOutlineThickness(3);
			win->draw(rect);

			txt.setFont(font);
			txt.setString("LEAVE");
			if (pos.x > rect.getPosition().x&& pos.x < rect.getPosition().x + rect.getSize().x && pos.y > rect.getPosition().y&& pos.y < rect.getPosition().y + rect.getSize().y)
				txt.setColor(sf::Color::White);
			else txt.setColor(sf::Color(0, 68, 27));
			txt.setCharacterSize(30);
			txt.setPosition(rect.getPosition().x, rect.getPosition().y);
			win->draw(txt);

			win->display();
		}
		break;
		case 10:
			while (win->pollEvent(event))
			{
				/*if (selectedObj.size() > 0) {
					(unit*)
					
					cout << 
				}*/
				if (event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
					for (int i = 0; i < 3; i++) {
						th[i].terminate();
					}
					win->close();
					isAnalizyng = 1;
				}
				if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
					if (interfacestatys != 1) interfacestatys = 0;
					waitforclick = 0;
				}
				else
					if (event.type == sf::Event::MouseButtonReleased) {
						if (event.key.code == sf::Mouse::Right && !waitforclick) {
							sf::Vector2i pos = sf::Mouse::getPosition(*win);
							if (pos.x >= 0 && pos.x < WIN_WIDTH && pos.y >= 0 && pos.y < WIN_HEIGHT) {
								int i = frScreentoGameY(pos.y, pointy);
								int j = frScreentoGameX(pos.x, pointx);

								for (int k = 0; k < selectedObj.size(); k++) {
									doingobj.push_back(selectedObj[k]);
									doI.push_back(i);
									doJ.push_back(j);
									doCM.push_back(0);
								}
							}
						}
					}
			}

			

			if (!isAnalizyng) {
				win->clear();
				draw(pointx, pointy);
				drawInterFace();
				sf::Texture txt;
				txt.loadFromImage(minimap);
				sf::Sprite spr;
				spr.setTexture(txt);
				spr.setPosition(WIN_WIDTH + (200 - 128) / 2, WIN_HEIGHT - 128);
				win->draw(spr);
				win->display();
			}
			break;
		}

	}

	return 0;
}