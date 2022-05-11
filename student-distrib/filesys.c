#include "filesys.h"
#include "system_call.h"

/* void init_filesys()
 * Sets pointers to start of blocks
 * inputs: addr - pointer to start of blocks (files system module)
 * outputs: none
 * side effects: none
 */
void init_filesys(uint32_t* addr) {
    pcb_struct* pcb = (pcb_struct*)(EIGHT_MB - EIGHT_KB*(process_num+1));
    bb = (boot_block*)addr;                     // boot block is first block
    inodes = addr + KB;                         // first inode is right after boot block
    inodes_struct_ptr = (inode*)inodes;
    data = addr + (bb->inode_count + 1) * KB;   // data blocks are after inodes

    int i;
    for(i = FD_MIN; i <= FD_MAX; i++){                     // initialize file descriptors
        pcb->file_descriptor[i].flags = 0;
    }
}

/* uint32_t read_dentry_by_name()
 * Searches for dentry by name
 * inputs: fname  - name of desired dentry
 *         dentry - struct to be populated if found
 * outputs: 0 if found, -1 else 
 * side effects: If found, input dentry is populated
 */
uint32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry) {
    uint32_t i = 32;
    uint8_t str1[DENTRY_FILE_NAME_LEN];

    if(strlen((int8_t*)fname) > DENTRY_FILE_NAME_LEN){ // file name is too long
        return -1;
    }

    // Look for dentry that matches input name
    for(i = 0; i < bb->dir_count; i++) {
        strncpy((int8_t*)str1, (int8_t*)bb->direntries[i].filename, DENTRY_FILE_NAME_LEN);
        if(strncmp((int8_t*)str1, (int8_t*) fname, DENTRY_FILE_NAME_LEN) == 0) { // Dentry found
            return read_dentry_by_index(i, dentry);
        }
    }

    // Dentry not found
    return -1;
}


/* uint32_t read_dentry_by_index()
 * Reads dentry at given index
 * inputs: index  - index of desired dentry
 *         dentry - struct to be populated if found
 * outputs: 0 if valid index, -1 else
 * side effects: If index valid, input dentry is populated
 */
uint32_t read_dentry_by_index(uint32_t index, dentry_t* dentry) {
    // Index valid?
    if(index < 0 || index >= bb->dir_count) return -1;

    // Populate input dentry
    dentry->file_type = bb->direntries[index].file_type;
    strncpy((int8_t*)dentry->filename, (int8_t*)bb->direntries[index].filename, DENTRY_FILE_NAME_LEN);
    dentry->inode_num = bb->direntries[index].inode_num;
    // strncpy((int8_t*)dentry->reserved, (int8_t*)bb->direntries[index].reserved, DENTRY_RESERVED); //should not change

    // Success
    return 0;
}

/* uint32_t read_data()
 * Reads data from an inode
 * inputs: inode_index  - index of inode to read
 *         offset       - starting point to read in file in bytes
 *         buf          - array of bytes read
 *         length       - number of bytes to read
 * 
 * outputs: 0       - EOF reached (length > file length)
 *          -1      - error
 *          else    - number of bytes read
 * side effects: Populates buf with the bytes read
 */
