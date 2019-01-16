#ifndef GLOBAL_H_INCLUDED
#define GLOBAL_H_INCLUDED


#define MAX 256

typedef struct{
    int pageNumber;
    int frameNumber;

}entry;

typedef struct {
    int Front, Rear, SizeQ;
    entry arrayq[MAX];
}Queue;


#endif // GLOBAL_H_INCLUDED
