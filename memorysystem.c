#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<time.h>
#include<string.h>
#include<stdint.h>

#define MEMSIZE 500
#define MAX_STACK_SIZE 200
#define MAX_HEAP_SIZE 300
#define MAX_FRAMES 5
#define MIN_FRAME_SIZE 10
#define MAX_FRAME_SIZE 80
#define BUFFER_METADATA_SIZE 8

// Frame metadata structure
struct __attribute__((__packed__)) framestatus{
	int number; // frame number
    char name[8]; // function name representing the frame
	int functionaddress; // address of function in code section (will be randomly generated in this case)
	int frameaddress; // starting address of frame belonging to this header in Stack
	uint8_t used; // boolean value to check if frame is used or not
}; 

// Free list structure
struct freelist{
	int start; // start address of free region
	int size; // size of free region
	struct freelist* next; // pointer to next free region
};

// Structure to store regions currently allocated on heap - for performing allocations and deallocations
struct allocated{
	char name[8]; // name of the buffer on heap
	int startaddress; // start address of buffer on heap
	struct allocated* next; // pointer to next allocated buffer
};

/* Global Variables and Initialization */
char memory[MEMSIZE];
struct framestatus frameStatusList[MAX_FRAMES]; // frame status list
int frame_counter, top_StackFrame; // frame counter, stack pointer
struct freelist fl_node;
struct freelist* fl_head = &fl_node; // free list head
struct allocated a_node;
struct allocated* a_head = &a_node; // allocated list head

// Initialization Function
void init(){
	srand(time(NULL)); // seed random number generator
	frame_counter = 0; // initialize frame counter
	top_StackFrame = 393; // initialize stack frame head
	
	/* Initalize the Free List Head */
	fl_head->start = 0;
	fl_head->size = MAX_HEAP_SIZE;
	fl_head->next = NULL; 

	/* Initialize the Allocated List */
	strcpy(a_head->name, "");
	a_head->startaddress = 0;
	a_head->next = NULL;

	/* Copy Contents to memory to complete memory initialization */
	memcpy(&memory[394], &frameStatusList, sizeof(frameStatusList));
}

// Prototypes for functions
void CF(char* functionname, int functionaddress);
void DF();
void CI(char* integername, int integervalue);
void CD(char* doublename, double doublevalue);
void CC(char* charname, char charvalue);
void CH(char* buffername, int size);
void DH(char* buffername);
void SM();

int main(){
	printf("#--------------------Stack and Heap Memory Management--------------------#\n");
	printf("Press Q or q to quit the shell\n"); init();
	while(true){
		char input[3], name[8];
		int functionaddress, intval, buffer_size;
		double doubleval;
		char charval;
		printf("prompt>>>");
		scanf("%s", input);
		if(strcmp(input, "CF") == 0){
			scanf("%s %d", name, &functionaddress);
			CF(name, functionaddress);
		}
		else if(strcmp(input, "DF") == 0) DF();
		else if(strcmp(input, "CI") == 0){
			scanf("%s %d", name, &intval);
			CI(name, intval);
		}
		else if(strcmp(input, "CD") == 0){
			scanf("%s %lf", name, &doubleval);
			CD(name, doubleval);
		}
		else if(strcmp(input, "CC") == 0){
			scanf("%s %c", name, &charval);
			CC(name, charval);
		}
		else if(strcmp(input, "CH") == 0){
			scanf("%s %d", name, &buffer_size);
			CH(name, buffer_size);
		}
		else if(strcmp(input, "DH") == 0){
			scanf("%s", name);
			DH(name);
		}
		else if(strcmp(input, "SM") == 0) SM();
		else if(strcmp(input, "Q") == 0 || strcmp(input, "q") == 0) exit(EXIT_SUCCESS);
		else printf("Invalid input, please try again\n");
	}
	return 0;
}

