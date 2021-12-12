#pragma once
float pointx = 64 * 64;
float pointy = 64 * 64;

sf::RenderWindow* win;

float heightMap[129][129];

class object;

object* objMap[129][129];

int sand = 200;
int forest = 400;
int mount = 800;

int step;

float r = 4;

void square(int i, int j) {
	int a, b, c, d;
	int count = 4;
	if (i == 0) { a = 0; count--; }
	else a = heightMap[i - (step / 2)][j];
	if (i == 128) { b = 0; count--; }
	else b = heightMap[i + (step / 2)][j];
	if (j == 0) { c = 0; count--; }
	else c = heightMap[i][j - (step / 2)];
	if (j == 128) { d = 0; count--; }
	else d = heightMap[i][j + (step / 2)];

	int random = 2.f * (float)step * r;

	heightMap[i][j] = (a + b + c + d) / count;
	heightMap[i][j] += (-random / 2 + (rand() % random));
}

void diamondsquare() {

	step = 128;

	while (step != 1) {
		for (int i = 0; i + step <= 128; i += step) {
			for (int j = 0; j + step <= 128; j += step) {
				int random = 2 * step * r;
				heightMap[i + (step / 2)][j + (step / 2)] = (heightMap[i][j] + heightMap[i + step][j] + heightMap[i][j + step] + heightMap[i + step][j + step]) / 4;
				heightMap[i + (step / 2)][j + (step / 2)] += (-random / 2 + (rand() % random));
			}
		}
		for (int i = 0; i + step <= 128; i += step) {
			for (int j = 0; j + step <= 128; j += step) {
				square(i, j + (step / 2));
				square(i + step, j + (step / 2));
				square(i + (step / 2), j);
				square(i + (step / 2), j + step);
			}
		}
		step = step / 2;
	}

	for (int i = 0; i <= 128; i++) {
		for (int j = 0; j <= 128; j++) {
			objMap[i][j] = NULL;
		}
	}

	for (int i = 0; i < 129; i++) {
		for (int j = 0; j < 129; j++) {
			if (heightMap[i][j] <= 0) heightMap[i][j] = 0;
			if (heightMap[i][j] >= 1023) heightMap[i][j] = 1023;
		}
	}
	for (int i = 0; i < 129; i++) {
		heightMap[i][0] = forest + 101;
		heightMap[0][i] = forest + 101;
		heightMap[i][128] = forest + 101;
		heightMap[128][i] = forest + 101;
	}
}