uint32_t read_data(uint32_t inode_index, uint32_t offset, uint8_t* buf, uint32_t length) {
    inode* src;
    int32_t bytes_to_read;
    int32_t i, curr_data_block;
    int32_t result, addr;
    int32_t block_offset, buf_offset;
    int8_t* buf_ptr;
    int32_t count;
    result = 0;

    // inode valid?
    if(inode_index < 0 || inode_index >= bb->inode_count) return -1;

    // Get pointer to desired inode
    src = (inode*)(inodes + inode_index * KB);

    // Offset valid?
    if(offset < 0 || offset >= src->length) return 0;

    // Read length
    bytes_to_read = length;
    if(offset + bytes_to_read > src->length) {                 // EOF? 
        bytes_to_read -= offset + bytes_to_read - src->length;
    }

    buf_ptr = (int8_t*) buf;                // pointer to buffer
    block_offset = offset;                  // offset in bytes from current block
    buf_offset = 0;                       
    i = offset/FOUR_KB;                        // index of blocks array in inode
    if(i > 0) block_offset -= i*FOUR_KB;
    curr_data_block = src->data_blocks[i];  // data block index

    if(block_offset != 0) {                 // starting mid block?
        addr = (int32_t)data + curr_data_block*FOUR_KB + block_offset;
        if(block_offset + bytes_to_read < FOUR_KB) {  // also ends mid block?
            // strncpy(buf_ptr, (int8_t*)addr, bytes_to_read);
            for(count = 0; count < bytes_to_read; count ++){
                buf_ptr[count] = ((int8_t*)addr)[count];
            }
            result += bytes_to_read;
            bytes_to_read = 0;
        }
        else {
            // strncpy(buf_ptr, (int8_t*)addr, FOUR_KB-block_offset);
            for(count = 0; count < FOUR_KB-block_offset; count ++){
                buf_ptr[count] = ((int8_t*)addr)[count];
            }
            bytes_to_read -= FOUR_KB-block_offset;
            buf_offset += FOUR_KB-block_offset;
            result += FOUR_KB-block_offset;
            i++;
            curr_data_block = src->data_blocks[i];
        }

    }

    while(bytes_to_read/FOUR_KB > 0) {         // whole block
        addr = (int32_t)data + curr_data_block*FOUR_KB;
        // strncpy(buf_ptr + buf_offset, (int8_t*)addr, FOUR_KB);
        for(count = 0; count < FOUR_KB; count ++){
            (buf_ptr + buf_offset)[count] = ((int8_t*)addr)[count];
        }
        bytes_to_read -= FOUR_KB;
        buf_offset += FOUR_KB;
        result += FOUR_KB;
        i++;
        curr_data_block = src->data_blocks[i];
    }

    if(bytes_to_read > 0) { 
        addr = (int32_t)data + curr_data_block*FOUR_KB;
        // strncpy(buf_ptr + buf_offset, (int8_t*)addr, bytes_to_read);
        for(count = 0; count < bytes_to_read; count ++){
            (buf_ptr + buf_offset)[count] = ((int8_t*)addr)[count];
        }
        result += bytes_to_read;
    }

    return result;
}


/* uint32_t file_read()
 * File read() reads count bytes of data from file into buf
 * inputs: fd           - file descriptor
 *         buf          - array to write into
 *         nbytes       - number of bytes to read
 * outputs: This call returns the number of bytes read.
 * side effects: file_descriptor.position incremented
 */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes){ // TODO: offset
    int i;
    pcb_struct* pcb = (pcb_struct*)(EIGHT_MB - EIGHT_KB*(process_num+1));
    uint32_t read_bytes;

    // Check if the file is closed
    if(pcb->file_descriptor[fd].flags == 0){
        return -1;
    }

    //Clear buffer
    for(i=0; i<nbytes; i++){ // for (BUFFER_LIM - 1) chars
        ((char*)buf)[i] = '\0';  // fill it up with NULLs
    }
    memset(buf, 0, nbytes);
    
    // Read the data into the buffer
    read_bytes = read_data(pcb->file_descriptor[fd].inode, pcb->file_descriptor[fd].file_position, (uint8_t*)buf, nbytes);
    if(read_bytes == -1){
        return -1; // failed to read data
    }
    pcb->file_descriptor[fd].file_position += read_bytes;
    return read_bytes;
}

/* uint32_t file_write()
 * Should always return -1 to indicate failure since the file system is read-only.
 * inputs:  fd     - Not used
 *          buf    - Not used
 *          nbytes - Not used
 * outputs: -1    - should always be failure
 * side effects: none
 */
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes){
    return -1;
}


