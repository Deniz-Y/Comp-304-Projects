#include "queue.c"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <stdbool.h>
int simulationTime = 120;    // simulation time
int seed = 10;               // seed for randomness
int emergencyFrequency = 30; // frequency of emergency gift requests from New Zealand

void* ElfA(void *arg); // the one that can paint
void* ElfB(void *arg); // the one that can assemble
void* Santa(void *arg); 
void* ControlThread(void *arg); // handles printing and queues (up to you)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int counter;
pthread_mutex_t packaging_mutex; 
pthread_mutex_t painting_mutex; 
pthread_mutex_t assembly_mutex; 
pthread_mutex_t qa_mutex; 
pthread_mutex_t delivery_mutex; 
pthread_mutex_t painting_and_qa_mutex; 
pthread_mutex_t assembly_and_qa_mutex; 
Queue *packaging;
Queue *painting;
Queue *assembly;
Queue *qa;
Queue *delivery;
Queue *painting_and_qa;
Queue *assembly_and_qa;
Queue *emergency;

int packagingA ;//for debugging elfa nın kaç kere packaging yaptığını en son output olarak verecek
int packagingB; //for debugging
int santa_delivery;//for debugging
int assemblyB; //for debugging
int paintingA; //for debugging
int santa_qa; //for debugging

//bool Qa_Done5 = false;
//bool Qa_Done4 = false;
//bool Painting_Done = false;
//bool Assembly_Done = false;

//for NEW ZEALAND PART:
bool Qa_Done5_new= false;
bool Qa_Done4_new = false;
bool Painting_Done_new = false;
bool Assembly_Done_new = false;

bool newZe_packaging = false;
bool newZe_painting = false;
bool newZe_assembly = false;
bool newZe_delivery= false;
bool newZe_qa = false;

Task newZealand;

time_t start_simulation;


// newzealand type 4 ve 5 booleanlarının isimlerini değiştir.
// newzelaand için queue.c değişti düzelt
//while loopların içi? bence şey yap current time al while o == ilk time + simulationTime
//type 4 ve type 5 aynı anda ne demek? mutex olmasın mı?
//aynı anda iki kişi farklı işler yapabilir mi? diğeri sleep halindeyken öbürü çalışabilir mi? mutex kaldırmalı mıyım o zmana sleepten önce??
// pthread sleeper function sorr
//condition varibale kullanıcaz mu
//queueu yı değiştirdim okey mi?
//en sona ret. boole= false ekle ve queueu.c ekle
// wile loop içi
//random
//log ekle
//if else öncelik
//emergency queue ya girer mi?
// son soru sor??
//örnek değerler doğru mu pdfte?
int t1;//for debugging hangi type'ın kaç kere geldiğini output olarak en son verecek
int t2;//for debugging
int t3;//for debugging
int t4;//for debugging
int t5;//for debugging



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// pthread sleeper function
int pthread_sleep (int seconds)
{
    pthread_mutex_t mutex;
    pthread_cond_t conditionvar;
    struct timespec timetoexpire;
    if(pthread_mutex_init(&mutex,NULL))
    {
        return -1;
    }
    if(pthread_cond_init(&conditionvar,NULL))
    {
        return -1;
    }
    struct timeval tp;
    //When to expire is an absolute time, so get the current time and add it to our delay time
    gettimeofday(&tp, NULL);
    timetoexpire.tv_sec = tp.tv_sec + seconds; timetoexpire.tv_nsec = tp.tv_usec * 1000;
    
    pthread_mutex_lock(&mutex);
    int res =  pthread_cond_timedwait(&conditionvar, &mutex, &timetoexpire);
    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&conditionvar);
    
    //Upon successful completion, a value of zero shall be returned
    return res;
}


