#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
/* This example handles a 12 x 12 mesh, on 4 MPI processes only.
*/
#define maxn 12
int main(int argc, char** argv)
{
    int rank, value, size, errcnt, toterr, i, j;
    int up,down;
    MPI_Status status;
    double xlocal[(12/4)+2][12];
    char* pOutputFileName = (char*) malloc(20 * sizeof(char));
    FILE *pFile;
    
    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    if(size != 4) MPI_Abort(MPI_COMM_WORLD, 1);
    // Fill the data as specified
    // Fill in the process's section (middle rows) with its own rank

    for (i=1; i<=maxn/size; i++)
        for (j=0; j<maxn; j++)
            xlocal[i][j] = rank;
    // Fill in the ghost points with -1, ghost points are in the first row and last row

    for (j=0; j<maxn; j++) {
        xlocal[0][j] = -1;
        xlocal[maxn/size+1][j] = -1;
    }
    //MPI_PROC_NULL is used so that there won't be an invalid rank error. Hence when rank + 1 is more than or equal to size or when rank -1 is less than 0.
    up = rank+1;
    down = rank -1;
    if (up >= size)
        up = MPI_PROC_NULL;
    if (down < 0)
        down = MPI_PROC_NULL;
        
    // Send the last row of the middle rows to upper rank process
    // Receive a row from the lower rank process and this is the ghostpoints in the first row
    // MPI_Sendrecv call moves data up
        MPI_Sendrecv(xlocal[maxn/size], maxn, MPI_DOUBLE, up, 0, xlocal[0], maxn, MPI_DOUBLE, down, 0, MPI_COMM_WORLD, &status);
        
    // Send the first row of the middle rows to lower rank process
    // Receive a row from the upper tank process and this is the ghostpoints in the last row
    // MPI_Sendrecv call moves data down
        MPI_Sendrecv(xlocal[1], maxn, MPI_DOUBLE, down, 0, xlocal[maxn/size+1], maxn, MPI_DOUBLE, up, 0, MPI_COMM_WORLD, &status);
        
   

    /* Check that we have the correct results */
    errcnt = 0;
    for (i=1; i<=maxn/size; i++)    
        for (j=0; j<maxn; j++)
            if (xlocal[i][j] != rank) errcnt++;

    for (j=0; j<maxn; j++) {
        if (xlocal[0][j] != rank - 1)
            errcnt++;
        if (rank < size-1 && xlocal[maxn/size+1][j] != rank + 1)
            errcnt++;
    }
    MPI_Reduce( &errcnt, &toterr, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD );
    
    // Write xlocal of each process into a unique text file, the rank is appended into the text file name

    snprintf(pOutputFileName, 20, "process_%d.txt", rank);
    pFile = fopen(pOutputFileName, "w");
    for (i = 0; i <= maxn/size+1; i++){
        for (j=0; j<maxn; j++){
            fprintf(pFile, "%.0f\t", xlocal[i][j]);
    }
    fprintf(pFile, "\n");
    }
    fclose(pFile);
    if (rank == 0) {
        if (toterr) printf( "! found %d errors\n", toterr );
    else
        printf( "No errors\n" );
    }
    free(pOutputFileName);
    MPI_Finalize( );
    return 0;
}
