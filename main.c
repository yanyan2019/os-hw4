#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>

/* The structure of this program is base on the sample code provided in Assignment 4 handout
 * and the exploration: Synchronization Mechanisms Beyond Mutex on Cnavas
 */

// buffer size
#define BF_LINE 50
#define LINE_SIZE 1000

// output size at the end
#define OUT_SIZE 80

// Value for exiting all the threads
#define END_MARKER "STOP\n"

// Buffer 1 for input thread and line separator thread
char buffer_1[BF_LINE*LINE_SIZE];
// total count of items in Buffer 1
int count_1 = 0;
// for input thread to trace the next input's location in Buffer 1
int input_index = 0;
// for line separator to trace where to pick up in Buffer 1
int lineSep_index = 0;
// Initialize the mutex for Buffer 1
pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 1
pthread_cond_t full_1 = PTHREAD_COND_INITIALIZER;



// Buffer 2 for line separator and plus sign thread
char buffer_2[BF_LINE*LINE_SIZE];
//init_buffer(buffer_2);
//total count of items in Buffer 2
int count_2 = 0;
// for line separator to find the location of next input in Buffer 2
int lineSep_index2 = 0;
// for plus sign thread to find where to pick up in Buffer 2
int plus_index = 0;
// initialize the mutex for Buffer 2
pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;
// initialize the condition variable for buffer 2
pthread_cond_t full_2 = PTHREAD_COND_INITIALIZER;



// Buffer 3 for plus sign thread and output thread
char buffer_3[BF_LINE*LINE_SIZE];
// total count of characters in Buffer 3
int count_3 = 0;
// for plus sign thread to find where to input in Buffer 3
int plus_index2 = 0;
// for output thread to find where to pick up in Buffer 3
int out_index = 0;
// initialize the mutex for Buffer 3
pthread_mutex_t mutex_3 = PTHREAD_MUTEX_INITIALIZER;
// initialize the condition variable for buffer 3
pthread_cond_t full_3 = PTHREAD_COND_INITIALIZER;



// function to put input to buffer 1
void put_buffer_1(char * temp){
	// lock mutex before putting into  the buffer
	pthread_mutex_lock(&mutex_1);
	// put the value into the buffer
	strcat(buffer_1, temp);

	//printf("buffer_1: %s\n", buffer_1);	
	
	// update input index and total count
	input_index += strlen(temp);
	count_1 = strlen(buffer_1);
	// signal to the line separator that the buffer is not empty
	pthread_cond_signal(&full_1);
	// unlock mutex
	pthread_mutex_unlock(&mutex_1);
}

// function for input thread to start
void* get_input(){
	char * temp;
	char *pch;
	size_t len;
	ssize_t  nread;
	int pos = -1;
	do{
	// loop until the content is STOP 
		// read line from stdin
		nread = getline(&temp, &len, stdin);
		//fflush(stdin);
		// put value into Buffer 1
		put_buffer_1(temp);
		if(pch){
			pos = pch - temp;	
		}
	}
	while((pch = strstr(temp, END_MARKER)) == NULL && temp[pos-1] != '\n');
}

//function to get output from buffer 1
void get_buffer_1(char temp[]){
	// lock mutex before getting the data
	pthread_mutex_lock(&mutex_1);	
	while(count_1 == 0){
		pthread_cond_wait(&full_1,&mutex_1);
	}
	strcpy(temp,buffer_1);

	// update total count and output index
	lineSep_index = strlen(buffer_1);;
	count_1 -= strlen(buffer_1);
	// unlock mutex
	pthread_mutex_unlock(&mutex_1);
}

// function to replace newline character with space character
void replace(char temp[]){
	char *pch;
	int i, pos = -1;

	if((pch = strstr(temp, END_MARKER)) != NULL){
		pos = pch - temp;
		//printf("pos: %i\n", pos);
		//printf("character: %c\n", temp[pos]);
		//return;	
	}

		// loop through the line to find newline character
		for(i = 0; i < strlen(temp); i++){
			//intf("found in index: %i\n", i);
			// replace '\n' with ' '
			if(temp[i] == '\n' && i != (pos+4)){
				//printf("found in index: %i\n", i);

				temp[i] = ' ';
			}
		}
	
}

// function to put item to buffer 2
void put_buffer_2(char temp[]){
	// lock mutex before putting the data into the buffer
	pthread_mutex_lock(&mutex_2);
	// put item to buffer 2
	strcpy(buffer_2, temp);
	//printf("buffer_2:%s\n ", buffer_2);
	// update total count and index
	lineSep_index2 += strlen(buffer_2);
	count_2 = strlen(buffer_2);
	// signal to plus sign thread that buffer_2 is not empty
	pthread_cond_signal(&full_2);
	// unlock mutex
	pthread_mutex_unlock(&mutex_2);
}	

