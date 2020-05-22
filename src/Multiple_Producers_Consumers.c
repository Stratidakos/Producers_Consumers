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
        int token;
	circular_buffer *cb;
} PRODUCERS_ARGUMENTS;


pthread_mutex_t the_mutex;
pthread_cond_t condc, condp;
int cflag_txt = 0;//gia prwth fora
int pflag_txt = 0;//gia prwth fora
int SEED = 0;//global gia to seed


void* producer(void *args) {
  int i , N , rc ,s , k , tokenid ;
  circular_buffer *cb;
  PRODUCERS_ARGUMENTS *productArgs;
  productArgs = (PRODUCERS_ARGUMENTS *) args;
  //seed = productArgs->seed_value;
  s =  productArgs->Stacksize;
  N = productArgs->number_to_produce;
  int numbers[N];
  tokenid = productArgs->token;
  //printf("Starting producer with id %d\n", tokenid);
  cb = productArgs->cb;


  for (i = 0; i <N; i++) {

	rc = pthread_mutex_lock(&the_mutex);	/* protect buffer */
   

    if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		pthread_exit(&rc);
	}	
    while (cb->count == s  )	{//PREPEI NA TO KSANADW AYTO!!!!	       /* If there is something 
				//	  in the buffer then wait */


      rc = pthread_cond_wait(&condp, &the_mutex);

      if (rc != 0) {	
	   printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
	   pthread_exit(&rc);
      }


   }

   
   srand(SEED);
   SEED =SEED  +1;
   int Produced_Number = (int)(rand()%256);
   numbers[i] = Produced_Number;
   
    cb_push_back(cb, &Produced_Number);
      //printf("Producer %d : %d \n",tokenid , Produced_Number);
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
    fprintf(fp , "Producer %d : %d \n", tokenid , Produced_Number);
    fclose(fp);		
     
  
	
    rc = pthread_cond_broadcast(&condc);	/* wake up consumer */
     
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


printf("Producer %d :" , tokenid);
 for(k=0 ; k<N; k++){
   if(k < N && k < (N-1)){
          printf("%d ,",numbers[k]);
  
     }else{
         printf("%d \n",numbers[k]);
          
     }
  }
  pthread_exit(0);
}

void* consumer(void *args) {
  int i , rc , N , k , tokenid;
   int tmp_read;
  circular_buffer *cb;
  PRODUCERS_ARGUMENTS *productArgs;
  productArgs = (PRODUCERS_ARGUMENTS *) args;
  N = productArgs->number_to_produce;
  int numbers[N];
  tokenid = productArgs->token;
  cb = productArgs->cb;

  //printf("Starting consumer with id %d\n", tokenid);
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
     //printf("Consumer %d : %d \n", tokenid , tmp_read);
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
    fprintf(fp , "Consumer %d : %d \n", tokenid , tmp_read);
    fclose(fp);		
    


    rc = pthread_cond_broadcast(&condp);	/* wake up consumer */

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
  
 printf("\nConsumer %d :" , tokenid);
  for(k=1 ; k<=N; k++){
     if(k == N){
          printf("%d \n ",numbers[k]); 
  
     }else{
        printf("%d ,",numbers[k]);
     } 
 }
  pthread_exit(0);
}

int main(int argc, char **argv) {
  int rc , i ;
 
  
   if(argc != 6) //Check the number of command line arguments
  {
    printf("You must run this program with 5 arguments\n");
    return -1;
  }  
  int num_producer = atoi(argv[1]);
  int num_consumer = atoi(argv[2]);
  int Numbers = atoi(argv[3]);
  int Stack_size = atoi(argv[4]);
  int seed = atoi(argv[5]);
  SEED=seed;
  pthread_t producers[num_producer];
  pthread_t consumers[num_consumer]; 
 
  circular_buffer cb;

   cb_init(&cb, Stack_size, sizeof(int));
  
  

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
   
   PRODUCERS_ARGUMENTS *producersArgs = malloc(sizeof(PRODUCERS_ARGUMENTS) * num_producer);
   if (producersArgs == NULL) {
	printf("Not enough memory!\n");
	return -1;
   }

   int threadId;
   for (i = 0; i < num_producer; i++) {
  	producersArgs[i].seed_value = seed;
  	producersArgs[i].number_to_produce = Numbers;
 	producersArgs[i].Stacksize = Stack_size;
	producersArgs[i].cb = &cb;
	threadId = i + 1;
        producersArgs[i].token = threadId;
         
       rc = pthread_create(&producers[i], NULL, producer , &producersArgs[i]);
       if (rc != 0) {
    	printf("ERROR: return code from pthread_create() is %d\n", rc);
       	exit(-1);
	}
    }

    PRODUCERS_ARGUMENTS *consumerArgs = malloc(sizeof(PRODUCERS_ARGUMENTS) * num_consumer);
   if (consumerArgs == NULL) {
	printf("Not enough memory!\n");
	return -1;
   }

   for (i = 0; i < num_consumer; i++) {
  	consumerArgs[i].seed_value = seed;
  	consumerArgs[i].number_to_produce = Numbers;
 	consumerArgs[i].Stacksize = Stack_size;
	consumerArgs[i].cb = &cb;
	consumerArgs[i].token = i + 1;
       
       rc = pthread_create(&consumers[i], NULL, consumer , &consumerArgs[i]);
        if (rc != 0) {
    	printf("ERROR: return code from pthread_create() is %d\n", rc);
       	exit(-1);
	}
   
   }
 
   //Join all the threads
 
   for (i = 0;i < num_producer;i++) {
    	rc = pthread_join(producers[i], NULL);
		if (rc != 0) {
			printf("ERROR: return code from pthread_join() is %d\n", rc);
			exit(-1);		
		}
   }

  for (i = 0; i < num_consumer;i++) {
    	rc = pthread_join(consumers[i], NULL);
		if (rc != 0) {
			printf("ERROR: return code from pthread_join() is %d\n", rc);
			exit(-1);		
		}
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
 
	cb_free(&cb);//free memory of circular buffer
	free(producersArgs);
	free(consumerArgs);
 return -1;
}
