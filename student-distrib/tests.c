#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "idt_handler.h"
#include "i8259.h"
#include "paging.h"
#include "filesys.h"
#include "rtc.h"
#include "terminal.h"
#include "keyboard.h"
#include "system_call.h"

#define PASS 				1
#define FAIL 				0
#define INT_START 			32
#define INT_END 			47
#define SYS_CALL 			128 //(0x80)
#define EXECEPTION_COUNT 	20
#define PAGE_DIR_0_VAL 		4255779
#define VIDEO       		0xB8000
#define FOUR_MB				0x400000
#define FOUR_BYTES 			4
#define THIRTY_TWO 			32
#define FNAME_LENGTH		32
#define BUF_TWENTY_THREE 	23
#define BUF_THIRTY_TWO		32
#define BUF_ONE_EIGHTY 		180
#define BUF_THREE_HUNDRED   300

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

extern char terminal_buffer[128];

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}
static inline void syscall(){
	asm volatile("int $128"); // 128 = 0x80 for system call
}
static inline void assertion_failure_ss(){
	asm volatile("int $1"); // ss is at 1
}

char buf[128]; // 128 is the buffer max size, buf is the buffer we fill


/* Checkpoint 1 tests */

/* IDT Test
 * 
 * Asserts that all IDT entries are populated correctly 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	disable_irq(8); //disable the rtc on pin 8
	clear(); //clear screen
	TEST_HEADER;

	int i;
	int result = PASS;
	// Exceptions
	for (i = 0; i < EXECEPTION_COUNT; i++){ 
		if ((idt[i].offset_15_00 == NULL) &&  // Check the populated handler functions
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
		if ((idt[i].seg_selector != KERNEL_CS) ||  // Check the reset of the idt fields
			(idt[i].reserved3 != 1) ||
			(idt[i].reserved2 != 1) ||  
			(idt[i].reserved1 != 1) ||  
			(idt[i].reserved0 != 0) ||  
			(idt[i].size != 1) ||  
			(idt[i].dpl != 0) ||  
			(idt[i].present != 1)){
			assertion_failure();
			result = FAIL;
		}
	}
	// Interrupts	
	for (i = INT_START; i < INT_END; i++){ 
		if ((idt[i].offset_15_00 == NULL) && // Check the populated handler functions
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
		if ((idt[i].seg_selector != KERNEL_CS) ||  // Check the reset of the idt fields
			(idt[i].reserved3 != 0) ||
			(idt[i].reserved2 != 1) ||  
			(idt[i].reserved1 != 1) ||  
			(idt[i].reserved0 != 0) ||  
			(idt[i].size != 1) ||  
			(idt[i].dpl != 0) ||  
			(idt[i].present != 1)){
			assertion_failure();
			result = FAIL;
		}
	}
	// System Calls
	if ((idt[SYS_CALL].offset_15_00 == NULL) &&  // Check the populated handler functions
		(idt[SYS_CALL].offset_31_16 == NULL)){ 
		assertion_failure();
		result = FAIL;
	}
	if ((idt[SYS_CALL].seg_selector != KERNEL_CS) ||  // Check the reset of the idt fields
		(idt[SYS_CALL].reserved3 != 1) ||
		(idt[SYS_CALL].reserved2 != 1) ||  
		(idt[SYS_CALL].reserved1 != 1) ||  
		(idt[SYS_CALL].reserved0 != 0) ||  
		(idt[SYS_CALL].size != 1) ||  
		(idt[SYS_CALL].dpl != 3) ||  // priority level 3
		(idt[SYS_CALL].present != 1)){
		assertion_failure();
		result = FAIL;
	}
	
	return result;
}

/* Exception Test (INT 0)
 * 
 * Asserts that division by zero exception works correctly
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Disables RTC interrupts
 * Coverage: Exception handler, IDT coverage
 * Files: idt_handler.h, idt_helper.h, intr_linkage.S
 */
int exception_test_div_by_zero(){
	disable_irq(8); //disable the rtc on pin 8
	clear(); //clear screen
	TEST_HEADER;
	int result = PASS;	
	// int i;
	// if (i == 3/0){ // Testing dividing anything by 0, we chose 3 here
	// 	result = FAIL;
	// }
	return result;
}

/* Exception Test (INT 1)
 * 
 * Asserts that single step exception works correctly
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Disables RTC interrupts
 * Coverage: Exception handler, IDT coverage
 * Files: idt_handler.h, idt_helper.h, intr_linkage.S
 */
int exception_test_ss(){
	disable_irq(8); //disable the rtc on pin 8
	clear(); //clear screen
	TEST_HEADER;
	int result = PASS;
	assertion_failure_ss(); // call ss exception
	result = FAIL;
	return result;
}

/* RTC Test 
 * 
 * Clears the screen and let RTC triggers the interrupts
 * Inputs: None
 * Outputs: PASS
 * Side Effects: None
 * Coverage: RTC init, Paginig, IDT populate, and PIC RTC handler
 * Files: i8259.h, idt_handler.h, rtc.h
 */
int rtc_test() {
	clear();	
	return PASS;
}	

