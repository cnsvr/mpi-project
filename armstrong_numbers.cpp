#include <stdio.h>
#include <algorithm>
#include <mpi.h>
#include <random>
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <string.h>

#define MAX_SIZE 1000000 

using namespace std;
/*
MPI_Send(void* message, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm, communicator);
MPI_Recv(void* data, int count, MPI_Datatype datatype, int from, int tag, MPI_Comm comm, MPI_Status* status);
*/

void generateAndShuffleArray(int a[],int len){
  int i;
  unsigned seed = 0;
  for (i = 0; i < len; i++)
  {
    a[i] = i + 1;
  }
  std::random_device rd;
  std::mt19937 g(rd());
  // shuffle array
  std::shuffle(a,a+len,g);
}

bool checkArmstrongNumber (int num) {
  int originalNum;
  int remainder;
  int n = 0;
  int result = 0;
  int power;

  originalNum = num;

  while (originalNum != 0) {
    originalNum /= 10;
    ++n;
  }
  originalNum = num;
  while (originalNum != 0) {
    remainder = originalNum % 10;

    // pow() returns a double value
    // round() returns the equivalent int
    power = round(pow(remainder, n));
    result += power;
    originalNum /= 10;
  }

  if (result == num)
    return true;
   else
    return false;
}

int findArmstrongNumbers(int a[],int index,int subsize) {
  int i;
  int sum = 0;
  for (i = index; i < index+subsize; i++) 
  { 
    if (!checkArmstrongNumber(a[i])) {
      a[i]= -1;
    } else {
      sum += a[i];
    }
  }

  return sum;
  
}




