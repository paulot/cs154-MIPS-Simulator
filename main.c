
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "functions.h"

InstInfo * pipelineInsts[5];

typedef enum { FETCH   = 0,
               DECODE  = 1,
               EXECUTE = 2, 
               MEMORY  = 3,
               WRITE   = 4
             } Instructions;

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

	while (pc <= maxpc + 4) {
		fetch(pipelineInsts[FETCH]);
		decode(pipelineInsts[DECODE]);
		execute(pipelineInsts[EXECUTE]);
		memory(pipelineInsts[MEMORY]);
		writeback(pipelineInsts[WRITE]);

                printP2(pipelineInsts[FETCH],
                pipelineInsts[DECODE],
                pipelineInsts[EXECUTE],
                pipelineInsts[MEMORY],
                pipelineInsts[WRITE],
                pc - 1);

		// Fill the remainder of the pipeline
		writebackInst   = *pipelineInsts[MEMORY];
		memoryInst      = *pipelineInsts[EXECUTE];
		executeInst     = *pipelineInsts[DECODE];
		decodeInst      = *pipelineInsts[FETCH];
		if (instnum <= maxpc) instnum++;
		if (instnum > maxpc)  fetchInst.inst = 0;
		pipelineInsts[FETCH]   = instPtr;

		//declare var's for MUX
		int forward1 = 0;
		int forward2 = 0;
		//check for execute dependencies
		if (pipelineInsts[DECODE] != 0 && pipelineInsts[EXECUTE] != 0) {
			if (pipelineInsts[EXECUTE]->fields.rd == pipelineInsts[DECODE]->fields.rt)
				forward1 = 2;
			if (pipelineInsts[EXECUTE]->fields.rd == pipelineInsts[DECODE]->fields.rs)
				forward2 = 2;
		}
		//check for memory dependencies
		if (pipelineInsts[DECODE] != 0 && pipelineInsts[MEMORY] != 0) {
			if (pipelineInsts[MEMORY]->fields.rd == pipelineInsts[DECODE]->fields.rt)
				forward1 = 1;	
			if (pipelineInsts[MEMORY]->fields.rd == pipelineInsts[DECODE]->fields.rs)
				forward2 = 1;
		}
		//MUX to choose ALU inputs between either rs & rt or last ALU output (forwarding)
		pipelineInsts[EXECUTE]->input1 = (forward1 == 1 || 2) ? pipelineInsts[EXECUTE]->aluout : regfile[pipelineInsts[EXECUTE]->fields.rs]; 
		pipelineInsts[EXECUTE]->input2 = (forward1 == 1 || 2) ? pipelineInsts[EXECUTE]->aluout : regfile[pipelineInsts[EXECUTE]->fields.rt]; 

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