/* Keyboard Test 
 * 
 * Disable RTC, clears the screen, and let keyboard key press triggers interrupts
 * Inputs: None
 * Outputs: PASS
 * Side Effects: Disables RTC interrupts, screen is cleared to display typed texts
 * Coverage: Keyboard init, IDT populate, and PIC keyboard handler
 * Files: keyboard.h, i8259.h, idt_handler.h
 */
int keyboard_test(){
	disable_irq(8); //disable the rtc on pin 8
	//clear(); //clear screen
	TEST_HEADER;
	printf("Testing the keyboard inputs: \n");

	return PASS;
}

/* Terminal Test 
 * 
 * Disable RTC, clears the screen, and let keyboard key press triggers interrupts. Lets user test terminal 
 * terminal read and write by using enter to read to terminal and ctrl + w to write to terminal
 * Inputs: None
 * Outputs: PASS
 * Side Effects: Disables RTC interrupts, screen is cleared to display typed texts
 * Coverage: Keyboard init, IDT populate, and PIC keyboard handler
 * Files: keyboard.h, i8259.h, idt_handler.h
 */
int terminal_test(){
	disable_irq(8);
	TEST_HEADER;
	printf("Testing the terminal: \n");
	//printf("Type in your command. To read it, press enter. To write it, press ctrl + w.\n");
	printf("391OS>");
	while(1){
		terminal_read(0, buf, 128);
		terminal_write(1, buf, 128);
	
	}
	return PASS;
}

/* Terminal Null Test 
 * 
 * Disable RTC, clears the screen, and inputs empty buffer to terminal read and write.
 * Inputs: None
 * Outputs: PASS
 * Side Effects: Disables RTC interrupts, screen is cleared to display typed texts
 * Coverage: Keyboard init, IDT populate, and PIC keyboard handler
 * Files: keyboard.h, i8259.h, idt_handler.h
 */
// int terminal_read_write_null(){
// 	disable_irq(8);
// 	TEST_HEADER;
// 	char buffer = "";
// 	terminal_read(buffer, strlen(buffer));
// 	terminal_write(buffer, strlen(buffer));
// 	return PASS;

// }

/* System Call Test (INT x80)
 * 
 * Calls system call INT x80
 * Inputs: None
 * Outputs: PASS
 * Side Effects: Disables RTC interrupts, screen is cleared
 * Coverage: System call handler, populate IDT
 * Files: idt_handler.h, idt_helper.h, intr_linkage.S
 */
int syscall_test(){
	disable_irq(8); //disable the rtc on pin 8
	clear(); //clear screen
	TEST_HEADER;
	syscall(); // call system call
	return PASS;
}

/* Paging Test 
 * 
 * Test paging and accessibility
 * Inputs: None
 * Outputs: FAIL
 * Side Effects: Disables RTC interrupts, screen is cleared
 * Coverage: accessible address, inaccessible address, values contained in page directory
 * Files: paging.h
 */
int paging_test_1() {
	uint32_t* accessible_address = (uint32_t *)VIDEO;
	uint32_t* inaccessible_address = (uint32_t *)(VIDEO-1);
	// uint32_t* inaccessible_address = 0;
	disable_irq(8); //disable the rtc on pin 8
	clear(); //clear screen
	TEST_HEADER;
	printf("\n \n Values contained in Page Directory [0]: 0x%x = 0b10000010000000000100011\n", *page_dir_pointer);
	printf("Accessed accessible address at video memory location: 0x%x \n", *accessible_address);
	printf("Accessing inaccessible address at video memory - 1 location: \n");
	printf("0x%x", *inaccessible_address);
	printf("INCORRECT: Accessed inaccessible addresss at video memory - 1 location: \n");
	return FAIL;
}	








/* Checkpoint 2 tests */

// /* file_read_invalid_buffer_limit_test
//  * 
//  * Test if we can read unopened file
//  * Inputs: None
//  * Outputs: PASS
//  * Side Effects: Screen cleared
//  * Coverage: Reading an unopened file
//  * Files: filesys.h
//  */
// int file_read_invalid_buffer_limit_test(void){
// 	disable_irq(8); //disable the rtc on pin 8
// 	clear(); //clear screen
// 	printf("\n \n");
// 	TEST_HEADER;
	
// 	uint8_t buf[BUF_THIRTY_TWO] = "";
// 	if(file_read(3, buf, BUF_THIRTY_TWO) == -1){ // set fd to 3, a valid file descriptor
// 		printf("Failed to read data or file closed \n");
// 		return PASS;
// 	}
// 	printf("Read 32 bytes of file data: \n %s \n", buf);
// 	return FAIL;
// }

// /* file_read_test_buf_23_test
//  * 
//  * Read file with buffer size 23
//  * Inputs: None
//  * Outputs: PASS
//  * Side Effects: Screen cleared
//  * Coverage: opening file, reading file
//  * Files: filesys.h
//  */
// int file_read_test_buf_23_test(void){
// 	disable_irq(8); //disable the rtc on pin 8
// 	clear(); //clear screen
// 	printf("\n \n");
// 	TEST_HEADER;

