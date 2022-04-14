#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#define EXIT 1
#define ORDERED 2 
#define UNORDERED 3

int master_io(MPI_Comm master_comm, MPI_Comm comm);
int slave_io(MPI_Comm master_comm, MPI_Comm comm);
int main(int argc, char **argv)

{
    int rank;
     MPI_Comm new_comm;
     MPI_Init(&argc, &argv);
     MPI_Comm_rank(MPI_COMM_WORLD, &rank);
     MPI_Comm_split( MPI_COMM_WORLD,rank == 0, 0, &new_comm);
     if (rank == 0)
        master_io( MPI_COMM_WORLD, new_comm );
     else
        slave_io( MPI_COMM_WORLD, new_comm );
     MPI_Finalize();
     return 0;
}


/* This is the master */
int master_io(MPI_Comm master_comm, MPI_Comm comm)
{
     int i,j, size, n, msg;
     char buf[256], buf2[256];
     MPI_Status status;
     MPI_Comm_size( master_comm, &size );
     /*number of slaves is number of processes without master*/
     n = size - 1;
     /*while there are still slaves*/
     while (n>0) {
     /*receive without restrictions on rank or message tag*/
        MPI_Recv(buf, 256, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, master_comm, &status); 
     /*cases based on the tag*/
     switch (status.MPI_TAG) {
     /*slave exit case, total number of slaves decreases by 1*/
         case EXIT: 
            n--; 
            break;
     /*slaves print message directly, output is regardless of slave rank*/
         case UNORDERED:
            fputs(buf, stdout);
            break;
     /*send msgs according to rank by checking rank of process that sent the message.
     if current loop matches rank, msg is printed otherwise, the correct msg is found*/
         case ORDERED: 
            msg = status.MPI_SOURCE;
                for (i=1; i<size; i++) {
                    if (i == msg)
                        fputs(buf, stdout);
                    else {
                        MPI_Recv( buf2, 256, MPI_CHAR, i, ORDERED, master_comm, &status);
                        fputs(buf2, stdout);}
                    }
             break;
    }
    }
    return 0;
}


/* This is the slave */
int slave_io(MPI_Comm master_comm, MPI_Comm comm)
{
     char buf[256];
     int rank;

     MPI_Comm_rank(comm, &rank);
     sprintf(buf, "Hello from slave %d\n", rank);
     MPI_Send( buf, strlen(buf) + 1, MPI_CHAR, 0, ORDERED, master_comm );

     sprintf( buf, "Goodbye from slave %d\n", rank );
     MPI_Send( buf, strlen(buf) + 1, MPI_CHAR, 0, ORDERED, master_comm );
     
     sprintf( buf, "I'm exiting (%d)\n", rank );
     MPI_Send( buf, strlen(buf) + 1, MPI_CHAR, 0, UNORDERED, master_comm );
     
     MPI_Send( buf, 0, MPI_CHAR, 0, EXIT, master_comm );
     return 0;
}
