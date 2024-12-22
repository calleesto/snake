#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>
#include <cstdlib> 
#include <time.h>

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

//screen resolution
#define SCREEN_WIDTH			640
#define SCREEN_HEIGHT			480
#define SNAKE_SPAWN_X			300
#define SNAKE_SPAWN_Y			400

#define INITIAL_SNAKE_LENGTH 	5 // default: 5	//so 1 head image and n-1 body images
#define MAX_SNAKE_LENGTH		1000
#define SNAKE_BODY_GAP			4
#define SELF_HIT_SENSITIVITY	10

#define SPEED_CONST				150 //default: 150
#define SPEED_UP_MULTIPLIER		1.1
#define SLOW_DOWN_MULTIPLIER	0.8
#define SPEED_UP_INTERVAL		5

#define SENSITIVITY				20
#define UNIT					5

#define RED_UPPER_LIMIT			8
#define RED_BOTTOM_LIMIT		5

#define RED_POINTS				10
#define BLUE_POINTS				10

#define BOARD_COLOR				grey
#define BORDER_COLOR			grey


//TODO
//CHANGE ARRAY TO DYNAMIC

typedef struct GameState {
	SDL_Event event;
	SDL_Surface* charset;
	SDL_Surface* eti;
	SDL_Surface* blue_dot;
	SDL_Surface* red_dot;
	int quit;
	double worldTime;
	bool keys[SDL_NUM_SCANCODES] = { false };
	double delta;
	double timeTracker;
	double speedUpTimer;
	double redBonusTimer;
	double progressTimer;
	double fpsTimer;
	double fps;
	double movement_speed;
	double chase_delay;
	int frames;
	bool collision;
	bool blueDotSpawned; 
	bool redDotSpawned;
	double blue_dot_x;
	double blue_dot_y;
	double red_dot_x;
	double red_dot_y;
	bool blueDotReached;
	bool redDotReached;
	bool showProgressBar;
	double speed_const;
	bool no_red_cords;
	int red_interval;
	int points = 0;
	int fiftyfifty;
}state;

typedef struct Snake {
	double snake_x[MAX_SNAKE_LENGTH];
	double snake_y[MAX_SNAKE_LENGTH];
	bool isAlive;
	bool movingUp;
	bool movingLeft;
	bool movingDown;
	bool movingRight;
	int currentSnakeLength;
	SDL_Surface* headUp;
	SDL_Surface* headLeft;
	SDL_Surface* headDown;
	SDL_Surface* headRight;
	SDL_Surface* body;
	SDL_Surface* activeImage;
}snake;

typedef struct Colors {
	int black;
	int red;
	int green;
	int blue;
	int white;
	int purple;
	int yellow;
	int cyan;
	int morro;
	int light_purple;
	int grey;
	char text[128];
}colors;

//default sdl functions
void DrawString(SDL_Surface* screen, int x, int y, const char* text, SDL_Surface* charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while (*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
	};
};

void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
};

void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32*)p = color;
};

void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for (int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
	};
};

