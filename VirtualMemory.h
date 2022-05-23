#pragma once

#include "MemoryConstants2.h"
#include <cmath>
#include <cstdio>
#include "PhysicalMemory.h"
#include <algorithm>

struct PageInfo{
    uint64_t pageNum;
    int frameNum;
    int layer;
    int fatherFrame;
    int pageIndInFatherPage;
};

uint64_t translateAdress(uint64_t virtualAddress);
int findNewFrame(uint64_t virtualAddress, int lookingFrame);
PageInfo choosePageToEvict(PageInfo curFrame, uint64_t destination, int lookingFrame);
PageInfo comparePagesDestinations(PageInfo *p1, PageInfo *p2, uint64_t destination, int lookingFrame);
int biggestFrameNum(int curFrameInd, int layer);
void fillFrameWithZeros(int frameInd);

/*
 * Initialize the virtual memory.
 */
void VMinitialize(){

}


/* Reads a word from the given virtual address
 * and puts its content in *value.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMread(uint64_t virtualAddress, word_t* value){
    auto physicalAddress = translateAdress(virtualAddress);
    PMread(physicalAddress, value);
    return 1;

}
int VMwrite(uint64_t virtualAddress, word_t value){
    auto physicalAddress = translateAdress(virtualAddress);
    uint64_t dest = virtualAddress>> OFFSET_WIDTH;
    PMwrite(physicalAddress, value);
    return 1;
}

uint64_t translateAdress(uint64_t virtualAddress){
    uint64_t dest = virtualAddress>> OFFSET_WIDTH;
    int chunks[TABLES_DEPTH + 1];
    // split to chunks
    for (int layer = TABLES_DEPTH ; layer >= 0; layer--){
        auto chunk = virtualAddress % PAGE_SIZE;
        virtualAddress = virtualAddress >> OFFSET_WIDTH;
        chunks[layer] = chunk;
        //printf("%d\n", chunks[layer]);
    }
    //travel down im tables tree
    word_t nextTable;
    word_t currentTable = 0;
    for (int layer = 0 ; layer < TABLES_DEPTH-1; layer++){
        PMread(currentTable * PAGE_SIZE + chunks[layer], &nextTable);
        if(nextTable == 0){
            auto newFrame = findNewFrame(dest, currentTable);
            fillFrameWithZeros(newFrame);
            PMwrite(currentTable * PAGE_SIZE + chunks[layer], newFrame);
            nextTable = newFrame;
        }
        currentTable = nextTable;
    }

    PMread(currentTable * PAGE_SIZE + chunks[TABLES_DEPTH-1], &nextTable);
    if(nextTable == 0){
        auto newFrame = findNewFrame(dest, currentTable);
        PMrestore(newFrame, dest);
        PMwrite(currentTable * PAGE_SIZE + chunks[TABLES_DEPTH-1], newFrame);
        nextTable = newFrame;
    }

    auto offset = chunks[TABLES_DEPTH];
    return (nextTable<<OFFSET_WIDTH) + offset;

}

void fillFrameWithZeros(int frameInd) {
    for(int ind = 0; ind < PAGE_SIZE; ind++)
    {
        PMwrite(frameInd * PAGE_SIZE + ind, 0 );
    }
    fprintf(stdin,"filling %d ewith zeros", frameInd);
}


int findNewFrame(uint64_t virtualAddress, int lookingFrame){
    //CHECK RAM NOT FULL
    auto biggestFrame = biggestFrameNum(0,0);
    if(biggestFrame != NUM_FRAMES - 1){
        return biggestFrame + 1;
    }
    // RAM IS FULL
    PageInfo rootPage = {0,0,0,0};
    PageInfo chosenPage = choosePageToEvict(rootPage,virtualAddress, lookingFrame);
    if(chosenPage.layer == TABLES_DEPTH){
        PMevict(chosenPage.frameNum,chosenPage.pageNum);
    }
    fillFrameWithZeros(chosenPage.frameNum);
    PMwrite(chosenPage.fatherFrame * PAGE_SIZE + chosenPage.pageIndInFatherPage,0);
    return chosenPage.frameNum;
}

int biggestFrameNum(int curFrameInd, int layer) {
    if (layer == TABLES_DEPTH){
        return curFrameInd;
    }
    int biggestInd = curFrameInd;
    for(int ind = 0; ind < PAGE_SIZE; ind++)
    {
        word_t value;
        PMread(curFrameInd*PAGE_SIZE+ind, &value);
        if(value == 0)
        {
            continue;
        }
        biggestInd = std::max(biggestInd, biggestFrameNum(value, layer+1));
    }
    return biggestInd;
}


PageInfo choosePageToEvict(PageInfo curFrame, uint64_t destination, int lookingFrame) {
    //Check if the page is a leaf
    if(curFrame.layer == TABLES_DEPTH){
        return curFrame;
    }
    int ind;
    //check if the frame has no children = unused
    for(ind = 0; ind < PAGE_SIZE; ind++)
    {
        word_t value;
        PMread(curFrame.frameNum*PAGE_SIZE+ind, &value);
        if(value != 0)
            break;
    }
    if (ind == PAGE_SIZE){
        return curFrame;
    }

    //THE frame has children
    PageInfo bestResult = {};
    for(ind = 0; ind < PAGE_SIZE; ind++){
        word_t value;
        PMread(curFrame.frameNum*PAGE_SIZE+ind, &value);
        if(value == 0){
            continue;
        }
        PageInfo curChild = { (curFrame.pageNum<<OFFSET_WIDTH)+ind, value , curFrame.layer+1, curFrame.frameNum, ind};
        auto curResult = choosePageToEvict(curChild, destination, lookingFrame);
        bestResult = comparePagesDestinations(&curResult, &bestResult, destination, lookingFrame);
        }
    return bestResult;
    }

PageInfo comparePagesDestinations(PageInfo *p1, PageInfo *p2, uint64_t destination, int lookingFrame) {
    if(p1->frameNum==0 || p1->frameNum == lookingFrame){
        return *p2;
    }
    if(p2->frameNum==0|| p2->frameNum == lookingFrame){
        return *p1;
    }
    uint64_t d1 = destination - p1->pageNum;
    uint64_t d2 = NUM_PAGES - (destination - p1->pageNum);
    auto p1Score = std::min(d1,d2);
    d1 = destination - p2->pageNum;
    d2 = NUM_PAGES - (destination - p2->pageNum);
    auto p2Score = std::min(d1,d2);
    if(p1->layer < p2->layer){
        return *p1;
    }
    if(p1->layer > p2->layer){
        return *p2;
    }
    if(p1Score > p2Score){
        return *p1;
    }
    return *p2;
}