int var_errors(int var_type){
	if(frame_counter == 0){
		fprintf(stderr, "Error: Stack is empty, no frames to create variable in\n"); return -1;
	}
	if(var_type == 1){
		if(top_StackFrame - sizeof(int) < 300){
			fprintf(stderr, "Error: Stack limit reached, not enough memory to create an integer\n"); return -1;
		}
		if(frameStatusList[frame_counter - 1].frameaddress - (top_StackFrame - sizeof(int)) > MAX_FRAME_SIZE){
			fprintf(stderr, "Error: Frame is full, cannot create more data on it\n"); return -1;
		}
	}
	else if(var_type == 2){
		if(top_StackFrame - sizeof(double) < 300){
			fprintf(stderr, "Error: Stack limit reached, not enough memory to create a double\n"); return -1;
		}
		if(frameStatusList[frame_counter - 1].frameaddress - (top_StackFrame - sizeof(double)) > MAX_FRAME_SIZE){
			fprintf(stderr, "Error: Frame is full, cannot create more data on it\n"); return -1;
		}
	}
	else if(var_type == 3){
		if(top_StackFrame - sizeof(char) < 300){
			fprintf(stderr, "Error: Stack limit reached, not enough memory to create a char\n"); return -1;
		}
		if(frameStatusList[frame_counter - 1].frameaddress - (top_StackFrame - sizeof(char)) > MAX_FRAME_SIZE){
			fprintf(stderr, "Error: Frame is full, cannot create more data on it\n"); return -1;
		}
	} return 0;
}

void CF(char* functionname, int functionaddress){
	if(strlen(functionname) > 8){
		fprintf(stderr, "Error: Function name exceeds 8 characters\n"); return;
	}
	if(MEMSIZE - (top_StackFrame + 1) + MIN_FRAME_SIZE > MAX_STACK_SIZE){
		fprintf(stderr, "Error: Stack Overflow, not enough memory available for new function\n"); return;
	}
	if(frame_counter == MAX_FRAMES){
		fprintf(stderr, "Error: Cannot create another frame, maximum number of frames have been reached\n"); return;
	}
	for(int i = 0; i < frame_counter; i++){
		if(strcmp(frameStatusList[i].name, functionname) == 0){
			fprintf(stderr, "Error: Function already exists with the given name\n"); return;
		}
	}
	frameStatusList[frame_counter].used = true;
	frameStatusList[frame_counter].number = frame_counter;
	strncpy(frameStatusList[frame_counter].name, functionname, sizeof(frameStatusList[frame_counter].name));
	if(frame_counter > 0 && ((frameStatusList[frame_counter - 1].frameaddress - top_StackFrame) < 10)) top_StackFrame = frameStatusList[frame_counter - 1].frameaddress - 10;
	frameStatusList[frame_counter].functionaddress = functionaddress;
	frameStatusList[frame_counter].frameaddress = top_StackFrame;
	memcpy(&memory[394], &frameStatusList, sizeof(frameStatusList));
	frame_counter++;
	printf("Created frame %s with address %d\n", functionname, functionaddress); return;
}

void DF(){
	if(frame_counter == 0){
		fprintf(stderr, "Error: Stack is empty, no frames to delete\n"); return;
	}
	for(int i = top_StackFrame; i < frameStatusList[frame_counter - 1].frameaddress; i++) memory[i] = 0;
	if(frame_counter == 1) top_StackFrame = 393;
	else top_StackFrame = frameStatusList[frame_counter - 1].frameaddress;
	frame_counter--;
	frameStatusList[frame_counter] = (struct framestatus){0};
	memcpy(&memory[394], &frameStatusList, sizeof(frameStatusList));
	printf("Deleted Frame\n"); return;
}

void CI(char* integername, int integervalue){
	if(var_errors(1) == -1) return;
	top_StackFrame -= sizeof(int);
	memcpy(&memory[top_StackFrame], &integervalue, sizeof(int));
	printf("Created integer %s with value %d\n", integername, integervalue); return;
}

void CD(char* doublename, double doublevalue){
	if(var_errors(2) == -1) return;
	top_StackFrame -= sizeof(double);
	memcpy(&memory[top_StackFrame], &doublevalue, sizeof(double));
	printf("Created double %s with value %lf\n", doublename, doublevalue); return;
}

