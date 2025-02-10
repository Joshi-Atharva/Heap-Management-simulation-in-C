#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "heap.h"

HeapElement heap[HEAP_SIZE];
DNode* flptr; DNode* alptr; // free and allocated list pointers
DNode* dlptr; // pointer to dictionary List

// ------helper functions------
    void setData(address_t addr, data_t d) {
        extern HeapElement heap[];
        heap[addr].element.data = d;
        heap[addr].isHeader = FALSE;
    }
    // sets heap[addr].element.header to dptr
    void setHeader(address_t addr, DNode hNode) {
        extern HeapElement heap[];
        heap[addr].element.header = hNode;
        heap[addr].isHeader = TRUE;
    }
    metadata_t MakeMetadata(address_t start_val, address_t end_val, const char* name_str, Boolean isFree_val) {
        metadata_t ret_val;
        ret_val.start = start_val;
        ret_val.end = end_val;
        strcpy(ret_val.varName, name_str);
        ret_val.isFree = isFree_val;
        return ret_val;
    }
    // checks if varName_str is already in variable names dictionary, returns address if yes
    address_t SearchInDict(const char* varName_str) {
        address_t ret_val;
        ret_val = -1;
        extern DNode* dlptr;
        DNode* ptr = dlptr; Boolean found = FALSE, done = FALSE;
        while( ptr != NULL && !found && !done ) {
            if( strcmp(ptr->metadata.varName, varName_str) > 0 ) {
                done = TRUE;
            }
            else if( strcmp(ptr->metadata.varName, varName_str) == 0 ) {
                done = TRUE; found = TRUE;
            }
            else {
                ptr = ptr->next;
            }
        }
        if( found ) {
            ret_val = ptr->metadata.start;
        }
        else {
            ret_val = -1;
        }

        return ret_val;
    }
    Boolean FindAdjacentFree(address_t addr, DNode **prevpptr, DNode **nextpptr) {
        extern DNode* flptr;
        DNode *fprev, *fnext;
        fprev = NULL; fnext = flptr; Boolean found = FALSE, done = FALSE;
        while( fnext != NULL && !done ) {
            if( fprev != NULL ) { 
                if( (fprev->metadata.end < addr) && (addr < fnext->metadata.start) ) {
                    done = TRUE; found = TRUE;
                }
                else if( addr < fprev->metadata.start ){
                    done = TRUE;
                }
                else {
                    fprev = fnext;
                    fnext = fnext->next;
                }
            }
            else {
                if( (addr < fnext->metadata.start) ) {
                    done = TRUE; found = TRUE;
                }
                else {
                    fprev = fnext;
                    fnext = fnext->next;
                }
            }
        }
        if( found ) {
            *prevpptr = fprev;
            *nextpptr = fnext;
        }
        return found;
    }


// display functions
void printHeap() {
    extern HeapElement heap[];
    char elementType[50];
    for(address_t i = 0; i<HEAP_SIZE; i++) {
        if( heap[i].isHeader == FALSE ) { 
            strcpy(elementType, "Data");
        }
        else {
            strcpy(elementType, "Header");
        }
        if( heap[i].isHeader == FALSE ) {
            printf(
                "Heap element %d:\n"
                    "\telement type: %s\n"
                    "\telement value: %d\n", i, elementType, heap[i].element.data
            );
        }
        else {
            printf(
                "Heap element %d:\n"
                    "\telement type: %s\n"
                    "\telement: %p\n", i, elementType, &heap[i].element.header
            );
        }
    }
}
void fprintHeap(const char* filename) {
    extern HeapElement heap[];
    FILE* file_ptr = fopen(filename, "w");
    char elementType[50];
    for(address_t i = 0; i<HEAP_SIZE; i++) {
        if( heap[i].isHeader == FALSE ) { 
            strcpy(elementType, "Data");
        }
        else {
            strcpy(elementType, "Header");
        }
        if( heap[i].isHeader == FALSE ) {
            fprintf(file_ptr,
                "Heap element %d:\n"
                    "\telement type: %s\n"
                    "\telement value: %d\n", i, elementType, heap[i].element.data
            );
        }
        else {
            fprintf(file_ptr,
                "Heap element %d:\n"
                    "\telement type: %s\n"
                    "\t\tvariable name: %s\n"
                    "\t\tstart: %d, end: %d, isFree = %s\n", 
                    i, elementType, 
                    heap[i].element.header.metadata.varName, 
                    heap[i].element.header.metadata.start,
                    heap[i].element.header.metadata.end,
                    (heap[i].element.header.metadata.isFree==TRUE)?"TRUE":"FALSE"
            );
        }
    }
    fclose(file_ptr);
}
void PrintStatus(const char* opcode, status_code sc) {
    if( sc == SUCCESS ) {
        printf("Operation: %s successful\n", opcode);
    }
    else {
        printf("Operation: %s failed\n", opcode);
    }
}

