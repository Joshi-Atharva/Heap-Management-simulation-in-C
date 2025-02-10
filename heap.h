#ifndef HEAP_H
#define HEAP_H

#define HEAP_SIZE 1000
#define NAME_SIZE 100
typedef enum{FALSE, TRUE} Boolean;
typedef enum{FAILURE, SUCCESS} status_code;

typedef int address_t;

typedef struct MetadataTag {
    address_t start;
    address_t end;
    char varName[NAME_SIZE];
    Boolean isFree;
}metadata_t;

typedef int data_t;

typedef struct DLLNodeTag {
    metadata_t metadata;
    struct DLLNodeTag* prev;
    struct DLLNodeTag* next;
}DNode;

typedef struct HeapElementTag {
    union u_tag{
        data_t data;
        DNode header;
    }element;
    Boolean isHeader;
}HeapElement;

typedef struct DictItemTag {
    char varName[NAME_SIZE];
    address_t addr;
}dict_t;

// ------helper functions------
    void setData(address_t addr, data_t d);
    // sets heap[addr].element.header to dptr
    void setHeader(address_t addr, DNode hNode);
    metadata_t MakeMetadata(address_t start_val, address_t end_val, const char* name_str, Boolean isFree_val);
    // checks if varName_str is already in variable names dictionary, returns address if yes
    address_t SearchInDict(const char* varName_str);
    Boolean FindAdjacentFree(address_t addr, DNode **prevpptr, DNode **nextpptr);

// display functions
void printHeap();
void fprintHeap(const char* filename);
void PrintStatus(const char* opcode, status_code sc);

// ------core operations------
    void InitialiseHeap();
    address_t Allocate(const char* varName_str, int size);
    status_code Assign(const char* varName_str, int offset, data_t d);
    status_code Free(const char* varName_str);
// -----secondary core------
    void InsertIntoAlloc(address_t newStart);
    void DeleteFromAlloc(address_t addr);
    status_code InsertToDict(const char* varName_str, address_t addr);
    status_code DeleteFromDict(const char* varName_str);    
    void FreeDictList();
#endif