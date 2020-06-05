/*
Student Name: Furkan Cansever
Student Number: 2016400349
Compile Status: Compiling
Program Status: Working
*/

#include <stdio.h>
#include <algorithm>
#include <mpi.h>
#include <random>
#include <math.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>

#define MAX_SIZE 1000000

using namespace std;

// It generates an array contains numbers from 1 to len and shuffles array.
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

// Returns true if given num is Armstrong number, otherwise false.
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

/*
  Finds Armstrong numbers and returns sum of these numbers.
  If element of array is not Armstrong number, changes value as -1.
*/
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
  int my_rank;                           // identifier of processors
  int num_proc;                          // total number of processors
  int arraymsg = 1;                      // setting a message type for array.
  int indexmsg = 2;                      // setting a message type for index.
  int summsg = 3;                        // setting a message type for sum.
  int numworkers;                        // number of worker processors  
  int array_len;                         // array length of array.
  int index;                             // index into the array.
  int prev_sum;                          // local sum of previous processors.
  int source;                            // origin task id of processors
  int dest;                              // destination task id to send message
  int i;                                 // loop variable
  int rc;                                // return error code
  int chunksize;                         // for partitioning the array
  int array[MAX_SIZE];                   // initial array
  int armstrong_numbers[MAX_SIZE];       // armstrong_numbers array
  int local_sum;                         // sum of armstrong_numbers of each processor
  int all_sum;                           // sum of all numbers in armstrong_numbers array.
  MPI_File fh;                           // MPI File object to write armstrong numbers.
  MPI_Status status;
  
  // Initialize the MPI environment.
  rc = MPI_Init(&argc,&argv);
     
  // Determine size of the global communicator 
   rc|= MPI_Comm_size(MPI_COMM_WORLD,&num_proc);


  // Determine rank in the global communicator 
   rc|= MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);

  if (rc != 0)
    printf ("error initializing MPI and obtaining task ID information\n");

  numworkers = num_proc-1;
  array_len = atoi(argv[1]);
  chunksize = (array_len / numworkers);

  /* MASTER PROCESS */
  if (my_rank == 0) {
      
    double start = MPI_Wtime();

    // Generate and shuffle array.
    generateAndShuffleArray(array,array_len);

    index = 0;
    // Send each worker task its portion of the array.
    for (dest = 1; dest <= numworkers; dest++)
    {
  
      fflush(stdout);
                
      // Send index value so that each processor knows where to start in the data array.
      MPI_Send(&index, 1, MPI_INT, dest, indexmsg, MPI_COMM_WORLD);

      // Send each process a chunksize bit of data starting at the  index position.
      MPI_Send(&array[index], chunksize, MPI_INT, dest, arraymsg, MPI_COMM_WORLD);
        index = index + chunksize;
    }

    for (i=1; i<= numworkers; i++)
    {
      source = i;

      /*
      Receive index value so that master knows which portion of 
      the armstrong array the following data will be stored in. 
      */
      MPI_Recv(&index, 1, MPI_INT, source, indexmsg, MPI_COMM_WORLD,
              &status);


      // Receive chunksize of the results array

      MPI_Recv(&armstrong_numbers[index], chunksize, MPI_INT, source, arraymsg,
              MPI_COMM_WORLD, &status);

    }
      
      // Receive sum of all armstrong_numbers from last numworkers processor.

      MPI_Recv(&all_sum,1,MPI_INT,numworkers,summsg, MPI_COMM_WORLD,&status);

      printf("MASTER: Sum of all Armstrong numbers = %d\n",all_sum);
          
      // Sort armstrong_numbers array.

      std::sort(armstrong_numbers,armstrong_numbers+array_len);
            
            
      
      // Remove armstrong.txt file if it exists.
      remove( "armstrong.txt" );

      
      // Writes armstrong numbers to armstrong.txt file.
      MPI_File_open(MPI_COMM_SELF, "armstrong.txt",MPI_MODE_CREATE | MPI_MODE_WRONLY,MPI_INFO_NULL,&fh);
      
      for (int i=0; i < array_len; i++)
      {
        char buf[42];
        if(armstrong_numbers[i] != -1)
        {
          snprintf(buf,42,"%d  ",armstrong_numbers[i]);
          MPI_File_write(fh,buf,strlen(buf), MPI_CHAR,&status);
        }
      }
      MPI_File_close(&fh);

      double end = MPI_Wtime();

      std::cout << "The process MASTER took " << end - start << " seconds to run." << std::endl;
         
    }
    
    if (my_rank > 0)
    {

      /* WORKER PROCESSORS */
      
      double start = MPI_Wtime();

      source = 0;
      // Receives index of subarray from master and find armstrong_numbers to be sent to MASTER.
      MPI_Recv(&index, 1, MPI_INT, source, indexmsg, MPI_COMM_WORLD,
               &status);
        
        
      // Receives subarray from master and find armstrong_numbers to be sent to MASTER.
      MPI_Recv(&armstrong_numbers[index], chunksize, MPI_INT, source, arraymsg,
               MPI_COMM_WORLD, &status);

      // Find armstrong_numbers and sum of armstrong number of current processor.
      local_sum = findArmstrongNumbers(armstrong_numbers,index,chunksize);

      printf("Sum of Armstrong numbers in Process %d = %d\n",my_rank,local_sum);

      printf("Number of tasks in Process %d = %d\n",my_rank,chunksize);

      // Send index of armstrong_numbers array to MASTER

      MPI_Send(&index, 1, MPI_INT, 0, indexmsg, MPI_COMM_WORLD);

      // Send armstrong_numbers array to MASTER
      MPI_Send(&armstrong_numbers[index], chunksize, MPI_FLOAT, 0, arraymsg,
               MPI_COMM_WORLD);

      // Send local_sum to next process 
      if(my_rank == 1) 
      {
        MPI_Send(&local_sum,1,MPI_INT,my_rank+1,summsg,MPI_COMM_WORLD);
      } else 
      {
        // Firstly, recieve from previous processors and add local_sum and send to next_processor.
        if (my_rank != numworkers)
        {
          MPI_Recv(&prev_sum,1,MPI_INT,my_rank-1,summsg,MPI_COMM_WORLD,&status);
          int sum = local_sum + prev_sum;
          MPI_Send(&sum,1,MPI_INT,my_rank+1,summsg,MPI_COMM_WORLD);
        } else 
        {
          // This is last worker process
          MPI_Recv(&prev_sum,1,MPI_INT,my_rank-1,summsg,MPI_COMM_WORLD,&status);
          int sum = local_sum + prev_sum;
          MPI_Send(&sum,1,MPI_INT,0,summsg,MPI_COMM_WORLD);
        }
      }

      double end = MPI_Wtime();
      std::cout << "The processor " <<my_rank << " took " << end - start << " seconds to run." << std::endl;
    
    }

    MPI_File_close( &fh );
    MPI_Finalize();

    return 0;
}