// 	uint8_t buf[BUF_TWENTY_THREE] = "";
//     uint8_t test[FNAME_LENGTH] = "frame1.txt";
// 	int32_t read_bytes;
// 	uint32_t fd = 3; // set fd to 3, a valid file descriptor
// 	int i;
// 	if(file_open(test, fd) == -1){
// 		printf("Failed to open file \n");
// 		return FAIL;
// 	}
// 	while((read_bytes = file_read(fd, buf, BUF_TWENTY_THREE)) != 0){ // while not EOF
// 		if(read_bytes == -1){
// 			printf("Failed to read data \n");
// 			return FAIL;
// 		}else if(read_bytes == -2){
// 			printf("Read file is not a file, read the next file \n");
// 		}else{
// 			for(i = 0; i < read_bytes; i++){
// 				printf("%c", buf[i]);
// 			}
// 		}
// 	}	
// 	printf("\n");
// 	return PASS;
// }

// /* file_read_test_buf_180_test
//  * 
//  * Read file with buffer size 180
//  * Inputs: None
//  * Outputs: PASS
//  * Side Effects: Screen cleared
//  * Coverage: opening file, reading file
//  * Files: filesys.h
//  */
// int file_read_test_buf_180_test(void){
// 	disable_irq(8); //disable the rtc on pin 8
// 	clear(); //clear screen
// 	printf("\n \n");
// 	TEST_HEADER;

// 	uint8_t buf[BUF_ONE_EIGHTY] = "";
//     uint8_t test[FNAME_LENGTH] = "frame0.txt";
// 	int32_t read_bytes;
// 	uint32_t fd = 3; // set fd to 3, a valid file descriptor
// 	int i;
// 	if(file_open(test, fd) == -1){
// 		printf("Failed to open file \n");
// 		return FAIL;
// 	}
// 	while((read_bytes = file_read(fd, buf, BUF_ONE_EIGHTY)) != 0){ // while not EOF
// 		if(read_bytes == -1){
// 			printf("Failed to read data \n");
// 			return FAIL;
// 		}else if(read_bytes == -2){
// 			printf("Read file is not a file, read the next file \n");
// 		}else{
// 			for(i = 0; i < read_bytes; i++){
// 				printf("%c", buf[i]);
// 			}
// 		}
// 	}	
// 	printf("\n");
// 	return PASS;
// }

// /* file_read_invalid_file_name_test
//  * 
//  * Opening filename that exceeds 32 bytes
//  * Inputs: None
//  * Outputs: PASS
//  * Side Effects: Screen cleared
//  * Coverage: opening file
//  * Files: filesys.h
//  */
// int file_read_invalid_file_name_test(void){
// 	disable_irq(8); //disable the rtc on pin 8
// 	clear(); //clear screen
// 	printf("\n \n");
// 	TEST_HEADER;

//     uint8_t test[FNAME_LENGTH + 1] = "verylargetextwithverylongname.txt";
// 	uint32_t fd = 3; // set fd to 3, a valid file descriptor
// 	if(file_open(test, fd) != -1){
// 		printf("Successfully opened too lengthy name file \n");
// 		return FAIL;
// 	}
// 	return PASS;
// }

// /* file_read_large_file_fish_test
//  * 
//  * Read "Fish" executable file
//  * Inputs: None
//  * Outputs: PASS
//  * Side Effects: Screen cleared
//  * Coverage: opening file, reading file
//  * Files: filesys.h
//  */
// int file_read_large_file_fish_test(){
// 	disable_irq(8); //disable the rtc on pin 8
// 	clear(); //clear screen
// 	printf("\n \n");
// 	// TEST_HEADER;

// 	uint8_t buf[BUF_ONE_EIGHTY] = "";
//     uint8_t test[FNAME_LENGTH] = "fish";
// 	int32_t read_bytes;
// 	uint32_t fd = 3; // set fd to 3, a valid file descriptor
// 	int i;
// 	if(file_open(test, fd) == -1){
// 		printf("Failed to open file \n");
// 		return FAIL;
// 	}
// 	int count = 0;
// 	while((read_bytes = file_read(fd, buf, BUF_ONE_EIGHTY)) != 0){ // while not EOF
// 		if(read_bytes == -1){
// 			printf("Failed to read data \n");
// 			return FAIL;
// 		}else if(read_bytes == -2){
// 			printf("Read file is not a file, read the next file \n");
// 		}else{
// 			for(i = 0; i < read_bytes; i++){ // print out the content
// 				if(buf[i] != '\0'){
// 					// next line - only print the first and last part of the file 
// 					// since the text is way too long to show ELF and the final magic number on the same screen
// 					if(count < 79){ // print the first 79 characters
// 						printf("%c", buf[i]);
// 					}else if(count == 79){ // print the next line after the 79 characters
// 						printf("\n");
// 					}else if(read_bytes != BUF_ONE_EIGHTY){
// 						printf("%c", buf[i]);
// 					}
// 					count++;
// 				}
// 			}
// 		}
// 	}	
// 	printf("\nSuccessful File Read!");
// 	printf("\n");
// 	return PASS;
// }

