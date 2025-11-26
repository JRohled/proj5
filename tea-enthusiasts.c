#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <stdbool.h>
#include <errno.h>
#define MAX 6
#define SUPPLIERS 6
#define ENTHUSIASTS 14

const char* ingredients[] = {
    "Green Tea Leaf",
    "Black Tea Leaf",
    "Bergamot Oil",
    "Hibiiscus Leaf",
    "Puffed Rice",
    "Spices"
};

//Globals: Pouches array, mutex, semaphores
pthread_mutex_t tableLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t printLock = PTHREAD_MUTEX_INITIALIZER;

sem_t supplierSems[MAX];

int table[MAX];

//Functions
void *supplier(void* arg);
void *enthusiasts(void* arg);

int main() {
    printf("CS370 Project 5 - Tea Enthusiast Problem - Solved by Jacob Rohleder\n");
    printf("+-----------------------------------------+\n\n");
    //Init semaphores
    for(int i = 0; i < MAX; i++) {
        sem_init(&supplierSems[i], 0, 0);    
    }

    //Green: 0
    //Black: 1
    //Berg:  2
    //Hib:   3
    //Rice:  4
    //Spice: 5
   
    //Initialize suppliers
    pthread_t suppliersT[SUPPLIERS];
    for(int i = 0; i < SUPPLIERS; i++) {
        int *arg = malloc(sizeof(int));
        *arg = i;
        int e = pthread_create(&suppliersT[i], NULL, supplier, arg);
        if(e != 0) {
            printf("\nSupplier Thread %d Error\n ", e);
            return 0;
        }
    }

    //Initialize enthusiasts
    pthread_t enjoyersT[ENTHUSIASTS];
    for(int i = 0; i < ENTHUSIASTS; i++) {
        int *arg = malloc(sizeof(int));
        *arg = i;
        int e = pthread_create(&enjoyersT[i], NULL, enthusiasts, arg);
        if(e != 0){
            printf("\nEnthusiast Thread %d Error\n", e);
            return 0;
        } 
    }

    //Wait for enthusiasts(Suppliers run forever)
    for(int i = 0; i < ENTHUSIASTS; i++) {
        pthread_join(enjoyersT[i], NULL);
    }
  
    return 0;
}

//One pouch per random N amount of ingredients chosen for recipe
void *enthusiasts(void* arg) {
    int id = *((int*) arg);
    free(arg);
    int recipes = (rand() % 15) + 1;
    for(int x = 0; x < recipes; x++) {
        int recSize = (rand() % 3) + 2;
        char* recipe[recSize]; //Array of recipe strings
        bool used[MAX] = {false, false, false, false, false, false};
    
        //+--- Choose Ingredients ---+
        //Green(0) or Black(1) Tea Leaf
        int tea = rand() % 2;
        recipe[0] = (char*) ingredients[tea];
        used[tea] = true;
    

        // Other Ingredients
        for(int i = 1; i < recSize; i++) {
            int ing = (rand() % 6);
            while(used[ing]) {
                ing = (rand() % 6);
            }
            used[ing] = true;
            recipe[i] = (char*) ingredients[ing];
        }

        pthread_mutex_lock(&printLock);
        printf("\033[0;92m Enthusiast %d requesting tea with ingredients: \033[0m\n\t", id);
        for(int i = 0; i < recSize; i++) {
            printf("\033[0;92m %s \033[0m", recipe[i]);
        }    
        printf("\n");
        pthread_mutex_unlock(&printLock);

        // Get ingredients from Supplier Table
        for(int i = 0; i < MAX; i++) {
            //pthread_mutex_lock(&tableLock);
            if(used[i]) {
                //pthread_mutex_unlock(&tableLock);
                sem_wait(&supplierSems[i]);
                pthread_mutex_lock(&tableLock);
                table[i]--;
                pthread_mutex_unlock(&tableLock);
            }
            
        }

        pthread_mutex_lock(&printLock);
        printf("\033[0;92m > Enthusiast %d has received their tea \033[0m\n", id);
        pthread_mutex_unlock(&printLock);
        usleep(rand() % 5000);
    }

    printf("\033[0;92m Enthusiast %d is done \033[0m\n", id);
    return NULL;
}

void *supplier(void *arg) {
    int id = *((int*)arg);
    free(arg);
    int addPouches = (rand() % 10) + 1;
    //Loop until termination:
    while(true) {
        // - Increasing available pouches
        pthread_mutex_lock(&tableLock);
        if(table[id] == 0) {
            for(int i = 0; i < addPouches; i++) {
                sem_post(&supplierSems[id]);
                table[id]++;
            }
            // - Display message
            pthread_mutex_lock(&printLock);
            printf("\033[0;31m %s supplier added %d pouches.\033[0m\n", 
                (char*) ingredients[id], addPouches);
            pthread_mutex_unlock(&printLock);
            pthread_mutex_unlock(&tableLock);
            
            // Sleep until pouches are 0
            while(table[id] > 0) {
                usleep(100);
            }
        }
        pthread_mutex_unlock(&tableLock);
    }    
}