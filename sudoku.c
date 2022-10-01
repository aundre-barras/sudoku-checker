#include <stdio.h>  
#include <unistd.h> 
#include <stdlib.h>
#include <getopt.h>
#include <pthread.h>

/* These are the only two global variables allowed in your program */
static int verbose = 0;
static int use_fork = 0;


// This is a simple function to parse the --fork argument.
// It also supports --verbose, -v
void parse_args(int argc, char *argv[])
{
    int c;
    while (1)
    {
        static struct option long_options[] =
        {
            {"verbose", no_argument,       0, 'v'},
            {"fork",    no_argument,       0, 'f'},
            {0, 0, 0, 0}
        };
        int option_index = 0;
        c = getopt_long (argc, argv, "vf", long_options, &option_index);
        if (c == -1) break;

        switch (c)
        {
            case 'f':
                use_fork = 1;
                break;
            case 'v':
                verbose = 1;
                break;
            default:
                exit(1);
        }
    }
}

struct rcInfo { // row and col info
    int row;
    int col;
};

struct sudoku {
    int grid[9][9];
    int isValid;
};

struct sudoku * readSudokuFile(){
    struct sudoku* sudoku_file = NULL;
    sudoku_file = (struct sudoku*)malloc(sizeof(struct sudoku));
    sudoku_file->isValid = 1;
    int cur;
    for(int i = 0; i < 9; i++){
        for(int j = 0; j < 9; j++){
                cur = fgetc(stdin);
                while((cur == ' ') || (cur == '\n') || (cur == '\r')){
                    cur = fgetc(stdin);  
                }
                sudoku_file->grid[i][j] = cur - '0';
        }
    }
    return sudoku_file;
}

void printPuzzle(struct sudoku* p) {
    for(int i = 0; i < 9; i++)
    {
        for(int j = 0; j < 9; j++)
        {
            printf("%d ", p->grid[i][j]);
        }
        printf("\n");
    }
}

void validateCol(){

}

void validateRow(){

}

void validateGrid(){

}

int main(int argc, char *argv[])
{
    parse_args(argc, argv);

    if (verbose && use_fork) {
        printf("We are forking child processes as workers.\n");
    } else if (verbose) {
        printf("We are using worker threads.\n");
    }

    struct sudoku* sdk = readSudokuFile();
    // printPuzzle(sdk);

    pthread_t threads[27];
    int threadsIdx = 0;

    for(int i = 0; i < 9; i++){
        for(int j = 0; j < 9; j++){
            if(i == 0){ //col
                struct rcInfo* col_info = (struct rcInfo*)malloc(sizeof(struct rcInfo));
                col_info->row = i;
                col_info->col = j;
                pthread_create(&threads[threadsIdx++], NULL, validateCol, col_info);
            }
            if(j == 0){ //row
                struct rcInfo* row_info = (struct rcInfo*)malloc(sizeof(struct rcInfo));
                row_info->row = i;
                row_info->col = j;
                pthread_create(&threads[threadsIdx++], NULL, validateRow, row_info);
            }
            if(i%3 == 0 && j%3 == 0){ //3x3 grid
                struct rcInfo* grid_info = (struct rcInfo*)malloc(sizeof(struct rcInfo));
                grid_info->row = i;
                grid_info->col = j;
                pthread_create(&threads[threadsIdx++], NULL, validateGrid, grid_info);
            }
        }
    }

    for(int i = 0; i < 27; i++){
        pthread_join(threads[i],NULL);
    }

    if(sdk->isValid == 1)
        printf("The input is a valid Sudoku.\n");
    else
        printf("The input is not a valid Sudoku.\n");
    
    free(sdk);
    
    return 0;
}
