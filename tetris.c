#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <termios.h>

//Global vars
#define WIDTH 20
#define HEIGHT 15
#define EMPTY ' '
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

struct Position {
    u_int8_t x;
    u_int8_t y;
};

void clear_screen() {
    printf("\033[H");
    printf("\033[2J");
}

void draw_horizontal_line() {
    printf("<+");
    for (int i = 0; i < WIDTH; i++) printf("-");
    printf("+>\n");
}

void draw_area(char **area) {
    draw_horizontal_line();
    for (int i = 0; i < HEIGHT; i++) {
        printf("<!");
        for (int j = 0; j < WIDTH; j++) {
            printf("%c", area[i][j]);
        }
        printf("!>\n");
    }
    draw_horizontal_line();
    fflush(stdout);
}

void erase_block(char **area, struct Position *block) { //Deletes block based on coordinates
    for(int i = 0; i < 4; i++) {
        u_int8_t tmp_x = block[i].x;
        u_int8_t tmp_y = block[i].y;
        area[tmp_y][tmp_x] = EMPTY;
    }
}

void draw_block(char **area, struct Position *block) { //Translates the block coordinates into a drawn block
    for(int i = 0; i < 4; i++) {
        u_int8_t tmp_x = block[i].x;
        u_int8_t tmp_y = block[i].y;
        area[tmp_y][tmp_x] = FULL;
    }
}

size_t find_biggest_y(struct Position *block) { //Find the highest y (the one which is the most "southern")
    size_t index = 0;                           //Returns the index of the struct with the biggest y
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
bool move_block(char **area, struct Position *block, char direction) {
   erase_block(area, block);

    bool bounds_ok, collision_ok;

    switch(direction){
        case 'l': //left
            bounds_ok = true;
            collision_ok = true;

            //Check for bounds
            for(int i = 0; i < 4; i++) {
                if((block[i].x -1) < 0){
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
                if((block[i].x +1) >= WIDTH){
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
void spawn_block(char **area, struct Position  *block) {
    srand(time(NULL));
    int block_type = rand() % 7;
    switch(block_type) {
        case 0:
            block[0].y = 0;
            block[1].y = 1;
            block[2].y = 2;
            block[3].y = 3;

            block[0].x = 10;
            block[1].x = 10;
            block[2].x = 10;
            block[3].x = 10;
            break;
        case 1:
            block[0].y = 0;
            block[1].y = 0;
            block[2].y = 1;
            block[3].y = 1;

            block[0].x = 10;
            block[1].x = 11;
            block[2].x = 10;
            block[3].x = 11;
            break;
        case 2:
            block[0].y = 0;
            block[1].y = 1;
            block[2].y = 2;
            block[3].y = 2;

            block[0].x = 10;
            block[1].x = 10;
            block[2].x = 10;
            block[3].x = 11;
            break;
        case 3:
            block[0].y = 0;
            block[1].y = 1;
            block[2].y = 2;
            block[3].y = 2;

            block[0].x = 10;
            block[1].x = 10;
            block[2].x = 10;
            block[3].x = 9;
            break;
        case 4:
            block[0].y = 0;
            block[1].y = 0;
            block[2].y = 1;
            block[3].y = 1;

            block[0].x = 10;
            block[1].x = 11;
            block[2].x = 9;
            block[3].x = 8;
            break;
        case 5:
            block[0].y = 0;
            block[1].y = 0;
            block[2].y = 1;
            block[3].y = 1;

            block[0].x = 9;
            block[1].x = 10;
            block[2].x = 11;
            block[3].x = 12;
            break;
        case 6:
            block[0].y = 0;
            block[1].y = 1;
            block[2].y = 1;
            block[3].y = 1;

            block[0].x = 10;
            block[1].x = 9;
            block[2].x = 10;
            block[3].x = 11;
            break;
    }
    draw_block(area, block);
}

int main(void) {
    //Game area declaration on the heap
    char **play_area = malloc(HEIGHT * sizeof(char*));
    for(int i = 0; i < HEIGHT; i++){
        play_area[i] = malloc(WIDTH * sizeof(char));
    }

    //Game area initialization
    for(int i = 0; i < HEIGHT; i++){
        for(int j = 0; j < WIDTH; j++){
            play_area[i][j] = EMPTY;
        }
    }

    //Block array declaration and initialization
    struct Position *block = malloc(4*sizeof(struct Position));
    struct Position tmp = {0,0};
    for(int i = 0; i < 4; i++) block[i] = tmp;

    //Setting the terminal into the right mode
    struct termios info;
    tcgetattr(0, &info);
    info.c_lflag &= ~ICANON; //Disable canonical mode
    info.c_cc[VMIN] = 0; //Not waiting for input
    info.c_cc[VTIME] = 0; //No timeout
    tcsetattr(0, TCSANOW, &info); //Apply the settings now

    //Main game loop
    struct timespec start_time, stop_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time); //Get start time

    spawn_block(play_area, block);
    char input = '/';
    while (true) {
        clear_screen();
        draw_area(play_area);


        read(STDIN_FILENO, &input, 1); //Read char
        printf("%c", input);
        switch(input){
            case 'd':
                if(move_block(play_area, block, 'r')) spawn_block(play_area, block);
                input = '/';
                break;
            case 'a':
                if(move_block(play_area, block, 'l')) spawn_block(play_area, block);
                input = '/';
                break;
            case 's':
                if(move_block(play_area, block, 'd')) spawn_block(play_area, block);
                input = '/';
                break;
        }

        clock_gettime(CLOCK_MONOTONIC, &stop_time); //Get stop time
        //Getting the total enlapsed time in microseconds
        long enlapsed = (stop_time.tv_sec - start_time.tv_sec) * 1000000L + (stop_time.tv_nsec - start_time.tv_nsec) / 1000;
        if(enlapsed >= DELAY_2){
            clock_gettime(CLOCK_MONOTONIC, &start_time); //Reset start time
            if(move_block(play_area, block, '/')) spawn_block(play_area, block);
        }

        usleep(FPS_60);
    }

    //Reseting the terminal back into canonical mode
    tcgetattr(0, &info);
    info.c_lflag |= ICANON;
    tcsetattr(0, TCSANOW, &info);

    //Freeing the play_area array
    for(int i = 0; i < HEIGHT; i++) free(play_area[i]);
    free(play_area);

    //Freeing the block array
    free(block);

    return 0;
}
