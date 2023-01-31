/*Problematika Dekkerovog algoritma gdje gledamo kako riješiti problem kada su 2 procesa paralelno pokrenuta*/

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>


int* favouredThread; 
int* flags;

int* criticalSection;



void enterCriticalSection(int i, int j){
  flags[i] = 1;

  while(flags[j]){ 
    if(*favouredThread == j){ 
      flags[i] = 0; 
      while(*favouredThread == j); 
      flags[i] = 1; 
    }
  }

  
  if(i && !j) (*criticalSection)++; 
  else (*criticalSection)--; 
}

void exitCriticalSection(int i, int j){
  *favouredThread = j; 
  flags[i] = 0; 
}

void executeThread(int i){
  int k, m;
  int isThread1 = i==1; 
  int isThread2 = i==2; 

  for(k = 1; k<=5; k++){
    enterCriticalSection(isThread1, isThread2);

    for(m = 1; m<=5; m++){
      printf("thread: %d is entering critical section for the %d. time (this is %d/5 print statements) - value of critical section = %d\n", i, k, m, *criticalSection);
    }

    exitCriticalSection(isThread1, isThread2);
  }
}

void detachAndDestroySharedMemory(int id1, int id2){
  
  shmdt(favouredThread);
	shmdt(flags);
  
	shmctl(id1,IPC_RMID,NULL);
	shmctl(id2,IPC_RMID,NULL);
}


int main(){
  
  int favouredThreadShmId = shmget(IPC_PRIVATE, sizeof(int), SHM_R|SHM_W);
  int flagsShmId = shmget(IPC_PRIVATE, sizeof(int)*2, SHM_R|SHM_W);
  int criticalSectionShmId = shmget(IPC_PRIVATE, sizeof(int), SHM_R|SHM_W);
  
  favouredThread = shmat(favouredThreadShmId, NULL, 0);
  flags = shmat(flagsShmId, NULL, 0);
  criticalSection = shmat(criticalSectionShmId, NULL, 0);

  if(fork() == 0){
    
    executeThread(2);
  }else{
    
    executeThread(1);
    wait(NULL);
    detachAndDestroySharedMemory(favouredThreadShmId, flagsShmId);
  }

  return 0;
}