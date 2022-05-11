Checkpoint I
# GDT
### Issue 1: Boot looping: 
   The kernel kept looping in its boot procedure
   #### Fixes:
  - lgdt (load) the gdt_desc_ptr instead of _gdt
  - Fixed the struct alignment in the assembly file
# PIC and the devices
### Issue 1: IRQ1 no fire: 
Keyboard does not print out any key
#### Fix:
- enable_irq() and disable_irq() were swapped.
### Issue 2: IRQ1 only fires once:
Keyboard does not print out more than one key
#### Fix:
 - Changed RET to IRET in the assembly linkage for hardware interrupt handler
# Paging
### Issue 1: Boot looping: 
   The kernel kept looping in its boot procedure
#### Fixes:
- Not using the right page directory structure for the 4MB page (were using the one for page table originally)
- 0x80000000 to 0x80000001 for CR0 (see lecture 14 slides)
- Didn’t make a change to CR4, which enables 4MB pages
- Enable paging after setting CR4
- Need to enable the present bit for the first entry
- Address bits need to be changed based on how many bits are used
- Kernel page: 10 bits = 2^10 addresses, so 4GB/(2^10) = 4MB per address. So we should set the kernel’s address bits to just 1 (4MB)
- First page directory entry: 20 bits = 2^20 addresses, so 4GB/(2^20) = 4kb per address. So we need to divide the address of the first page table by 4kb
- Page table entry:  20 bits = 2^20 addresses, so 4GB/(2^20) = 4kb per address. So we just need to increment the address by 1 to set each 4kb pages
- Used a separate .S file instead of inline assembly instead to make sure the assembly part works
# RTC
### Issue 1: IRQ8 only fires once: 
   RTC stopped triggering after one interrupt
#### Fix:
- Cleared register C to allow for subsequent interrupts


*note: We took time working on the IDT, but once we figured it out, it worked right away. Thus, there were no bugs.

Checkpoint II
# RTC
### Issue 1: RTC Frequency: 
   RTC doesn't change in frequency 
#### Fix:
- The RTC actually does change in frequency when we set it, but the maximum frequency is 1024 instead of 8K like what was suggested on OSDEV
### Issue 2: RTC buffer: 
   RTC input has to be a buffer
#### Fix:
- Thought about the logic and implemented a way that converts integer into a buffer to pass through and convert it back into an integer again. Subtracted the value by the ASCII zero.

# File System
### Issue 1: Read_Data: 
   When we repeatedly read the data, the output did not pick up on what was left off.
#### Fix:
- Switched two lines of code since we need to first let result = bytes_to_read, then set bytes_to_read to 0
### Issue 2: Read_Data Buffer: 
   strncpy copies one more byte than it should have, so the fish frame is not aligned correctly. The magic numbers at the end of the large executable files also don't show up
#### Fix:
- Replace string copy from lib.c with just a simple for loop to copy the data. String copy stops at '/0'.
### Issue 3: Test Case for Read_Data: 
   Read data doesn't show case the last few bytes of the frame
#### Fix:
- Use putc instead of printf and print the amount of read bytes
### Issue 4: Address addition: 
   Adding numbers to pointers was not incrementing by the same amount
#### Fix: 
- Cast into integer, perform addition, then cast back into pointer


# Terminal + Keyboard Extension
### Issue 1: Row Overflow
   More than 80 characters printed in a row caused the initial few to be overwritten by the last few.
#### Fix:
- Printed a new line once 80 characters are reached, but didn't count the new line as a character in the buffer.
### Issue 2: Backspace After a New Line:
   When we use backspace to return to a previous line, the first character of the line would not get deleted. Instead, it would skip to the next line upon the last backspace, and break the terminal
#### Fix:
- Added the "391OS>" header at the start of the terminal, which prevented the backspace to interact with the bugged first character.
### Issue 3: Double new line after backspace fills a line with spaces: 
   Backspace does not delete a character, it replaces it with a space, and moves the cursor back. For some reason, this caused the system to newline twice upon an Enter command after backspace was used to go back to a previous line.
#### Fix:
- Adjusted the row and buffer counters to fit this phenomenon, so it does not cause any problems with the terminal. Since double newline only ever happened with an Enter command (not a row overflow), this is an adequate fix.
### Issue 4: Boot Loop Upon Screen Y Increment
   While fixing Issue 3, we tried to increment the y coordinate of the cursor so double newline does not occur, but the system started boot looping.
#### Fix:
- We had to revert the change and find another fix.
### Issue 5: Deleting a Tab at once 
   Tab counted as one character, but it took up 4 spaces. So, deleting it without causing counter issues was a problem.
