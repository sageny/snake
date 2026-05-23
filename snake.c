/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Snake game - 256x256 grid, '+' border, '#' for both snake body and food.
 * Keyboard arrows control direction.
 *
 * Compile: gcc -o snake snake.c -lncurses
 */

#include <curses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define GRID_WIDTH  256
#define GRID_HEIGHT 256
#define MAX_LEN     (GRID_WIDTH * GRID_HEIGHT)

/* Direction codes */
enum direction {
	DIR_NONE = 0,
	DIR_UP,
	DIR_DOWN,
	DIR_LEFT,
	DIR_RIGHT
};

/* Point on grid */
struct point {
	int x;
	int y;
};

/* Snake queue (circular buffer) */
struct snake {
	struct point body[MAX_LEN];
	int head;		/* index of head */
	int tail;		/* index of tail */
	int len;		/* current length */
};

static struct snake snake;
static struct point food;
static enum direction cur_dir, next_dir;
static int score;
static int game_over;

/*
 * Initialize snake with 3 segments horizontally.
 * Head at (128,128), body leftwards.
 */
static void snake_init(void)
{
	int i;
	int start_x = GRID_WIDTH / 2;
	int start_y = GRID_HEIGHT / 2;

	snake.head = 0;
	snake.tail = 2;
	snake.len = 3;

	snake.body[0].x = start_x;
	snake.body[0].y = start_y;

	snake.body[1].x = start_x - 1;
	snake.body[1].y = start_y;

	snake.body[2].x = start_x - 2;
	snake.body[2].y = start_y;
}

/* Add new head to snake */
static void snake_add_head(int x, int y)
{
	snake.head = (snake.head - 1 + MAX_LEN) % MAX_LEN;
	snake.body[snake.head].x = x;
	snake.body[snake.head].y = y;
	snake.len++;
}

/* Remove tail and return the removed point */
static struct point snake_remove_tail(void)
{
	struct point old_tail = snake.body[snake.tail];
	snake.tail = (snake.tail + 1) % MAX_LEN;
	snake.len--;
	return old_tail;
}

/* Get head position */
static struct point snake_get_head(void)
{
	return snake.body[snake.head];
}

/* Check if given coordinate is occupied by snake body */
static int snake_is_occupied(int x, int y)
{
	int i, idx;
	for (i = 0; i < snake.len; i++) {
		idx = (snake.head + i) % MAX_LEN;
		if (snake.body[idx].x == x && snake.body[idx].y == y)
			return 1;
	}
	return 0;
}

/*
 * Generate random food not on snake body.
 * Return 1 on success, 0 if grid is full (game win).
 */
static int generate_food(void)
{
	int attempts = 0;
	int max_attempts = GRID_WIDTH * GRID_HEIGHT * 2;
	int x, y;

	if (snake.len >= GRID_WIDTH * GRID_HEIGHT)
		return 0;	/* Grid full, win */

	while (attempts < max_attempts) {
		x = (rand() % GRID_WIDTH) + 1;
		y = (rand() % GRID_HEIGHT) + 1;
		if (!snake_is_occupied(x, y)) {
			food.x = x;
			food.y = y;
			return 1;
		}
		attempts++;
	}
	/* Fallback: linear search for empty cell */
	for (y = 1; y <= GRID_HEIGHT; y++) {
		for (x = 1; x <= GRID_WIDTH; x++) {
			if (!snake_is_occupied(x, y)) {
				food.x = x;
				food.y = y;
				return 1;
			}
		}
	}
	return 0;	/* fully occupied */
}

/* Draw border using '+' characters */
static void draw_border(void)
{
	int i;

	/* top and bottom borders */
	for (i = 0; i <= GRID_WIDTH + 1; i++) {
		mvaddch(0, i, '+');
		mvaddch(GRID_HEIGHT + 1, i, '+');
	}
	/* left and right borders */
	for (i = 1; i <= GRID_HEIGHT; i++) {
		mvaddch(i, 0, '+');
		mvaddch(i, GRID_WIDTH + 1, '+');
	}
}

/* Draw snake and food on screen */
static void draw_snake_and_food(void)
{
	int i, idx;
	struct point p;

	/* draw snake body */
	for (i = 0; i < snake.len; i++) {
		idx = (snake.head + i) % MAX_LEN;
		p = snake.body[idx];
		mvaddch(p.y, p.x, '#');
	}
	/* draw food */
	mvaddch(food.y, food.x, '#');
}

