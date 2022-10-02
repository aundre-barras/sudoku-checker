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

struct sudoku {
    int grid[9][9];
    int rowID;
    int colID;
};

struct sudoku * readSudokuFile(){
    struct sudoku* sudoku_file = NULL;
    sudoku_file = (struct sudoku*)malloc(sizeof(struct sudoku));
    int cur;
    for(int i = 0; i < 9; i++){
        for(int j = 0; j < 9; j++){
                cur = fgetc(stdin);
                while((cur == ' ') || (cur == '\n') || (cur == '\r') || (cur == '\t')){
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

static void *validateCol(void* sdk){
    // struct sudoku *scol = (struct sudoku*)sdk;
    struct sudoku scol = *((struct sudoku*)sdk);
    int flag = 0x0000;
    for(int i = 0; i < 9; i++){
        flag|=1<<(scol.grid[i][scol.colID]-1);
    }
    if(flag!=0x01FF){
        printf("Column %d doesn't have the required values.\n", scol.colID + 1);
        return 1;
    }
    return 0;
    // printf("Tested: col %d.\n", scol->colID + 1);
}

static void *validateRow(void* sdk){
    // struct sudoku *srow = (struct sudoku*)sdk;
    struct sudoku srow = *((struct sudoku*)sdk);
    int flag = 0x0000;
    for(int j = 0; j < 9; j++){
        flag|=1<<(srow.grid[srow.rowID][j]-1);
    }
    if(flag!=0x01FF){
        printf("Row %d doesn't have the required values.\n", srow.rowID + 1);
        return 1;
    }
    return 0;
    // printf("Tested: row %d.\n", srow->rowID + 1);
}

static void *validateSubgrid(void* sdk){
    // struct sudoku *ssg = (struct sudoku*)sdk;
    struct sudoku ssg = *((struct sudoku*)sdk);
    int flag = 0x0000;

    for(int i = ssg.rowID; i < ssg.rowID + 3; i++){
        for(int j = ssg.colID; j < ssg.colID + 3; j++){
            flag|=1<<(ssg.grid[i][j]-1);
        }
    }

    if(flag!=0x01FF){
        if(ssg.rowID == 0 && ssg.colID == 0){
            printf("The top left subgrid doesn't have the required values.\n");}
        else if(ssg.rowID == 0 && ssg.colID == 3){
            printf("The top center subgrid doesn't have the required values.\n");}
        else if(ssg.rowID == 0 && ssg.colID == 6){
            printf("The top right subgrid doesn't have the required values.\n");} 
        else if(ssg.rowID == 3 && ssg.colID == 0){
            printf("The middle left subgrid doesn't have the required values.\n");}
        else if(ssg.rowID == 3 && ssg.colID == 3){
            printf("The middle center subgrid doesn't have the required values.\n");}
        else if(ssg.rowID == 3 && ssg.colID == 6){
            printf("The middle right subgrid doesn't have the required values.\n");}
        else if(ssg.rowID == 6 && ssg.colID == 0){
            printf("The bottom left subgrid doesn't have the required values.\n");}
        else if(ssg.rowID == 6 && ssg.colID == 3){
            printf("The bottom middle subgrid doesn't have the required values.\n");}
        else if(ssg.rowID == 6 && ssg.colID == 6){
            printf("The bottom right subgrid doesn't have the required values.\n");}   
        return 1;
        }   
    return 0;
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
    void *status;
    int isValid = 1;

    for(int i = 0; i < 9; i++){
        struct sudoku* sdkc = (malloc(sizeof(struct sudoku)));
        *sdkc = *sdk;
        sdkc->colID = i;
        sdkc->rowID = 0;
        pthread_create(&threads[i], NULL, validateCol, sdkc);
        // printf("creating thread %d\n", i);
    }
    for(int i = 0; i < 9; i++){
        struct sudoku* sdkr = (malloc(sizeof(struct sudoku)));
        *sdkr = *sdk;
        sdkr->rowID = i;
        sdkr->colID = 0;
        pthread_create(&threads[i + 9], NULL, validateRow, sdkr);
        // printf("creating thread %d\n", i+9);

    }
    int sgCounter = 18;
    for(int i = 0; i < 9; i++){
        for(int j = 0; j < 9; j++){
            if(i%3 == 0 && j%3 == 0){
                struct sudoku* sdksg = (malloc(sizeof(struct sudoku)));
                *sdksg = *sdk;
                sdksg->colID = i;
                sdksg->rowID = j;
                pthread_create(&threads[sgCounter], NULL, validateSubgrid, (void *) sdksg);
                // printf("creating thread %d using %d and %d\n", sgCounter, i, j);
                sgCounter++;
            }
        }
    }
    
    for(int i = 0; i < 27; i++){
        pthread_join(threads[i], &status);
        if(status == 1){
            isValid = 0;
        }
    }

    int retVal = 0;
    if(isValid == 1)
        printf("The input is a valid Sudoku.\n");
    else{
        printf("The input is not a valid Sudoku.\n");
        retVal = 1;
    }
    free(sdk);
    
    return retVal;
}
