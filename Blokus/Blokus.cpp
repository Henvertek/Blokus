// Blokus.cpp: define el punto de entrada de la aplicación de consola.
//

#include "stdafx.h"
#include <iostream>
#include <windows.h>

using namespace std;

#define ESQ_SUP_IZQ (char) 201
#define ESQ_SUP_DER (char) 187

#define ESQ_INF_IZQ (char) 200
#define ESQ_INF_DER (char) 188

#define BOR_VER (char) 186
#define BOR_HOR (char) 205

#define ESQ_SUP_IZQ_F (char) 218
#define ESQ_SUP_DER_F (char) 191

#define ESQ_INF_IZQ_F (char) 192
#define ESQ_INF_DER_F (char) 217

#define BOR_VER_F (char) 179
#define BOR_HOR_F (char) 196

#define CON_CEN (char) 206
#define CON_IZQ (char) 204
#define CON_DER (char) 185

#define CON_SUP (char) 203
#define CON_INF (char) 202

#define AZUL 25
#define AZUL_2 144
#define AZUL_3 1
#define AMARILLO 110
#define AMARILLO_2 224
#define AMARILLO_3 6
#define ROJO 76
#define ROJO_2 192
#define ROJO_3 4
#define VERDE 42
#define VERDE_2 160
#define VERDE_3 2

#define BLANCO 8

#define FILL_1 (char) 220
#define FILL_2 (char) 219
#define FILL_3 (char) 223

#define BLANK_1 (char) 176
#define BLANK_2 (char) 177
#define BLANK_3 (char) 178


int PressAnyKey(const char *prompt)
{
	DWORD        mode;
	HANDLE       hstdin;
	INPUT_RECORD inrec;
	DWORD        count;

	/* Set the console mode to no-echo, raw input, */
	/* and no window or mouse events.              */
	hstdin = GetStdHandle(STD_INPUT_HANDLE);
	if (hstdin == INVALID_HANDLE_VALUE
		|| !GetConsoleMode(hstdin, &mode)
		|| !SetConsoleMode(hstdin, 0))
		return 0;
	FlushConsoleInputBuffer(hstdin);

	/* Get a single key RELEASE */
	do ReadConsoleInput(hstdin, &inrec, 1, &count);
	while ((inrec.EventType != KEY_EVENT) || inrec.Event.KeyEvent.bKeyDown);

	/* Restore the original console mode */
	SetConsoleMode(hstdin, mode);

	return inrec.Event.KeyEvent.wVirtualKeyCode;
}

struct player {
	int color;
	int pieces[21][5][5];
};

void rotate(int shape[5][5]) {
	int copy[5][5];
	for (int h = 0; h < 2; h++) {
		for (int i = 0; i < 5; i++) {
			for (int j = 0; j < 5; j++) {
				if (h == 0)copy[i][j] = shape[i][j];
				else shape[j][4 - i] = copy[i][j];
			}
		}
	}
}

void flip(int shape[5][5]) {
	int aux;
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 2; j++) {
			aux = shape[i][j];
			shape[i][j] = shape[i][4 - j];
			shape[i][4 - j] = aux;
		}
	}
}

void printBoard(int b[20][20]) {
	HANDLE hConsole;
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, 7);
	cout << "   " << ESQ_SUP_IZQ;
	for (int i = 0; i < 21; i++) {
		cout << BOR_HOR << BOR_HOR << BOR_HOR << BOR_HOR;
	}
	cout << BOR_HOR << ESQ_SUP_DER << endl << "   " << BOR_VER;
	for (int i = 0; i < 21; i++) {
		cout << "    ";
	}
	cout << " " << BOR_VER << endl;
	for (float i = 0; i < 20; i += 0.5) {
		cout << "   " << BOR_VER << "   ";
		for (int j = 0; j < 20; j++) {
			SetConsoleTextAttribute(hConsole, b[(int)i][j]);
			if (b[(int)i][j] != BLANCO) {
				if (i == (int)i)cout << "  " << FILL_1 << " ";
				else cout << " " << FILL_3 << FILL_3 << " ";
			}
			else {
				if (i == (int)i)cout << " " << FILL_1 << FILL_1 << " ";
				else cout << " " << FILL_3 << FILL_3 << " ";
			}
			SetConsoleTextAttribute(hConsole, 7);
		}
		cout << "  " << BOR_VER << endl;
	}
	cout << "   " << BOR_VER;
	for (int i = 0; i < 21; i++) {
		cout << "    ";
	}
	cout << " " << BOR_VER << endl;
	cout << "   " << ESQ_INF_IZQ;
	for (int i = 0; i < 21; i++) {
		cout << BOR_HOR << BOR_HOR << BOR_HOR << BOR_HOR;
	}
	cout << BOR_HOR << ESQ_INF_DER;
}