int main(int argc,char **argv){
    // -t (int) => simulation time in seconds
    // -s (int) => change the random seed
    for(int i=1; i<argc; i++){
        if(!strcmp(argv[i], "-t")) {simulationTime = atoi(argv[++i]);}
        else if(!strcmp(argv[i], "-s"))  {seed = atoi(argv[++i]);}
    }
    
    srand(seed); // feed the seed
    
    painting = ConstructQueue(1000);
    packaging = ConstructQueue(1000);
    assembly = ConstructQueue(1000);
    qa = ConstructQueue(1000);
    delivery = ConstructQueue(1000);
    assembly_and_qa = ConstructQueue(1000);
    painting_and_qa = ConstructQueue(1000);
    emergency = ConstructQueue(1000);
    /* Queue usage example
        Queue *myQ = ConstructQueue(1000);
        Task t;
        t.ID = myID;
        t.type = 2;
        Enqueue(myQ, t);
        Task ret = Dequeue(myQ);
        DestructQueue(myQ);
    */
    

    // your code goes here
    // you can simulate gift request creation in here, 
    // but make sure to launch the threads first
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
     /* Initialize mutex and condition variable objects */
    pthread_mutex_init(&assembly_and_qa_mutex, NULL);
    pthread_mutex_init(&painting_and_qa_mutex, NULL); 
    pthread_mutex_init(&packaging_mutex, NULL);
    pthread_mutex_init(&painting_mutex, NULL);
    pthread_mutex_init(&assembly_mutex, NULL);
    pthread_mutex_init(&qa_mutex, NULL);
    pthread_mutex_init(&delivery_mutex, NULL);
    

    /* For portability, explicitly create threads in a joinable state */
    

    pthread_t threads[3];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    pthread_create(&threads[0], &attr, ElfA, NULL); //ElfA
    pthread_create(&threads[1], &attr, ElfB, NULL);   //ElfB
    pthread_create(&threads[2], &attr, Santa, NULL);  //Santa


    ControlThread(NULL);

    
   /* Wait for all threads to complete */
   for (int i = 0; i < 3; i++) {
    pthread_join(threads[i], NULL);
   }
   printf (" t1 %d  t2 %d t3 %d t4 %d t5 %d \n",t1,t2,t3,t4,t5);
   printf ("PackagingA %d  PackagingB  %d  Santa_delivery  %d  assemblyB %d  paintingA %d santa_qa %d threads. Done.\n",  packagingA,packagingB,santa_delivery,assemblyB,paintingA,santa_qa);

   /* Clean up and exit */
   pthread_attr_destroy(&attr);
   pthread_mutex_destroy(&packaging_mutex);
   pthread_mutex_destroy(&painting_mutex);
   pthread_mutex_destroy(&assembly_mutex);
   pthread_mutex_destroy(&qa_mutex);
   pthread_mutex_destroy(&delivery_mutex);
   pthread_mutex_destroy(&painting_and_qa_mutex);
   pthread_mutex_destroy(&assembly_and_qa_mutex);

   DestructQueue(packaging);
   DestructQueue(painting);
   DestructQueue(assembly);
   DestructQueue(delivery);
   DestructQueue(qa);
   DestructQueue(assembly_and_qa);
   DestructQueue(painting_and_qa);
   DestructQueue(emergency);
   pthread_exit (NULL); 
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    return 0;
}