// ------core operations------
    void InitialiseHeap() { 
        extern DNode* flptr; extern DNode* alptr; extern DNode* dlptr;
        extern HeapElement heap[];
        DNode nhNode;
        nhNode.metadata = MakeMetadata(1, HEAP_SIZE - 1, "HEAP", TRUE);
        nhNode.next = NULL; nhNode.prev = NULL;
        setHeader(0, nhNode);
        flptr = &(heap[0].element.header);

        // Initialising variable list 
        dlptr = NULL;
    }
    address_t Allocate(const char* varName_str, int size) {
        extern DNode* flptr; extern HeapElement heap[];
        address_t ret_addr;
        DNode* fptr = flptr; Boolean found = FALSE;
        status_code sc = SUCCESS;
        if( SearchInDict(varName_str) != -1 ) { /* error condition: var name in Dictionary */
            ret_addr = -1;
        }
        else {
            while( fptr != NULL && !found ) {
                if( fptr->metadata.isFree && (fptr->metadata.end - fptr->metadata.start + 1 >= size) ) {
                    found = TRUE;
                }
                else {
                    fptr = fptr->next;
                }
            }

            if( found ) {
                /* update free list */
                DNode* prevptr = fptr->prev; DNode* nextptr = fptr->next;


                address_t nextFreeAddr = fptr->metadata.start + size;
                if( fptr->metadata.end - nextFreeAddr <= 1 ) { 
                /* ie if free partition is too small - allocate full space */
                    size = fptr->metadata.end - fptr->metadata.start + 1;
                    if( prevptr != NULL ) {
                        prevptr->next = nextptr;
                    }
                    else {
                        flptr = nextptr;
                    }
                }
                else { /* nextFreeAddr - fptr->metadata.end > 1 */
                    // create partition if size required is sufficiently less than the free block size
                    DNode nexthNode;
                    nexthNode.metadata = MakeMetadata(nextFreeAddr+1, fptr->metadata.end, "HEAP", TRUE);
                    // update heap[] with header 
                    nexthNode.next = nextptr;
                    nexthNode.prev = prevptr;
                    heap[nextFreeAddr].element.header = nexthNode;
                    heap[nextFreeAddr].isHeader = TRUE;
                    // update freelist
                    if( prevptr != NULL ) {
                        prevptr->next = &(heap[nextFreeAddr].element.header);
                    }
                    else {
                        flptr = &(heap[nextFreeAddr].element.header);
                    }
                }


                /* Allocated List update */
                sc = InsertToDict(varName_str, fptr->metadata.start - 1);
                if(sc) {
                    /* convert to allocated Node */
                    fptr->metadata.isFree = FALSE;
                    fptr->metadata.end = fptr->metadata.start + size - 1;
                    fptr->metadata.start = fptr->metadata.start - 1;
                    strcpy(fptr->metadata.varName, varName_str);
                    InsertIntoAlloc(fptr->metadata.start);
                    ret_addr = fptr->metadata.start + 1;
                }
                else ret_addr = -1;
            }
            else { /* !found */
                ret_addr = -1; // not allocated
            }

        }
        return ret_addr;
    }
    status_code Assign(const char* varName_str, int offset, data_t d) {
        extern HeapElement heap[];
        address_t addr = SearchInDict(varName_str);
        status_code sc = SUCCESS;
        if( addr == -1 ) sc = FAILURE;
        else { // addr != -1
            if( (addr + 1 + offset) <= heap[addr].element.header.metadata.end ) {
                heap[addr+1+offset].element.data = d;
                heap[addr+1+offset].isHeader = FALSE;
            }
            else {
                sc = FAILURE; printf("ERROR: Out of bounds reference for variable %s\n", varName_str);
            }
        }
        return sc;
    }
    status_code Free(const char* varName_str) {
        status_code sc = SUCCESS;
        extern DNode* flptr;
        address_t addr = SearchInDict(varName_str);
        DNode *fprev, *fnext;
        
        /* search for adjacent blocks in free list */
        Boolean found = FindAdjacentFree(addr, &fprev, &fnext);

        /* remove from allocated list */
        if( addr != -1 ) {
            DeleteFromAlloc(addr);
        }

        if( found && (addr != -1) ) {
            /* check if contiguous */
            if( fprev != NULL ) {
                if( (fprev->metadata.end + 1 == addr) && heap[addr].element.header.metadata.end + 1 == fnext->metadata.start - 1 ) {
                    /* merge three */
                    heap[addr].isHeader = FALSE;
                    heap[addr].element.header.metadata.isFree = TRUE;

                    address_t nextAddr = fnext->metadata.start - 1;
                    address_t newEnd = fnext->metadata.end;

                    fprev->metadata.end = newEnd;
                    fprev->next = fnext->next;
                    if( fnext->next != NULL ) {
                        (fnext->next)->prev = fprev;
                    }

                    heap[nextAddr].isHeader = FALSE;

                }
                else if( fprev->metadata.end + 1 == addr ) {
                    /* merge first 2*/
                    fprev->metadata.end = heap[addr].element.header.metadata.end;
                    fprev->next = fnext; // trivial assignment

                    heap[addr].isHeader = FALSE;
                }

            } /* end of conditional fprev != NULL */
            else if( heap[addr].element.header.metadata.end + 1 == fnext->metadata.start - 1 ) {
                /* merge last 2*/
                heap[addr].element.header.metadata.end = fnext->metadata.end;
                heap[addr].element.header.next = fnext->next;
                heap[addr].element.header.prev = fprev;
                heap[addr].element.header.metadata.start = heap[addr].element.header.metadata.start + 1; // due to different conventions/meanings of start address of allocated node and free node
                address_t nextAddr = fnext->metadata.start - 1;
                heap[nextAddr].isHeader = FALSE;

                /* reinitialise flptr */
                flptr = &(heap[addr].element.header);
            }
            else { /* none contiguous */
                if(fprev != NULL) { /* this condition will never arise - if that is the case then if part will be executed */
                    fprev->next = &(heap[addr].element.header);
                }
                else { /* fprev = NULL =>  reinitialise flptr */
                    if( flptr != NULL ) { /* InsertAtStart operation */
                        flptr->prev = &(heap[addr].element.header);
                        heap[addr].element.header.next = flptr;
                    }
                    flptr = &(heap[addr].element.header);
                }
                heap[addr].element.header.prev = fprev;
                heap[addr].element.header.next = fnext;
                fnext->prev = &(heap[addr].element.header);
                heap[addr].element.header.metadata.start = heap[addr].element.header.metadata.start + 1; // due to different conventions/meanings of start address of allocated node and free node
            }
            
            sc = DeleteFromDict(varName_str);
        }
        else { /* !found */
            sc = FAILURE;
        }
        return sc;
    }

