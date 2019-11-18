// Single producer, mutiple consumers, multiple bounded buffer, predefined number of elements.
// Jizhou Che 20032291 scyjc1.
// Zike Li 20031423 scyzl2.

#include "coursework.h"
#include "linkedlist.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

// The binary semaphore for synchronising access to the buffer.
sem_t sync;
// Counting semaphore representing the number of jobs in the buffer.
sem_t full;
// Counting semaphore representing the number of no full buffer.
sem_t empty;
int buffer[MAX_PRIORITY];
int numberOfConsumerFinish = 0;
// Bounded buffer.
struct element * bufferHead = NULL;
struct element * bufferTail = NULL;

// Number of items in the buffer.
int items = 0;
// Number of items already produced by the producer.
int produced = 0;
// Number of items already consumed by the consumer.
int consumed = 0;

// Visualise the number of elements currently in the buffer.
void print_buffer() {
	for (int i = 0; i < items; i++) {
		printf("*");
	}
	printf("\n");
}

void * producer() {
	// Keep a local record of the number of items after the last time I produced.
	int items_temp;
	for (int i = 0; i < NUMBER_OF_JOBS; i++) {
		sem_wait(&empty);
		sem_wait(&sync);
		// Produce an item.
		char * c = malloc(sizeof(char));
		* c = '*';
		addLast(c, &bufferHead, &bufferTail);
		items++;
		// Remember the number of items left after I have consumed that item.
		items_temp = items;
		// Update the number of produced items.
		produced++;
		// Increment the number of jobs in the buffer, and wake up the consumer if applicable.
		sem_post(&full);
		printf("Producer 1, Produced = %d, Consumed = %d: ", produced, consumed);
		print_buffer();
		sem_post(&sync);
		// Check whether there are empty buffers.
		// Use the local copy to remind me whether I should go to sleep.
		// In case I get interrupted at this point and the number of full buffers is different when I come back.
		if (items_temp == MAX_BUFFER_SIZE) {
			// Go to sleep if the buffer is full.
			sem_wait(&empty);
		}
	}
	pthread_exit(NULL);
}

void * consumer(void *id) {
	int consumerId = *((int *)id);	
	while(consumed < NUMBER_OF_JOBS) {
		// Go to sleep if no full buffer, otherwise decrement the jobs counter.
			
			if(numberOfConsumerFinish!=0)
				pthread_exit(NULL);
			sem_wait(&full);
			sem_wait(&sync);
			// Consume an item.
			char * c = removeFirst(&bufferHead, &bufferTail);
			// Memory deallocation.
			free(c);
			items--;
			// Update the number of consumed items.
			consumed++;
			printf("Consumer %d, Produced = %d, Consumed = %d: ", consumerId, produced, consumed);
			print_buffer();
			// Wake up the producer if applicable.
			if (items == MAX_BUFFER_SIZE - 1) {
				// Wake up the producer if the buffer is no longer full.
				sem_post(&empty);
			}
			sem_post(&sync);
		}
	numberOfConsumerFinish++;
	sem_post(&full);
	pthread_exit(NULL);
}

int main() {
	// Initialise semaphores.
	sem_init(&sync, 0, 1);
	sem_init(&empty, 0, NUMBER_OF_JOBS);
	sem_init(&full, 0, 0);
	// Create the producer thread
	pthread_t tProducer;
	// Create multiple consumer threads
	pthread_t tConsumers[NUMBER_OF_CONSUMERS];
	pthread_create(&tProducer, NULL, producer, NULL);
	for(int i=0;i<NUMBER_OF_CONSUMERS;i++) {
		int id = i+1;
		pthread_create(&tConsumers[i], NULL, consumer,(void *)&id );
	}
	// Tell the main thread to wait for both threads to finish.
	pthread_join(tProducer, NULL);
	for(int i=0;i<NUMBER_OF_CONSUMERS;i++) {
		pthread_join(tConsumers[i], NULL);
	}
	printf("fin\n");
	// Print out the values of all semaphores.
	int sSync, sEmpty, sFull;
	sem_getvalue(&sync, &sSync);
	sem_getvalue(&empty, &sEmpty);
	sem_getvalue(&full, &sFull);
	printf("sSync = %d, sEmpty = %d, sFull = %d\n", sSync, sEmpty, sFull);
	
	return 0;
}
