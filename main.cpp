/*
   Main program for the virtual memory project.
   Make all of your modifications to this file.
   You may add or rearrange any code or data as you need.
   The header files page_table.h and disk.h explain
   how to use the page table and disk interfaces.
   */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <queue>

#include "page_table.h"
#include "disk.h"
#include "program.h"


int algorithmCode = 0; // 0 for rand, 1 for fifo, 2 for custom
int framesInTable = 0; // keep track of how many pages have r/w bits set

std::vector<int> frameStatus; // vector will keep track of all frames and their current r/w bits 
std::queue<int> frameAssignmentQueue; // keeps track of which frames are assigned in which order for FIFO

void page_fault_handler(struct page_table *pt, int page)
{
 //TODO distinguish between the types of page fault by looking at bitmasked values in frameStatus
  int targetFrame;
  if (framesInTable < page_table_get_nframes(pt)) { // for any strategy, we put the initial n pages into the first n frames (compulsory misses) 
    targetFrame = framesInTable;
    page_table_set_entry(pt, page, targetFrame, PROT_READ|PROT_WRITE);
    printf("putting page: %d in frame: %d\n", page, targetFrame);
    frameAssignmentQueue.push(targetFrame);
    frameStatus[targetFrame] = PROT_READ; 
    framesInTable++;
  } else { // once frames are at capacity we need to evict one to resolve the fault; employ the strategy specified in launch arguments
    switch(algorithmCode) {
      case 0:
        targetFrame = rand() % page_table_get_nframes(pt); // gets random frame
         
        break;
        // TODO implement rand
      case 1:
        break;
        // TODO implement fifo
      case 2:
        break;
        // TODO implement custom
    }
  }

  //page_table_set_entry(pt, page, page, PROT_READ|PROT_WRITE);
  //exit(1);
  //printf("page fault on page #%d\n", page);
  //exit(1);
}

int main(int argc, char *argv[])
{
  if (argc != 5) {
    printf
      ("use: virtmem <npages> <nframes> <rand|fifo|lru|custom> <sort|scan|focus>\n");
    return 1;
  }

  int npages = atoi(argv[1]);
  int nframes = atoi(argv[2]);
  const char *program = argv[4];
  const char *algorithm = argv[3];

  frameStatus.resize(nframes, 0); // initializes frame status vector   

  struct disk *disk = disk_open("myvirtualdisk", npages);
  if (!disk) {
    fprintf(stderr, "couldn't create virtual disk: %s\n",
        strerror(errno));
    return 1;
  }


  struct page_table *pt =
    page_table_create(npages, nframes, page_fault_handler);
  if (!pt) {
    fprintf(stderr, "couldn't create page table: %s\n",
        strerror(errno));
    return 1;
  }

  char *virtmem = page_table_get_virtmem(pt);

  char *physmem = page_table_get_physmem(pt);

  if(!strcmp(algorithm, "rand")) {
    algorithmCode = 0;
  } else if(!strcmp(algorithm,"fifo")) {
    algorithmCode = 1;
  } else if(!strcmp(algorithm,"custom")) {
    algorithmCode = 2;
  } else {
    fprintf(stderr,"unknown algorithm: %s\n",argv[3]);
    return 1;
  }


  if (!strcmp(program, "sort")) {
    sort_program(virtmem, npages * PAGE_SIZE);

  } else if (!strcmp(program, "scan")) {
    scan_program(virtmem, npages * PAGE_SIZE);

  } else if (!strcmp(program, "focus")) {
    focus_program(virtmem, npages * PAGE_SIZE);

  } else {
    fprintf(stderr, "unknown program: %s\n", argv[3]);
    return 1;
  }

  page_table_delete(pt);
  disk_close(disk);

  return 0;
}
