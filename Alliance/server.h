#include "gameplay.h"
#include <SFML/Network.hpp>
#include <time.h>
#include <iostream>
#include <Windows.h>
#include <thread>
#include <vector>

using namespace std;

enum TYPE_OF_SENDER {
	RESPONSE,
	REQUEST
};

volatile bool receiving = 0;

sf::Thread* analth = NULL;

volatile int analcount = 0;

int playercount = 1;

std::vector<object*> selectedObj;
std::list<object*> doingobj;
std::vector<object*> deadlst;

sf::Event event;

int keyl;

sf::Image minimap;

void deleteall() {
	while (allobjs.size()) {
		deadlst.push_back(allobjs.front());
		deadlst.back()->die();
	}
	while (deadlst.size() > 0) {
		delete deadlst.back();
		deadlst.pop_back();
	}
}

int createMap(int key, const int playercount) {
	srand(key);

	heightMap[0][0] = rand() % mount;
	heightMap[128][0] = rand() % mount;
	heightMap[0][128] = rand() % mount;
	heightMap[128][128] = rand() % mount;

	diamondsquare();
	feeltrees(heightMap);

	int coorI[8] = { 0 };
	int coorJ[8] = { 0 };

	bool f = 1;
	bool t = 1;

	for (int k = 0; k < playercount; k++) {
		if (t == 1) {
			t = 0;
			for (int i = 14; i < 115; i++) {
				for (int j = 14; j < 115; j++) {
					if (heightMap[i][j] < forest + 110 && heightMap[i][j] > forest + 90) {
						for (int m = 0; m < k; m++) {
							if (pow(i - coorI[m], 2) + pow(j - coorJ[m], 2) < 3600) {
								f = 0;
								break;
							}
						}
						if (f) {
							if (coorI[k] != 0) {
								coorI[k] = i;
								coorJ[k] = j;
								t = 1;
							}
							else {
								int r = rand() % 100;
								if (r == 0) {
									coorI[k] = i;
									coorJ[k] = j;
									t = 1;
								}
							}
							break;
							break;
						}
						f = 1;
					}
				}
			}
		}
	}

	if (t == 1)
	for (int k = 0; k < playercount; k++) {
		if (t == 1) {
			t = 0;
			for (int i = 14; i < 115; i++) {
				for (int j = 14; j < 115; j++) {
					if (heightMap[i][j] > sand + 50 && heightMap[i][j] < forest - 100) {
						for (int m = 0; m < k; m++) {
							if (pow(i - coorI[m], 2) + pow(j - coorJ[m], 2) < 2025) {
								f = 0;
								break;
							}
						}

						if (f)
						for (int m = 4; m < k + 4; m++) {
							if (pow(i - coorI[m], 2) + pow(j - coorJ[m], 2) < 2025) {
								f = 0;
								break;
							}
						}

						if (f) {
							if (coorI[k + 4] != 0) {
								coorI[k + 4] = i;
								coorJ[k + 4] = j;
								t = 1;
							}
							else {
								int r = rand() % 100;
								if (r == 0) {
									coorI[k + 4] = i;
									coorJ[k + 4] = j;
									t = 1;
								}
							}
							break;
							break;
						}
						f = 1;
					}
				}
			}
		}
	}

	if (t) {
		for (int k = 0; k < playercount; k++) {
			for (int i = coorI[k] - 5; i <= coorI[k] + 5; i++) {
				for (int j = coorJ[k] - 5; j <= coorJ[k] + 10; j++) {
					if (objMap[i][j] != NULL) {
						deadlst.push_back(objMap[i][j]);
						objMap[i][j]->die();
					}
				}
			}

			objMap[coorI[k]][coorJ[k]] = new base(coorI[k], coorJ[k], k);
				objMap[coorI[k]][coorJ[k]]->gotdmg(-500);
				objMap[coorI[k]][coorJ[k]]->gotdmg(-500);
				objMap[coorI[k]][coorJ[k]]->gotdmg(-499);

			for (int j = coorJ[k] - 2; j <= coorJ[k] + 2; j++) {
				objMap[coorI[k] - 3][j] = new civil(coorI[k] - 3, j, k);
			}

			for (int i = coorI[k] - 3; i <= coorI[k] + 3; i++) objMap[i][coorJ[k] + 7] = new mine(i, coorJ[k] + 7);

			if (k == myteam) {
				pointx = coorJ[k] * 64 - win->getSize().x / 2;
				pointy = coorI[k] * 64 - win->getSize().y / 2;
			}
		}

		for (int k = 4; k < playercount + 4; k++) {
			for (int i = coorI[k] - 5; i <= coorI[k] + 5; i++) {
				for (int j = coorJ[k] - 5; j <= coorJ[k] + 10; j++) {
					if (objMap[i][j] != NULL) {
						deadlst.push_back(objMap[i][j]);
						objMap[i][j]->die();
					}
				}
			}

			for (int i = coorI[k] - 2; i <= coorI[k] + 2; i++)
				for (int j = coorJ[k] - 2; j <= coorJ[k] + 2; j++) objMap[i][j] = new mine(i, j);
		}

		minimap.create(129, 129);

		for (int i = 0; i < 129; i++) {
			for (int j = 0; j < 129; j++) {
				if (heightMap[i][j] <= sand)
					minimap.setPixel(j, i, sf::Color(0, 0, 55 + heightMap[i][j]));
				else
					if (heightMap[i][j] <= forest) { minimap.setPixel(j, i, sf::Color(127 + heightMap[i][j] / 4, 127 + heightMap[i][j] / 4, 0)); }
					else
						if (heightMap[i][j] <= mount) { minimap.setPixel(j, i, sf::Color(0, 255 - heightMap[i][j] / 4, 0)); }
						else
							minimap.setPixel(j, i, sf::Color(heightMap[i][j] / 4, heightMap[i][j] / 4, heightMap[i][j] / 4));
			}
		}
		return key;
	}
	else {
		deleteall();
		key = rand();
		return createMap(key, playercount);
	}
}