void* ElfA(void *arg){
      


           while(counter != simulationTime + 20){ ///WHİLE LOOP İÇİ NE OLMALI???
              
  
               struct timeval current_time; // be able to check to time, you can delete it
               gettimeofday(&current_time, NULL); 
               
               
               
               if(newZe_packaging){
               
                         printf("EMERGENCYY: Packaging A time: %d Task ID: %d \n ",current_time.tv_sec,newZealand.ID);  
                         newZe_packaging = false;
      		         packagingA = packagingA + 1;
                         pthread_sleep(1);
                         newZe_delivery = true; //now you can switch next job

                }else if(newZe_painting){

                        printf("EMERGENCYY: Painting A time: %d Task ID: %d \n ",current_time.tv_sec,newZealand.ID);  
                        newZe_painting = false;
                        if(newZealand.type == 4){
                           
                                  Painting_Done_new = true;
                  
                                  if(Qa_Done4_new){ 
                                  
                                     Qa_Done4_new = false;
                                     Painting_Done_new = false;  
      		                     paintingA = paintingA + 1;
                                     pthread_sleep(3);
                                     newZe_packaging = true; //now you can switch next job
                                  }
                          }else if(newZealand.type == 2){  //ret.type = 2
                          
                                  paintingA = paintingA + 1;
                                  pthread_sleep(3);                        
                                  newZe_packaging = true; //now you can switch next job
                         } 
                    
                } 
                
                
                
                pthread_mutex_lock(&packaging_mutex);
                if(!isEmpty(packaging)){
                                                         
      		          Task ret = Dequeue(packaging);
      		          printf("Packaging A time: %d Task ID: %d \n",current_time.tv_sec,ret.ID);      
      		          packagingA = packagingA + 1;
      		          pthread_mutex_unlock(&packaging_mutex);
      		          
                          pthread_sleep(1);
                          
                          pthread_mutex_lock(&delivery_mutex);
                          Enqueue(delivery, ret);
                          pthread_mutex_unlock(&delivery_mutex);                
                }else{
                          pthread_mutex_unlock(&packaging_mutex);
                               
                
                	pthread_mutex_lock(&painting_mutex);
                	if(!isEmpty(painting)){
                                                  
                         	Task ret = Dequeue(painting);
                         	printf("Painting A time: %d Task ID: %d \n",current_time.tv_sec,ret.ID); 
                         	paintingA = paintingA + 1;
                         	pthread_mutex_unlock(&painting_mutex);
                         
                         	pthread_sleep(3); 
                              		          
                         	if(ret.type == 4){
                         
                  		   	pthread_mutex_lock(&painting_and_qa_mutex);
                  		   	if(Contains(painting_and_qa,ret.ID)){
                                     
                                     	pthread_mutex_lock(&packaging_mutex);               
                                     	Enqueue(packaging,ret);
                                     	pthread_mutex_unlock(&packaging_mutex);    
  
                                  	}else{
                                  
                                     	Enqueue(painting_and_qa,ret);
                                  	}                                 
                                  	pthread_mutex_unlock(&painting_and_qa_mutex);            
                                  
                  		  
                         	}else if(ret.type == 2){
                                                                    
                                  	pthread_mutex_lock(&packaging_mutex);
                                  	Enqueue(packaging, ret);
                                  	pthread_mutex_unlock(&packaging_mutex);
                        	}                        
                	}else{
                
                     		pthread_mutex_unlock(&painting_mutex);    
               		}
               }
                
          } //while end

}

