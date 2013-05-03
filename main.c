
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "functions.c"

InstInfo * pipelineInsts[5];

typedef enum { FETCH   = 0,
               DECODE  = 1,
               EXECUTE = 2, 
               MEMORY  = 3,
               WRITE   = 4
             } Instructions;

#define isEmpty(step) pipelineInsts[step]->inst == 0

// Defining booleans
typedef int bool;
#define true  1
#define false 0

// Let's just make this c++ already....
#define not !
#define and &&
#define or  ||

int main(int argc, char *argv[])
{
	InstInfo fetchInst;
	InstInfo decodeInst;
        decodeInst.inst = 0;
	InstInfo executeInst;
        executeInst.inst = 0;
	InstInfo memoryInst;
        memoryInst.inst = 0;
	InstInfo writebackInst;
        writebackInst.inst = 0;
	InstInfo *instPtr = &fetchInst;

	int instnum = 0;
	int maxpc;
	FILE *program;
	if (argc != 2)
	{
		printf("Usage: sim filename\n");
		exit(0);
	}

	maxpc = load(argv[1]);
	//printLoad(maxpc);

    pipelineInsts[FETCH]   = instPtr;
    pipelineInsts[DECODE]  = &decodeInst;
    pipelineInsts[EXECUTE] = &executeInst;
    pipelineInsts[MEMORY]  = &memoryInst;
    pipelineInsts[WRITE]   = &writebackInst;

    // Structure to forward registers
    int forwardReg[2];

	while (pc <= maxpc + 4) {
		writeback(pipelineInsts[WRITE]);
		memory(pipelineInsts[MEMORY]);
		execute(pipelineInsts[EXECUTE]);
		decode(pipelineInsts[DECODE]);
		fetch(pipelineInsts[FETCH]);

        printP2(pipelineInsts[FETCH],
                pipelineInsts[DECODE],
                pipelineInsts[EXECUTE],
                pipelineInsts[MEMORY],
                pipelineInsts[WRITE],
                pc - 1);

		// Fill the remainder of the pipeline
		writebackInst = *pipelineInsts[MEMORY];
		memoryInst    = *pipelineInsts[EXECUTE];
		executeInst   = *pipelineInsts[DECODE];
		decodeInst    = *pipelineInsts[FETCH];
		if (instnum <= maxpc) instnum++;
		if (instnum > maxpc)  fetchInst.inst = 0;
		pipelineInsts[FETCH]   = instPtr;

        // Begin pipelining
        // Detect RAW

        // RAW happends from memory to execute, we need to check 
        // if the instruction that was last in execute (now at memory)
        // has a dependency with the instruction currently at execute
        // For example:
        // R2 <- R1 + R3   | x-- m
        // R4 <- R2 + R3   |   ->x
        if (not isEmpty(MEMORY) and not isEmpty(EXECUTE)) {
            // Check for RAW based on type of instruction
            if (pipelineInsts[MEMORY]->destreg == pipelineInsts[EXECUTE]->input1) {
                // printf("FORWARDING THIS MOTHERFUCKER\n\n");
                if (pipelineInsts[EXECUTE]->signals.mtr == 0) { // Forward aluout
                    pipelineInsts[EXECUTE]->s1data = pipelineInsts[MEMORY]->aluout;
                    // printf("THE NEW S1 is: %d\n\n", pipelineInsts[EXECUTE]->s1data);
                }
            } 
            if (pipelineInsts[MEMORY]->destreg == pipelineInsts[EXECUTE]->input2) {
                if (pipelineInsts[EXECUTE]->signals.mtr == 0) // Forward aluout
                    pipelineInsts[EXECUTE]->s2data = pipelineInsts[MEMORY]->aluout;
            } 
            if (pipelineInsts[MEMORY]->destreg == pipelineInsts[EXECUTE]->destreg) {
                if (pipelineInsts[EXECUTE]->signals.mw == 1) {
                    pipelineInsts[EXECUTE]->s2data = (pipelineInsts[MEMORY]->signals.mtr == 0) ?
                                                        pipelineInsts[MEMORY]->aluout :
                                                        pipelineInsts[MEMORY]->memout;
                }
            }
        }
        // Detect a writeback to memory forward (just happend in simulator lol)
        if (not isEmpty(WRITE) and not isEmpty(EXECUTE)) {
            if (pipelineInsts[WRITE]->destreg == pipelineInsts[EXECUTE]->input1) {
                if (pipelineInsts[EXECUTE]->signals.mtr == 0) // Forward aluout
                    pipelineInsts[EXECUTE]->s1data = pipelineInsts[WRITE]->aluout;
            } 
            if (pipelineInsts[WRITE]->destreg == pipelineInsts[EXECUTE]->input2) {
                if (pipelineInsts[EXECUTE]->signals.mtr == 0) // Forward aluout
                    pipelineInsts[EXECUTE]->s2data = pipelineInsts[WRITE]->aluout;
            } 
            if (pipelineInsts[WRITE]->destreg == pipelineInsts[EXECUTE]->destreg) {
                if (pipelineInsts[EXECUTE]->signals.mw == 1) {
                    // printf ("FORWARDING DATA\n");
                    pipelineInsts[EXECUTE]->s2data = (pipelineInsts[WRITE]->signals.mtr == 0) ?
                                                        pipelineInsts[WRITE]->aluout :
                                                        pipelineInsts[WRITE]->memout;
                    // printf ("forwarding %d\n", pipelineInsts[EXECUTE]->s2data);
                }
            }
        }

    }

	// put in your own variables
	printf("Cycles: %d\n", pc);
	printf("Instructions Executed: %d\n", maxpc + 1);
	exit(0);
}



