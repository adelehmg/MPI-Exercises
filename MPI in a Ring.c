#include <stdio.h>
#include <mpi.h>

int main(int argc, char **argv){
    int rank, s_value, r_value, size, i;
    MPI_Status status;
    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    do {if (rank == 0) {
        printf("Enter a round number: ");
        fflush(stdout);
        scanf( "%d", &s_value );
        
        // user input, s_value is set to r_value, the receive value
        r_value = s_value;
        //Sends the value, count = 1 as only sending 1 integer at a time
        //type is MPI_INT as only round numbers are input
        //send value to i+1 process; the next process in the cycle
        MPI_Send(&r_value, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
        
        printf( "Process %d got %d from Process %d\n", rank, r_value, size -1);
        fflush(stdout);
        }else {
        fflush(stdout);
        
        //receives value from i-1, the process ranked before it
        MPI_Recv(&r_value, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, &status);
        
        //if reaches last process
        //continues the loop, past the last process
        if (rank < size - 1)
               {MPI_Send(&r_value, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);}
        //next print statement not on same line as user prompt print statement
        printf("\n");
        printf( "Process %d got %d from Process %d\n", rank, r_value, rank -1);
        fflush(stdout);}} 
    while (r_value >= 0);
    MPI_Finalize( );
    return 0;}

