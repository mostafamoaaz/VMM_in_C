#include "global.h"
#include "queue.h"
#include "queue.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int physicalMemory [256][256];
int pageFaultCounter = 0;
int tlbHitCounter = 0;
int pageTableCounter = 0;
int addressReadCounter = 0;
int FAFrame = 0;
Queue tlb;
Queue pageTable;



int checkTLB(int pageNumber, int offset, int logicalAddress, int addressReadCounter,FILE* outputFile);
int checkPageTable(int pageNumber, int offset, int logicalAddress, int addressReadCounter,FILE* outputFile);
int pageFaultHandler(int pageNumber);
int updateTLB(int pageNumber, int frameNumber);
int updatePageTable(int pageNumber, int frameNumber);
int updateTLBCounter(int latestEntryIndex);
int updatepageTableCounter(int latestEntryIndex);
int readPhysicalMemory(int frameNumber,int offset);

int main()
{
    CreateQueue(&tlb);
    CreateQueue(&pageTable);
    FILE *addressFile = fopen("addresses.txt","r");
    FILE *outputFile = fopen("output.txt","w");
    if (addressFile == NULL){
        printf("error opening addresses file\n");
        return 0;
    }
    char line[8];
    int tlbHit = 0;
    int pageTableTrue = 0;
    while (fgets(line ,sizeof(line) ,addressFile)!= NULL){
        printf("\n\n\ntlbhitcounter : %d\npagefaultcounter : %d\n\n\n",tlbHitCounter,pageFaultCounter);
        int logicalAddress = atoi(line);
        int offset = logicalAddress & 255;
        int pageNumber = (logicalAddress & 65280) >> 8;
        printf("%d-Logical address is: %d\nPageNumber is: %d\nOffset: %d\n",addressReadCounter,logicalAddress,pageNumber,offset);
        addressReadCounter += 1 ;
        tlbHit = checkTLB(pageNumber, offset, logicalAddress, addressReadCounter, outputFile);
        if (tlbHit == 1){
            tlbHitCounter += 1;
        }else{
            pageTableTrue = checkPageTable(pageNumber, offset, logicalAddress, addressReadCounter, outputFile);
        }
        if (pageTableTrue != 1 && tlbHit != 1){
            printf("This is a page fault!\n");
            pageFaultHandler(pageNumber);
            checkTLB(pageNumber, offset, logicalAddress, addressReadCounter, outputFile);
        }
    }

    float pageFaultRate = pageFaultCounter / addressReadCounter ;
    float tlbHitRate = tlbHitCounter / addressReadCounter ;
    fprintf(outputFile, "Number of translated address: %d\n", addressReadCounter);
    fprintf(outputFile, "Number of page fault: %d\n", pageFaultCounter);
    fprintf(outputFile, "Page fault rate: %f\n", pageFaultRate);
    fprintf(outputFile, "Number of TLB hits: %d\n", tlbHitCounter);
    fprintf(outputFile, "TLB hit rate: %f\n", tlbHitRate);

    fclose(outputFile);
    fclose(addressFile);

    return 0;
}

int checkTLB( int pageNumber, int offset, int logicalAddress, int addressReadCounter, FILE* outputFile){
    for(int i = 0; i <= 16 ; i++){
        entry x;
        Dequeue(&tlb , &x);
        if(pageNumber==x.pageNumber){
                printf("Page Number %d found in TLB!!\n",pageNumber);
                int frameNumber = x.frameNumber;
                Enqueue(&tlb , x);
                int data =readPhysicalMemory(frameNumber, offset);
                int physicalAddress = (frameNumber << 8) | offset;
                fprintf(&outputFile,"%d Virtual address: %d Physical address: %d Value: %d\n",addressReadCounter,logicalAddress,physicalAddress,data);
                printf("%d Virtual address: %d Physical address: %d Value: %d\n",addressReadCounter,logicalAddress,physicalAddress,data);
                updateTLBCounter(i);
                return 1;
        }
    }
        return 0;
}

