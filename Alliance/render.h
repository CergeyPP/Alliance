#include <SFML\Graphics.hpp>
#include "server.h"
#include <math.h>

sf::Texture landspace[3];

volatile bool isAnalizyng = 0;

void drawMap() {

	sf::Image img;

	while (win->isOpen()) 
	{
		//sf::Texture txt;
		//txt.loadFromImage(minimap);
		//sf::Sprite spr;
		//spr.setTexture(txt);
		//spr.setPosition(WIN_WIDTH + (200 - 128) / 2, WIN_HEIGHT - 128);
		//win->draw(spr);

		img = minimap;
		if (!isAnalizyng) {

			mtx.lock();
			for (int i = 0; i < 129; i++) {
				for (int j = 0; j < 129; j++) {
					if (objMap[i][j] != NULL) {
						if (objMap[i][j]->isTree()) {
							img.setPixel(j, i, sf::Color::Green);
						}
						else if (objMap[i][j]->getTeam() < 0) {
							img.setPixel(j, i, sf::Color::Yellow);
						}
						else {
							img.setPixel(j, i, teamcolors[objMap[i][j]->getTeam()]);
						}
					}
					else {
						if (heightMap[i][j] <= sand)
							img.setPixel(j, i, sf::Color(0, 0, 55 + heightMap[i][j]));
						else
							if (heightMap[i][j] <= forest) { img.setPixel(j, i, sf::Color(127 + heightMap[i][j] / 4, 127 + heightMap[i][j] / 4, 0)); }
							else
								if (heightMap[i][j] <= mount) { img.setPixel(j, i, sf::Color(0, 255 - heightMap[i][j] / 4, 0)); }
								else
									img.setPixel(j, i, sf::Color(heightMap[i][j] / 4, heightMap[i][j] / 4, heightMap[i][j] / 4));


					}
				}
			}
			mtx.unlock();
		}

		minimap = img;

	}

}