// /* file_read_large_file_grep_test
//  * 
//  * Read "grep" executable file
//  * Inputs: None
//  * Outputs: PASS
//  * Side Effects: Screen cleared
//  * Coverage: opening file, reading file
//  * Files: filesys.h
//  */
// int file_read_large_file_grep_test(){
// 	disable_irq(8); //disable the rtc on pin 8
// 	clear(); //clear screen
// 	printf("\n \n");
// 	// TEST_HEADER;

// 	uint8_t buf[BUF_THREE_HUNDRED] = "";
//     uint8_t test[FNAME_LENGTH] = "grep";
// 	int32_t read_bytes;
// 	uint32_t fd = 3; // set fd to 3, a valid file descriptor
// 	int i;
// 	if(file_open(test, fd) == -1){
// 		printf("Failed to open file \n");
// 		return FAIL;
// 	}
// 	int count = 0;
// 	while((read_bytes = file_read(fd, buf, BUF_THREE_HUNDRED)) != 0){ // while not EOF
// 		if(read_bytes == -1){
// 			printf("Failed to read data \n");
// 			return FAIL;
// 		}else if(read_bytes == -2){
// 			printf("Read file is not a file, read the next file \n");
// 		}else{
// 			for(i = 0; i < read_bytes; i++){ // print out the content
// 				// next line - only print the first and last part of the file 
// 				// since the text is way too long to show ELF and the final magic number on the same screen
// 				if(count < 79){ // print the first 79 characters
// 					printf("%c", buf[i]);
// 				}else if(count == 79){ // print the next line after the 79th character
// 					printf("\n");
// 				}else if(read_bytes != BUF_THREE_HUNDRED){
// 					printf("%c", buf[i]);
// 				}
// 				count++;
// 			}
// 		}
// 	}	
// 	printf("\nSuccessful File Read!");
// 	printf("\n");
// 	return PASS;
// }

// /* file_read_large_file_ls_test
//  * 
//  * Read "ls" executable file
//  * Inputs: None
//  * Outputs: PASS
//  * Side Effects: Screen cleared
//  * Coverage: opening file, reading file
//  * Files: filesys.h
//  */
// int file_read_large_file_ls_test(){
// 	disable_irq(8); //disable the rtc on pin 8
// 	clear(); //clear screen
// 	printf("\n \n");
// 	// TEST_HEADER;

// 	uint8_t buf[BUF_ONE_EIGHTY] = "";
//     uint8_t test[FNAME_LENGTH] = "ls";
// 	int32_t read_bytes;
// 	uint32_t fd = 3; // set fd to 3, a valid file descriptor
// 	int i;
// 	if(file_open(test, fd) == -1){
// 		printf("Failed to open file \n");
// 		return FAIL;
// 	}
// 	//file size is 5300+
// 	int count = 0;
// 	while((read_bytes = file_read(fd, buf, BUF_ONE_EIGHTY)) != 0){ // while not EOF
// 		if(read_bytes == -1){
// 			printf("Failed to read data \n");
// 			return FAIL;
// 		}else if(read_bytes == -2){
// 			printf("Read file is not a file, read the next file \n");
// 		}else{
// 			for(i = 0; i < read_bytes; i++){ // print out the content
// 				if(buf[i] != '\0'){
// 					printf("%c", buf[i]);
// 					count++;
// 				}
// 				if(count >= 79){ // next line after the 79th characters
// 					printf("\n");
// 					count = 0;
// 				}
// 			}
// 		}
// 	}	
// 	printf("\nSuccessful File Read!");
// 	printf("\n");
// 	return PASS;
// }

// /* file_read_large_file_fish_test
//  * 
//  * Opening file with different fd
//  * Inputs: None
//  * Outputs: PASS
//  * Side Effects: Screen cleared
//  * Coverage: opening file, closing file
//  * Files: filesys.h
//  */
// int file_open_close_test(void){
// 	disable_irq(8); //disable the rtc on pin 8
// 	clear(); //clear screen
// 	printf("\n \n");
// 	TEST_HEADER;

//     uint8_t test1[FNAME_LENGTH] = "frame0";
//     uint8_t test2[FNAME_LENGTH] = "frame0.txt";
// 	uint32_t fd; // open a random valid fd

// 	fd = 0; // Invalid fd
// 	if(file_open(test2, 0) == -1){ // invalid fd
// 		printf("Successful invalid fd test \n");
// 	}else{
// 		return FAIL;
// 	}
// 	if(file_close(0) == -1){ // invalid fd
// 		printf("Successful invalid fd test \n");
// 	}else{
// 		return FAIL;
// 	}
	
// 	fd = 3; // Valid fd
// 	if(file_open(test1, fd) == -1){ // invalid file name
// 		printf("Successful invalid file name test  \n");
// 	}else{
// 		return FAIL;
// 	}
// 	if(file_open(test2, fd) == 0){ // valid fd and file name
// 		printf("Successful file opened \n");
// 	}else{
// 		return FAIL;
// 	}
// 	if(file_close(fd) == 0){ // valid fd and file name
// 		printf("Successful file closed \n");
// 	}else{
// 		return FAIL;
// 	}
// 	printf("\n");
// 	return PASS;
// }