/* Refresh entire game screen */
static void redraw_screen(void)
{
	clear();
	draw_border();
	draw_snake_and_food();
	mvprintw(GRID_HEIGHT + 2, 2, "Score: %d  Press 'q' to quit", score);
	refresh();
}

/* Update game state: move snake, check collisions, eat food */
static void update_game(void)
{
	struct point new_head;
	struct point old_tail;
	int ate_food;

	/* commit queued direction if not opposite */
	if ((next_dir == DIR_UP && cur_dir != DIR_DOWN) ||
	    (next_dir == DIR_DOWN && cur_dir != DIR_UP) ||
	    (next_dir == DIR_LEFT && cur_dir != DIR_RIGHT) ||
	    (next_dir == DIR_RIGHT && cur_dir != DIR_LEFT)) {
		cur_dir = next_dir;
	}

	/* calculate new head position */
	new_head = snake_get_head();
	switch (cur_dir) {
	case DIR_UP:
		new_head.y--;
		break;
	case DIR_DOWN:
		new_head.y++;
		break;
	case DIR_LEFT:
		new_head.x--;
		break;
	case DIR_RIGHT:
		new_head.x++;
		break;
	default:
		return;
	}

	/* border collision */
	if (new_head.x < 1 || new_head.x > GRID_WIDTH ||
	    new_head.y < 1 || new_head.y > GRID_HEIGHT) {
		game_over = 1;
		return;
	}

	ate_food = (new_head.x == food.x && new_head.y == food.y);

	if (ate_food) {
		/* grow: add head, keep tail */
		snake_add_head(new_head.x, new_head.y);
		score++;
		if (!generate_food()) {
			/* grid full -> win */
			game_over = 1;
		}
	} else {
		/* normal move: add head, remove tail */
		snake_add_head(new_head.x, new_head.y);
		old_tail = snake_remove_tail();

		/* self collision (head hit body) */
		if (snake_is_occupied(new_head.x, new_head.y)) {
			/* head may overlap with body only because it hit itself */
			game_over = 1;
			return;
		}
		/* mark old tail position as empty (no extra action needed) */
		(void)old_tail;
	}
}

/* Initialize game variables */
static void init_game(void)
{
	srand((unsigned int)time(NULL));
	snake_init();
	cur_dir = DIR_RIGHT;
	next_dir = DIR_RIGHT;
	score = 0;
	game_over = 0;
	if (!generate_food())
		game_over = 1;	/* no free cell at start (impossible) */
}

/* Cleanup and exit */
static void cleanup_and_exit(void)
{
	endwin();
	exit(0);
}

int main(void)
{
	int ch;
	struct timespec req, rem;
	unsigned long last_move;
	unsigned long now;

	/* Initialize ncurses */
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	curs_set(0);
	nodelay(stdscr, TRUE);	/* non-blocking getch */

	if (LINES < GRID_HEIGHT + 5 || COLS < GRID_WIDTH + 4) {
		endwin();
		fprintf(stderr, "Terminal too small. Need at least %dx%d.\n",
			GRID_WIDTH + 4, GRID_HEIGHT + 5);
		return 1;
	}

	init_game();
	redraw_screen();

	last_move = (unsigned long)time(NULL);
	req.tv_sec = 0;
	req.tv_nsec = 100000000L;	/* 100 ms per move */

	while (!game_over) {
		/* handle keyboard input */
		ch = getch();
		if (ch != ERR) {
			switch (ch) {
			case 'q':
			case 'Q':
				game_over = 1;
				break;
			case KEY_UP:
				next_dir = DIR_UP;
				break;
			case KEY_DOWN:
				next_dir = DIR_DOWN;
				break;
			case KEY_LEFT:
				next_dir = DIR_LEFT;
				break;
			case KEY_RIGHT:
				next_dir = DIR_RIGHT;
				break;
			}
		}

		/* time-based move */
		now = (unsigned long)time(NULL) * 1000 +
		      (unsigned long)((struct timespec){0}).tv_nsec / 1000000;
		/* simplified: use nanosleep for consistent delay */
		nanosleep(&req, &rem);

		update_game();
		redraw_screen();
	}

	/* game over message */
	clear();
	mvprintw(GRID_HEIGHT / 2, GRID_WIDTH / 2 - 10,
		 "GAME OVER! Score: %d  Press any key", score);
	refresh();
	nodelay(stdscr, FALSE);
	getch();
	cleanup_and_exit();
	return 0;
}