void CC(char* charname, char charvalue){
	if(var_errors(3) == -1) return;
	top_StackFrame -= sizeof(char);
	memcpy(&memory[top_StackFrame], &charvalue, sizeof(char));
	printf("Created char %s with value %c\n", charname, charvalue); return;
}

void CH(char* buffername, int size){
	if(size <= 0){
		fprintf(stderr, "Error: Size of buffer cannot be less than or equal to 0\n"); return;
	}
	int total_size = size + BUFFER_METADATA_SIZE;
	
	if(frame_counter == 0){
		fprintf(stderr, "Error: Stack is empty, no frames to create pointer to buffer in\n"); return;
	}
	if(top_StackFrame - sizeof(int) < 300){
		fprintf(stderr, "Error: Stack limit reached, not enough memory to create a pointer to buffer\n"); return;
	}
	if(frameStatusList[frame_counter - 1].frameaddress - (top_StackFrame - sizeof(int)) > 80){
		fprintf(stderr, "Error: Frame is full, cannot create more data on it\n"); return;
	}

	struct freelist* curr = fl_head;
	while(curr != NULL && curr->size < total_size) curr = curr->next;
	if(curr == NULL){
		fprintf(stderr, "Error: The heap is full, cannot create more data on it\n"); return;
	}
	int magic = rand(), heapStart = curr->start;
	memcpy(&memory[heapStart], &size, sizeof(int));
	memcpy(&memory[heapStart + sizeof(int)], &magic, sizeof(int));	
	for(int i = heapStart + BUFFER_METADATA_SIZE; i < heapStart + total_size; i++) memory[i] = 'a' + (rand() % 26);
	
	curr->start += total_size;
	curr->size -= total_size;

	struct allocated* newAlloc = (struct allocated*) malloc(sizeof(struct allocated));
	strcpy(newAlloc->name, buffername);
	newAlloc->startaddress = heapStart;
	newAlloc->next = a_head->next;
	a_head->next = newAlloc;

	top_StackFrame -= sizeof(int);
	int bufferAddress = heapStart;
	memcpy(&memory[top_StackFrame], &bufferAddress, sizeof(int));

	printf("Created buffer %s with size %d\n", buffername, size); return;	
}

void DH(char* buffername){
	if(a_head == NULL){
		fprintf(stderr, "Error: The pointer is NULL or already de-allocated\n"); return;
	}
	struct allocated* prev = NULL, *curr = a_head;
	while(curr != NULL && strcmp(curr->name, buffername) != 0){
		prev = curr; curr = curr->next;
	}
	if(curr == NULL){
		fprintf(stderr, "Error: The pointer is NULL or already de-allocated\n"); return;
	}
	int bufferSize = *(int*)(memory + curr->startaddress);
	memset(memory + curr->startaddress, 0, bufferSize + BUFFER_METADATA_SIZE);
	
	if(prev != NULL) prev->next = curr->next;
	else a_head = curr->next;

	struct freelist* newFree = (struct freelist*) malloc(sizeof(struct freelist));
	newFree->start = curr->startaddress;
	newFree->size = bufferSize + BUFFER_METADATA_SIZE;
	newFree->next = fl_head;
	fl_head = newFree;

	free(curr);

	printf("De-allocated buffer %s\n", buffername); return;
}

void printFreeList(){
	struct freelist* curr = fl_head;
	printf("Free List: \n");
	while(curr != NULL){
		printf("Start: %d, Size: %d\n", curr->start, curr->size);
		curr = curr->next;
	}
	printf("\n");
}

void printAllocatedList(){
	struct allocated* curr = a_head->next;
	printf("Allocated List: \n");
	while(curr != NULL){
		printf("Name: %s, Start Address: %d\n", curr->name, curr->startaddress);
		curr = curr->next;
	}
	printf("\n");
}

void SM(){
	for(int i = MEMSIZE; i >= 0; i--)
		printf("%d: %X \n",i,  memory[i]);

	printFreeList(); printAllocatedList();

    printf("#-------------------------------------------------------------#\n");
}