// /* directory_read_write_test
//  * 
//  * Prints directory entries
//  * Inputs: None
//  * Outputs: PASS
//  * Side Effects: Screen cleared
//  * Coverage: opening directory, reading directory, writing directory
//  * Files: filesys.h
//  */
// int directory_read_write_test(void){
// 	disable_irq(8); //disable the rtc on pin 8
// 	clear(); //clear screen
// 	printf("\n \n");
// 	TEST_HEADER;

// 	uint32_t fd = 4; // open a random valid fd 4
// 	uint8_t buf[BUF_THIRTY_TWO] = "";
//     uint8_t test1[FNAME_LENGTH] = ".";
// 	uint32_t read_bytes;

// 	if(directory_open(test1, fd) == -1){
// 		printf("Failed to open directory \n");
// 		return FAIL;
// 	}
// 	while((read_bytes = directory_read(fd, buf, 0)) != 0){
// 		if(read_bytes == -1){
// 			printf("Failed to read directory \n");
// 			return FAIL;
// 		}else{
// 			printf("File name: %s \n", buf);
// 		}
// 	}
// 	printf("Successfully listed all files \n ");

// 	if(directory_write(fd, buf, FNAME_LENGTH) != -1) {
// 		printf("ERROR: read only system! \n");
// 		return FAIL;
// 	}

// 	return PASS;
// }

// /* directory_open_close_test
//  * 
//  * Prints directory entries
//  * Inputs: None
//  * Outputs: PASS
//  * Side Effects: Screen cleared
//  * Coverage: opening directory
//  * Files: filesys.h
//  */
// int directory_open_close_test(void){
// 	disable_irq(8); //disable the rtc on pin 8
// 	clear(); //clear screen
// 	printf("\n \n");
// 	TEST_HEADER;
	
// 	uint32_t fd = 4; // open a random valid fd 4
//     uint8_t test1[FNAME_LENGTH] = ".";

// 	if(directory_open(test1, fd) == -1){
// 		printf("Failed to open directory \n");
// 		return FAIL;
// 	}else{
// 		printf("Successfully opened directory \n");
// 	}

// 	if(directory_close(fd) != 0) {
// 		printf("Failed to close directory \n");
// 		return FAIL;
// 	}else{
// 		printf("Successfully closed directory \n");
// 	}
// 	return PASS;
// }

// /* rtc_open_close_test
//  * 
//  * Opening RTC
//  * Inputs: None
//  * Outputs: PASS
//  * Side Effects: Screen cleared
//  * Coverage: opening rtc, closing rtc
//  * Files: filesys.h
//  */
// int rtc_open_close_test(void){
// 	clear();	
// 	printf("\n \n");
// 	TEST_HEADER;
	
//     uint8_t test1[FNAME_LENGTH] = "syserr";
//     uint8_t test2[FNAME_LENGTH] = "rtc";
// 	uint32_t fd; // open a random valid fd
	
// 	fd = 3; // Valid fd
// 	if(rtc_open(test1, fd) == -1){ // valid file name
// 		printf("Successful wrong file type test  \n");
// 	}else{
// 		return FAIL;
// 	}
// 	if(rtc_open(test2, fd) == 0){ // valid file name
// 		printf("Successful RTC open \n");
// 	}else{
// 		return FAIL;
// 	}
// 	if(rtc_close(fd) == 0){ // valid fd and file name
// 		printf("Successful file closed \n");
// 	}else{
// 		return FAIL;
// 	}
// 	return PASS;
// }

// /* rtc_read_write_test
//  * 
//  * reads and writes RTC
//  * Inputs: None
//  * Outputs: PASS
//  * Side Effects: Screen cleared
//  * Coverage: writing rtc, reading rtc
//  * Files: filesys.h
//  */
// int rtc_read_write_test(void){
// 	clear();	
// 	// printf("\n");
// 	TEST_HEADER;

// 	int8_t buf[FNAME_LENGTH] = "";
// 	int32_t radix = 10;
// 	int Virtual_interrupt_count = 0;
// 	int32_t num_bytes;
// 	int32_t fd = 3; // set fd to 3, a valid file descriptor
	

// 	itoa((uint32_t)2048, buf, radix); // Set frequency to 2048
// 	if(rtc_write(fd, (uint8_t *)buf, FOUR_BYTES) == -1){
// 		printf("Successful test for invalid frequency 2048 \n");
// 	}

// 	itoa((uint32_t)3, buf, radix); // Set frequency to 3
// 	if(rtc_write(fd, (uint8_t *)buf, strlen(buf)) == -1){
// 		printf("Successful test for invalid frequency 3 \n");
// 	}
	
