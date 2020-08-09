#pragma once
#pragma once
#include "winsock2.h"
#include <stdio.h>

struct MESSAGE
{
	time_t sending_time;
	char message[250];
	int sequence_num;
} ;

struct MessageQueue
{
	int front, rear, size;
	unsigned capacity;
	MESSAGE* array;
};


///<summary>
/// Creating Message Queue. 
///</summary>
///<param name ="capacity"> Capacity of queue.</param>
///<returns> Created message queue.</returns>
struct MessageQueue* CreateMessageQueue(unsigned capacity)
{
    struct MessageQueue* queue = (struct MessageQueue*) malloc(sizeof(struct MessageQueue));
	queue->capacity = capacity;
	queue->front = queue->size = 0;
	queue->rear = capacity - 1;  
	queue->array = (MESSAGE*)malloc(queue->capacity * sizeof(MESSAGE));
	return queue;
}


///<summary>
/// Checking if size of message queue is equal to it's capacity. 
///</summary>
///<param name ="queue"> Queue to check.</param>
///<returns> Indicates if message queue is full..</returns>
int IsFullMessageQueue(struct MessageQueue* queue)
{
	return (queue->size == queue->capacity);
}

///<summary>
/// Checking if size of message queue is equal to zero. 
///</summary>
///<param name ="queue"> Queue to check.</param>
///<returns> Indicates if message queue is empty.</returns> 
int IsEmptyMessageQueue(struct MessageQueue* queue)
{
	return (queue->size == 0);
}


///<summary>
/// Adding element to message queue. 
///</summary>
///<param name ="queue"> Queue to add to.</param>
///<param name ="topic"> Topic message structure to add.</param>
///<returns> No return value.</returns> 
void Enqueue(struct MessageQueue* queue, MESSAGE message)
{
	if (IsFullMessageQueue(queue))
		return;
	queue->rear = (queue->rear + 1) % queue->capacity;
	queue->array[queue->rear] = message;
	queue->size = queue->size + 1;
}


///<summary>
/// Deleting (poping) element from message queue. 
///</summary>
///<param name ="queue"> Queue to delete from.</param>
///<returns> Poped element.</returns> 
MESSAGE Dequeue(struct MessageQueue* queue)
{
	if (!IsEmptyMessageQueue(queue)) {
		MESSAGE item = queue->array[queue->front];
		queue->front = (queue->front + 1) % queue->capacity;
		queue->size = queue->size - 1;
		return item;
	}
}
