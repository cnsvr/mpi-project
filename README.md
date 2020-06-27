# MPI Project

## This is a project about MPI(Message Passing Interface) Parallel Programming.

# Introduction

In this project, you are going to experience parallel programming with C/C++/Python using MPI library. You will implement a parallel algorithm for finding Armstrong numbers.

An Armstrong number is a number that is equal to the sum of its own digits each raised to the power of the number of digits. For example 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 153, 370, 371, 407, 1634, 8208, 9474, 54748, 92727, 93084, 548834, ... are Armstrong numbers.

# Explanation

In this project, our aim is to implement finding Armstrong numbers using MPI. This way, we will be able to divide the task of finding Armstrong numbers, into several processes, and observe and practice parallel programming.

Write a program for n+1 processors in the master-worker programming style (1 master, n workers).

The master process will create an array of numbers, from 1 to A. You will divide the array and send A/n (random) numbers to each worker, which will check for Armstrong numbers in their partition. When they have finished processing, they will send their results (the Armstrong numbers in their partition) to the master process, which will sort and print these results (the Armstrong numbers between 1 and A) into armstrong.txt.

Suppose A = 20, and we work with 5 processors (n=4):

* **Master Process** : 
  * Will create an array of numbers from 1 to 20 array = {1,2,3,4,5,.......19,20} array should be divided into n=4 parts randomly (all 4 partitions should have 5 random numbers. 
  * When the partitions are combined, all numbers between 1..A should be covered)

* **Worker Process 1** : 
  * Will receive the first partition of the array (with A/n = 20/4 = 5 numbers)array1 = {5, 12, 7, 1, 6} 
  * Will find the Armstrong numbers in its partition 
  * Will send the found Armstrong numbers in its partition to Master process

* **Worker Process 2**: 
  * Will receive the second partition of the array (with A/n = 20/4 = 5 numbers) array2 = {3, 18, 19, 2, 10}
  * Will find the Armstrong numbers in its partition
  * Will send the found Armstrong numbers in its partition to Master process
  
 * **Worker Process 3**: 
  * array3 = {16, 8, 11, 20, 14} 
  * ...
* **Worker Process 4** : 
  * array4 = {15, 9, 13, 4, 17} 
  * ...
  
After completion of finding the Armstrong numbers in each partition, each pro- cess should calculate the sum of their Armstrong numbers, and print the results in the console, as in the example below:
  * Sum of Armstrong numbers in Process 1 = ...
  * Sum of Armstrong numbers in Process 2 = ... 
  * Sum of Armstrong numbers in Process 3 = ... 
  * Sum of Armstrong numbers in Process 4 = ...

After calculation of the sums by each process, each process should send their sum to the next process (1 to 2, 2 to 3, and so on), each of which will add the received sum to their own sum, resulting in a collective sum of Armstrong numbers. The last worker (Process 4 in this case) should send the final sum to master, which will print the result:

  * MASTER: Sum of all Armstrong numbers = ...
  
  
## HOW TO RUN

Firstly install openmpi to your computer

    https://www.open-mpi.org/software/ompi/v2.0/

Then, build the .cpp file with mpic++

    mpic++ armstrong_numbers.cpp -o output_name_of_program
    
Then, create a hostfile in current directory,
 
    echo “our_localhost_name slots=number_of_processors” >> hostfile

Then, run the program with hostfile
  
    mpirun -np number_of_processors --hostfile hostfile program_of_name args


  