//function for line separator to start
void* line_sep(){
	char temp[BF_LINE*LINE_SIZE];
	memset(temp ,'\0', sizeof(temp));
	int result;
	char * pch;
//	printf("\ntemp  in lineSep: %s\n", temp);
	while((pch = strstr(temp, END_MARKER)) == NULL){	
		// get from buffer 1
		get_buffer_1(temp);		
		// replace '\n' with ' '
		replace(temp);
		// put to buffer 2
		//printf("temp after replace :%s\n", temp);
		put_buffer_2(temp);
	}
}

// function to get input from buffer 2
void get_buffer_2(char temp[]){
	// lock before getting input
	pthread_mutex_lock(&mutex_2);
	while(count_2 == 0){
		pthread_cond_wait(&full_2, &mutex_2);
	}
	strcpy(temp, buffer_2);
	//printf("buffer_2:%s\n",buffer_2 );
	plus_index = strlen(buffer_2);
	count_2 -= strlen(buffer_2);
	pthread_mutex_unlock(&mutex_2);	
}

// function to replace plus
void replace_plus(char temp[]){
	int i, j;
	for(i = 0; i < strlen(temp); i++){
		if(temp[i] == '+'){
			if(i+1 < strlen(temp) && temp[i+1] == '+'){
				
				// replace ++ with ^
				temp[i] = '^';
				// update array
				for(j = i+1; j < strlen(temp); j++){
					temp[j] = temp[j+1];
				}
			}
		}
	}
}

// function to input to buffer 3
void put_buffer_3(char temp[]){
	// lock	mutex before inputting data
	pthread_mutex_lock(&mutex_3);
	// store data into buffer 3
	strcat(buffer_3, temp);
	// update total count and input index
	plus_index2 += strlen(temp);
	count_3 = strlen(buffer_3);
	//set condtion
	pthread_cond_signal(&full_3);
	// unlock mutex
	pthread_mutex_unlock(&mutex_3);
}

// function for plus sign to start with
void* plus_sign(){
	char temp[LINE_SIZE*BF_LINE];
	int result;
	char* pch;
	while((pch = strstr(temp, END_MARKER)) == NULL){
		// get input from buffer2
		get_buffer_2(temp);		
		// replace ++ with ^		
		replace_plus(temp);
		//printf("temp: after plus: %s\n", temp);
		// put item to buffer 3
		put_buffer_3(temp);
		//printf("buffer_3: %s, strlen of buffer: %i\n\n", buffer_3, strlen(buffer_3));
	}
}

// function to get input from buffer 3
void get_buffer_3(char array[]){
	int i, j = 0;
	// lock before getting data
	pthread_mutex_lock(&mutex_3);

	while(count_3 == 0){
		pthread_cond_wait(&full_3, &mutex_3);
	}
	
	// get values
	//memset(array, '\0', sizeof(array));
	strcpy(array, buffer_3);
	//printf("array: %s\n", array);
	//fflush(stdout);
	//unlock mutex
	pthread_mutex_unlock(&mutex_3);	
}

//function for output thread to start with
void* output(){
	int i;
	// array for output
	char out_array[BF_LINE* LINE_SIZE];
	
	// check for end marker
	char *pch; 
	// get data from buffer 3
	do{
		get_buffer_3(out_array);
	}
  	while((pch = strstr(out_array, END_MARKER)) == NULL);
	//	get_buffer_3(out_array);
	//}
	
	// remove STOP\n
	for(i = (strlen(out_array)- 5); i < strlen(out_array); i += 5){
		out_array[i] = '\0';
	}
		
	int mod = strlen(out_array) % 80;
	int div = strlen(out_array)/80;

	int j, count = 0, total_ct = strlen(out_array);
//printf("Final array: %s, strlen: %i\n", out_array, strlen(out_array) );

	for(j = 0; j < strlen(out_array); j++){
		//printf("j: %i\n",j);
		printf("%c", out_array[j]);
        	fflush(stdout);
		count++;
		if(count % OUT_SIZE == 0 && count != 0){
			//printf("\ncount: %i\n", count);	
			printf("\n");
			fflush(stdout);
			count = 0;
			total_ct -= OUT_SIZE;
		}
		if(total_ct < OUT_SIZE){
			//printf("count-j %i\n",count_3-j);
			break;
		}

	}
	printf("\n");	
}

// main function
int main(){
	pthread_t input_t, line_t, plus_t, output_t;
	
	// create threads
	pthread_create(&input_t, NULL, get_input, NULL);
	pthread_create(&line_t, NULL, line_sep, NULL);	
	pthread_create(&plus_t, NULL, plus_sign, NULL);
	pthread_create(&output_t, NULL, output, NULL);

	// wait for thread to terminate
	pthread_join(input_t, NULL);
	pthread_join(line_t, NULL);
	pthread_join(plus_t, NULL);
	pthread_join(output_t,NULL);

	return EXIT_SUCCESS;
}