/* uint32_t file_open()
 * File open() initialize any temporary structures, return 0
 * inputs:   filename    -    name of the file you wish to open
 *           fd          -    file descriptor
 * outputs:  0   -   Successfuly opened
 *          -1   -   If the named file does not exist or no descriptors are free
 * side effects: file_descriptor.flag and file_descriptor.inode set
 */
int32_t file_open(const uint8_t* filename){ // TODO: remove int32_t fd for the next checkpoint
    //Goal: The call should find the directory entry corresponding to the named file, 
    // allocate an unused file descriptor, and set up any data necessary to handle the given type of file.
    //“Opening” these files consists of storing appropriate jump tables in these two locations in the file array, and marking the files as in-use.
    
    // If the named file does not exist or no descriptors are free, the call returns -1.

    dentry_t dentry;
    pcb_struct* pcb = (pcb_struct*)(EIGHT_MB - EIGHT_KB*(process_num+1));
    int i;
    int32_t fd = -1;   //invalid fd (used if all fd slots are open/unavailable)  

    for(i = FD_MIN; i <= FD_MAX; i++){     //Find an open slot in the fd
        if(pcb->file_descriptor[i].flags == 0){
            fd = i;
            break;
        }
    }

    if((fd == -1) || fd < FD_MIN || fd > FD_MAX){ // invalid descriptor (none existing, stdin, stdout)
        return -1;
    }
    if(read_dentry_by_name(filename, &dentry) == -1){ // If the named file does not exist
        return -1; 
    }
    if(dentry.file_type != REGULAR_FILE_TYPE){ // If wrong file type
        return -1;
    }

    if(pcb->file_descriptor[fd].flags == 0){
        pcb->file_descriptor[fd].file_operations_table_pointer = &file_fop;
        pcb->file_descriptor[fd].inode = dentry.inode_num; // 0 for directories and RTC
        pcb->file_descriptor[fd].file_position = 0; // starts at the initial position
        pcb->file_descriptor[fd].flags = 1; // descriptor in-use
    }else{
        return -1; // descriptor is not free
    }
    return fd;
}

/* uint32_t file_close()
 * File close() undo what you did in the open function
 * inputs:   fd  -   File Descriptor
 * outputs:  0   -   Successfuly closed
 *          -1   -   invalid descriptor (none existing, stdin, stdout)
 * side effects: file_descriptor.flag cleared
 */

int32_t file_close(int32_t fd){
    pcb_struct* pcb = (pcb_struct*)(EIGHT_MB - EIGHT_KB*(process_num+1));
    if(fd < FD_MIN || fd > FD_MAX){ // invalid descriptor (none existing, stdin, stdout)
        return -1;
    }
    
    if(pcb->file_descriptor[fd].flags == 1){
        pcb->file_descriptor[fd].flags = 0; // descriptor not used
    }else{
        // Do nothing, descriptor is free already
    }
    return 0;
}



/* uint32_t directory_read()
 * Directory read() read files filename by filename, including “.”
 * inputs: fd       -    file descriptor
 *         buf      -    buffer to read the file into
 *         nbytes   -    Not used
 * outputs: This call returns the number of bytes read.
 * side effects: file_descriptor.position incremented
 */
