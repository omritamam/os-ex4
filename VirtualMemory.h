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
    long long int fatherLinkAddress;
};

uint64_t translateAdress(uint64_t virtualAddress);
int findNewFrame(uint64_t virtualAddress);
PageInfo choosePageToEvict(PageInfo curFrame, uint64_t destination);
PageInfo comparePagesDestinations(PageInfo *p1, PageInfo *p2, uint64_t destination);
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
    for (int layer = 0 ; layer < TABLES_DEPTH; layer++){
        PMread(currentTable * PAGE_SIZE + chunks[layer], &nextTable);
        if(nextTable == 0){
            auto newFrame = findNewFrame(dest);
            fillFrameWithZeros(newFrame);
            PMwrite(currentTable * PAGE_SIZE + chunks[layer], newFrame);
            nextTable = newFrame;
        }
        currentTable = nextTable;
    }
    PMrestore(nextTable, dest);
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


int findNewFrame(uint64_t virtualAddress){
    //CHECK RAM NOT FULL
    auto biggestFrame = biggestFrameNum(0,0);
    if(biggestFrame != NUM_FRAMES - 1){
        return biggestFrame + 1;
    }
    // RAM IS FULL
    PageInfo rootPage = {0,0,0,0};
    PageInfo chosenPage = choosePageToEvict(rootPage,virtualAddress);
    PMevict(chosenPage.frameNum,chosenPage.pageNum);
    PMwrite(chosenPage.fatherLinkAddress,0);
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


PageInfo choosePageToEvict(PageInfo curFrame, uint64_t destination) {
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
        long long int x;
        x = curFrame.frameNum * PAGE_SIZE + ind;
        PageInfo curChild = { (curFrame.pageNum<<OFFSET_WIDTH)+ind, value , curFrame.layer+1, x};
        auto curResult = choosePageToEvict(curChild, destination);
        bestResult = comparePagesDestinations(&curResult, &bestResult, destination);
        }
    return bestResult;
    }

PageInfo comparePagesDestinations(PageInfo *p1, PageInfo *p2, uint64_t destination) {
    if(p1->frameNum==0){
        return *p2;
    }
    if(p2->frameNum==0){
        return *p1;
    }
    uint64_t d1 = destination - p1->pageNum;
    uint64_t d2 = NUM_PAGES - (destination - p1->pageNum);
    auto p1Score = std::min(d1,d2);
    d1 = destination - p2->pageNum;
    d2 = NUM_PAGES - (destination - p2->pageNum);
    auto p2Score = std::min(d1,d2);
    if(p1Score < p2Score){
        return *p1;
    }
    return *p2;
}