void DrawRectangle(SDL_Surface* screen, int x, int y, int l, int k, Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for (i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
};

int loadAllImages(SDL_Window* window, SDL_Renderer* renderer, SDL_Surface* screen, SDL_Texture* scrtex, GameState* state, Snake* snake) {
	const char* filename = "->/gojo.bmp";
	state->charset = SDL_LoadBMP("./cs8x8.bmp");
	if (state->charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};
	SDL_SetColorKey(state->charset, true, 0x000000);

	state->eti = SDL_LoadBMP("./eti.bmp");
	if (state->eti == NULL) {
		printf("SDL_LoadBMP(eti.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(state->charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	snake->headUp = SDL_LoadBMP("./headUp.bmp");
	if (snake->headUp == NULL) {
		printf("SDL_LoadBMP(headUp.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(state->charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	snake->headLeft = SDL_LoadBMP("./headLeft.bmp");
	if (snake->headLeft == NULL) {
		printf("SDL_LoadBMP(headLeft.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(state->charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	snake->headDown = SDL_LoadBMP("./headDown.bmp");
	if (snake->headDown == NULL) {
		printf("SDL_LoadBMP(headDown.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(state->charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	snake->headRight = SDL_LoadBMP("./headRight.bmp");
	if (snake->headRight == NULL) {
		printf("SDL_LoadBMP(headRight.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(state->charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	snake->body = SDL_LoadBMP("./body.bmp");
	if (snake->body == NULL) {
		printf("SDL_LoadBMP(body.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(state->charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	state->blue_dot = SDL_LoadBMP("./blue_dot.bmp");
	if (state->blue_dot == NULL) {
		printf("SDL_LoadBMP(blue_dot.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(state->charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	state->red_dot = SDL_LoadBMP("./red_dot.bmp");
	if (state->red_dot == NULL) {
		printf("SDL_LoadBMP(red_dot.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(state->charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};
}

int initializeSDL(GameState* state, Snake* snake, Colors* colors, SDL_Event* event, SDL_Surface** screen, SDL_Texture** scrtex, SDL_Window** window, SDL_Renderer** renderer) {
	int rc;

	//initialize sdl
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}

	// tryb pe³noekranowy / fullscreen mode
	//rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, window, renderer);

	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, window, renderer);

	if (rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
	};

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(*renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(*renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(*window, "Szablon do zdania drugiego 2017");

	(*screen) = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
		0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	(*scrtex) = SDL_CreateTexture(*renderer, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		SCREEN_WIDTH, SCREEN_HEIGHT);

	loadAllImages(*window, *renderer, *screen, *scrtex, state, snake);

	//initialize text

	//initialize colors for map
	colors->black = SDL_MapRGB((*screen)->format, 0x00, 0x00, 0x00);
	colors->white = SDL_MapRGB((*screen)->format, 0xFF, 0xFF, 0xFF);
	colors->green = SDL_MapRGB((*screen)->format, 0x00, 0xFF, 0x00);
	colors->red = SDL_MapRGB((*screen)->format, 0xFF, 0x00, 0x00);
	colors->blue = SDL_MapRGB((*screen)->format, 0x11, 0x11, 0xCC);
	colors->purple = SDL_MapRGB((*screen)->format, 0x77, 0x11, 0xCC);
	colors->yellow = SDL_MapRGB((*screen)->format, 0xFF, 0xFF, 0x00);
	colors->cyan = SDL_MapRGB((*screen)->format, 0x00, 0xFF, 0xFF);
	colors->light_purple = SDL_MapRGB((*screen)->format, 0xB4, 0xAC, 0xFF);
	colors->morro = SDL_MapRGB((*screen)->format, 0x33, 0x33, 0x00);
	colors->grey = SDL_MapRGB((*screen)->format, 0x55, 0x55, 0x55);
}

void printProgressBar(SDL_Surface* screen, GameState* state, Colors* colors) {
	if (state->progressTimer > 0 && state->progressTimer < 1) {
		sprintf(colors->text, "[|||            ]");
		DrawString(screen, screen->w / 2 - strlen(colors->text) * 8 / 2, 42, colors->text, state->charset);
	}
	else if (state->progressTimer >= 1 && state->progressTimer < 2) {
		sprintf(colors->text, "[||||||         ]");
		DrawString(screen, screen->w / 2 - strlen(colors->text) * 8 / 2, 42, colors->text, state->charset);
	}
	else if (state->progressTimer >= 2 && state->progressTimer < 3) {
		sprintf(colors->text, "[|||||||||      ]");
		DrawString(screen, screen->w / 2 - strlen(colors->text) * 8 / 2, 42, colors->text, state->charset);
	}
	else if (state->progressTimer >= 3 && state->progressTimer < 4) {
		sprintf(colors->text, "[||||||||||||   ]");
		DrawString(screen, screen->w / 2 - strlen(colors->text) * 8 / 2, 42, colors->text, state->charset);
	}
	else if (state->progressTimer >= 4 && state->progressTimer < 5) {
		sprintf(colors->text, "[|||||||||||||||]");
		DrawString(screen, screen->w / 2 - strlen(colors->text) * 8 / 2, 42, colors->text, state->charset);
	}
	else if (state->progressTimer >= 5) {
		state->progressTimer = 0; 
		state->redBonusTimer = 0;
		state->showProgressBar = false;
	}
	else if (state->progressTimer == 0) {
		sprintf(colors->text, "");
		DrawString(screen, screen->w / 2 - strlen(colors->text) * 8 / 2, 42, colors->text, state->charset);
	}

}

void printAllVisuals(SDL_Surface* screen, GameState* state, Colors* colors, Snake* snake) {
	SDL_FillRect(screen, NULL, colors->black);

	DrawLine(screen, 0, 0, SCREEN_WIDTH, 1, 0, colors->BORDER_COLOR);
	DrawLine(screen, 0, 0, SCREEN_HEIGHT, 0, 1, colors->BORDER_COLOR);
	DrawLine(screen, SCREEN_WIDTH - 1, 0, SCREEN_HEIGHT, 0, 1, colors->BORDER_COLOR);
	DrawLine(screen, 0, SCREEN_HEIGHT - 1, SCREEN_WIDTH, 1, 0, colors->BORDER_COLOR);

	DrawRectangle(screen, 3, 3, SCREEN_WIDTH - 6, 51, colors->black, colors->BOARD_COLOR);

	sprintf(colors->text, "Time: [%.1lf]s, FPS: [%.0lf], IMPLEMENTED ELEMENTS: 1,2,3,4 ", state->worldTime, state->fps);
	DrawString(screen, screen->w / 2 - strlen(colors->text) * 8 / 2, 10, colors->text, state->charset);

	//sprintf(colors->text, "Esc - EXIT PROGRAM, n - NEW GAME");
	//DrawString(screen, screen->w / 2 - strlen(colors->text) * 8 / 2, 26, colors->text, state->charset);

	//sprintf(colors->text, "W - CLIMB LADDER, A - GO LEFT, S - DESCEND LADDER,  D - GO RIGHT");
	//DrawString(screen, screen->w / 2 - strlen(colors->text) * 8 / 2, 42, colors->text, element->charset);

	sprintf(colors->text, "Points: [%d], Speed: [%.0lf], Length: [%d]", state->points, state->speed_const, snake->currentSnakeLength);
	DrawString(screen, screen->w / 2 - strlen(colors->text) * 8 / 2, 26, colors->text, state->charset);

	if (state->showProgressBar == true) {
		printProgressBar(screen, state, colors);
	}

	//sprintf(colors->text, "snakex %f, snakey %f", snake->snake_x[0], snake->snake_y[0]);
	//DrawString(screen, screen->w / 2 - strlen(colors->text) * 8 / 2, 42, colors->text, state->charset);

	//sprintf(colors->text, "showprogressbar: %d", state->showProgressBar);
	//DrawString(screen, screen->w / 2 - strlen(colors->text) * 8 / 2, 58, colors->text, state->charset);

	//sprintf(colors->text, "tracker %f", state->timeTracker);
	//DrawString(screen, screen->w / 2 - strlen(colors->text) * 8 / 2, 74, colors->text, state->charset);
}

void updateRenderer(SDL_Texture* scrtex, SDL_Surface* screen, SDL_Renderer* renderer) {
	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	SDL_RenderCopy(renderer, scrtex, NULL, NULL);
	SDL_RenderPresent(renderer);
}

void closingGame(SDL_Window* window, SDL_Renderer* renderer, SDL_Surface* screen, SDL_Texture* scrtex, GameState* state) {
	SDL_FreeSurface(state->charset);
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void oneDirection(Snake* snake, int w, int a, int s, int d) {
	if (w) {
		snake->movingUp = true;
		snake->movingLeft = false;
		snake->movingDown = false;
		snake->movingRight = false;
	}
	else if (a) {
		snake->movingUp = false;
		snake->movingLeft = true;
		snake->movingDown = false;
		snake->movingRight = false;
	}
	else if (s) {
		snake->movingUp = false;
		snake->movingLeft = false;
		snake->movingDown = true;
		snake->movingRight = false;
	}
	else if (d) {
		snake->movingUp = false;
		snake->movingLeft = false;
		snake->movingDown = false;
		snake->movingRight = true;
	}
}

void keystrokeReact(GameState* state, Snake* snake) {
	while (SDL_PollEvent(&(state->event))) {
		switch (state->event.type) {
		case SDL_QUIT:
			state->quit = 1;
			break;
		case SDL_KEYDOWN:
			state->keys[state->event.key.keysym.scancode] = true;
			switch (state->event.key.keysym.sym) {
			case SDLK_ESCAPE:
				state->quit = 1;
				break;
			case SDLK_q:
				state->quit = 1;
				break;
			case SDLK_n:
				//put all of snake off the map
				for (int i = 1; i < MAX_SNAKE_LENGTH; i++) {
					snake->snake_x[i] = 1000;
					snake->snake_y[i] = 1000;
				}
				state->quit = false;
				state->collision = false;
				state->worldTime = 0;
				snake->snake_x[0] = SNAKE_SPAWN_X;
				snake->snake_y[0] = SNAKE_SPAWN_Y;
				oneDirection(snake, 1, 0, 0, 0);
				snake->isAlive = 1;
				state->blueDotSpawned = false;
				state->redDotSpawned = false;
				state->blueDotReached = true;
				state->redDotReached = true;
				snake->currentSnakeLength = INITIAL_SNAKE_LENGTH;
				state->speed_const = SPEED_CONST;
				state->speedUpTimer = 0;
				state->redBonusTimer = 0;
				state->progressTimer = 0;
				state->no_red_cords = false;
				state->showProgressBar = false;
				state->red_interval = (rand() % RED_UPPER_LIMIT) + RED_BOTTOM_LIMIT;
				state->points = 0;
				break;
			}
			break;
		case SDL_KEYUP:
			state->keys[state->event.key.keysym.scancode] = false;
			break;
		}
	}
}

void wallInteraction(Snake* snake, GameState* state) {
	//upper wall interaction
	if (snake->movingUp && snake->snake_y[0] - state->movement_speed <= 65) {
		if (snake->snake_x[0] + state->movement_speed < SCREEN_WIDTH) {
			oneDirection(snake, 0, 0, 0, 1); // right
		}
		else {
			oneDirection(snake, 0, 1, 0, 0); // left
		}
	}

	//lefthand wall interaction
	if (snake->movingLeft && snake->snake_x[0] - state->movement_speed <= 0) {
		if (snake->snake_y[0] - state->movement_speed > 0) {
			oneDirection(snake, 1, 0, 0, 0); // up
		}
		else {
			oneDirection(snake, 0, 0, 1, 0); // down
		}
	}


	//bottom wall interaction
	if (snake->movingDown && snake->snake_y[0] + state->movement_speed >= SCREEN_HEIGHT) {
		if (snake->snake_x[0] - state->movement_speed > 0) {
			oneDirection(snake, 0, 1, 0, 0); // left
			
		}
		else {
			oneDirection(snake, 0, 0, 0, 1); // right
		}
	}


	//righthand wall interaction 
	if (snake->movingRight && snake->snake_x[0] + state->movement_speed >= SCREEN_WIDTH) {
		if (snake->snake_y[0] + state->movement_speed < SCREEN_HEIGHT) {
			oneDirection(snake, 0, 0, 1, 0); // down
		}
		else {
			oneDirection(snake, 1, 0, 0, 0); // up
		}
	}

}

void snakeMovement(GameState* state, Snake* snake) {
	// W
	if (state->keys[SDL_SCANCODE_W] && !snake->movingDown) {
		if (snake->snake_y[0] - state->movement_speed > 0) {
			oneDirection(snake, 1, 0, 0, 0); // up
		}
	}

	// A
	if (state->keys[SDL_SCANCODE_A] && !snake->movingRight) {
		if (snake->snake_x[0] - state->movement_speed > 0) {
			oneDirection(snake, 0, 1, 0, 0); // left
		}
	}

	// S
	if (state->keys[SDL_SCANCODE_S] && !snake->movingUp) {
		if (snake->snake_y[0] + state->movement_speed < SCREEN_HEIGHT) {
			oneDirection(snake, 0, 0, 1, 0); // down
		}
	}

	// D
	if (state->keys[SDL_SCANCODE_D] && !snake->movingLeft) {
		if (snake->snake_x[0] + state->movement_speed < SCREEN_WIDTH) {
			oneDirection(snake, 0, 0, 0, 1); // right
		}
	}
}

void moveSnake(Snake* snake, GameState* state) {
	if (snake->movingUp) {
		snake->activeImage = snake->headUp;
		snake->snake_y[0] -= state->movement_speed;
	}
	else if (snake->movingLeft) {
		snake->activeImage = snake->headLeft;
		snake->snake_x[0] -= state->movement_speed;
	}
	else if (snake->movingDown) {
		snake->activeImage = snake->headDown;
		snake->snake_y[0] += state->movement_speed;
	}
	else if (snake->movingRight) {
		snake->activeImage = snake->headRight;
		snake->snake_x[0] += state->movement_speed;
	}
}

void drawSnake(Snake* snake, SDL_Surface* screen) {
	for (int i = 1; i < snake->currentSnakeLength; i++) {
		DrawSurface(screen, snake->body, snake->snake_x[i], snake->snake_y[i]);
	}
	DrawSurface(screen, snake->activeImage, snake->snake_x[0], snake->snake_y[0]);
}

void setSnake(Snake* snake) {
	snake->isAlive = true;
	snake->movingUp = true;
	snake->movingLeft = false;
	snake->movingDown = false;
	snake->movingRight = false;
	snake->snake_x[0] = SNAKE_SPAWN_X;
	snake->snake_y[0] = SNAKE_SPAWN_Y;
	snake->currentSnakeLength = INITIAL_SNAKE_LENGTH;

	for (int i = 1; i < INITIAL_SNAKE_LENGTH; i++) {
		snake->snake_x[i] = snake->snake_x[0];
	}
}

void chaseHead(Snake* snake, GameState* state) {
	if (state->timeTracker > state->chase_delay) {
		for (int i = snake->currentSnakeLength - 1; i > 0; i--) {
			snake->snake_x[i] = snake->snake_x[i - 1];
			snake->snake_y[i] = snake->snake_y[i - 1];
		}
		state->timeTracker = 0.0;
	}
}

void getPhantomPoints(Snake* snake, double *pointX, double *pointY) {
	//the bmps are drawn from the top left hand corner and are 16pixels in width as well as in hight
	//when getting the phantom points i increment or decrement by 8 n-times to get the phantom point i want depending on the direction of the snake
	if (snake->movingUp) {
		*pointX = snake->snake_x[0] + 8;
		*pointY = snake->snake_y[0] - 8;
	}
	else if (snake->movingLeft) {
		*pointX = snake->snake_x[0] - 8;
		*pointY = snake->snake_y[0] + 8;
	}
	else if (snake->movingDown) {
		*pointX = snake->snake_x[0] + 8;
		*pointY = snake->snake_y[0] + 24;
	}
	else if (snake->movingRight) {
		*pointX = snake->snake_x[0] + 24;
		*pointY = snake->snake_y[0] + 8;
	}
}

void selfHit(Snake* snake, GameState* state) {
	double x, y;
	getPhantomPoints(snake, &x, &y);

	for (int i = 1; i < snake->currentSnakeLength; i++) {
		if (x <= snake->snake_x[i]+16 && x >= snake->snake_x[i] && y >= snake->snake_y[i] && y <= snake->snake_y[i]+16) {
			state->collision = true;
		}
	}	
}

void randCoords(double* x, double* y) {
	*x = (rand() % (SCREEN_WIDTH - 20)) + 20;
	*y = (rand() % (SCREEN_HEIGHT- 70)) + 70;
}

void spawnBlueDot(SDL_Surface* screen, GameState* state) {
	if (state->blueDotSpawned == false) {
		randCoords(&state->blue_dot_x, &state->blue_dot_y);
	}
	DrawSurface(screen, state->blue_dot, state->blue_dot_x, state->blue_dot_y);
	state->blueDotSpawned = true;
}

void blueDotReached(Snake* snake, GameState* state) {
	double x, y;
	getPhantomPoints(snake, &x, &y);
	if (x <= state->blue_dot_x + SENSITIVITY + 16 && x >= state->blue_dot_x - SENSITIVITY && y >= state->blue_dot_y - SENSITIVITY && y <= state->blue_dot_y + SENSITIVITY + 16) {
		state->blueDotSpawned = false;
		state->blueDotReached = true;
		for (int i = 0; i < UNIT; i++) {
			snake->snake_x[snake->currentSnakeLength + i] = 1000;
			snake->snake_y[snake->currentSnakeLength + i] = 1000;
		}
		snake->currentSnakeLength += UNIT;
		state->points += BLUE_POINTS;
	}
}

void speedUp(GameState* state) {
	if (state->speedUpTimer > SPEED_UP_INTERVAL) {
		state->speed_const *= SPEED_UP_MULTIPLIER;
		state->speedUpTimer -= SPEED_UP_INTERVAL;
	}
}





void spawnRedDot(SDL_Surface* screen, GameState* state) {
	if (state->no_red_cords == true) {
		randCoords(&state->red_dot_x, &state->red_dot_y);
		state->no_red_cords = false;
	}
	
	DrawSurface(screen, state->red_dot, state->red_dot_x, state->red_dot_y);
	state->redDotSpawned = true;
}

void redDotReached(Snake* snake, GameState* state) {
	double x, y;
	getPhantomPoints(snake, &x, &y);
	if (x <= state->red_dot_x + SENSITIVITY + 16 && x >= state->red_dot_x - SENSITIVITY && y >= state->red_dot_y - SENSITIVITY && y <= state->red_dot_y + SENSITIVITY + 16 && state->showProgressBar == true) {
		state->redDotReached = true;
		state->redBonusTimer = 0;
		state->progressTimer = 0;
		state->showProgressBar = false;
		state->points += RED_POINTS;
		if (state->fiftyfifty == 1) {
			for (int i = 0; i < UNIT; i++) {
				snake->snake_x[snake->currentSnakeLength - i] = 1000;
				snake->snake_y[snake->currentSnakeLength - i] = 1000;
			}
			snake->currentSnakeLength -= UNIT;
		}
		else if (state->fiftyfifty == 2) {
			state->speed_const *= SLOW_DOWN_MULTIPLIER;
			state->movement_speed = state->speed_const * state->delta;
			state->chase_delay = 15.0 / state->speed_const;
		}
		
	}
}

void redDotBonus(SDL_Surface* screen, GameState* state) {
	if ((int)state->redBonusTimer % state->red_interval == 0 && state->redBonusTimer > 1 && state->redDotReached == false) {
		state->redDotSpawned = false;
		state->showProgressBar = true;
	}
	if (state->showProgressBar == true) {
		spawnRedDot(screen, state);
	}
	if (state->progressTimer == 0) {
		state->no_red_cords = true;
	}
}

void snakeLengthMin(Snake* snake) {
	if (snake->currentSnakeLength < INITIAL_SNAKE_LENGTH) {
		snake->currentSnakeLength = INITIAL_SNAKE_LENGTH;
	}
}

void mainFunctions(SDL_Surface* screen, SDL_Texture* scrtex, SDL_Renderer* renderer, GameState* state, Colors* colors, Snake* snake) {
	printAllVisuals(screen, state, colors, snake);
	snakeLengthMin(snake);
	spawnBlueDot(screen, state);
	blueDotReached(snake, state);
	redDotReached(snake, state);
	keystrokeReact(state, snake);
	snakeMovement(state, snake);
	moveSnake(snake, state);
	wallInteraction(snake, state);
	drawSnake(snake, screen);
	chaseHead(snake, state);
	selfHit(snake, state);
	speedUp(state);
	redDotBonus(screen, state);
	updateRenderer(scrtex, screen, renderer);
}

void timerChanges(GameState* state) {
	state->fpsTimer += state->delta;
	if (state->fpsTimer > 0.5) {
		state->fps = state->frames * 2;
		state->frames = 0;
		state->fpsTimer -= 0.5;
	};
	state->worldTime += state->delta;
	state->timeTracker += state->delta;
	state->speedUpTimer += state->delta;
	if (state->redBonusTimer == 0) {
		state->redDotReached = false;
		state->showProgressBar = false;
		state->red_interval = (rand() % RED_UPPER_LIMIT) + RED_BOTTOM_LIMIT;
	}
	if (state->showProgressBar == false) {
		state->redBonusTimer += state->delta;
	}
	if (state->showProgressBar == true) {
		state->progressTimer += state->delta;
	}
}


void setVariables(GameState* state) {
	state->frames = 0;
	state->fpsTimer = 0;
	state->timeTracker = 0.0;
	state->fps = 0;
	state->quit = 0;
	state->collision = false;
	state->worldTime = 0;
	state->speedUpTimer = 0;
	state->redBonusTimer = 0;
	state->progressTimer = 0;
	state->blueDotSpawned = false;
	state->redDotSpawned = false;
	state->blueDotReached = true;
	state->redDotReached = false;
	state->speed_const = SPEED_CONST;
	state->showProgressBar = false;
	state->no_red_cords = false;
	state->red_interval = (rand() % RED_UPPER_LIMIT) + RED_BOTTOM_LIMIT;
	state->points = 0;
}


#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char** argv) {
	GameState state;
	Snake snake;
	Colors colors;

	SDL_Event event{};
	SDL_Surface* screen;
	SDL_Texture* scrtex;
	SDL_Window* window;
	SDL_Renderer* renderer;

	initializeSDL(&state, &snake, &colors, &event, &screen, &scrtex, &window, &renderer);
	int t1, t2;

	t1 = SDL_GetTicks();

	setVariables(&state);

	setSnake(&snake);

	srand(time(NULL));

	while (!state.quit) {
		if (!state.collision) {
			
			t2 = SDL_GetTicks();
			state.delta = (t2 - t1) * 0.001;
			t1 = t2;

			state.movement_speed = state.speed_const * state.delta;
			state.chase_delay = 15.0/state.speed_const;
			state.fiftyfifty = rand() % 2 + 1;

			timerChanges(&state);

			mainFunctions(screen, scrtex, renderer, &state, &colors, &snake);

			state.frames++;
		}
		else {
			keystrokeReact(&state, &snake);
		}
	}

	closingGame(window, renderer, screen, scrtex, &state);
	return 0;
};