int32_t directory_read(int32_t fd, void* buf, int32_t nbytes){
// Goal: In the case of reads to the directory, only the filename should be provided (as much as fits, or all 32 bytes), and
// subsequent reads should read from successive directory entries until the last is reached, at which point read should
// repeatedly return 0.
    
    dentry_t dentry;
    int idx;
    pcb_struct* pcb = (pcb_struct*)(EIGHT_MB - EIGHT_KB*(process_num+1));

    // Check for invalid descriptor
    if(fd < FD_MIN || fd > FD_MAX){ // invalid descriptor (none existing, stdin, stdout)
        return -1;
    }

    // Check if the file is closed
    if(pcb->file_descriptor[fd].flags == 0){
        return -1;
    }

    // If the initial file position is at or beyond the end of file, 0 shall be returned 
    if(pcb->file_descriptor[fd].file_position >= bb->dir_count){
        return 0;
    }

    //Clear buffer
    for(idx=0; idx<nbytes; idx++){ // for (BUFFER_LIM - 1) chars
        ((char*)buf)[idx] = '\0';  // fill it up with NULLs
    }
    memset(buf, 0, nbytes);

    // Grab the dentry
    if(read_dentry_by_index(pcb->file_descriptor[fd].file_position, &dentry) == -1){ 
        return -1;//failed
    }

    // Copy the file name from the dentry to the buffer you passed in
    for(idx = 0; idx < DENTRY_FILE_NAME_LEN; idx ++){
        ((uint8_t*)buf)[idx] = dentry.filename[idx];
    }

    // printf("File type: %d, ", dentry.file_type);

    // Increment the file position by 1 each time you read a directory
    pcb->file_descriptor[fd].file_position++;

    // Return the bytes read
    return DENTRY_FILE_NAME_LEN;// Always reading full 32 bytes

}

/* uint32_t directory_write()
 * Should always return -1 to indicate failure since the file system is read-only.
 * inputs:  fd     - Not used
 *          buf    - Not used
 *          nbytes - Not used
 * outputs: -1    - should always be failure
 * side effects: none
 */
int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes){
    return -1;
}


/* uint32_t directory_open()
 * Directory open() opens a directory file
 * inputs:  filename - directory name to open
 *          fd       - file descriptor
 * outputs:  0   -   Successfuly opened
 *          -1   -   If the named file does not exist or no descriptors are free
 * side effects: file_descriptor.flag set
 */
int32_t directory_open(const uint8_t* filename){
    //Goal: The call should find the directory entry corresponding to the named file, 
    //allocate an unused file descriptor, and set up any data necessary to handle the given type of file (directory,
    //RTC device, or regular file)

    dentry_t dentry;
    pcb_struct* pcb = (pcb_struct*)(EIGHT_MB - EIGHT_KB*(process_num+1));
    
    int i;
    int32_t fd = -1;   //invalid fd (used if all fd slots are open/unavailable)  

    for(i = FD_MIN; i <= FD_MAX; i++){     //Find an open slot in the fd
        if(pcb->file_descriptor[i].flags == 0){
            fd = i;
            break;
        }
    }

    if((fd == -1) || fd < FD_MIN || fd > FD_MAX){ // invalid descriptor (none existing, stdin, stdout)
        return -1;
    }
    if(read_dentry_by_name(filename, &dentry) == -1){ // If the named file does not exist
        return -1; 
    }
    if(dentry.file_type != DIRECTORY_FILE_TYPE){ // If wrong file type
        return -1;
    }

    if(pcb->file_descriptor[fd].flags == 0){
        pcb->file_descriptor[fd].file_operations_table_pointer = &directory_fop;
        pcb->file_descriptor[fd].inode = 0; // 0 for directories and RTC
        pcb->file_descriptor[fd].file_position = 0; // starts at the initial position
        pcb->file_descriptor[fd].flags = 1; // descriptor in-use
    }else{
        return -1; // descriptor is not free
    }
    return fd;
}

/* uint32_t directory_close()
 * Directory close() does nothing
 * inputs:   fd  - File descriptor
 * outputs:  0   -   Always 0
 * side effects: none
 */
int32_t directory_close(int32_t fd){
    pcb_struct* pcb = (pcb_struct*)(EIGHT_MB - EIGHT_KB*(process_num+1));

    if(fd < FD_MIN || fd > FD_MAX){ // invalid descriptor (none existing, stdin, stdout)
        return -1;
    }

    if(pcb->file_descriptor[fd].flags == 1){
        pcb->file_descriptor[fd].flags = 0; // descriptor in-use
    }else{
        return -1; // descriptor is not free
    }
    return 0; // Doesn't need to do anything it seems from the discussion slides
}