void* ElfB(void *arg){


      while(counter != simulationTime + 20){

               struct timeval current_time; // be able to check to time, you can delete it
               gettimeofday(&current_time, NULL);  
               
                if(newZe_packaging){
               
                         printf("EMERGENCYY: Packaging B time: %d Task ID: %d \n",current_time.tv_sec,newZealand.ID);  
                         newZe_packaging = false;
      		         packagingA = packagingA + 1;
                         pthread_sleep(1);
                         newZe_delivery = true; //now you can switch next job
                        
            
                }else if(newZe_assembly){
                
                        printf("EMERGENCYY: Assembly B time: %d Task ID: %d \n",current_time.tv_sec,newZealand.ID);  
                        newZe_assembly = false;
                        if(newZealand.type == 5){
                           
                                  Assembly_Done_new = true; 
                  
                                  if(Qa_Done5_new){ 
                                  
                                     Qa_Done5_new = false;
                                     Assembly_Done_new = false; 
                                     assemblyB = assemblyB + 1;
                                     pthread_sleep(2);
                                     newZe_packaging= true; //now you can switch next job
                                  }
                          }else if(newZealand.type == 3){  //ret.type = 3
                          
                                  assemblyB = assemblyB + 1;
                                  pthread_sleep(2);
                                  newZe_packaging= true; //now you can switch next job

                         }  
                         

                }
                pthread_mutex_lock(&packaging_mutex);
                if(!isEmpty(packaging)){
                                                
                         Task ret = Dequeue(packaging);
                         printf("Packaging B time: %d Task ID: %d \n",current_time.tv_sec,ret.ID);
      		         packagingB = packagingB + 1;
      		         pthread_mutex_unlock(&packaging_mutex);
                         
                         pthread_sleep(1);
                         
                         pthread_mutex_lock(&delivery_mutex);
                         Enqueue(delivery,ret);
                         pthread_mutex_unlock(&delivery_mutex);
               }else{
                         pthread_mutex_unlock(&packaging_mutex);              
               
               
               	         pthread_mutex_lock(&assembly_mutex);          
                         if(!isEmpty(assembly)){
                                                 
                         	Task ret = Dequeue(assembly);
                         	printf("Assembly B time: %d Task ID: %d \n",current_time.tv_sec,ret.ID); 
                         	assemblyB= assemblyB + 1;
                         	pthread_mutex_unlock(&assembly_mutex);   
                         	pthread_sleep(2);
                         
                         	if(ret.type == 5){
                         
                            		pthread_mutex_lock(&assembly_and_qa_mutex);                           
                            		if(Contains(assembly_and_qa,ret.ID)){ 
                                 
                                 		pthread_mutex_lock(&packaging_mutex); 
                                 		Enqueue(packaging, ret);
                                 		pthread_mutex_unlock(&packaging_mutex); 
               
                            		}else{
                            
                                 		Enqueue(assembly_and_qa,ret);                              
                            		}
                            
                            		pthread_mutex_unlock(&assembly_and_qa_mutex);
                                                             
                        	}else if(ret.type == 3){  
                                                                 
                           		pthread_mutex_lock(&packaging_mutex); 
                           		Enqueue(packaging, ret);   
                           		pthread_mutex_unlock(&packaging_mutex);       
                       		}                                      
               		 }else{
                
                       	      pthread_mutex_unlock(&assembly_mutex);                  
               		}
            }
      }//while end
}

