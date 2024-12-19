#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>
#include <cstdlib> 

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

//screen resolution
#define SCREEN_WIDTH			640
#define SCREEN_HEIGHT			480
#define SNAKE_SPAWN_X			300
#define SNAKE_SPAWN_Y			400
#define MOVEMENT_SPEED			0.05
#define INITIAL_SNAKE_LENGTH 	100		//so 1 head image and n-1 body images
#define MAX_SNAKE_LENGTH		100
#define SNAKE_BODY_GAP			4
#define SELF_HIT_SENSITIVITY	10
#define CHASE_DELAY				0.185

typedef struct GameState {
	SDL_Event event;
	SDL_Surface* charset;
	SDL_Surface* eti;
	int quit;
	double worldTime;
	bool keys[SDL_NUM_SCANCODES] = { false };
	double delta;
	double timeTracker;
	double fpsTimer;
	double fps;
	int frames;
	bool collision;
}state;

typedef struct Snake {
	double snake_x[MAX_SNAKE_LENGTH];
	double snake_y[MAX_SNAKE_LENGTH];
	bool isAlive;
	bool movingUp;
	bool movingLeft;
	bool movingDown;
	bool movingRight;
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

void printAllVisuals(SDL_Surface* screen, GameState* state, Colors* colors, Snake* snake) {
	SDL_FillRect(screen, NULL, colors->black);

	state->fpsTimer += state->delta;
	if (state->fpsTimer > 0.5) {
		state->fps = state->frames * 2;
		state->frames = 0;
		state->fpsTimer -= 0.5;
	};

	DrawLine(screen, 0, 0, SCREEN_WIDTH, 1, 0, colors->grey);
	DrawLine(screen, 0, 0, SCREEN_HEIGHT, 0, 1, colors->grey);
	DrawLine(screen, SCREEN_WIDTH - 1, 0, SCREEN_HEIGHT, 0, 1, colors->grey);
	DrawLine(screen, 0, SCREEN_HEIGHT - 1, SCREEN_WIDTH, 1, 0, colors->grey);

	DrawRectangle(screen, 3, 3, SCREEN_WIDTH - 6, 51, colors->black, colors->grey);

	sprintf(colors->text, "RUNNING TIME = %.1lf s  %.0lf frames / s", state->worldTime, state->fps);
	DrawString(screen, screen->w / 2 - strlen(colors->text) * 8 / 2, 10, colors->text, state->charset);

	sprintf(colors->text, "Esc - EXIT PROGRAM, n - NEW GAME");
	DrawString(screen, screen->w / 2 - strlen(colors->text) * 8 / 2, 26, colors->text, state->charset);

	//sprintf(colors->text, "W - CLIMB LADDER, A - GO LEFT, S - DESCEND LADDER,  D - GO RIGHT");
	//DrawString(screen, screen->w / 2 - strlen(colors->text) * 8 / 2, 42, colors->text, element->charset);

	sprintf(colors->text, "IMPLEMENTED ELEMENTS: 1,2,3,4");
	DrawString(screen, screen->w / 2 - strlen(colors->text) * 8 / 2, 42, colors->text, state->charset);

	//sprintf(colors->text, "snakex %f, snakey %f", snake->snake_x[0], snake->snake_y[0]);
	//DrawString(screen, screen->w / 2 - strlen(colors->text) * 8 / 2, 42, colors->text, state->charset);

	//sprintf(colors->text, "snakex %f, snakey %f", snake->snake_x[1], snake->snake_y[1]);
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
				break;
			}
			break;
		case SDL_KEYUP:
			state->keys[state->event.key.keysym.scancode] = false;
			break;
		}
	}
}

void wallInteraction(Snake* snake) {
	//upper wall interaction
	if (snake->movingUp && snake->snake_y[0] - MOVEMENT_SPEED <= 0) {
		if (snake->snake_x[0] + MOVEMENT_SPEED < SCREEN_WIDTH) {
			oneDirection(snake, 0, 0, 0, 1); // right
		}
		else {
			oneDirection(snake, 0, 1, 0, 0); // left
		}
	}

	//lefthand wall interaction
	if (snake->movingLeft && snake->snake_x[0] - MOVEMENT_SPEED <= 0) {
		if (snake->snake_y[0] - MOVEMENT_SPEED > 0) {
			oneDirection(snake, 1, 0, 0, 0); // up
		}
		else {
			oneDirection(snake, 0, 0, 1, 0); // down
		}
	}


	//bottom wall interaction
	if (snake->movingDown && snake->snake_y[0] + MOVEMENT_SPEED >= SCREEN_HEIGHT) {
		if (snake->snake_x[0] - MOVEMENT_SPEED > 0) {
			oneDirection(snake, 0, 1, 0, 0); // left
			
		}
		else {
			oneDirection(snake, 0, 0, 0, 1); // right
		}
	}


	//righthand wall interaction 
	if (snake->movingRight && snake->snake_x[0] + MOVEMENT_SPEED >= SCREEN_WIDTH) {
		if (snake->snake_y[0] + MOVEMENT_SPEED < SCREEN_HEIGHT) {
			oneDirection(snake, 0, 0, 1, 0); // down
		}
		else {
			oneDirection(snake, 1, 0, 0, 0); // up
		}
	}

}

