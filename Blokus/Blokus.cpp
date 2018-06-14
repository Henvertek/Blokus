// Blokus.cpp: define el punto de entrada de la aplicación de consola.
//

#include "stdafx.h"
#include <iostream>
#include <Windows.h>

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
#define FILL_4 (char) 254

#define BLANK_1 (char) 176
#define BLANK_2 (char) 177
#define BLANK_3 (char) 178

#define RE_PAG 33
#define AV_PAG 34
#define INICIO 36
#define FIN 35
#define LEFT_ARR 37
#define UP_ARR 38
#define RIGHT_ARR 39
#define DOWN_ARR 40
#define FLIP_KEY 70
#define ROTATE_KEY 82
#define ESCAPE 27
#define ENTER 13

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

void goTo(int x, int y) {
	COORD pos = { x, y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

struct player {
	int color;
	bool pieces[22][5][5];
	bool first = true;
	bool surrender = false;
	bool lastPlayed1x1 = false;
	int score = 0;
	int taken[22];
	int piecesPlaced = 0;
};

int playerScore(struct player &guy) {
	int total = 0;
	for (int i = 0; i < 21; i++) {
		for (int j = 0; j < 5; j++) {
			for (int k = 0; k < 5; k++) {
				total += guy.pieces[i][j][k];
			}
		}
	}
	total = 89 - total - total;
	if (total == 89) {
		total += 15;
		if (guy.lastPlayed1x1) total += 5;
	}
	return total;
}

void rotate(bool shape[5][5]) {
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

void flip(bool shape[5][5]) {
	int aux;
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 2; j++) {
			aux = shape[i][j];
			shape[i][j] = shape[i][4 - j];
			shape[i][4 - j] = aux;
		}
	}
}

bool valid(int b[20][20], int x, int y, struct player &subj, int index, bool first) {
	bool corner = false;
	bool cornerpiece = false;
	int color = subj.color;
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			if (subj.pieces[index][i][j]) {
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
				if (first) {
					if ((i + y == 0 || i + y == 19) && (j + x == 0 || j + x == 19)) cornerpiece = true;
				}
				else {
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
	}
	return corner || (cornerpiece && first);
}

void assignShape(bool dest[5][5], bool templ[5][5]) {
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			dest[i][j] = templ[i][j];
		}
	}
}

void setPiece(int b[20][20], int ind, int x, int y, struct player &subject) {
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			if (subject.pieces[ind][i][j])b[i + y][j + x] = subject.color;
		}
	}
	subject.taken[subject.piecesPlaced] = ind;
	subject.piecesPlaced++;
	assignShape(subject.pieces[ind], subject.pieces[21]);
}

int getNum(char text[10]) {
	int output = 0;
	for (int i = 0; text[i] != '\0'; i++) {
		output *= 10;
		output += text[i] - 48;
	}
	return output;
}

void printSquare(int x, int y) {
	goTo(x, y);
	cout << "  " << FILL_1 << " ";
	goTo(x, y + 1);
	cout << " " << FILL_3 << FILL_3 << " ";
	goTo(x, y);
}

void refreshPiece(int pieza[5][5], int x, int y, int newX, int newY, int board[20][20], int color) {
	goTo(x, y);
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), board[i + y][j + x]);
			if (i + y <= 19 && i + y >= 0 && j + x <= 19 && j + x >= 0)printSquare((x + j) * 4 + 7, (y + i) * 2 + 2);
		}
	}
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
			if (pieza[i][j] && i + newY <= 19 && i + newY >= 0 && j +newX <= 19 && j + newX >= 0)printSquare((newX + j) * 4 + 7, (i + newY) * 2 + 2);
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BLANCO);
		}
	}
	goTo(100, 44);
}