int main(int argc, char** argv){
    int my_rank;
    int rank;
    int num_proc;
    int arraymsg = 1;    /* setting a message type */
   	int indexmsg = 2; 
    int numworkers;
    int array_len;
    int index;
    int quotient;
    int prev_sum;
    int source;
    int i;
    int rem;
    int rc;
    int chunksize;
    int sub_start;
    int sub_len;
    int sub_size;
    int count;
    int array[MAX_SIZE];
    int temp_array[MAX_SIZE];
    int armstrong_numbers[MAX_SIZE];
    int local_sum;
    int all_sum;
    MPI_Status   status;
    vector<int>result;
    ofstream myfile;
     MPI_File fh;

  /*
    generateAndShuffleArray(array, sizeof(array));

    for (size_t i = 0; i < 10; i++)
    {
      printf("%d\n", array[i]);
    }
    */
    rc = MPI_Init(&argc,&argv);
     
   /*--------------------------------------------*/
  /*  Determine size of the global communicator */
 /*--------------------------------------------*/
   rc|= MPI_Comm_size(MPI_COMM_WORLD,&num_proc);


   /*--------------------------------------------*/
  /*  Determine rank in the global communicator */
 /*--------------------------------------------*/
   rc|= MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);

   if (rc != 0)
    printf ("error initializing MPI and obtaining task ID information\n");

   numworkers = num_proc-1;
   array_len = atoi(argv[1]);
   chunksize = (array_len / numworkers);

    /* MASTER PROCESS */
    if (my_rank == 0) {
        
      numworkers = num_proc - 1;

      generateAndShuffleArray(array,array_len);

      index = 0;
      // Send data to processors

      for (rank = 1; rank <= numworkers; rank++)
      {
        // printf("Sending to worker task %d\n",rank);
        fflush(stdout);
        
       MPI_Send(&index, 1, MPI_INT, rank, indexmsg, MPI_COMM_WORLD);

    /*-----------------------------------------------------------*/
   /* Send each process a chunksize bit of data starting at the */
  /* index position.                                           */
 /*-----------------------------------------------------------*/

         MPI_Send(&array[index], chunksize, MPI_INT, rank, arraymsg,
                  MPI_COMM_WORLD);
         index = index + chunksize;
      }

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

         MPI_Recv(&armstrong_numbers[index], chunksize, MPI_INT, source, arraymsg,
                  MPI_COMM_WORLD, &status);

      
      

      }
      
      MPI_Recv(&all_sum,1,MPI_INT,numworkers,2, MPI_COMM_WORLD,&status);

      printf("MASTER: Sum of all Armstrong numbers = %d\n",all_sum);

      std::sort(armstrong_numbers,armstrong_numbers+array_len);
      /* 
    MPI_File_open(MPI_COMM_SELF, "armstrong.txt", MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &fh);      // MPI_File_write( fh,&armstrong_numbers, array_len , MPI_INT, &status );
    MPI_File_set_view(fh, 0, MPI_INT, MPI_INT, "native", MPI_INFO_NULL);
    MPI_File_write( fh,&armstrong_numbers, array_len, MPI_INT, &status );
    */

    /* 
      for (i = 0; i < array_len; i++){
        if(armstrong_numbers[i] != -1) {
          MPI_File_set_view(fh, 0, MPI_INT, MPI_INT, "native", MPI_INFO_NULL);
          MPI_File_write( fh,&armstrong_numbers[i], 1, MPI_INT, &status );
        }
      }
      */

      MPI_File_open(MPI_COMM_SELF, "armstrong.txt",MPI_MODE_CREATE | MPI_MODE_WRONLY,MPI_INFO_NULL,&fh);
      for (int i=0; i < array_len; i++){
            char buf[42];
            //fprintf(f,"%d \n",i);
            if(armstrong_numbers[i] != -1) {
              snprintf(buf,42,"%d  ",armstrong_numbers[i]);
              MPI_File_write(fh,buf,strlen(buf), MPI_CHAR,&status);
            }
        }
        //        fclose(f);
        MPI_File_close(&fh);
         

    }
    if (my_rank > 0)
    {

        /* WORKER PROCESSORS */
      
      source = 0;
      // Recieve subarray from master and find armstrong_numbers to be sent to MASTER.

     MPI_Recv(&index, 1, MPI_INT, source, indexmsg, MPI_COMM_WORLD,
               &status);

   /*-------------------------------------------------*/
  /* Receive chunksize bit of data starting at index */
 /*-------------------------------------------------*/

      MPI_Recv(&armstrong_numbers[index], chunksize, MPI_INT, source, arraymsg,
               MPI_COMM_WORLD, &status);


      // Find armstrong_numbers
      local_sum = findArmstrongNumbers(armstrong_numbers,index,chunksize);

      printf("Sum of Armstrong numbers in Process %d = %d\n",my_rank,local_sum);

      

      // Send armstrong_numbers to MASTER

      MPI_Send(&index, 1, MPI_INT, 0, indexmsg, MPI_COMM_WORLD);

   /*-----------------------------------------------*/
  /* Send chunksize bit of results back to master  */
 /*-----------------------------------------------*/

      MPI_Send(&armstrong_numbers[index], chunksize, MPI_FLOAT, 0, arraymsg,
               MPI_COMM_WORLD);


      // Send local_sum to next process 
      if(my_rank == 1) {
        MPI_Send(&local_sum,1,MPI_INT,my_rank+1,2,MPI_COMM_WORLD);
      } else {
        // Firstly, recieve from previous processors and add local_sum and send to next_processor.
        if (my_rank != numworkers){
          MPI_Recv(&prev_sum,1,MPI_INT,my_rank-1,2,MPI_COMM_WORLD,&status);
          int sum = local_sum + prev_sum;
          MPI_Send(&sum,1,MPI_INT,my_rank+1,2,MPI_COMM_WORLD);
        } else {
          // This is last worker process
          MPI_Recv(&prev_sum,1,MPI_INT,my_rank-1,2,MPI_COMM_WORLD,&status);
          int sum = local_sum + prev_sum;
          MPI_Send(&sum,1,MPI_INT,0,2,MPI_COMM_WORLD);
        }

      }

    }

    MPI_File_close( &fh );
    MPI_Finalize();


    return 0;
}
