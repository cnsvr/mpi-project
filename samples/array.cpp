#include "mpi.h"
#include <stdio.h>


#define	ARRAYSIZE	60000
#define MASTER		0         /* taskid of first process */

int main(int argc, char** argv)
{
int	numtasks,           /* total number of MPI tasks in partition */
	numworkers,         /* number of worker tasks */
	taskid,	            /* task identifier */
	rc,                 /* return error code */
	dest,               /* destination task id to send message */
	index,              /* index into the array */
	i,                  /* loop variable */
   	arraymsg = 1,       /* setting a message type */
   	indexmsg = 2,       /* setting a message type */
	source,             /* origin task id of message */
	chunksize;          /* for partitioning the array */
float	data[ARRAYSIZE],    /* the intial array */
	result[ARRAYSIZE];  /* for holding results of array operations */
MPI_Status status;

/*------------------------ initializations -------------------------------
  Find out how many tasks are in this partition and what my task id is.
  Then define the number of worker tasks and the array partition size as
  chunksize. 
  Note:  For this example, the MP_PROCS environment variable should be set
  to an odd number...to insure even distribution of the array to numtasks-1
  worker tasks.
-------------------------------------------------------------------------*/

   /*-----------------*/
  /*  Initialize MPI */
 /*-----------------*/

   rc = MPI_Init(&argc,&argv);
     
   /*--------------------------------------------*/
  /*  Determine size of the global communicator */
 /*--------------------------------------------*/
   rc|= MPI_Comm_size(MPI_COMM_WORLD,&numtasks);


   /*--------------------------------------------*/
  /*  Determine rank in the global communicator */
 /*--------------------------------------------*/
   rc|= MPI_Comm_rank(MPI_COMM_WORLD,&taskid);

   if (rc != 0)
    printf ("error initializing MPI and obtaining task ID information\n");

   
   numworkers = numtasks-1;
   chunksize = (ARRAYSIZE / numworkers);
   printf("chunksize %d\n",chunksize);

/**************************** master task ********************************/

   if (taskid == MASTER)
   {
      printf("MASTER: number of worker tasks will be= %d\n",numworkers);
      fflush(stdout);

   /*----------------------*/
  /* Initialize the array */
 /*----------------------*/

      for(i=0; i<ARRAYSIZE; i++) 
         data[i] =  0.0;
      index = 0;

   /*------------------------------------------------*/
  /* Send each worker task its portion of the array */
 /*------------------------------------------------*/

      for (dest=1; dest<= numworkers; dest++)
      {
         printf("Sending to worker task %d\n",dest);
         fflush(stdout);

    /*--------------------------------------------------------------*/
   /* Send index value so that each processor knows where to start */
  /* in the data array.                                           */
 /*--------------------------------------------------------------*/

         MPI_Send(&index, 1, MPI_INT, dest, indexmsg, MPI_COMM_WORLD);

    /*-----------------------------------------------------------*/
   /* Send each process a chunksize bit of data starting at the */
  /* index position.                                           */
 /*-----------------------------------------------------------*/

         MPI_Send(&data[index], chunksize, MPI_FLOAT, dest, arraymsg,
                  MPI_COMM_WORLD);
         index = index + chunksize;
      }

    /*------------------------------------------------------------*/
   /* Wait to receive back the results from each worker task and */
  /* print a few sample values.                                 */
 /*------------------------------------------------------------*/

      for (i=1; i<= numworkers; i++)
      {
         source = i;

    /*-----------------------------------------------------------*/
   /* Receive index value so that master knows which portion of */
  /* the results array the following data will be stored in.   */
 /*-----------------------------------------------------------*/

         MPI_Recv(&index, 1, MPI_INT, source, indexmsg, MPI_COMM_WORLD,
                  &status);

   /*----------------------------------------*/
  /* Receive chunksize of the results array */
 /*----------------------------------------*/

         MPI_Recv(&result[index], chunksize, MPI_FLOAT, source, arraymsg,
                  MPI_COMM_WORLD, &status);

   /*---------------------------------*/
  /* Print some sample result values */
 /*---------------------------------*/

        printf("---------------------------------------------------\n");
        printf("MASTER: Sample results from worker task = %d\n",source);
        printf("   result[%d]=%f\n", index, result[index]);
        printf("   result[%d]=%f\n", index+1, result[index+1]);
        printf("   result[%d]=%f\n\n", index+2, result[index+2]);
        fflush(stdout);
      }
      printf("MASTER: All Done! \n");
   }

/**************************** worker task ********************************/

   if (taskid > MASTER)
   {

      source = MASTER;

    /*------------------------------------------------------------------*/
   /* Receive index value from the master indicating start position in */
  /* data array.                                                      */
 /*------------------------------------------------------------------*/
      printf("index %d\n", index);
      MPI_Recv(&index, 1, MPI_INT, source, indexmsg, MPI_COMM_WORLD,
               &status);

   /*-------------------------------------------------*/
  /* Receive chunksize bit of data starting at index */
 /*-------------------------------------------------*/

      MPI_Recv(&result[index], chunksize, MPI_FLOAT, source, arraymsg,
               MPI_COMM_WORLD, &status);

   /*-----------------------------------------------------------*/
  /* Do a simple value assignment to each of my array elements */
 /*-----------------------------------------------------------*/

      for(i=index; i < index + chunksize; i++)
         result[i] = i;

      dest = MASTER;

    /*------------------------------------------------------------------*/
   /* Send index value so that master knows which portion of data each */
  /* process was working on.                                          */
 /*------------------------------------------------------------------*/

      MPI_Send(&index, 1, MPI_INT, dest, indexmsg, MPI_COMM_WORLD);

   /*-----------------------------------------------*/
  /* Send chunksize bit of results back to master  */
 /*-----------------------------------------------*/

      MPI_Send(&result[index], chunksize, MPI_FLOAT, MASTER, arraymsg,
               MPI_COMM_WORLD);
   }

   /*--------------*/
  /* Finalize MPI */
 /*--------------*/

   MPI_Finalize();
}