void previewBoard(int b[20][20], struct player &subject) {
	int x = 8, y = 8, previewColor, invalidColor;
	switch (subject.color) {
	case AZUL:
		previewColor = AZUL_2;
		invalidColor = AZUL_3;
		break;
	case VERDE:
		previewColor = VERDE_2;
		invalidColor = VERDE_3;
		break;
	case AMARILLO:
		previewColor = AMARILLO_2;
		invalidColor = AMARILLO_3;
		break;
	default:
		previewColor = ROJO_2;
		invalidColor = ROJO_3;
	}

	int key;
	char input[10] = "21";
	int selected = 21;
	bool chosen = false;
	bool cool = valid(b, x, y, subject, selected, subject.first);
	HANDLE hConsole;
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	cout << "   " << ESQ_SUP_IZQ;
	for (int i = 0; i < 21; i++) {
		cout << BOR_HOR << BOR_HOR << BOR_HOR << BOR_HOR;
	}
	cout << BOR_HOR << BOR_HOR << ESQ_SUP_DER << endl << "   " << BOR_VER;
	for (int i = 0; i < 21; i++) {
		cout << "    ";
	}
	cout << "  " << BOR_VER << endl;
	for (float i = 0; i < 20; i += 0.5) {
		cout << "   " << BOR_VER << "   ";
		for (int j = 0; j < 20; j++) {
			if (i >= y && j >= x && i < y + 5 && j < x + 5 && subject.pieces[selected][(int)i - y][j - x]) {//hay que imprimir la pieza
				if (cool)SetConsoleTextAttribute(hConsole, previewColor);
				else SetConsoleTextAttribute(hConsole, invalidColor);
				if (i == (int)i)cout << "  " << FILL_1 << " ";
				else cout << " " << FILL_3 << FILL_3 << " ";
				SetConsoleTextAttribute(hConsole, 7);
			}
			else {//hay que imprimir el tablero
				SetConsoleTextAttribute(hConsole, b[(int)i][j]);
				if (i == (int)i)cout << "  " << FILL_1 << " ";
				else cout << " " << FILL_3 << FILL_3 << " ";
			}
			SetConsoleTextAttribute(hConsole, 7);
		}
		cout << "   " << BOR_VER << "   ";//BORDE DERECHO DEL TABLERO
		int newI = i * 2 / 5;
		int r = int(i * 2) % 5;
		for (int j = 0; (j < 4 - (newI == 5) * 3) && (newI < 6); j++) {
			int indice = (newI * 4) + j;//shit
			for (int c = 0; c < 5; c++) {
				if (subject.pieces[indice][r][c]) {
					if (indice != selected)SetConsoleTextAttribute(hConsole, subject.color);
					cout << FILL_2 << FILL_2;
					SetConsoleTextAttribute(hConsole, 7);
				}
				else {
					if (c == 0 && r == 0) {
						int num = (newI * 4) + j + 1;
						if (num < 10) cout << 0;
						cout << num;
					}
					else cout << "  ";
				}
			}
		}
		cout << endl;
	}
	cout << "   " << BOR_VER;
	for (int i = 0; i < 21; i++) {
		cout << "    ";
	}
	cout << "  " << BOR_VER << endl;
	cout << "   " << ESQ_INF_IZQ;
	for (int i = 0; i < 21; i++) {
		cout << BOR_HOR << BOR_HOR << BOR_HOR << BOR_HOR;
	}
	cout << BOR_HOR << BOR_HOR << ESQ_INF_DER << "    >>> ";
	do {
		while (selected < -1 || selected > 20 || strlen(input) == 0) {
			cin.getline(input, 10);
			if (strcmp(input, "pass") == 0) {
				selected = -1;
			}
			else if (strcmp(input, "surrender") == 0) {
				subject.surrender = true;
				selected = -1;
			}
			else {
				selected = getNum(input) - 1;
			}
			key = 0;
		}
		if (chosen) {
			key = PressAnyKey("");
		}
		chosen = true;
		int prevX = x, prevY = y;
		switch (key) {
		case RE_PAG:
			y -= 5;
			break;
		case UP_ARR:
			y--;
			break;
		case AV_PAG:
			y += 5;
			break;
		case DOWN_ARR:
			y++;
			break;
		case INICIO:
			x -= 5;
			break;
		case LEFT_ARR:
			x--;
			break;
		case FIN:
			x += 5;
			break;
		case RIGHT_ARR:
			x++;
			break;
		case ROTATE_KEY:
			rotate(subject.pieces[selected]);
			break;
		case FLIP_KEY:
			flip(subject.pieces[selected]);
			break;
		case ENTER:
			if (valid(b, x, y, subject, selected, subject.first)) {
				setPiece(b, selected, x, y, subject);
				subject.first = false;
				if (selected == 1) subject.lastPlayed1x1 == true;
				return;
			}
			break;
		case ESCAPE:
			selected = 21;
			chosen = false;
			break;
		}
		if (x < -2) x = -2;
		if (x > 17) x = 17;
		if (y < -2) y = -2;
		if (y > 17) y = 17;
		refreshPiece(subject, selected, prevX, prevY, x, y, b);
	} while (selected != -1);
}

int main() {
	int tablero[20][20];
	for (int i = 0; i < 20; i++) {
		for (int j = 0; j < 20; j++) {
			tablero[i][j] = BLANCO;
		}
	}
	struct player jugadores[4];

	jugadores[0].color = AZUL;
	jugadores[1].color = AMARILLO;
	jugadores[2].color = ROJO;
	jugadores[3].color = VERDE;

	bool models[22][5][5] = { { { false, false, false, false, false },
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
															{ false, false, false, true, false },
															{ false, true, true, true, false },
															{ false, false, false, false, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, true, true, false },
															{ false, true, true, false, false },
															{ false, false, false, false, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, true, false, false, false },
															{ false, true, true, true, true },
															{ false, false, false, false, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, true, false, false },
															{ false, false, true, false, false },
															{ false, true, true, true, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, false, false, false },
															{ false, false, true, true, true },
															{ false, false, true, false, false },
															{ false, false, true, false, false } },

														{ { false, false, false, false, false },
															{ false, false, false, false, false },
															{ false, false, true, true, false },
															{ true, true, true, false, false },
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
															{ false, false, true, false, false },
															{ true, true, true, true, false },
															{ false, false, false, false, false },
															{ false, false, false, false, false } },

														{ { false, false, false, false, false },
															{ false, false, false, false, false },
															{ false, false, false, false, false },
															{ false, false, false, false, false },
															{ false, false, false, false, false } } };
	for (int h = 0; h < 4; h++) {
		for (int i = 0; i < 22; i++) {
			assignShape(jugadores[h].pieces[i], models[i]);
		}
	}
	int maxPlayers, maxColors;
	cin.ignore(1);
	do {
		cout << "Elija la cantidad de jugadores(2 / 4):\n>>> ";
		cin >> maxPlayers;
	} while (maxPlayers != 2 && maxPlayers != 4);
	do {
		cout << "Elija la cantidad de colores(2 / 4):\n>>> ";
		cin >> maxColors;
	} while (maxColors != 2 && maxColors != 4);
	system("cls");
	do {
		int rendidos = 0;
		for (int i = 0; i < maxColors; i++) {
			if (!jugadores[i].surrender) {
				previewBoard(tablero, jugadores[i]);
				system("cls");
			}
			else rendidos++;
		}
	} while (1);

	return 0;
}