void listenHost(int);

class Host : public player {

	int port;

	sf::TcpListener lstr;
public:

	sf::TcpSocket* clients[3] = { NULL };

	sf::Thread* thrs[4] = { NULL };

	void waitCon() {
		sf::TcpListener lstr;

		while (1) {

			lstr.listen(port);
			int i = 0;
			while (clients[i] != NULL && i < 3) { i++; }
			if (i < 3) {
				sf::TcpSocket* sock = new sf::TcpSocket;
				lstr.accept(*sock);
				clients[i] = sock;
				playercount++;
				sf::Packet packet;
				packet << 0 << 0 << i + 1 << MS_SYS_MYTEAM << 0;
				clients[i]->send(packet);
				if (thrs[i + 1] != NULL) {
					thrs[i + 1]->terminate();
					delete thrs[i + 1];
				}
				thrs[i + 1] = new sf::Thread(&listenHost, i);
				thrs[i + 1]->launch();
				sf::sleep(sf::microseconds(10));
			}
			i = 0;
		}
	}

	Host(int port) {
		this->port = port;
		for (int i = 0; i < 1; i++) {
			clients[i] = NULL;
		}
		thrs[0] = new sf::Thread(&Host::waitCon, this);
		thrs[0]->launch();
	}

	void startgame() override {
		keyl = createMap(time(NULL), playercount);
		thrs[0]->terminate();
		delete thrs[0];

		vector<int> vec;
		vec.push_back(0);
		for (int i = 0; i < 3; i++) {
			if (clients[i] != NULL)
				vec.push_back(i + 1);
		}
		int yourt = rand() % vec.size();
		myteam = vec[yourt];
		vec.erase(vec.begin() + yourt);

		for (int i = 0; i < 3; i++) {
			if (clients[i] != NULL) {
				sf::Packet packet;
				int yourt = rand() % vec.size();
				packet << keyl << playercount << vec[yourt] << MS_SYS_GENERKEY << 0;
				vec.erase(vec.begin() + yourt);
				// send on ip address later
				clients[i]->send(packet);
			}
		}
		winstatys = 10;
	}