#### Fix: 
- We used the keyboard_buffer and the buffer_ct to keep track of all tabs in the current pending buffer. If backspace was called on a Tab, it had a special case where it deleted 4 spaces at once, and only decrement the counters by 1 character.
### Issue 5: Test Issues
   Testing the terminal using the while loop caused either an infinite loop or the kernel getting stuck.
#### Fix: 
- Changed the implementation of terminal_read to check for the Enter key, and for Enter key not to call terminal_read. Additionally, we set the enter_read flag to 0 at the end of the while loop, so it stops at the next terminal_read until enter is pressed again while filling up the keyboard buffer to be copied.


Checkpoint III
# Execute
### Issue 1: Execute page faulted after context switch
#### Fix: Image was not copied correctly into the 128MB location, used a simpler method to copy the image instead
### Issue 2: Execute has general protection fault after context switch
#### Fix: In paging, the priority level should be set to user level
### Issue 3: After execute, shell program keeps on printing 391OS> and doesn't stop
#### Fix: Clear flag for keyboard buffer after pressing enter
### Issue 4: When we run another program in Shell, it prints "Program ended abruptly"
#### Fix: Fixed the return value from execute
### Issue 5: Keyboard doesn't respond after opening Shell
#### Fix: enable interrupt again before IRET

# Halt
### Issue 1: Program page faulted after halting
#### Fix: Added a jump label back to the end of execute
### Issue 2: Exit on shell crashs the program
#### Fix: Added jump labels, restored all values for the shell to restart after exit
### Issue 3: System halt only takes in 1 byte status but exception status number is 256
#### Fix: created another function called exception_halt with the same content

# General System Call and Assembly Linkage
### Issue 1: File operations table pointer could not call the appropriate functions
#### Fix: Changed it to be an actual pointer instead of an integer in the PCB Struct
### Issue 2: Files were opened into stdin or stdout
#### Fix: Manually filled up fd numbers 0 and 1 before opening files
### Issue 3:  Warning: Initialization with incompatible pointer type 
#### Fix: Adjusted the input variable types of system calls for file, dir, rtc and terminal

Checkpoint IV
# Getargs
### Issue 1: changing rtc frequency did not work
#### Fix: in rtc.c, replaced the original complicated conversion for input buffer to a simple type casting to int32_t

# Syserr 8
### Issue 1: syserr 8 page faults and returned to user program
#### Fix: changed jbe to jle for signed comparison for eax value

# Vidmap
### Issue 1: none
#### Fix: none

# cat
### Issue 1: cat prints out random extra characters
#### Fix: We counted the number of bytes written inside terminal_write, and returned that value instead of nbytes.
### Issue 2: cat prints out extra characters that belongs to other files
#### Fix: We realized the bug happens when cat is called on two different files and the first file has more characters that the second one, which meant that the buffer had some residue left at the end. So we cleared it inside file_read, before cat reads the next file.


Checkpoint V
# Scheduling
### Issue 1: Running Hello on multiple terminals have issues. Terminal buffer writes to different user programs.
#### Fix: Terminal read needs a critical section, should use the schedule_idx instead of the visualized index for pcb. Also note that disable and enable IRQ have sti() at the end so interrupts would be enabled.
### Issue 2: RTC slows down significant after running scheduling
#### Fix: The virtualized frequency now only needs one-third of its original value because RTC writes to different programs one by one
### Issue 3: Running counter blocks the other programs on other terminals
#### Fix: Removed some disable_irq() that isn't necessary.
### Issue 4: Running fish after cat RTC slows down significantly
#### Fix: Open and close all file descriptors correctly, use the schedule index to get the pcb instead of using process_num. Virtualize the desired RTC frequency for each terminal separately.
### Issue 5: 
#### Fix: 

# Multi-Screen
### Issue 1: Row overflow for hello
#### Fix: Our terminal deals with row overflow by counting characters instead of checking cursor coordinates. That did not account for the case where hello program asks for user's name. We added a case for it where the character count takes it into account.
### Issue 2: Getargs issue with garbage input
#### Fix: The buffer that's passed in by the user program had the character ASCII 0xEC in the buffer even if there where no arguments. We added a buffer clear before the args are read.
### Issue 3: Running fish and counter on different terminals write to different terminals still
#### Fix: Made the terminal_write a critical section
### Issue 4: Ctrl + L doen't clear the currently active screen
#### Fix: Change video_mem variable to constant video screen address in clear screen function
### Issue 5: TAB would not newline
#### Fix: Our newline logic also did not take TAB into account (CP2 bug). We made it so it checks for row overflow for each of the 4 spaces, instead of the TAB as a whole.
### Issue 6: Shell does not deal with \n at the end of the buffer
#### Fix: We were not able to fix this since we cannot debug the user program itself. Instead, we added special cases in parse_args and terminal_read