//     printf("Desired Frequency: 1 \n");
// 	itoa((uint32_t)1, buf, radix); // Set frequency to 1
// 	rtc_write(fd, (uint8_t *)buf, strlen(buf)); 
// 	while(Virtual_interrupt_count < 4){ // Print for 4 seconds
// 		rtc_read(fd, (uint8_t *)buf, num_bytes);
// 		Virtual_interrupt_count++;
// 	}

//     printf("\nDesired Frequency: 2 \n");
// 	itoa((uint32_t)2, buf, radix); // Set frequency to 2
// 	rtc_write(fd, (uint8_t *)buf, strlen(buf)); 
// 	while(Virtual_interrupt_count < 8){ // Print for 4 seconds
// 		rtc_read(fd, (uint8_t *)buf, num_bytes);
// 		Virtual_interrupt_count++;
// 	}
	
//     printf("\nDesired Frequency: 4 \n");
// 	itoa((uint32_t)4, buf, radix); // Set frequency to 4
// 	rtc_write(fd, (uint8_t *)buf, strlen(buf)); 
// 	while(Virtual_interrupt_count < 16){ // Print for 4 seconds
// 		rtc_read(fd, (uint8_t *)buf, num_bytes);
// 		Virtual_interrupt_count++;
// 	}
	
//     printf("\nDesired Frequency: 8 \n");
// 	itoa((uint32_t)8, buf, radix); // Set frequency to 8
// 	rtc_write(fd, (uint8_t *)buf, strlen(buf)); 
// 	while(Virtual_interrupt_count < 32){   // Print for 4 seconds
// 		rtc_read(fd, (uint8_t *)buf, num_bytes);
// 		Virtual_interrupt_count++;
// 	}
	
//     printf("\nDesired Frequency: 16 \n");
// 	itoa((uint32_t)16, buf, radix); // Set frequency to 16
// 	rtc_write(fd, (uint8_t *)buf, strlen(buf)); 
// 	while(Virtual_interrupt_count < 64){ // Print for 4 seconds
// 		rtc_read(fd, (uint8_t *)buf, num_bytes);
// 		Virtual_interrupt_count++;
// 	}
	
//     printf("\nDesired Frequency: 32 \n");
// 	itoa((uint32_t)32, buf, radix); // Set frequency to 32
// 	rtc_write(fd, (uint8_t *)buf, strlen(buf)); 
// 	while(Virtual_interrupt_count < 128){ // Print for 4 seconds
// 		rtc_read(fd, (uint8_t *)buf, num_bytes);
// 		Virtual_interrupt_count++;
// 	}
	
//     printf("\nDesired Frequency: 64 \n");
// 	itoa((uint32_t)64, buf, radix); // Set frequency to 64
// 	rtc_write(fd, (uint8_t *)buf, strlen(buf)); 
// 	while(Virtual_interrupt_count < 256){ // Print for 4 seconds
// 		rtc_read(fd, (uint8_t *)buf, num_bytes);
// 		Virtual_interrupt_count++;
// 	}
	
//     printf("\nDesired Frequency: 128 \n");
// 	itoa((uint32_t)128, buf, radix); // Set frequency to 128
// 	rtc_write(fd, (uint8_t *)buf, strlen(buf)); 
// 	while(Virtual_interrupt_count < 512){ // Print for 4 seconds
// 		rtc_read(fd, (uint8_t *)buf, num_bytes);
// 		Virtual_interrupt_count++;
// 	}
	
//     printf("\nDesired Frequency: 256 \n");
// 	itoa((uint32_t)256, buf, radix); // Set frequency to 256
// 	rtc_write(fd, (uint8_t *)buf, strlen(buf)); 
// 	while(Virtual_interrupt_count < 1024){ // Print for 4 seconds
// 		rtc_read(fd, (uint8_t *)buf, num_bytes);
// 		Virtual_interrupt_count++;
// 	}
	
//     printf("\nDesired Frequency: 512 \n");
// 	itoa((uint32_t)512, buf, radix); // Set frequency to 512
// 	rtc_write(fd, (uint8_t *)buf, strlen(buf)); 
// 	while(Virtual_interrupt_count < 2048){ // Print for 4 seconds
// 		rtc_read(fd, (uint8_t *)buf, num_bytes);
// 		Virtual_interrupt_count++;
// 	}
	
//     printf("\nDesired Frequency: 1024 \n");
// 	itoa((uint32_t)1024, buf, radix); // Set frequency to 1024
// 	rtc_write(fd, (uint8_t *)buf, strlen(buf)); 
// 	while(Virtual_interrupt_count < 4096){ // Print for 4 seconds
// 		rtc_read(fd, (uint8_t *)buf, num_bytes);
// 		Virtual_interrupt_count++;
// 	}

// 	printf("\n");
// 	return PASS;
// }

/* Checkpoint 3 tests */