	void sendActivity(int i, int j, int team, TYPE_MSG tip, int addoper) override {
		sf::Packet packet;
		packet << i << j << team << (int)tip << addoper << RESPONSE;
		if (tip != MS_GAM_CREATE && tip != MS_GAM_LEAVE_MINE && tip != MS_GAM_HIT && tip != MS_GAM_COLLECT) {
			unit* un = (unit*)objMap[i][j];
			un->waitServer = 1;
		}

		this->getActivity(packet);
		// send on ip address later
		for (int i = 0; i < 3; i++) {
			if (clients[i] != NULL) {
				clients[i]->send(packet);
			}
		}

		sf::sleep(sf::microseconds(10));
	}

	void getActivity(sf::Packet pack) override {
		int i, j, team, tip, addoper, sndr;
		TYPE_MSG typ;
		TYPE_OF_SENDER sender;
		pack >> i;
		pack >> j;
		pack >> team;
		pack >> tip;
		pack >> addoper;
		pack >> sndr;
		typ = (TYPE_MSG)tip;
		sender = (TYPE_OF_SENDER)sndr;


		switch (sender) {
		case REQUEST: {
			switch (typ) {
			case MS_SYS_NOTHING: break;
			case MS_GAM_CREATE: { // addoper = unit|build(0|1) * 10 + type
				sf::Packet pack;
				pack << i << j << team << tip << addoper << RESPONSE;
				getActivity(pack);
				for (int k = 0; k < 3; k++) {
					if (clients[k] != NULL) {
						clients[k]->send(pack);
					}
				}
				break;
			}
			case MS_GAM_HIT: {// addoper = dmg
				if (objMap[i][j] == NULL) break;

				sf::Packet pack;
				pack << i << j << team << tip << addoper << RESPONSE;
				getActivity(pack);
				for (int k = 0; k < 3; k++) {
					if (clients[k] != NULL) {
						clients[k]->send(pack);
					}
				}

				break;
			}
			case MS_GAM_MOVE: {// addoper = i * 1000 + j
				if (objMap[i][j] == NULL) break;
				if (objMap[i][j]->isUnit() == 0) break;

				if (objMap[addoper / 1000][addoper % 1000] != NULL) break;

				sf::Packet pack;
				pack << i << j << team << tip << addoper << RESPONSE;
				getActivity(pack);
				for (int k = 0; k < 3; k++) {
					if (clients[k] != NULL) {
						clients[k]->send(pack);
					}
				}
				break;
			}
			case MS_GAM_ATTACK: {// addoper = nexti * 1000 + nextj
				if (objMap[i][j] == NULL) break;
				if (objMap[i][j]->isUnit() == 0) break;

				if (objMap[addoper / 1000][addoper % 1000] == NULL) break;

				sf::Packet pack;
				pack << i << j << team << tip << addoper << RESPONSE;
				getActivity(pack);
				for (int k = 0; k < 3; k++) {
					if (clients[k] != NULL) {
						clients[k]->send(pack);
					}
				}
			}
				break;
			case MS_GAM_COLLECT: {// int tip = addoper / 100; int count = addoper % 100;
				if (objMap[i][j] == NULL) break;
				if (objMap[i][j]->isUnit() == 0) break;

				unit* un = (unit*)objMap[i][j];
				if (un->getType() != 0) break;

				sf::Packet pack;
				pack << i << j << team << tip << addoper << RESPONSE;
				getActivity(pack);
				for (int k = 0; k < 3; k++) {
					if (clients[k] != NULL) {
						clients[k]->send(pack);
					}
				}
			}
				break;
			case MS_GAM_PUT: // civil* un = (civil*)objMap[i][j];
				if (objMap[i][j] == NULL) break;
				if (objMap[i][j]->isUnit() == 0) break;

				unit* un = (unit*)objMap[i][j];
				if (un->getType() != 0) break;

				sf::Packet pack;
				pack << i << j << team << tip << addoper << RESPONSE;
				getActivity(pack);
				for (int k = 0; k < 3; k++) {
					if (clients[k] != NULL) {
						clients[k]->send(pack);
					}
				}
				break;
			}
		}
		break;
		case RESPONSE:
			switch (typ) {
			case MS_SYS_NOTHING: break;
			case MS_GAM_CREATE: // addoper = unit|build(0|1) * 10 + type
				switch (addoper / 10) {
				case 0:
					switch (addoper % 10) {
					case 0:
						objMap[i][j] = new civil(i, j, team);
						break;
					case 1:
						objMap[i][j] = new footman(i, j, team);
						break;
					case 2:
						objMap[i][j] = new archer(i, j, team);
						break;
					case 3:
						objMap[i][j] = new knight(i, j, team);
						break;
					case 4:
						objMap[i][j] = new mage(i, j, team);
						break;
					case 5:
						objMap[i][j] = new catapult(i, j, team);
						break;
					case 6:
						objMap[i][j] = new earthknight(i, j, team);
						break;
					case 7:
						objMap[i][j] = new dragon(i, j, team);
						break;
					case 8:
						objMap[i][j] = new gryphon(i, j, team);
						break;
					case 9:
						objMap[i][j] = new flymachine(i, j, team);
						break;
					}
					break;
				case 1:
					switch (addoper % 10) {
					case 0:
						objMap[i][j] = new base(i, j, team);
						break;
					case 1:
						objMap[i][j] = new barracks(i, j, team);
						break;
					case 2:
						objMap[i][j] = new farm(i, j, team);
						break;
					case 3:
						objMap[i][j] = new factory(i, j, team);
						break;
					}
					break;
				}
				break;
			case MS_GAM_HIT: // addoper = dmg
				
				objMap[i][j]->gotdmg(addoper);

			break;
			case MS_GAM_MOVE: {// addoper = i * 1000 + j
				
				int nexti = addoper / 1000;
				int nextj = addoper % 1000;

				unit* un = (unit*)objMap[i][j];
				un->waitServer = 0;

				if (un->anim > 0) {
					un->anim = 0;
					if (un->animstatys == 1) {
						objMap[un->i][un->j] = NULL;
						un->i = un->nexti;
						un->j = un->nextj;
						un->animstatys = 0;
					}
				}

				if (objMap[nexti][nextj] != NULL && objMap[nexti][nextj] != un) {
					if (team != myteam) {
						un->statys = 0;
						break;
					}
					un->wait = 64;
					break;
				}

				if (team != myteam) {
					un->statys = 1;
					un->wayI = nexti;
					un->wayJ = nextj;
				}
				un->nexti = nexti;
				un->nextj = nextj;
				un->direction = un->getdir(un->nexti, un->nextj);
				if (objMap[un->nexti][un->nextj] == NULL || objMap[un->nexti][un->nextj] == un) {
					un->anim++;
					objMap[un->nexti][un->nextj] = un;
				}
				else {
					un->wait = 64;
				}
				un->animstatys = 1;
				break;
			}
			case MS_GAM_ATTACK: { // addoper = nexti * 1000 + nextj

				unit* un = (unit*)objMap[i][j];
				un->waitServer = 0;

				if (un->anim > 0) {
					un->anim = 0;
					if (un->animstatys == 1) {
						objMap[un->i][un->j] = NULL;
						un->i = un->nexti;
						un->j = un->nextj;
						un->animstatys = 0;
					}
				}

				if (team != myteam) {
					un->statys = 9;
				}
				un->attTarget = objMap[addoper / 1000][addoper % 1000];
				un->animstatys = 5;
				un->nexti = un->i;
				un->nextj = un->j;

			}
				break;
			case MS_SYS_MYTEAM:
				myteam = team;
				break;
			case MS_GAM_COLLECT: { // int tip = addoper / 100; int count = addoper % 100;

				civil* un = (civil*)objMap[i][j];
				un->waitServer = 0;

				if (addoper / 100 != un->typestorage) un->storage = 0;
				un->storage += addoper % 100;
				un->typestorage = addoper / 100;
			}
				break;
			case MS_GAM_PUT: {// civil* un = (civil*)objMap[i][j];
				civil* un = (civil*)objMap[i][j];
				un->waitServer = 0;

				teams[un->getTeam()]->sendMessage(5 - un->typestorage + 1, &un->storage);
				un->storage = 0;
				un->typestorage = 0;
			}
				break;
			case MS_SYS_GENERKEY:
				myteam = team;
				createMap(i, j);
				break;
			case MS_SYS_DISCONNECT:
				if (team != myteam) {
					clients[team - 1]->disconnect();
					delete clients[team - 1];
					clients[team - 1] = NULL;
				}
				break;
			case MS_GAM_NEXTTICK:
				anal();
				break;
			}
		}
	}

