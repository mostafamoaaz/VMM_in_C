#include "queue.h"

void CreateQueue(Queue *pq){
    pq->Front=0;
    pq->Rear=-1;
    pq->SizeQ=0;
}

void Enqueue(Queue* pq,entry i){
    pq->Rear = (pq->Rear+1)%MAX;
    pq->arrayq[pq->Rear] = i;
    ++(pq->SizeQ);
}

void Dequeue(Queue *pq,entry *i){
    *i = pq->arrayq[pq->Front];
    pq->Front = (pq->Front+1)%MAX;
    --pq->SizeQ;
}

int GetSize(Queue q){
    return q.SizeQ;
}

int IsEmptyQ(Queue q){
    return(q.SizeQ == 0);
}

int IsFullQ(Queue q){
    return (q.SizeQ == MAX);
}
