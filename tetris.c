#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <termios.h>
#include <string.h>
#include <stdbool.h>

//Macros
#define WIDTH  10
#define HEIGHT 20
#define EMPTY '.'
#define FULL '#'
#define DELAY_0 1000000  // 1000 ms
#define DELAY_1 750000   // 750 ms
#define DELAY_2 500000   // 500 ms
#define DELAY_3 250000   // 250 ms
#define DELAY_4 150000   // 150 ms
#define DELAY_5 125000   // 125 ms
#define DELAY_6 100000   // 100 ms
#define DELAY_7 75000    // 75 ms
#define DELAY_8 50000    // 50 ms
#define DELAY_9 25000    // 25 ms
#define FPS_60  16667
#define FPS_30  33333

//Global vars
bool running = true;

struct Position {
    u_int8_t x;
    u_int8_t y;
};

//Play area operations
void clear_screen();
void draw_horizontal_line();
void draw_area(char area[][WIDTH], int score);

//Block operations
void erase_block(char area[][WIDTH], struct Position *block);
void draw_block(char area[][WIDTH], struct Position *block);
bool move_block(char area[][WIDTH], struct Position *block, char direction);
void spawn_block(char area[][WIDTH], struct Position *block, int block_type);
void rotate_block(char area[][WIDTH], struct Position *block);

//Score counting functions
void check_lines(char area[][WIDTH], int *score);
void clear_line(char area[][WIDTH], size_t line);

//Helper functions
size_t find_biggest_y(struct Position *block);

int main(void) {
    //Game area declaration
    char play_area[HEIGHT][WIDTH];

    //Game area initialization
    for(int i = 0; i < HEIGHT; i++){
        for(int j = 0; j < WIDTH; j++){
            play_area[i][j] = EMPTY;
        }
    }

    struct Position block[4];
    int block_type = -1; //-1 means the block has not spawned yet

    //Setting the terminal into the right mode
    struct termios info;
    tcgetattr(0, &info);
    info.c_lflag &= ~ICANON; //Disable canonical mode
    info.c_cc[VMIN] = 0; //Not waiting for input
    info.c_cc[VTIME] = 0; //No timeout
    tcsetattr(0, TCSANOW, &info); //Apply

    //Main game loop
    struct timespec start_time, stop_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time); //Get start time

    char input = '/';
    int score = 0;
    spawn_block(play_area, block, block_type);
    while (running) {
        clear_screen();
        draw_area(play_area, score);
        check_lines(play_area, &score);

        read(STDIN_FILENO, &input, 1); //Read char
        printf("%c", input);
        switch(input){
            case 'd':
                if(move_block(play_area, block, 'r')) spawn_block(play_area, block, block_type);
                input = '/';
                break;
            case 'a':
                if(move_block(play_area, block, 'l')) spawn_block(play_area, block, block_type);
                input = '/';
                break;
            case 's':
                if(move_block(play_area, block, 'd')) spawn_block(play_area, block, block_type);
                input = '/';
                break;
            case 'x':
                rotate_block(play_area, block);
                input = '/';
                break;
            case 'e':
                running = false;
                break;
        }

        clock_gettime(CLOCK_MONOTONIC, &stop_time); //Get stop time
        //Getting the total enlapsed time in microseconds
        long enlapsed = (stop_time.tv_sec - start_time.tv_sec) * 1000000L + (stop_time.tv_nsec - start_time.tv_nsec) / 1000;
        if(enlapsed >= DELAY_2){
            clock_gettime(CLOCK_MONOTONIC, &start_time); //Reset start time
            if(move_block(play_area, block, '/')) spawn_block(play_area, block, block_type);
        }

        usleep(FPS_60);
    }

    //Reseting the terminal back into canonical mode
    tcgetattr(0, &info);
    info.c_lflag |= ICANON;
    tcsetattr(0, TCSANOW, &info);


    return 0;
}

void clear_screen() {
    printf("\033[H");
    printf("\033[2J");
}

void draw_horizontal_line() {
    printf("<+");
    for (int i = 0; i < WIDTH*3; i++) printf("-"); //*3 because of the extra 2 spaces in " %c " in draw_area()
    printf("+>\n");
}

void draw_area(char area[][WIDTH], int score) {
    draw_horizontal_line();
    for (int i = 0; i < HEIGHT; i++) {
        printf("<!");
        for (int j = 0; j < WIDTH; j++) {
            if(area[i][j] == FULL) {
                printf( " %s ", "■");
            } else{
                printf(" %c ", EMPTY);
            }
        }
        printf("!>\n");
    }
    draw_horizontal_line();
    printf("Score: %d\n", score);
    fflush(stdout);
}

//Deletes block based on coordinates
void erase_block(char area[][WIDTH], struct Position *block) {
    for(int i = 0; i < 4; i++) {
        u_int8_t tmp_x = block[i].x;
        u_int8_t tmp_y = block[i].y;
        area[tmp_y][tmp_x] = EMPTY;
    }
}