	~Host() override {
		mtx.lock();
		for (int i = 0; i < 4; i++) {
			if (thrs[i] != NULL) thrs[i]->terminate();
		}

		for (int i = 0; i < 3; i++) {
			if (clients[i] != NULL) {
				sf::Packet pack;
				pack << 0 << 0 << myteam << MS_SYS_DISCONNECT << 0;
				clients[i]->send(pack);
			}
		}
		winstatys = 0;
		mtx.unlock();
	}

	friend void listenHost(int);
};

void listenHost(int i) {
	Host* host = (Host*)output;

	while (1) {
		sf::Packet pack;
		host->clients[i]->receive(pack);
		if (pack.getDataSize() > 0) {
			receiving = 1;
			mtx.lock();
			output->getActivity(pack);
			mtx.unlock();
			receiving = 0;
		}
	}
}

void lis();

void analizeObjects();

class Client : public player {

	bool connected = 0;

	sf::Thread* thr;

public:

	sf::TcpSocket server;

	Client(sf::IpAddress addr, int port) {

		if (server.connect(addr, port, sf::seconds(3)) == sf::Socket::Done) {
			connected = 1;
			sf::Packet pack;
			thr = new sf::Thread(lis);
			thr->launch();
		}
		else {
			connected = 0;
		}

	}