int load_program_paging() {
	// clear();
	//uint32_t i, count;
	//uint8_t* mb_128;
	TEST_HEADER;
	// uint8_t* file_name = "ls";
    // dentry_t curr_dentry;
    // read_dentry_by_name(file_name, &curr_dentry);
	// paging_execute(file_name);
	// load_program(&curr_dentry, 0);

	// mb_128 = (uint8_t*)USER_ADDR;

	clear();
	uint8_t cmd[FNAME_LENGTH] = "shell";
	execute(cmd);
	return PASS;



	// count = 0;
	// for(i = 0; i < 1000; i++) {
	// 	if(count == 70) {
	// 		printf("\n");
	// 		count = 0;
	// 	}
	// 	printf("%c", *(mb_128+i));
	// 	count++;
	// }

	// clear();

	// cmd = "ls";
	// execute(cmd);
	// count = 0;
	// for(i = 0; i < 1000; i++) {
	// 	if(count == 70) {
	// 		printf("\n");
	// 		count = 0;
	// 	}
	// 	printf("%c", *(mb_128+i));
	// 	count++;
	// }

	// clear();

	// cmd = "shell";
	// execute(cmd);
	// count = 0;
	// for(i = 0; i < 1000; i++) {
	// 	if(count == 70) {
	// 		printf("\n");
	// 		count = 0;
	// 	}
	// 	printf("%c", *(mb_128+i));
	// 	count++;
	// }

	// clear();

	// cmd = "ls";
	// execute(cmd);

	// count = 0;
	// for(i = 0; i < 1000; i++) {
	// 	if(count == 70) {
	// 		printf("\n");
	// 		count = 0;
	// 	}
	// 	printf("%c", *(mb_128+i));
	// 	count++;
	// }

}

int system_regular_file_test(){
	uint8_t test[FNAME_LENGTH] = "frame0.txt";
	uint8_t buf[FNAME_LENGTH] = "buffer";
	uint8_t buf2[FNAME_LENGTH] = "buffer";
	process_num++;
	int i;
	clear();
	pcb_struct* pcb = (pcb_struct*)(EIGHT_MB - EIGHT_KB*(process_num+1));	
	pcb->file_descriptor[0].flags = 1;
	pcb->file_descriptor[1].flags = 1;

	printf("File descriptor entries BEFORE open: \n");
	for(i = 0; i < 8; i++) {
		printf("%d: ", i);
		if(pcb->file_descriptor[i].flags == 1) printf("ACTIVE ");
		else printf("INACTIVE ");
		if(i==4) printf("\n");
	}
	printf("\n\n");
	printf("Opening %s...\n\n", test);
	if(open(test) == -1) return FAIL;
	printf("File descriptor entries AFTER open: \n");
	for(i = 0; i < 8; i++) {
		printf("%d: ", i);
		if(pcb->file_descriptor[i].flags == 1) printf("ACTIVE ");
		else printf("INACTIVE ");
		if(i==4) printf("\n");
	}
	if(pcb->file_descriptor[2].flags != 1) return FAIL;
	printf("\n\n");
	printf("Reading 20 bytes of %s...\n", test);
	if(read(2, buf, 20) == -1) return FAIL;
	printf("Read Result: %s\n\n", buf);
	printf("Writing 20 bytes to %s...\n", test);
	write(2, buf, 20);
	pcb->file_descriptor[2].file_position -= 20;
	if(read(2, buf2, 20) == -1) return FAIL;
	printf("Write Result: ERROR: READ-ONLY SYSTEM\n\n");
	if(strncmp((int8_t*)buf, (int8_t*)buf2, FNAME_LENGTH) != 0) return FAIL;

	printf("Closing %s...\n\n", test);
	if(close(2) == -1) return FAIL;
	if(pcb->file_descriptor[2].flags == 1) return FAIL;
	printf("File descriptor entries AFTER close: \n");
	for(i = 0; i < 8; i++) {
		printf("%d: ", i);
		if(pcb->file_descriptor[i].flags == 1) printf("ACTIVE ");
		else printf("INACTIVE ");
		if(i==4) printf("\n");
	}
	printf("\n\n");
	return PASS;
}

int system_regular_dir_test(){
	uint8_t test[FNAME_LENGTH] = ".";
	uint8_t buf[FNAME_LENGTH] = "buffer";
	process_num++;
	int i;
	clear();
	pcb_struct* pcb = (pcb_struct*)(EIGHT_MB - EIGHT_KB*(process_num+1));	
	pcb->file_descriptor[0].flags = 1;
	pcb->file_descriptor[1].flags = 1;

	printf("File descriptor entries BEFORE open: \n");
	for(i = 0; i < 8; i++) {
		printf("%d: ", i);
		if(pcb->file_descriptor[i].flags == 1) printf("ACTIVE ");
		else printf("INACTIVE ");
		if(i==4) printf("\n");
	}
	printf("\n\n");
	printf("Opening directory '%s'\n\n", test);
	if(open(test) == -1) return FAIL;
	printf("File descriptor entries AFTER open: \n");
	for(i = 0; i < 8; i++) {
		printf("%d: ", i);
		if(pcb->file_descriptor[i].flags == 1) printf("ACTIVE ");
		else printf("INACTIVE ");
		if(i==4) printf("\n");
	}
	if(pcb->file_descriptor[2].flags != 1) return FAIL;
	printf("\n\n");
	printf("Directory file position BEFORE read: %d\n", pcb->file_descriptor[2].file_position);
	printf("Reading directory '%s'\n", test);
	if(read(2, buf, 20) == -1) return FAIL;
	printf("Directory file position AFTER read: %d\n\n", pcb->file_descriptor[2].file_position);
	printf("Writing to directory '%s'\n", test);
	write(2, buf, 20);
	printf("ERROR: READ-ONLY SYSTEM\n\n");

	printf("Closing directory '%s'\n\n", test);
	if(close(2) == -1) return FAIL;
	if(pcb->file_descriptor[2].flags == 1) return FAIL;
	printf("File descriptor entries AFTER close: \n");
	for(i = 0; i < 8; i++) {
		printf("%d: ", i);
		if(pcb->file_descriptor[i].flags == 1) printf("ACTIVE ");
		else printf("INACTIVE ");
		if(i==4) printf("\n");
	}
	printf("\n\n");
	return PASS;
}

