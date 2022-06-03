#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"
#include "register.h"
#define _crt_secure_no_warnings

#define buffer_size 1024
#define memory_size 0x100000
#define register_size 32

unsigned int invertEndian(unsigned int data)
{
    unsigned char c[4];

    c[3] = (unsigned char)data; data = data >> 8;
    c[2] = (unsigned char)data; data = data >> 8;
    c[1] = (unsigned char)data; data = data >> 8;
    c[0] = (unsigned char)data;

    return *(unsigned int*)c;
}

const int M_SIZE = 1024;
unsigned char MEMORY[M_SIZE];

unsigned int memoryRead(unsigned int addr) {
    unsigned int data;

    if ((addr & 3) != 0) { // check unaligned access?
        printf("memoryRead(): Unaligned access error!\n");
        addr &= 0xFFFFFFFC; // make aligned address
    }

    data = *(unsigned int*)&MEMORY[addr];
    data = invertEndian(data);

    return data;
}

void memoryWrite(unsigned int addr, unsigned int data) {
    if ((addr & 3) != 0) { // check unaligned access?
        printf("memoryWrite(): Unaligned access error!\n");
        addr &= 0xFFFFFFFC; // ignore the least 2 bits
    }

    *(unsigned int*)&MEMORY[addr] = invertEndian(data);

    return;
}

unsigned int IR;   // Instruction Register

// R-type instructin decoding
void decodeRtype(unsigned int fct)
{
    unsigned int fcth, fctl;

    fctl = fct & 0x7;
    fcth = (fct & 0x38) >> 3;

    if (fcth == 0) {
        if (fctl == 0) printf("sll\n");
        else if (fctl == 2) printf("srl\n");
        else if (fctl == 4) printf("sra\n");
        else printf("Undefined instruction\n");
    }
    else if (fcth == 1) {
        if (fctl == 0) printf("jr\n");
        else if (fctl == 4) printf("syscall\n");
        else printf("Undefined instruction\n");
    }
    else if (fcth == 2) {
        if (fctl == 0) printf("mfhi\n");
        else if (fctl == 2) printf("mflo\n");
        else printf("Undefined instruction\n");
    }
    else if (fcth == 3) {
        if (fctl == 0) printf("mul\n");
        else printf("Undefined instruction\n");
    }
    else if (fcth == 4) {
        if (fctl == 0) printf("add\n");
        else if (fctl == 2) printf("sub\n");
        else if (fctl == 4) printf("and\n");
        else if (fctl == 5) printf("or\n");
        else if (fctl == 6) printf("xor\n");
        else if (fctl == 7) printf("nor\n");
        else printf("Undefined instruction\n");
    }
    else if (fcth == 5) {
        if (fctl == 2) printf("slt\n");
        else printf("Undefined instruction\n");
    }
    else printf("Undefined instruction\n");
}

// instruction decoding
void instructionDecode(void)
{
    unsigned int opc, fct;

    unsigned int opch, opcl;

    opc = IR >> 26;
    fct = IR & 0x3f;

    printf("Opc: %2x, Fct: %2x, Inst: ", opc, fct);

    opcl = opc & 0x7;
    opch = (opc & 0x38) >> 3;

    if (opch == 0) {
        if (opcl == 0) {
            decodeRtype(fct);
        }
        else if (opcl == 1) printf("bltz\n");
        else if (opcl == 2) printf("j\n");
        else if (opcl == 3) printf("jal\n");
        else if (opcl == 4) printf("beq\n");
        else if (opcl == 5) printf("bne\n");
        else printf("Undefined instruction\n");
    }
    else if (opch == 1) {
        if (opcl == 0) printf("addi\n");
        else if (opcl == 2) printf("slti\n");
        else if (opcl == 4) printf("andi\n");
        else if (opcl == 5) printf("ori\n");
        else if (opcl == 6) printf("xori\n");
        else if (opcl == 7) printf("lui\n");
        else printf("Undefined instruction\n");
    }
    else if (opch == 4) {
        if (opcl == 0) printf("lb\n");
        else if (opcl == 3) printf("lw\n");
        else if (opcl == 4) printf("lbu\n");
        else printf("Undefined instruction\n");
    }
    else if (opch == 5) {
        if (opcl == 0) printf("sb\n");
        else if (opcl == 3) printf("sw\n");
        else printf("Undefined instruction\n");
    }
    else printf("Undefined instruction\n");
}

int main(void) {
    char* cmd = (char*)malloc(sizeof(char) * 100); //command
    char* filename = (char*)malloc(sizeof(char) * 100);
    char* rn = (char*)malloc(sizeof(char) * 10); //input number
    char* value = (char*)malloc(sizeof(char) * 10); //input value
    unsigned int temp_rn, temp_v;
    int EXIT = 1, is_loaded = 0, step_result = 0;

    while (EXIT) {
        printf("enter command: ");
        fflush(stdout);
        scanf_s("%s", cmd, 20 * sizeof(char));
        getchar();
        if (!strcmp(cmd, "x")) {
            printf("exit the program\n");
            EXIT = 0;
            break;
        }
        else if (!strcmp(cmd, "l")) { //load
            errno_t err;
            FILE* fptr = NULL;
            err = fopen_s(&fptr, "as_ex01_arith.bin", "rb");
            if (err) {
                printf("can not open file\n");
                break;
            }
            else {
                printf("file is opened\n");
            }

            unsigned char binary_buffer[1024];
            unsigned int data;
            unsigned int addr;

            unsigned int iCount;   // # of instructions
            unsigned int dCount;   // # of data
            // read instruction and data numbers

            if (fread(&data, sizeof(data), 1, fptr) < 0) {
                printf("!");
            }


            iCount = invertEndian(data);
            fread(&data, sizeof(data), 1, fptr);
            dCount = invertEndian(data);
            printf("number of instructions: %d, number of data: %d\n", iCount, dCount);

            //=======================================================
            // load to memory

        // Load to memory
            addr = 0;
            for (int i = 0; i < (int)iCount; i++) {
                fread(&data, sizeof(unsigned int), 1, fptr);
                //int offset = i + 2; // 명령어 길이, 데이터 길이 워드 다음 인덱스
                data = invertEndian(data);
                //printf("In file, instruction is: ");

                MEM(0x400000 + 4 * i, data, 1, 2);
                addr += 4;
            }
            for (int i = 0; i < (int)dCount; i++) {
                fread(&data, sizeof(unsigned int), 1, fptr);
                //int offset = i + 2 + iCount; // 명령어 길이, 데이터 길이, 명령어 워드 다음 인덱스
                data = invertEndian(data);
                printf("파일에서 읽은 데이터: %x\n", data);
                MEM(0x10000000 + 4 * i, data, 1, 2);

            }

            fclose(fptr);

        }
        else if (!strcmp(cmd, "m")) {
            viewMemory(STAK_START, STAK_START + 8);
        }
        else if (!strcmp(cmd, "r")) {
            viewRegister();
        }
        else if (!strcmp(cmd, "g")) {
            //goProgram();
            
        }
        else if (!strcmp(cmd, "s")) {
            //stepProgram();
        }
        else if (!strcmp(cmd, "j")) {
            //stepProgram();
        }
        else if (!strcmp(cmd, "sm")) {
            //stepProgram();
        }
        else if (!strcmp(cmd, "sr")) {
            //stepProgram();
        }
         
    }
    printf("program exit\n");
    free(filename);
    free(cmd);
    free(rn);
    free(value);


    return 0;
}