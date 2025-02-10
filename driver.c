#include "heap.h"
#include <stdio.h>
#include <string.h>

int main() {
    address_t addr; status_code sc;
    InitialiseHeap();
    fprintHeap("HeapStateInit.txt");
    int choice;
    Boolean Exit = FALSE;
    char var[NAME_SIZE]; int size; data_t d; int offset;
    /* opening the log file */
    FILE* file_ptr = fopen("HeapLog.txt", "w");
    while(Exit == FALSE) {
        printf("Enter number corresponding to option:\n"
            "\t1) Allocate\n"
            "\t2) Assign\n"
            "\t3) Free\n"
            "\t4) Write current Heap state to file\n"
            "\t0) Exit\n"
        );
        scanf("%d", &choice);
        switch(choice) {
            case 1:
                
                printf("Enter variable name: ");
                scanf("%s", var);
                printf("Enter Size: ");
                scanf("%d", &size);
                addr = Allocate(var, size); sc = (addr==-1)?(FAILURE):(SUCCESS); PrintStatus("Allocate()", sc);
                if( file_ptr != NULL ) {
                    fprintf(file_ptr, "Op: Allocate(%s, %d) %s\n",var, size, (sc==SUCCESS)?("successful"):("failed"));
                    fflush(file_ptr);
                }
                
                break;
            case 2:
                printf("Enter LHS (variable name): ");
                scanf("%s", var);
                printf("Enter address offset: ");
                scanf("%d", &offset);
                printf("Enter RHS (data literal): "); /* scope of further improvement: assigning value of one var to another */
                scanf("%d", &d);
                sc = Assign(var, offset, d); PrintStatus("Assign()", sc);
                if( file_ptr != NULL ) {
                    fprintf(file_ptr, "Op: Assign(%s, %d, %d) %s\n",var, offset, size, (sc==SUCCESS)?("successful"):("failed"));
                    fflush(file_ptr);                    
                }
                break;
            case 3:
                printf("Enter variable to be freed: ");
                scanf("%s", var);
                sc = Free(var); PrintStatus("Free(var)", sc);
                if( file_ptr != NULL ) {
                    fprintf(file_ptr, "Op: Free(%s) %s\n", var, (sc==SUCCESS)?("successful"):("failed"));
                    fflush(file_ptr);                      
                }
                break;
            case 4:
                printf("Enter the file name: ");
                scanf("%s", var);
                if( file_ptr != NULL ) {
                    fprintf(file_ptr, "Op: Written into file %s\n", var);
                    fflush(file_ptr);
                }
                fprintHeap(var);
                break;
            case 0: 
                Exit = TRUE;
                if( file_ptr != NULL ) {
                    fprintf(file_ptr, "EXITING\n");
                    fflush(file_ptr);
                }
                break;
            default:
                Exit = TRUE;
        }

    }
    fprintHeap("HeapState.txt");
    fclose(file_ptr); // closing log file

    // freeing dictionary linked list
    FreeDictList();

    return 0;
}