int checkPageTable(int pageNumber, int offset, int logicalAddress, int addressReadCounter,FILE* outputFile){
    for(int i = 0; i <= 256 ; i++){
        entry x;
        Dequeue(&pageTable , &x);
        if(pageNumber==x.pageNumber){
                printf("Page Number %d found in pageTable!!\n",pageNumber);
                int frameNumber = x.frameNumber;
                Enqueue(&pageTable , x);
                int data =readPhysicalMemory(frameNumber, offset);
                int physicalAddress = (frameNumber << 8) | offset;
                fprintf(&outputFile,"%d Virtual address: %d Physical address: %d Value: %d\n",addressReadCounter,logicalAddress,physicalAddress,data);
                printf("%d Virtual address: %d Physical address: %d Value: %d\n",addressReadCounter,logicalAddress,physicalAddress,data);
                updatepageTableCounter(i);
                return 1;
        }
    }
    return 0;
}

int pageFaultHandler(int pageNumber){
    if (pageNumber < 256){
        int frameNumber = FAFrame;
        FILE* binaryFile = fopen("BACKING_STORE.bin","rb");
        unsigned char buffer[256];
        fread(buffer, sizeof(buffer), pageNumber*256, binaryFile);
        for( int i = 0 ; i < 256 ; i++){
            physicalMemory[FAFrame][i] = buffer[i];
        }
        fclose(binaryFile);
        FAFrame++;
        printf("Found page \"%d\" in the backing store!\n",pageNumber);
        updateTLB(pageNumber, frameNumber);
        updatePageTable(pageNumber, frameNumber);

    }else{
        printf("Page \"%d\" is out of bound!\n");
        return;
    }
}

int updateTLB(int pageNumber, int frameNumber){
    entry newEntry,oldEntry;
    newEntry.pageNumber = pageNumber;
    newEntry.frameNumber = frameNumber;
    if (GetSize(tlb) < 16){
        Enqueue(&tlb,newEntry);
    }else{
        Dequeue(&tlb,&oldEntry);
        Enqueue(&tlb,newEntry);
    }
    printf("Successfully update TLB with pageNumber:  %d, frameNumber: %d!\n",pageNumber,frameNumber);

}

int updatePageTable(int pageNumber, int frameNumber){
    entry newEntry,oldEntry;
    newEntry.pageNumber = pageNumber;
    newEntry.frameNumber = frameNumber;
    if (GetSize(pageTable) < 256){
        Enqueue(&pageTable,newEntry);
    }else{
        Dequeue(&pageTable,&oldEntry);
        Enqueue(&pageTable,newEntry);
    }
    printf("Successfully update pageTable with pageNumber:  %d, frameNumber: %d!\n",pageNumber,frameNumber);

}

int updateTLBCounter(int latestEntryIndex){
    entry recentEntry;
    int size = GetSize(tlb);
    entry array[16];
    int i = 0;
    while(!IsEmptyQ(tlb)){
        Dequeue(&tlb ,&recentEntry);
        array[i] = recentEntry;
        i += 1;
    }
    i = 0;
    while(i != size){
        if(i == latestEntryIndex){
            i++;
            continue;
        }
        Enqueue(&tlb,array[i]);
        i += 1;
    }
    Enqueue(&tlb,array[latestEntryIndex]);
    printf("Successfully update TLB with new sequence using LRU!\n");
}


int updatepageTableCounter(int latestEntryIndex){
    entry recentEntry;
    int size = GetSize(pageTable);
    entry array[256];
    int i = 0;
    while(!IsEmptyQ(pageTable)){
        Dequeue(&pageTable ,&recentEntry);
        array[i] = recentEntry;
        i += 1;
    }
    i = 0;
    while(i != size){
        if(i == latestEntryIndex){
            i++;
            continue;
        }
        Enqueue(&pageTable,array[i]);
        i += 1;
    }
    Enqueue(&pageTable,array[latestEntryIndex]);
    printf("Successfully update PageTable with new sequence using LRU!\n");
}

int readPhysicalMemory(int frameNumber,int offset){
    if (frameNumber < 256 && offset < 256){
        int data = physicalMemory[frameNumber][offset];
        printf("Successfully read frameNumber: \"%d\" offset: \"%d\"\'s data: \"%d\" in physical memory \n",frameNumber,offset,data);
        return data;
    }
    return 0;
}