int system_rtc_test(){
	clear();
	uint8_t test[FNAME_LENGTH] = "rtc";
	int8_t buf[FNAME_LENGTH] = "";
	process_num++;
	int32_t radix = 10;
	int i;
	itoa((uint32_t)1024, buf, radix); // Set frequency to 1024

	pcb_struct* pcb = (pcb_struct*)(EIGHT_MB - EIGHT_KB*(process_num+1));
	pcb->file_descriptor[0].flags = 1;
	pcb->file_descriptor[1].flags = 1;

	printf("File descriptor entries BEFORE open: \n");
	for(i = 0; i < 8; i++) {
		printf("%d: ", i);
		if(pcb->file_descriptor[i].flags == 1) printf("ACTIVE ");
		else printf("INACTIVE ");
		if(i==4) printf("\n");
	}
	printf("\n\n");
	printf("Opening %s...\n\n", test);
	if(open(test) == -1) return FAIL;
	printf("File descriptor entries AFTER open: \n");
	for(i = 0; i < 8; i++) {
		printf("%d: ", i);
		if(pcb->file_descriptor[i].flags == 1) printf("ACTIVE ");
		else printf("INACTIVE ");
		if(i==4) printf("\n");
	}
	if(pcb->file_descriptor[2].flags != 1) return FAIL;
	printf("\n\n");
	printf("Reading %s...\n", test);
	read(2, buf, 20);
	printf("\n\n");
	printf("Writing to %s...\n", test);
	if(write(2, buf, 4) == -1) return FAIL;

	printf("Closing %s...\n\n", test);
	if(close(2) == -1) return FAIL;
	if(pcb->file_descriptor[2].flags == 1) return FAIL;
	printf("File descriptor entries AFTER close: \n");
	for(i = 0; i < 8; i++) {
		printf("%d: ", i);
		if(pcb->file_descriptor[i].flags == 1) printf("ACTIVE ");
		else printf("INACTIVE ");
		if(i==4) printf("\n");
	}
	printf("\n\n");
	return PASS;
}

/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	// Checkpoint 1 tests
	// TEST_OUTPUT("idt_test", idt_test());
	// TEST_OUTPUT("exception_test_div_by_zero", exception_test_div_by_zero());
	// TEST_OUTPUT("exception_test_ss", exception_test_ss());
	// TEST_OUTPUT("syscall_test", syscall_test());
	// TEST_OUTPUT("keyboard_test", keyboard_test());
	// TEST_OUTPUT("paging_test", paging_test_1());
	// rtc_test();

	// Checkpoint 2 tests
	// terminal_test();
	// TEST_OUTPUT("file_read_invalid_buffer_limit_test", file_read_invalid_buffer_limit_test()); 
	// TEST_OUTPUT("directory_open_close_test", directory_open_close_test());
	// TEST_OUTPUT("directory_read_write_test", directory_read_write_test());
	// TEST_OUTPUT("file_open_close_test", file_open_close_test());
	// file_read_large_file_fish_test(); // NOTE: Print out first part and end of the file cuz text can't fit in a single screen
	// file_read_large_file_grep_test(); // NOTE: Print out first part and end of the file cuz text can't fit in a single screen
	// file_read_large_file_ls_test(); // NOTE: Prints out the entire file: shows that we can read a file bigger than 4k bytes
	// TEST_OUTPUT("file_read_invalid_file_name_test", file_read_invalid_file_name_test());
	// TEST_OUTPUT("file_read_test_buf_23", file_read_test_buf_23_test());
	// TEST_OUTPUT("file_read_test_buf_180", file_read_test_buf_180_test());
	// TEST_OUTPUT("rtc_read_write_test", rtc_read_write_test());
	// TEST_OUTPUT("rtc_open_close_test", rtc_open_close_test());
	// terminal_read_write_null();

	// Checkpoint 3 tests
	// TEST_OUTPUT("load_program_paging", load_program_paging());
	// TEST_OUTPUT("system_regular_file_test", system_regular_file_test());
	// TEST_OUTPUT("system_regular_dir_test", system_regular_dir_test());
	// TEST_OUTPUT("system_rtc_test", system_rtc_test());
	// TEST_OUTPUT("system_rtc_test", system_rtc_test());
}