// manages Santa's tasks
void* Santa(void *arg){


         while(counter != simulationTime + 20){  //WHİLE LOOP İÇİ NE OLMALI
            
         struct timeval current_time; // be able to check to time, you can delete it
         gettimeofday(&current_time, NULL);  
                
          if(newZe_delivery){
           
             printf("EMERGENCYY: Santa Delivery time: %d Task ID: %d \n",current_time.tv_sec,newZealand.ID);  
             newZe_delivery = false;
      	     santa_delivery = santa_delivery + 1;  
             pthread_sleep(1);

             
          }else if(newZe_qa){

             printf("EMERGENCYY: Santa QA time: %d Task ID: %d \n",current_time.tv_sec,newZealand.ID);  
             newZe_qa = false;
             if(newZealand.type == 4){
             
                Qa_Done4_new = true;     
                if(Painting_Done_new){ 

                   Painting_Done_new = false;
                   Qa_Done4_new = false;
                   santa_qa = santa_qa + 1;  
                   pthread_sleep(1);               
                   newZe_packaging = true; //now you can switch next job                                    
                }
              }else if(newZealand.type == 5){ 
             
                  Qa_Done5_new = true;
                  if(Assembly_Done_new){ 
                    
                    Assembly_Done_new = false;
                    Qa_Done5_new = false;
                    santa_qa = santa_qa + 1;  
                    pthread_sleep(1);
                    newZe_packaging = true; //now you can switch next job
                  }         
              } 

              
        }else{ // no gift for newZealand */
                         
                                
               
             int priority = 0;
             pthread_mutex_lock(&delivery_mutex);
             if(isEmpty(delivery)){
             
                pthread_mutex_unlock(&delivery_mutex);
                
                pthread_mutex_lock(&qa_mutex);
                if(!isEmpty(qa)){
                    priority = 2; // QA prioritizes
                }
                pthread_mutex_unlock(&qa_mutex);
       
             }else{
             
                pthread_mutex_unlock(&delivery_mutex);
                pthread_mutex_lock(&qa_mutex);
                if(qa->size >= 3){
                
                    pthread_mutex_unlock(&qa_mutex);
                    priority = 2; // QA prioritizes
                    
                }else{
                    pthread_mutex_unlock(&qa_mutex);
                    priority = 1; // delivery prioritizes
                }
             } 
             

             if(priority == 1){ 
                    
                    pthread_mutex_lock(&delivery_mutex);                  
                    Task ret = Dequeue(delivery); 
                    printf("Santa Delivery time: %d Task ID: %d \n",current_time.tv_sec,ret.ID);                      
      		    santa_delivery = santa_delivery + 1; 
      		    pthread_mutex_unlock(&delivery_mutex);

     		    pthread_sleep(1);
     
     		    
     	      }else if(priority == 2){
     	             
     	             pthread_mutex_lock(&qa_mutex);    	                     
     	             Task ret = Dequeue(qa);
     	             printf("Santa QA time: %d Task ID: %d \n",current_time.tv_sec,ret.ID);
     	             santa_qa = santa_qa + 1;
     	             pthread_mutex_unlock(&qa_mutex);
     	             
     	             pthread_sleep(1);  
     	             
                     if(ret.type == 4){
                            
                            pthread_mutex_lock(&painting_and_qa_mutex);
                            if(Contains(painting_and_qa,ret.ID)){ 
                                                                
                                 pthread_mutex_lock(&packaging_mutex);    
                                 Enqueue(packaging, ret);  
                                 pthread_mutex_unlock(&packaging_mutex); 
                                                                      
                           }else{
                           
                                Enqueue(painting_and_qa,ret);
                           }
                           pthread_mutex_unlock(&painting_and_qa_mutex);
                                                                                    
                   }else if(ret.type == 5){ 
                                                      
                           pthread_mutex_lock(&assembly_and_qa_mutex); 
                    
                           if(Contains(assembly_and_qa,ret.ID)){ 
                                 
                                 pthread_mutex_lock(&packaging_mutex); 
                                 Enqueue(packaging, ret);
                                 pthread_mutex_unlock(&packaging_mutex);    
                                                              
                           }else{
                           
                                Enqueue(assembly_and_qa,ret);
                           }
                           pthread_mutex_unlock(&assembly_and_qa_mutex);
                  }

              }//priority end
             }
          } //while end
}