#define LEFT_ARR 37
#define UP_ARR 38
#define RIGHT_ARR 39
#define DOWN_ARR 40
#define FLIP_KEY 70
#define ROTATE_KEY 82
#define ESCAPE 27
#define ENTER 13

bool valid(int b[20][20], int x, int y, int piece[5][5], int color) {
	bool corner = false;
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			if (piece[i][j] != BLANCO) {
				if ((i + y >= 20) || (i + y < 0) || (j + x >= 20) || (j + x < 0)) return false;
				else if (b[i + y][j + x] != BLANCO) return false;
				else if (i + y > 0) {
					if (b[i + y - 1][j + x] == color) {
						return false;
					}
				}
				if (i + y < 19) {
					if (b[i + y + 1][j + x] == color) {
						return false;
					}
				}
				if (j + x > 0) {
					if (b[i + y][j + x - 1] == color) {
						return false;
					}
				}
				if (j + x < 19) {
					if (b[i + y][j + x + 1] == color) {
						return false;
					}
				}
				//vamos bien, falta chequear esquina con esquina
				if (j + x > 0) {
					if (i + y > 0) {
						if (b[i + y - 1][j + x - 1] == color) corner = true;
					}
					if (i + y < 19) {
						if (b[i + y + 1][j + x - 1] == color) corner = true;
					}
				}
				if (j + x < 19) {
					if (i + y > 0) {
						if (b[i + y - 1][j + x + 1] == color) corner = true;
					}
					if (i + y < 19) {
						if (b[i + y + 1][j + x + 1] == color) corner = true;
					}
				}
			}
		}
	}
	return corner;
}

void setPiece(int b[20][20], int piece[5][5], int x, int y) {
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			if(piece[i][j] != BLANCO)b[i + y][j + x] = piece[i][j];
		}
	}
}

void previewBoard(int b[20][20], int piece[5][5], int color) {
	int x, y, previewColor, invalidColor;
	switch (color) {
	case AZUL:
		x = 0;
		y = 0;
		previewColor = AZUL_2;
		invalidColor = AZUL_3;
		break;
	case VERDE:
		x = 15;
		y = 0;
		previewColor = VERDE_2;
		invalidColor = VERDE_3;
		break;
	case AMARILLO:
		x = 0;
		y = 15;
		previewColor = AMARILLO_2;
		invalidColor = AMARILLO_3;
		break;
	default:
		x = 15;
		y = 15;
		previewColor = ROJO_2;
		invalidColor = ROJO_3;
	}
	int key;
	do {
		key = PressAnyKey("");
		system("cls");
		switch (key) {
		case UP_ARR:
			y--;
			break;
		case DOWN_ARR:
			y++;
			break;
		case LEFT_ARR:
			x--;
			break;
		case RIGHT_ARR:
			x++;
			break;
		case ROTATE_KEY:
			rotate(piece);
			break;
		case FLIP_KEY:
			flip(piece);
			break;
		case ENTER:
			if (valid(b, x, y, piece, color)) {
				setPiece(b, piece, x, y);
				return;
			}
			break;
		case ESCAPE:
			break;
		}
		if (x < -4) x = -4;
		if (x > 19) x = 19;
		if (y < -4) y = -4;
		if (y > 19) y = 19;
		bool cool = valid(b, x, y, piece, color);
		HANDLE hConsole;
		hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, 7);
		cout << "   " << ESQ_SUP_IZQ;
		for (int i = 0; i < 21; i++) {
			cout << BOR_HOR << BOR_HOR << BOR_HOR << BOR_HOR;
		}
		cout << BOR_HOR << ESQ_SUP_DER << endl << "   " << BOR_VER;
		for (int i = 0; i < 21; i++) {
			cout << "    ";
		}
		cout << " " << BOR_VER << endl;
		for (float i = 0; i < 20; i += 0.5) {
			cout << "   " << BOR_VER << "   ";
			for (int j = 0; j < 20; j++) {
				if (i >= y && j >= x && i < y + 5 && j < x + 5 && piece[(int)i - y][j - x] != BLANCO) {//hay que imprimir la pieza
					if (cool)SetConsoleTextAttribute(hConsole, previewColor);
					else SetConsoleTextAttribute(hConsole, invalidColor);
					if (i == (int)i)cout << "  " << FILL_1 << " ";
					else cout << " " << FILL_3 << FILL_3 << " ";

					SetConsoleTextAttribute(hConsole, 7);
				}
				else {//hay que imprimir el tablero
					SetConsoleTextAttribute(hConsole, b[(int)i][j]);
					if (b[(int)i][j] != BLANCO) {
						if (i == (int)i)cout << "  " << FILL_1 << " ";
						else cout << " " << FILL_3 << FILL_3 << " ";
					}
					else {
						if (i == (int)i)cout << " " << FILL_1 << FILL_1 << " ";
						else cout << " " << FILL_3 << FILL_3 << " ";
					}
				}
				SetConsoleTextAttribute(hConsole, 7);
			}
			cout << "  " << BOR_VER << endl;
		}
		cout << "   " << BOR_VER;
		for (int i = 0; i < 21; i++) {
			cout << "    ";
		}
		cout << " " << BOR_VER << endl;
		cout << "   " << ESQ_INF_IZQ;
		for (int i = 0; i < 21; i++) {
			cout << BOR_HOR << BOR_HOR << BOR_HOR << BOR_HOR;
		}
		cout << BOR_HOR << ESQ_INF_DER;
	} while (key != ESCAPE);
}

