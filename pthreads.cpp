#include <iostream>
#include <thread>
#include <stdlib.h>
#include <unistd.h>
#include <bits/stdc++.h>
#include <pthread.h>
#include <stdio.h>
#include <random>
#include <mutex>
#include <cstdlib>
#include <sys/types.h>
#include <sys/syscall.h>
#include <time.h>


// Eric Engman
// 408-551-07
// Lab 3

int **mat;
int M;
int count;
int N;
int error;
int totalNumChanges = 0;
int totalZerosChanged = 0;
int totalOnesChanged = 0;
int firstThread = 0;
bool check = false;
FILE * pFile;

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

using namespace std;

bool checkBinary(){

  int total = (N*N);
  int zeros = 0;
  int ones = 0;

  // count the number of zeros and ones
  for(int i = 0; i < N; i++){
    for(int j = 0; j < N; j++){
      if(mat[i][j] == 0){
        zeros++;
      }else{
        ones++;
      }
    }
  }
  if(zeros == total || ones == total){
    return true;
  }

  return false;

}

void* matrix(void *arg){

  int counterX = 0;
  int counterY = 0;

  pthread_mutex_lock(&mut);
  if(firstThread == 0){
    fread(&mat, sizeof(mat), 1, fopen("binary.bin", "rb+"));
    fclose(fopen("binary.bin", "rb+"));
    firstThread++;
  }
  pthread_mutex_unlock(&mut);

  // Simulation process for threads, each one works at same time,
  while(!check){

    // create random square element
    int x = rand() % N;
    int y = rand() % N;

    for(int i = x - 1; i <= x + 1; i++){
      for(int j = y - 1; j <= y + 1; j++){
        if((i >= 0 && i <= N - 1) && (j >= 0 && j <= N - 1)){
          if(!(i == x && j == y)){
              if(mat[i][j] == 0){
                counterX++;
              }else{
                counterY++;
              }
          }
        }
      } // second for loop
    } // first for loop

    // compare number of 0's and 1's surrounding X and Y
    // lock each change individually, so as to not stop other threads
    // from advancing
    if(counterX >= counterY){
      pthread_mutex_lock(&mut);
      totalNumChanges++;
      totalOnesChanged++;
      mat[x][y] = 0;
      pthread_mutex_unlock(&mut);
    }else if(counterY > counterX){
      pthread_mutex_lock(&mut);
      totalNumChanges++;
      totalZerosChanged++;
      mat[x][y] = 1;
      pthread_mutex_unlock(&mut);
    }

    // reset counters for 0 and 1
    counterX = 0;
    counterY = 0;

    // after all the threads made their changes, write back to file
    pthread_mutex_lock(&mut);
    fwrite(&mat, sizeof(mat), 1, fopen("binary.bin", "rb+"));
    check = checkBinary();
    pthread_mutex_unlock(&mut);

    if(check == true){
      fclose(fopen("binary.bin", "rb+"));
      cout << "Goodbye from thread: " << syscall(__NR_gettid) << endl;
      pthread_exit(NULL);
      exit(0);
    }

  }
  pthread_exit(NULL);

}

int main(){

  cout << "Hello, please specify the the number of elements (N)" << endl;
  cin >> N;
  cout << "Please specify the number of threads you would like" << endl;
  cin >> M;

  // handle negative number of elements
  while(N < 3 || M < 1){
    cout << "Error: You typed in too few elements and/or threads" << endl;
    cout << "Please specify the number of elements (N > 2)" << endl;
    cin >> N;
    cout << "Please specify the number of threads (M > 0)" << endl;
    cin >> M;
  }

  int initialZeros = 0;
  int initialOnes = 0;

  // create matrix
  mat = new int*[N];
  for(int j = 0; j < N; j++){
    mat[j] = new int[N];
  }

  srand(time(NULL));
  for(int i = 0; i < N; i++){
    for(int j = 0; j < N; j++){
      mat[i][j] = rand() % 2;
      if(mat[i][j] == 0){
        initialZeros++;
      }else{
        initialOnes++;
      }
    }
  }

  pFile = fopen("binary.bin", "wb");
  cout << "File opened" << endl;
  if(pFile == NULL)
    exit(1);
  fwrite(&mat, sizeof(mat), 1, pFile);

  fclose(pFile);

  cout << endl;

  cout <<  "File contents before (randomly generated): " << endl;

  pFile = fopen("binary.bin", "rb");

  fread(&mat, sizeof(mat), 1, pFile);

  fclose(pFile);
  for(int i = 0; i < N; i++){
    for(int j = 0; j < N; j++){
      cout << mat[i][j] << " ";
    }
    cout << endl;
  }
  cout << endl;
  cout << "Initial zeros: " << initialZeros << endl;
  cout << "Initial ones: " << initialOnes << endl;

  // create threads
  pthread_t thread[M];
  for(int i = 0; i < M; i++){
    thread[i] = i;
  }

  // launch threads
  for(int i = 0; i < M; i++){
    if(error = pthread_create(&thread[i], NULL, matrix, (void*)i)){
      cout << "Error" << endl;
      exit(1);
    }
  }

  // wait for threads
  for(int i = 0; i < M; i++){
    pthread_join(thread[i], NULL);
  }

  cout << endl;
  // Read from file and print result
  cout << "Results from the file after calculations" << endl;
  fread(&mat, sizeof(mat), 1, fopen("binary.bin", "rb"));
  fclose(fopen("binary.bin", "rb"));
  for(int i = 0; i < N; i++){
    for(int j = 0; j < N; j++){
      cout << mat[i][j] << " ";
    }
    cout << endl;
  }

  // Print out total number of changes
  cout << "Total number of changes in binary file: " << totalNumChanges << endl;
  cout << "Ones inserted: " << totalOnesChanged << endl;
  cout << "Zeros inserted: " << totalZerosChanged << endl;
  cout << "Number of elements in matrix: " << (N*N) << endl;
  cout << "Number of threads used: " << M << endl;

  pthread_exit(NULL);

  // freeing each sub array
  for(int i = 0; i < N; i++){
    delete[] mat[i];
  }
  // free the array of pointers
  delete[] mat;

  return 0;
}
