#pragma once

#include "MemoryConstants.h"
#include <math.h>
#include <cstdio>
#include "PhysicalMemory.h"

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



    return 0;

}


int findNewFrame(uint64_t virtualAddress);

uint64_t translateAdress(uint64_t virtualAddress){
    uint64_t dest = virtualAddress>> OFFSET_WIDTH;
    int chunks[TABLES_DEPTH + 1];
    for (int layer = TABLES_DEPTH ; layer >= 0; layer--){
        auto chunk = virtualAddress % PAGE_SIZE;
        virtualAddress = virtualAddress >> OFFSET_WIDTH;
        chunks[layer] = chunk;
        printf("%d\n", chunks[layer]);
    }
    word_t nextTable;

    virtualAddress >> OFFSET_WIDTH;
    word_t currentTable = 0;
    for (int layer = 0 ; layer < TABLES_DEPTH; layer++){
        PMread(currentTable * PAGE_SIZE + chunks[layer], &nextTable);
        if(nextTable == 0){
            auto newFrame = findNewFrame(dest);
            PMwrite(currentTable * PAGE_SIZE + chunks[layer], newFrame);
        }
        currentTable = nextTable;
    }
    PMread(currentTable * PAGE_SIZE + chunks[TABLES_DEPTH], &nextTable);
    return nextTable;

}
struct PageInfo{
    u_int64_t pageNum;
    int frameNum;
    
};
int DFS(int i);

PageInfo choosePageToEvict(u_int64_t virtualAddress);

int findNewFrame(uint64_t virtualAddress){
    //CHECK RAM NOT FULL
    auto biggestFrame = biggestFrameNum();
    if(biggestFrame != NUM_FRAMES - 1){
        return biggestFrame + 1;
    }
    // RAM IS FULL
    PageInfo chosenPage = choosePageToEvict(virtualAddress);
    return DFS(0);


    return 0;
}

PageInfo choosePageToEvict(PageInfo curFrame, uint64_t destanation) {
    int ind;
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
        PageInfo curChild = { curFrame.pageNum<<OFFSET_WIDTH +ind, value };
        auto curResult = choosePageToEvict(curChild, destanation);
        if(//comparre 2 pages by cyclic destation){
        )//update best result
        }
    return bestResult;
    }


}

int DFS(int curFrame) {
    int ind = 0;
    for(ind = 0; ind < PAGE_SIZE; ind++)
    {
        word_t value;
        PMread(curFrame*PAGE_SIZE+ind, &value);
        if(value != 0)
            break;
    }
    if (ind == PAGE_SIZE){
        return curFrame;
    }

    // Recur for all the vertices adjacent
    // to this vertex
    int childInd;
    for (childInd = 0; childInd != PAGE_SIZE; ++childInd){
        word_t value;
        PMread(curFrame*PAGE_SIZE+childInd, &value);
        if(value != 0){
            auto unusedFrame = DFS(value);
            if (unusedFrame != 0){
                return unusedFrame;
            }
        }
    }
    return 0;
}
//
//int* splitAdress(uint64_t address) {
//    int res[TABLES_DEPTH+1];
//    for (int layer = TABLES_DEPTH ; layer == 0; layer--){
//        auto chunk = address % PAGE_SIZE;
//        address >> OFFSET_WIDTH;
//        res[layer] = chunk;
//    }
//    return res*;
//}

/* Writes a word to the given virtual address.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMwrite(uint64_t virtualAddress, word_t value);

int main(){
    VMread(61601, reinterpret_cast<word_t *>(3));
}