	void startgame() override {

	}

	bool isConnected() {
		return connected;
	}

	void sendActivity(int i, int j, int team, TYPE_MSG tip, int addoper) override {
		sf::Packet packet;
		packet << i << j << team << (int)tip << addoper << REQUEST;
		if (tip != MS_GAM_CREATE && tip != MS_GAM_LEAVE_MINE && tip != MS_GAM_HIT && tip != MS_GAM_COLLECT) {
			unit* un = (unit*)objMap[i][j];
			un->waitServer = 1;
		}
		// send on ip address later
		server.send(packet);
	}

	void getActivity(sf::Packet pack) override {
		int i, j, team, tip, addoper, sndr;
		TYPE_MSG typ;
		TYPE_OF_SENDER sender;
		pack >> i;
		pack >> j;
		pack >> team;
		pack >> tip;
		pack >> addoper;
		pack >> sndr;
		typ = (TYPE_MSG)tip;
		sender = (TYPE_OF_SENDER)sndr;

		if (sender == REQUEST) return;

		switch (typ) {
		case MS_SYS_NOTHING: break;
		case MS_GAM_CREATE: // addoper = unit|build(0|1) * 10 + type
			switch (addoper / 10) {
			case 0:
				switch (addoper % 10) {
				case 0:
					objMap[i][j] = new civil(i, j, team);
					break;
				case 1:
					objMap[i][j] = new footman(i, j, team);
					break;
				case 2:
					objMap[i][j] = new archer(i, j, team);
					break;
				case 3:
					objMap[i][j] = new knight(i, j, team);
					break;
				case 4:
					objMap[i][j] = new mage(i, j, team);
					break;
				case 5:
					objMap[i][j] = new catapult(i, j, team);
					break;
				case 6:
					objMap[i][j] = new earthknight(i, j, team);
					break;
				case 7:
					objMap[i][j] = new dragon(i, j, team);
					break;
				case 8:
					objMap[i][j] = new gryphon(i, j, team);
					break;
				case 9:
					objMap[i][j] = new flymachine(i, j, team);
					break;
				}
				break;
			case 1:
				switch (addoper % 10) {
				case 0:
					objMap[i][j] = new base(i, j, team);
					break;
				case 1:
					objMap[i][j] = new barracks(i, j, team);
					break;
				case 2:
					objMap[i][j] = new farm(i, j, team);
					break;
				case 3:
					objMap[i][j] = new factory(i, j, team);
					break;
				}
				break;
			}
			break;
		case MS_GAM_HIT: // addoper = dmg

			objMap[i][j]->gotdmg(addoper);

			break;
		break;
		case MS_GAM_MOVE: { // addoper = i * 1000 + j
			int nexti = addoper / 1000;
			int nextj = addoper % 1000;

			unit* un = (unit*)objMap[i][j];
			un->waitServer = 0;

			if (un->anim > 0) {
				un->anim = 0;
				if (un->animstatys == 1) {
					objMap[un->i][un->j] = NULL;
					un->i = un->nexti;
					un->j = un->nextj;
					un->animstatys = 0;
				}
			}

			if (objMap[nexti][nextj] != NULL && objMap[nexti][nextj] != un) {
				if (team != myteam) {
					un->statys = 0;
					break;
				}
				un->wait = 64;
				break;
			}

			if (team != myteam) {
				un->statys = 1;
				un->wayI = nexti;
				un->wayJ = nextj;
			}
			un->nexti = nexti;
			un->nextj = nextj;
			un->direction = un->getdir(un->nexti, un->nextj);
			if (objMap[un->nexti][un->nextj] == NULL || objMap[un->nexti][un->nextj] == un) {
				un->anim++;
				objMap[un->nexti][un->nextj] = un;
			}
			else {
				un->wait = 64;
			}
			un->animstatys = 1;
			break;
		}
		case MS_GAM_ATTACK: { // addoper = nexti * 1000 + nextj

			unit* un = (unit*)objMap[i][j];
			un->waitServer = 0;

			if (un->anim > 0) {
				un->anim = 0;
				if (un->animstatys == 1) {
					objMap[un->i][un->j] = NULL;
					un->i = un->nexti;
					un->j = un->nextj;
					un->animstatys = 0;
				}
			}

			if (team != myteam) {
				un->statys = 9;
			}
			un->attTarget = objMap[addoper / 1000][addoper % 1000];
			un->animstatys = 5;
			un->nexti = un->i;
			un->nextj = un->j;

		}
			break;
		case MS_SYS_MYTEAM:
			myteam = team;
			break;
		case MS_SYS_GENERKEY:
			myteam = team;
			createMap(i, j);
			winstatys = 10;
			startthrs();
			break;
		case MS_GAM_COLLECT: { // int tip = addoper / 100; int count = addoper % 100;

			civil* un = (civil*)objMap[i][j];
			un->waitServer = 0;

			if (addoper / 100 != un->typestorage) un->storage = 0;
			un->storage += addoper % 100;
			un->typestorage = addoper / 100;
		}
						   break;
		case MS_GAM_PUT: {// civil* un = (civil*)objMap[i][j];
			civil* un = (civil*)objMap[i][j];
			un->waitServer = 0;

			teams[un->getTeam()]->sendMessage(5 - un->typestorage + 1, &un->storage);
			un->storage = 0;
			un->typestorage = 0;
		}
					   break;
		case MS_SYS_DISCONNECT:
			if (team == 0) {
				winstatys = 0;

			}
			break;
		case MS_GAM_NEXTTICK:
			analcount++;
			break;
		}
	}

	~Client() override {
		sf::Packet pack;
		pack << 0 << 0 << myteam << MS_SYS_DISCONNECT << 0;
		server.send(pack);
		mtx.lock();
		thr->terminate();
		mtx.unlock();

		winstatys = 0;
	}

};

void lis() {

	Client* cl = (Client*)output;
	sf::Packet pack;
	while (1) {
		cl->server.receive(pack);
		if (pack.getDataSize() > 0) {
			mtx.lock();
			output->getActivity(pack);
			mtx.unlock();
		}
	}
	winstatys = 0;
}