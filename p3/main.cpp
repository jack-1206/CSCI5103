/*
Main program for the virtual memory project.
Make all of your modifications to this file.
You may add or rearrange any code or data as you need.
The header files page_table.h and disk.h explain
how to use the page table and disk interfaces.
*/

#include "page_table.h"
#include "disk.h"
#include "program.h"

#include <cassert>
#include <iostream>
#include <string.h>

using namespace std;

// Prototype for test program
typedef void (*program_f)(char *data, int length);


// Number of physical frames
int nframes;

// Pointer to disk for access from handlers
struct disk *disk = nullptr;


// Simple handler for pages == frames
void page_fault_handler_example(struct page_table *pt, int page) {
	cout << "page fault on page #" << page << endl;

	// Print the page table contents
	cout << "Before ---------------------------" << endl;
	page_table_print(pt);
	cout << "----------------------------------" << endl;

	// Map the page to the same frame number and set to read/write
	// TODO - Disable exit and enable page table update for example
	exit(1);
	//page_table_set_entry(pt, page, page, PROT_READ | PROT_WRITE);

	// Print the page table contents
	cout << "After ----------------------------" << endl;
	page_table_print(pt);
	cout << "----------------------------------" << endl;
}

// TODO - Handler(s) and page eviction algorithms
void rand_handler(struct page_table *pt, int page) {
	int frame,bits;
	page_table_get_entry(pt, page, &frame, &bits);
	if (!(bits&PROT_READ)){
		//find the frist free frame if available
		bool free_frame_flag;
		int free_frame_index;
		//loop through page table
		for (int frame_index = 0; frame_index < pt->nframes; frame_index++){
			free_frame_flag = true;
			for (int i = 0; i < pt->npages; i++){
				int f,b;
				page_table_get_entry(pt, i, &f, &b);
				if (b&PROT_READ){
					//if the bit is marked read, check if it is in physical memory at the same time
					//if so, then the frame is not free
					if (f==frame_index){
						free_frame_flag = false;
						break;
					}
				}
			}
			if(free_frame_flag){
				//a free frame is found
				free_frame_index = frame_index;
				break;
			}
		}
		if (free_frame_flag) 
		{
			//simple page reading, no eviction
			page_table_set_entry(pt, page, free_frame_index, PROT_READ); 
			disk_read(disk, page, &pt->physmem[free_frame_index*PAGE_SIZE]);
		}
		else
		{
			//randomly evict a page
			int evicted_page, ff,bb;
			while(1){
				evicted_page = rand() % pt->npages;
				page_table_get_entry(pt, evicted_page, &ff, &bb);
				if(bb&PROT_READ){
					//checking dirty bit
					if(bb&PROT_WRITE){
						disk_write(disk, evicted_page, &pt->physmem[ff*PAGE_SIZE]);
					}
					break;
				}
			}	
			
			disk_read(disk, page, &pt->physmem[ff*PAGE_SIZE]);
			page_table_set_entry(pt, page, ff, PROT_READ);
			page_table_set_entry(pt, evicted_page, ff, 0);
		} 
	}else{
		if(!(bits&PROT_WRITE))//if read then OR write
		{
			page_table_set_entry(pt, page, frame, PROT_READ|PROT_WRITE);			
		}
	}
}

int main(int argc, char *argv[]) {
	// Check argument count
	if (argc != 5) {
		cerr << "Usage: virtmem <npages> <nframes> <rand|fifo|custom> <sort|scan|focus>" << endl;
		exit(1);
	}

	// Parse command line arguments
	int npages = atoi(argv[1]);
	nframes = atoi(argv[2]);
	const char *algorithm = argv[3];
	const char *program_name = argv[4];

	// Validate the algorithm specified
	if ((strcmp(algorithm, "rand") != 0) &&
	    (strcmp(algorithm, "fifo") != 0) &&
	    (strcmp(algorithm, "custom") != 0)) {
		cerr << "ERROR: Unknown algorithm: " << algorithm << endl;
		exit(1);
	}

	// Validate the program specified
	program_f program = NULL;
	if (!strcmp(program_name, "sort")) {
		if (nframes < 2) {
			cerr << "ERROR: nFrames >= 2 for sort program" << endl;
			exit(1);
		}

		program = sort_program;
	}
	else if (!strcmp(program_name, "scan")) {
		program = scan_program;
	}
	else if (!strcmp(program_name, "focus")) {
		program = focus_program;
	}
	else {
		cerr << "ERROR: Unknown program: " << program_name << endl;
		exit(1);
	}

	// TODO - Any init needed

	// Create a virtual disk
	disk = disk_open("myvirtualdisk", npages);
	if (!disk) {
		cerr << "ERROR: Couldn't create virtual disk: " << strerror(errno) << endl;
		return 1;
	}

	// Create a page table
	struct page_table *pt = page_table_create(npages, nframes, page_fault_handler_example /* TODO - Replace with your handler(s)*/);
	if (!pt) {
		cerr << "ERROR: Couldn't create page table: " << strerror(errno) << endl;
		return 1;
	}

	// Run the specified program
	char *virtmem = page_table_get_virtmem(pt);
	program(virtmem, npages * PAGE_SIZE);

	// Clean up the page table and disk
	page_table_delete(pt);
	disk_close(disk);

	return 0;
}
