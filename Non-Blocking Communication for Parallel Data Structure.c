#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
/* This example handles a 12 x 12 mesh, on 4 MPI processes only.
*/
#define maxn 12
int main(int argc, char** argv)
{
    int rank, value, size, errcnt, toterr, i, j;
    MPI_Status status[4];
    MPI_Request req[4];
    int req_counter =0;
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
        
    //req_counter is used as request label such that it is dynamically labelled. Each request(Isend/Irecv) will add to req_counter
        
    // Send the last row of the middle rows to upper rank process
    // Receive a row from the lower rank process and this is the ghostpoints in the first row

    if (rank < size - 1)
        MPI_Isend(xlocal[maxn/size], maxn, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, &req[req_counter++]);
        
        
    if (rank > 0)
        MPI_Irecv(xlocal[0], maxn, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, &req[req_counter++]);
        
        
    // Send the first row of the middle rows to lower rank process
    // Receive a row from the upper rank process and this is the ghostpoints in the last row

    if (rank > 0)
        MPI_Isend(xlocal[1], maxn, MPI_DOUBLE, rank - 1, 1, MPI_COMM_WORLD, &req[req_counter++]);
        
        
    if (rank < size - 1)
        MPI_Irecv(xlocal[maxn/size+1], maxn, MPI_DOUBLE, rank + 1, 1, MPI_COMM_WORLD, &req[req_counter++]);
    
    //waits for all requests to be completed. number of requests is req_counter    
    MPI_Waitall(req_counter, req, status);
    //resets request counter to 0, all requests has been completed
    req_counter = 0;

    /* Check that we have the correct results */
    errcnt = 0;
    for (i=1; i<=maxn/size; i++)    
        for (j=0; j<maxn; j++)
            if (xlocal[i][j] != rank) 
                errcnt++;

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
