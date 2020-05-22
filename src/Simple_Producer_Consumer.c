#include "circular_buffer.h"
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

typedef struct PRODUCERS_ARGUMENTS {
	int seed_value;
	int number_to_produce;
	int Stacksize;
	circular_buffer cb;
} PRODUCERS_ARGUMENTS;


pthread_mutex_t the_mutex;
pthread_cond_t condc, condp;
int cflag_txt = 0;//gia prwth fora
int pflag_txt = 0;//gia prwth fora

void* producer(void *args) {
  int i , N , rc , seed ,s ,k ;
  circular_buffer *cb;
  PRODUCERS_ARGUMENTS *productArgs;
  productArgs = (PRODUCERS_ARGUMENTS *) args;
  seed = productArgs->seed_value;
  s =  productArgs->Stacksize;
  N = productArgs->number_to_produce;
 int numbers[N];
  cb = &productArgs->cb;


  for (i = 0; i < N; i++) {

	rc = pthread_mutex_lock(&the_mutex);	/* protect buffer */
   

    if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		pthread_exit(&rc);
	}	
    while (cb->count ==  s )	{	       /* If there is something 
					  in the buffer then wait */


      rc = pthread_cond_wait(&condp, &the_mutex);

      if (rc != 0) {	
	   printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
	   pthread_exit(&rc);
      }


   }

 
   srand(seed);
   seed =seed  +1;
   int Produced_Number = (int)(rand()%256);
   numbers[i] = Produced_Number;
   
    cb_push_back(cb, &Produced_Number);
    FILE *fp;
    if(pflag_txt == 0){ 
  
        fp=fopen("producers_in.txt", "w");
        pflag_txt = 1;
    }else{
        fp=fopen("producers_in.txt", "a");//append gia na sunexisei na grafei apo katw 
    }//Prepei na to kanw kai append!!!
    if(fp == NULL){
        exit(-1);
    }
    fprintf(fp , "Producer 1 : %d \n",Produced_Number);
    fclose(fp);		
     
  
	
    rc = pthread_cond_signal(&condc);	/* wake up consumer */
     
    if (rc != 0) {	
		printf("ERROR: return code from pthread_cond_signal() is %d\n", rc);
		pthread_exit(&rc);
    }	


    rc = pthread_mutex_unlock(&the_mutex);	/* release the buffer */

    if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		pthread_exit(&rc);
	}	

   
  }
 printf("Producer 1 :");
 for(k=0 ; k<N; k++){
   if(k < N && k < (N-1)){
          printf("%d ,",numbers[k]);
  
     }else{
         printf("%d ",numbers[k]);
          
     }
}
  
  pthread_exit(0);
}



void* consumer(void *args) {
  int i , rc , N , k ;
   int tmp_read;
  circular_buffer *cb;
  PRODUCERS_ARGUMENTS *productArgs;
  productArgs = (PRODUCERS_ARGUMENTS *) args;
  N = productArgs->number_to_produce;
  int numbers[N];
  cb = &productArgs->cb;

  for (i = 1; i <=N; i++) {
	//EDW 8A PAIRNEI APO TO BUFFER
     rc = pthread_mutex_lock(&the_mutex);	/* protect buffer */
    
    if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		pthread_exit(&rc);
	}	
    while (cb->count == 0)	{		/* If there is nothing in 
					   the buffer then wait */

      rc = pthread_cond_wait(&condc, &the_mutex);

       if (rc != 0) {	
		printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
		pthread_exit(&rc);
	}
         
	}
    
      cb_pop_front(cb, &tmp_read);

     numbers[i] = tmp_read;
     FILE *fp;
     if(cflag_txt == 0){
        fp=fopen("consumers_out.txt", "w");
        cflag_txt = 1;//allagh tou flag gia na grapsei apo katw ta upoloipa me append
     }else{
        fp=fopen("consumers_out.txt", "a");
     }//Prepei na to kanw kai append!!!
     if(fp == NULL){
        exit(-1);
    }
    fprintf(fp , "Consumer 1 : %d \n",tmp_read);
    fclose(fp);		
    


    rc = pthread_cond_signal(&condp);	/* wake up consumer */

     if (rc != 0) {	
	printf("ERROR: return code from pthread_cond_signal() is %d\n", rc);
	pthread_exit(&rc);
     }	 
   
    rc = pthread_mutex_unlock(&the_mutex);	/* release the buffer */

     if (rc != 0) {	
	printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
	pthread_exit(&rc);
     }	

  }
  
  printf("\nConsumer 1 :");
  for(k=1 ; k<=N; k++){
     if(k == N){
          printf("%d ",numbers[k]); 
  
     }else{
        printf("%d ,",numbers[k]);
     }
  }
  printf("\n");
  pthread_exit(0);
}
   
   
   

int main(int argc, char **argv) {
  pthread_t pro , con;
  int rc;
  
   if(argc != 6) //Check the number of command line arguments
  {
    printf("You must run this program with 5 arguments\n");
    return -1;
  }  

  int Numbers = atoi(argv[3]);
  int Stack_size = atoi(argv[4]);
  int seed = atoi(argv[5]);
 
  circular_buffer cb;


 PRODUCERS_ARGUMENTS productArgs;
  productArgs.seed_value = seed;
  productArgs.number_to_produce = Numbers;
  productArgs.Stacksize = Stack_size;
productArgs.cb = cb;

   cb_init(&productArgs.cb, Stack_size, sizeof(int));
  
  

  // Initialize the mutex and condition variables
  rc = pthread_mutex_init(&the_mutex, NULL);

  if (rc != 0) {
    	printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
       	exit(-1);
	}
	
  rc = pthread_cond_init(&condp, NULL);		/* Initialize consumer condition variable */

  if (rc != 0) {
    	printf("ERROR: return code from pthread_cond_init() is %d\n", rc);
       	exit(-1);
	}

  rc = pthread_cond_init(&condc, NULL);		/* Initialize producer condition variable */

  if (rc != 0) {
    	printf("ERROR: return code from pthread_cond_init() is %d\n", rc);
       	exit(-1);
	}

  // Create the threads

  rc = pthread_create(&pro, NULL, producer , &productArgs);

  if (rc != 0) {
    	printf("ERROR: return code from pthread_create() is %d\n", rc);
       	exit(-1);
	}

   rc = pthread_create(&con, NULL, consumer , &productArgs);

  if (rc != 0) {
    	printf("ERROR: return code from pthread_create() is %d\n", rc);
       	exit(-1);
	}

 
  rc = pthread_join(con, NULL);
  
  if (rc != 0) {
    	printf("ERROR: return code from pthread_join() is %d\n", rc);
       	exit(-1);
	}

  rc = pthread_join(pro, NULL);
  
  if (rc != 0) {
    	printf("ERROR: return code from pthread_join() is %d\n", rc);
       	exit(-1);
	}

  // Cleanup -- would happen automatically at end of program
  rc = pthread_mutex_destroy(&the_mutex);	/* Free up the_mutex */

  if (rc != 0) {
    	printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
       	exit(-1);
	}

  rc = pthread_cond_destroy(&condc);		/* Free up consumer condition variable */

  if (rc != 0) {
    	printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
       	exit(-1);
	}

  pthread_cond_destroy(&condp);		/* Free up producer condition variable */

   if (rc != 0) {
    	printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
       	exit(-1);
	}
	
	cb_free(&productArgs.cb);//free memory of circular buffer
	
 return -1;
}