/*
 * print out the loaded instructions.  This is to verify your loader
 * works.
 */
void printLoad(int max)
{
	int i;
	for(i=0;i<max;i++)
		printf("%d\n",instmem[i]);
}

/* print
 *
 * prints out the state of the simulator after each instruction
 */
void print(InstInfo *inst, int count)
{
	int i, j;
	printf("Instruction %d: %d\n",count,inst->inst);
	printf("%s\n\n",inst->string);
	printf("Fields:\nrd: %d\nrs: %d\nrt: %d\nimm: %d\n\n",
		inst->fields.rd, inst->fields.rs, inst->fields.rt, inst->fields.imm);
	printf("Control Bits:\nalu: %d\nmw: %d\nmr: %d\nmtr: %d\nasrc: %d\nbt: %d\nrdst: %d\nrw: %d\n\n",
		inst->signals.aluop, inst->signals.mw, inst->signals.mr, inst->signals.mtr, inst->signals.asrc,
		inst->signals.btype, inst->signals.rdst, inst->signals.rw);
	printf("ALU Result: %d\n\n",inst->aluout);
	if (inst->signals.mr == 1)
		printf("Mem Result: %d\n\n",inst->memout);
	else
		printf("Mem Result: X\n\n");
	for(i=0;i<8;i++)
	{
		for(j=0;j<32;j+=8)
			printf("$%d: %4d ",i+j,regfile[i+j]);
		printf("\n");
	}
	printf("\n");
}


void printP2(InstInfo *inst0, InstInfo *inst1, InstInfo *inst2, InstInfo *inst3, InstInfo *inst4,  int count)
{
	int i, j;
	printf("Cycle %d:\n",count);
	if(inst0->inst != 0)
		printf("Fetch instruction: %d\n", inst0->inst);
	else
		printf("Fetch instruction: \n");
	if(inst1->inst != 0)
		printf("Decode instruction: %s\n", inst1->string);
	else
		printf("Decode instruction: \n");
	if(inst2->inst !=0)
		printf("Execute instruction: %s\n", inst2->string);
	else
		printf("Execute instruction: \n");

	if(inst3->inst !=0)
		printf("Memory instruction: %s\n", inst3->string);
	else
		printf("Memory instruction: \n");

	if(inst4->inst !=0)
		printf("Writeback instruction: %s\n", inst4->string);
	else
		printf("Writeback instruction: \n");



	for(i=0;i<8;i++)
	{
		for(j=0;j<32;j+=8)
			printf("$%d: %4d ",i+j,regfile[i+j]);
		printf("\n");
	}
	printf("\n");
}