//Translates the block coordinates into a drawn block
void draw_block(char area[][WIDTH], struct Position *block) {
    for(int i = 0; i < 4; i++) {
        u_int8_t tmp_x = block[i].x;
        u_int8_t tmp_y = block[i].y;
        area[tmp_y][tmp_x] = FULL;
    }
}

/* Finds the highest y (the one which is the most "southern")
 * Returns the index of the struct with the biggest y
 */
size_t find_biggest_y(struct Position *block) {
    size_t index = 0;
    u_int8_t max = block[0].y;
    for(size_t i = 1; i < 4; i++) {
        if(block[i].y > max) {
            max = block[i].y;
            index = i;
        }
    }
    return index;
}

/*
 * The function moves the block to the desired location and also keeps the block
 * inside the bounds and also handles collision with other blocks.
 * Returns true if the block has been placed and its possible to spawn a new one
 */
bool move_block(char area[][WIDTH], struct Position *block, char direction) {
    erase_block(area, block);

    bool bounds_ok, collision_ok;

    switch(direction){
        case 'l': //left
            bounds_ok = true;
            collision_ok = true;

            //Check for bounds
            for(int i = 0; i < 4; i++) {
                if((block[i].x -1) < 0|| block[i].y >= HEIGHT) {
                    bounds_ok = false;
                    break;
                }
            }

            if(!bounds_ok) break;  //Jump out early because condition is not met

            //Check for collision
            for(int i = 0; i < 4; i++) {
                u_int8_t tmp_x = block[i].x - 1; //Check the block after it
                u_int8_t tmp_y = block[i].y;
                if(area[tmp_y][tmp_x] == FULL){
                    collision_ok = false;
                    break;
                }
            }

            if(bounds_ok && collision_ok) { //Move block only if both conditions are met
                block[0].x -= 1;
                block[1].x -= 1;
                block[2].x -= 1;
                block[3].x -= 1;
            }
            break;

        case 'r': //right
            bounds_ok = true;
            collision_ok = true;

            //Check for bounds
            for(int i = 0; i < 4; i++) {
                if((block[i].x +1) >= WIDTH || block[i].y >= HEIGHT){
                    bounds_ok = false;
                    break;
                }
            }

            if(!bounds_ok) break;  //Jump out early because condition is not met

            //Check for collision
            for(int i = 0; i < 4; i++) {
                u_int8_t tmp_x = block[i].x + 1; //Check the block after it
                u_int8_t tmp_y = block[i].y;
                if(area[tmp_y][tmp_x] == FULL){
                    collision_ok = false;
                    break;
                }
            }

            if(bounds_ok && collision_ok) { //Move block only if both conditions are met
                block[0].x += 1;
                block[1].x += 1;
                block[2].x += 1;
                block[3].x += 1;
            }
            break;

        case 'd': //two down
            bounds_ok = true;
            collision_ok = true;

            //Check for bounds
            for(int i = 0; i < 4; i++) {
                if((block[i].y +1)  >= HEIGHT){
                    bounds_ok = false;
                    break;
                }
            }

            if(!bounds_ok) break;  //Jump out early because condition is not met

        //Check for collision
        size_t index_y = find_biggest_y(block);
        u_int8_t tmp = block[index_y].y;
        for(int i = 0; i < 4; i++) {
            if(block[i].y == tmp){
                u_int8_t tmp_x = block[i].x;
                u_int8_t tmp_y = block[i].y + 1; //Check the block after it
                if(area[tmp_y][tmp_x] == FULL){
                    collision_ok = false;
                    break;
                }
            }
        }

        if(bounds_ok && collision_ok) { //Move block only if both conditions are met
            block[0].y += 1;
            block[1].y += 1;
            block[2].y += 1;
            block[3].y += 1;
        } else{
            draw_block(area, block);
            return true;
        }
        break;

        default: //one down
            bounds_ok = true;
            collision_ok = true;

            //Check for bounds
            for(int i = 0; i < 4; i++) {
                if((block[i].y +1)  >= HEIGHT){
                    bounds_ok = false;
                    break;
                }
            }

            if(!bounds_ok) { //Jump out early because condition is not met
                draw_block(area, block);
                return true;
            }

            //Check for collision
            index_y = find_biggest_y(block);
            tmp = block[index_y].y;
            for(int i = 0; i < 4; i++) {
                if(block[i].y == tmp){
                    u_int8_t tmp_x = block[i].x;
                    u_int8_t tmp_y = block[i].y + 1; //Check the block after it
                    if(area[tmp_y][tmp_x] == FULL){
                        collision_ok = false;
                        break;
                    }
                }
            }

            if(bounds_ok && collision_ok) { //Move block only if both conditions are met
                block[0].y += 1;
                block[1].y += 1;
                block[2].y += 1;
                block[3].y += 1;
            } else{
                draw_block(area, block);
                return true;
            }
            break;
    }
    draw_block(area, block);
    return false;
}