void snakeMovement(GameState* state, Snake* snake) {
	// W
	if (state->keys[SDL_SCANCODE_W]) {
		if (snake->snake_y[0] - MOVEMENT_SPEED > 0) {
			oneDirection(snake, 1, 0, 0, 0); // up
		}
	}

	// A
	if (state->keys[SDL_SCANCODE_A]) {
		if (snake->snake_x[0] - MOVEMENT_SPEED > 0) {
			oneDirection(snake, 0, 1, 0, 0); // left
		}
	}

	// S
	if (state->keys[SDL_SCANCODE_S]) {
		if (snake->snake_y[0] + MOVEMENT_SPEED < SCREEN_HEIGHT) {
			oneDirection(snake, 0, 0, 1, 0); // down
		}
	}

	// D
	if (state->keys[SDL_SCANCODE_D]) {
		if (snake->snake_x[0] + MOVEMENT_SPEED < SCREEN_WIDTH) {
			oneDirection(snake, 0, 0, 0, 1); // right
		}
	}
}

void moveSnake(Snake* snake) {
	if (snake->movingUp) {
		snake->activeImage = snake->headUp;
		snake->snake_y[0] -= MOVEMENT_SPEED;
	}
	else if (snake->movingLeft) {
		snake->activeImage = snake->headLeft;
		snake->snake_x[0] -= MOVEMENT_SPEED;
	}
	else if (snake->movingDown) {
		snake->activeImage = snake->headDown;
		snake->snake_y[0] += MOVEMENT_SPEED;
	}
	else if (snake->movingRight) {
		snake->activeImage = snake->headRight;
		snake->snake_x[0] += MOVEMENT_SPEED;
	}
}

void drawSnake(Snake* snake, SDL_Surface* screen) {
	for (int i = 1; i < INITIAL_SNAKE_LENGTH; i++) {
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

	for (int i = 1; i < INITIAL_SNAKE_LENGTH; i++) {
		snake->snake_x[i] = snake->snake_x[0];
	}
}

void chaseHead(Snake* snake, GameState* state) {
	if (state->timeTracker > CHASE_DELAY) {
		for (int i = INITIAL_SNAKE_LENGTH - 1; i > 0; i--) {
			snake->snake_x[i] = snake->snake_x[i - 1];
			snake->snake_y[i] = snake->snake_y[i - 1];
		}
		state->timeTracker = 0.0;
	}
}

void getPhantomPoints(Snake* snake, double *pointX, double *pointY) {
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

	for (int i = 1; i < INITIAL_SNAKE_LENGTH; i++) {
		if (x <= snake->snake_x[i]+16 && x >= snake->snake_x[i] && y >= snake->snake_y[i] && y <= snake->snake_y[i]+16) {
			state->collision = true;
		}
	}	
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
	double distance, etiSpeed;

	t1 = SDL_GetTicks();

	state.frames = 0;
	state.fpsTimer = 0;
	state.timeTracker = 0.0; 
	state.fps = 0;
	state.quit = 0;
	state.collision = false;
	state.worldTime = 0;
	distance = 0;
	etiSpeed = 1;


	setSnake(&snake);

	while (!state.quit) {
		if (!state.collision) {
			t2 = SDL_GetTicks();
			state.delta = (t2 - t1) * 0.001;
			t1 = t2;

			state.worldTime += state.delta;

			distance += etiSpeed * state.delta;
			state.timeTracker += state.delta;

			printAllVisuals(screen, &state, &colors, &snake);
			keystrokeReact(&state, &snake);
			snakeMovement(&state, &snake);
			moveSnake(&snake);
			wallInteraction(&snake);
			drawSnake(&snake, screen);
			chaseHead(&snake, &state);
			updateRenderer(scrtex, screen, renderer);
			selfHit(&snake, &state);
			state.frames++;
		}
		else {
			keystrokeReact(&state, &snake);
		}
	}

	closingGame(window, renderer, screen, scrtex, &state);
	return 0;
};