// -----secondary core------
    void InsertIntoAlloc(address_t newStart) {
        extern DNode* alptr; extern HeapElement heap[];
        DNode* aptr = alptr, *prev = NULL;
        Boolean found = FALSE;
        while( aptr != NULL && !found ) {
            if( aptr->metadata.start < newStart ) {
                prev = aptr;
                aptr = aptr->next;
            }
            else {
                found = TRUE;
            }
        }

        if( prev != NULL ) {    
            prev->next = &(heap[newStart].element.header);
        }
        else { /* update first pointer of allocated list */
            alptr = &(heap[newStart].element.header);
        }

        if( aptr != NULL ) {
            aptr->prev = &(heap[newStart].element.header);
        }

        heap[newStart].element.header.next = aptr;
        heap[newStart].element.header.prev = prev;
    }
    void DeleteFromAlloc(address_t addr) {
        extern DNode* alptr;
        DNode* aprev, *anext;
        aprev = heap[addr].element.header.prev;
        anext = heap[addr].element.header.next;
        heap[addr].element.header.metadata.isFree = TRUE;

        if( aprev != NULL ) {
            aprev->next = anext;
        }
        else { /* aprev = NULL => heap[addr].element.header is first node (alptr) */
            alptr = anext;
        }
        if( anext != NULL ) {
            anext->prev = aprev;
        }
    }

    status_code InsertToDict(const char* varName_str, address_t addr) {
        extern DNode* dlptr;
        status_code sc = SUCCESS;

        /* Searching */
        DNode* dptr = dlptr; DNode* dprev = NULL;
        Boolean found = FALSE;
        while( dptr != NULL && !found ) {
            if( strcmp(dptr->metadata.varName, varName_str) > 0 ) {
                found = TRUE;
            }
            else {
                dprev = dptr;
                dptr = dptr->next;
            }
        }
        
        /* Creating New Node and Insertion */
        DNode* nptr = (DNode*)malloc(sizeof(DNode));
        if( nptr != NULL ) {
            nptr->metadata = MakeMetadata(addr, -1, varName_str, FALSE);
            if( dprev != NULL ) {
                dprev->next = nptr;
                nptr->next = dptr;
                nptr->prev = dprev;
                if( dptr != NULL ) {
                    dptr->prev = nptr;
                }
            }
            else { // dprev == NULL
                nptr->next = dlptr;
                nptr->prev = NULL;
                if( dlptr != NULL ) dlptr->prev = nptr;
                dlptr = nptr;
            }

        }
        else { // nptr == NULL
            sc = FAILURE;
        }
        
        return sc;
    }
    status_code DeleteFromDict(const char* varName_str) {
        status_code sc = SUCCESS;
        extern DNode* dlptr;
        DNode* dptr = dlptr, *dprev = NULL; Boolean found = FALSE; 
        while( dptr != NULL && !found ) {
            if( strcmp(dptr->metadata.varName, varName_str) == 0 ) {
                found = 1;
            }
            else {
                dprev = dptr;
                dptr = dptr->next;
            }
        }
        /* remove node */
        if( found ) {
            dprev->next = dptr->next;
            if( dptr->next != NULL ) {
                (dptr->next)->prev = dprev;
            }
            free(dptr); dptr = NULL;
        }
        else {
            sc = FAILURE;
        }
        return sc;
    }

    void FreeDictList() {
    extern DNode* dlptr;
    DNode* dptr = dlptr, *prev = NULL;
    while( dptr != NULL ) {
        prev = dptr;
        dptr = dptr->next;
        free(dptr);
    }
}