void printPlayer(struct player subject) {
	HANDLE hConsole;
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	for (int i = 0; i < 5; i++) {
		for (float r = 0; r < 5; r += 0.5) {
			for (int j = 0; j < 4 + (i == 2); j++) {
				for (int c = 0; c < 5; c++) {
					SetConsoleTextAttribute(hConsole, subject.pieces[i * 4 + j][(int)r][c]);
					if (subject.pieces[i * 4 + j][(int)r][c] != BLANCO) {
						if (r == (int)r)cout << "  " << FILL_1 << " ";
						else cout << " " << FILL_3 << FILL_3 << " ";
					}
					else {
						if (r == (int)r)cout << " " << FILL_1 << FILL_1 << " ";
						else cout << " " << FILL_3 << FILL_3 << " ";
					}
				}
			}
			cout << endl;
		}
	}
}

void assignShape(int dest[5][5], bool templ[5][5], int color) {
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			if (templ[i][j])dest[i][j] = color;
			else dest[i][j] = BLANCO;
		}
	}
}

int main() {
	int tablero[20][20];
	HANDLE hConsole;
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	for (int i = 0; i < 20; i++) {
		for (int j = 0; j < 20; j++) {
			tablero[i][j] = BLANCO;
			if (rand() % 100 < 2) tablero[i][j] = ROJO;
			if (rand() % 100 < 2) tablero[i][j] = AZUL;
			if (rand() % 100 < 2) tablero[i][j] = AMARILLO;
			if (rand() % 100 < 2) tablero[i][j] = VERDE;
		}
	}
	struct player jugadores[4];
	jugadores[0].color = AZUL;
	jugadores[1].color = AMARILLO;
	jugadores[2].color = ROJO;
	jugadores[3].color = VERDE;
	bool models[21][5][5] = { { { false, false, false, false, false },
															{ false, false, false, false, false },
															{ false, false, true, false, false },
															{ false, false, false, false, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, false, false, false },
															{ false, false, true, true, false },
															{ false, false, false, false, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, true, false, false },
															{ false, true, true, false, false },
															{ false, false, false, false, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, false, false, false },
															{ false, true, true, true, false },
															{ false, false, false, false, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, false, false, false },
															{ false, false, true, true, false },
															{ false, false, true, true, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, true, false, false },
															{ false, true, true, true, false },
															{ false, false, false, false, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, false, false, false },
															{ false, true, true, true, true },
															{ false, false, false, false, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, false, false, false },
															{ false, false, false, true, false },
															{ false, true, true, true, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, false, true, true },
															{ false, false, true, true, false },
															{ false, false, false, false, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, false, false, false },
															{ false, true, false, false, false },
															{ false, true, true, true, true },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, true, false, false },
															{ false, false, true, false, false },
															{ false, true, true, true, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, false, true, false },
															{ false, false, false, true, false },
															{ false, true, true, true, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, false, false, false },
															{ false, false, false, true, true },
															{ false, true, true, true, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, false, true, false },
															{ false, true, true, true, false },
															{ false, true, false, false, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, false, false, false },
															{ true, true, true, true, true },
															{ false, false, false, false, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, false, false, false },
															{ false, false, true, true, false },
															{ false, true, true, true, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, false, true, false },
															{ false, false, true, true, false },
															{ false, true, true, false, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, false, false, false },
															{ false, true, false, true, false },
															{ false, true, true, true, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, true, false, false },
															{ false, true, true, true, false },
															{ false, false, false, true, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, true, false, false },
															{ false, true, true, true, false },
															{ false, false, true, false, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, false, false, false },
															{ false, false, false, true, false },
															{ false, true, true, true, true },
															{ false, false, false, false, false } } };
	for (int h = 0; h < 4; h++) {
		for (int i = 0; i < 21; i++) {
			assignShape(jugadores[h].pieces[i], models[i], jugadores[h].color);
		}
	}
	int k;
	int piece[5][5];
	while (1) {
		for (int i = 0; i < 4; i++) {
			printPlayer(jugadores[i]);
			cin >> k;
			assignShape(piece, models[k - 1], jugadores[i].color);
			previewBoard(tablero, piece, jugadores[i].color);
		}
	}
	return 0;
}