/*
 * Spawns block with the desired shape
 * 0 = flat line
 * 1 = square
 * 2 = L facing right
 * 3 = L facing left
 * 4 = "squiggly" block facing right
 * 5 = "squiggly" block facing left
 * 6 = "pyramid"
 */
void spawn_block(char area[][WIDTH], struct Position *block, int block_type) {
    srand(time(NULL));
    block_type = rand() % 7;
    uint mid = WIDTH/2;

    switch(block_type) {
        case 0:
            block[0].y = 0;
            block[1].y = 1;
            block[2].y = 2;
            block[3].y = 3;

            block[0].x = mid;
            block[1].x = mid;
            block[2].x = mid;
            block[3].x = mid;
            break;
        case 1:
            block[0].y = 0;
            block[1].y = 0;
            block[2].y = 1;
            block[3].y = 1;

            block[0].x = mid;
            block[1].x = mid+1;
            block[2].x = mid;
            block[3].x = mid+1;
            break;
        case 2:
            block[0].y = 0;
            block[1].y = 1;
            block[2].y = 2;
            block[3].y = 2;

            block[0].x = mid;
            block[1].x = mid;
            block[2].x = mid;
            block[3].x = mid+1;
            break;
        case 3:
            block[0].y = 0;
            block[1].y = 1;
            block[2].y = 2;
            block[3].y = 2;

            block[0].x = mid;
            block[1].x = mid;
            block[2].x = mid;
            block[3].x = mid-1;
            break;
        case 4:
            block[0].y = 0;
            block[1].y = 0;
            block[2].y = 1;
            block[3].y = 1;

            block[0].x = mid-1;
            block[1].x = mid;
            block[2].x = mid-1;
            block[3].x = mid-2;
            break;
        case 5:
            block[0].y = 0;
            block[1].y = 0;
            block[2].y = 1;
            block[3].y = 1;

            block[0].x = mid-2;
            block[1].x = mid-1;
            block[2].x = mid-1;
            block[3].x = mid;
            break;
        case 6:
            block[0].y = 0;
            block[1].y = 1;
            block[2].y = 1;
            block[3].y = 1;

            block[0].x = mid;
            block[1].x = mid-1;
            block[2].x = mid;
            block[3].x = mid+1;
            break;
    }
    draw_block(area, block);
}

// Function clears the specified line and moves the remains of blocks down
void clear_line(char area[][WIDTH], size_t line) {
    //Cler the full line
    for(size_t i = 0; i < WIDTH; i++) {
        area[line][i] = EMPTY;
    }

    //Move the blocks down
    for(size_t i = line; i > 0; i--) {
        for(size_t j = 0; j < WIDTH; j++) {
            if(area[i-1][j] == FULL) {
                area[i-1][j] = EMPTY;
                area[i][j] = FULL;
            }
        }
    }
}

/* Function checks for completed lines and incrementes the score appropriately
 *  n lines = n*100 points
 */
void check_lines(char area[][WIDTH], int *score) {
    u_int8_t total_lines = 0;
    u_int8_t row_sum = 0;

    //Count the score
    for(size_t i = 0; i < HEIGHT; i++) {
        for(size_t j = 0; j < WIDTH; j++) {
            if(area[i][j] == FULL) row_sum++;
        }
        if(row_sum == WIDTH) {
            clear_line(area, i);
            total_lines++;
        }
        row_sum = 0;
    }
    *score += (int)total_lines*100;
}

//Function rotates the block clockwise
void rotate_block(char area[][WIDTH], struct Position * block) {
    bool check_square_y = (block[0].y == block[1].y) && (block[2].y == block[3].y);
    bool check_square_x = (block[0].x == block[2].x) && (block[1].x == block[3].x);
    if(check_square_x && check_square_y) return; //Do not rotate if its a square
    
    struct Position tmp_arr[4];
    bool is_writable = true;

    int pivot_x = block[2].x;
    int pivot_y = block[2].y;

    erase_block(area, block);
    for(size_t i = 0; i < 4; i++) {  
        int tmp_x = block[i].x;
        int tmp_y = block[i].y;

        //x-x(pivot) y-y(pivot)
        tmp_x -= pivot_x;
        tmp_y -= pivot_y;

        // x,y -> y,-x
        int tmp = tmp_x * -1;
        tmp_x = tmp_y;
        tmp_y = tmp;

        //x += x(pivot) y += y(pivot)
        tmp_x += pivot_x;
        tmp_y += pivot_y;

        //Saving the new position
        if(!(tmp_x >= 0) || !(tmp_x < WIDTH)) is_writable = false; 
        if(!(tmp_y >= 0) || !(tmp_x < HEIGHT)) is_writable = false; 

        tmp_arr[i].x = tmp_x;
        tmp_arr[i].y = tmp_y;
    }

    for (size_t i = 0; i < 4 && is_writable; i++) {
        block[i].x = tmp_arr[i].x;
        block[i].y = tmp_arr[i].y;
    }
    draw_block(area, block);
}