// the function that controls queues and output
void* ControlThread(void *arg){

    counter = 0;
    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    int time = current_time.tv_sec;
    int emer_freg = emergencyFrequency;
     
    while(counter != simulationTime + 20){  // WHİLE LOOP İÇİ NE OLMALI?
           int random = (rand() % 100) + 1;
           
           
     if(counter < simulationTime){ // WHİLE LOOP İÇİ NE OLMALI?
  
// ---------------------------------------------------- TYPE 5------------------------------------------------------------------------------------------------------------------  	
	if (random <= 5 && random >= 1){ //type 5
	    
	    gettimeofday(&current_time, NULL);
	    Task t;
            t.ID = counter;
            t.type = 5;
            
            if(counter == emer_freg){
               
                newZealand = t;
                emer_freg = emergencyFrequency + emer_freg;
                newZe_assembly = true;
                newZe_qa= true;
                printf("\nEMERGENCY Type 5:  %d %d \n",counter,current_time.tv_sec);
            
            }else{  
                pthread_mutex_lock(&assembly_mutex);
                Enqueue(assembly, t);
                pthread_mutex_unlock(&assembly_mutex);
                
                pthread_mutex_lock(&qa_mutex);
                Enqueue(qa, t);
                pthread_mutex_unlock(&qa_mutex);
                printf("\nType 5:  %d %d \n",counter,current_time.tv_sec);
           }
                 
           t5 = t5 + 1;
	
       }
		
// ---------------------------------------------------- TYPE 4------------------------------------------------------------------------------------------------------------------  	
       if (random <= 10 && random >= 6){ //type 4
	  
	    gettimeofday(&current_time, NULL);
	    Task t;
            t.ID = counter;
            t.type = 4;
            
            if(counter == emer_freg){

                 newZealand = t;
                 emer_freg = emergencyFrequency + emer_freg;
                 newZe_painting = true;
                 newZe_qa= true;
                 printf("\nEMERGENCY Type 4:  %d %d\n",counter,current_time.tv_sec);
            
            }else{
                 pthread_mutex_lock(&painting_mutex);
                 Enqueue(painting, t);
                 pthread_mutex_unlock(&painting_mutex);
                 
                 pthread_mutex_lock(&qa_mutex);
                 Enqueue(qa, t);
                 pthread_mutex_unlock(&qa_mutex);
                 printf("\nType 4:  %d %d\n",counter,current_time.tv_sec);
           }
           
	   t4 = t4 + 1;
	}
		
// ---------------------------------------------------- TYPE 3------------------------------------------------------------------------------------------------------------------  		
	 if (random <= 30 && random>= 11){// type 3
	 
	    gettimeofday(&current_time, NULL);
	    Task t;
            t.ID = counter;
            t.type = 3;
             
            if(counter == emer_freg){
            
               //Enqueue_NewZealand(assembly, t);
               newZealand = t;
               emer_freg = emergencyFrequency + emer_freg;
               newZe_assembly = true;
               printf("\nEMERGENCY Type 3:  %d %d\n",counter,current_time.tv_sec); 
            }else{ 
                 pthread_mutex_lock(&assembly_mutex);
                 Enqueue(assembly, t);
                 pthread_mutex_unlock(&assembly_mutex);
                 printf("\nType 3:  %d %d\n",counter,current_time.tv_sec); 
            }
                       
           t3 = t3 + 1;
	
	}
		
// ---------------------------------------------------- TYPE 2------------------------------------------------------------------------------------------------------------------  		
	 if (random <= 50 && random>= 31){ // type 2
	    gettimeofday(&current_time, NULL);
	    Task t;
            t.ID = counter;
            t.type = 2;
            
            if( counter == emer_freg){

                //Enqueue_NewZealand(painting, t);
                newZealand = t;
                emer_freg = emergencyFrequency + emer_freg;
                newZe_painting= true;
                printf("\nEMERGENCY Type 2:  %d % d\n",counter,current_time.tv_sec); 
  
            }else{
                     pthread_mutex_lock(&painting_mutex);
                     //request time
                     //task arrival
                     Enqueue(painting, t);
                     pthread_mutex_unlock(&painting_mutex);
                     printf("\nType 2:  %d % d\n",counter,current_time.tv_sec); 
            }                     
            t2 = t2 + 1;
	}
		
// ---------------------------------------------------- TYPE 1------------------------------------------------------------------------------------------------------------------  		
	 if (random <= 90 && random>= 51){ //type 1  //90
	    gettimeofday(&current_time, NULL);
	    Task t;
            t.ID = counter;
            t.type = 1;

            if(counter == emer_freg){
                //Enqueue_NewZealand(packaging, t);
                newZealand = t;
                emer_freg = emergencyFrequency + emer_freg;
                newZe_packaging = true;
                printf("\nEMERGENCYY Type 1:  %d %d \n",counter,current_time.tv_sec);
                
            }else{ 
                 pthread_mutex_lock(&packaging_mutex);
                 Enqueue(packaging, t);
                 pthread_mutex_unlock(&packaging_mutex);
                 printf("\nType 1:  %d %d \n",counter,current_time.tv_sec);
            }           
            t1 = t1 + 1;
	}
// ---------------------------------------------------- TYPE 0------------------------------------------------------------------------------------------------------------------  		
	if (random <= 100 && random>= 91){ //type 0
	
	    gettimeofday(&current_time, NULL);
            printf("\nNO gift:  %d %d %d\n",counter,current_time.tv_sec);
	 
	}   
    }// end of  if(counter < simulationTime){ 
        sleep(1);
        counter = counter + 1;
    } // while loop end
    
}
