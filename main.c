
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "functions.h"

// TODO: Make a Makefile for the project 

int main(int argc, char *argv[])
{
	InstInfo curInst;
	InstInfo *instPtr = &curInst;
	int instnum = 0;
	int maxpc;
	FILE *program;
	if (argc != 2)
	{
		printf("Usage: sim filename\n");
		exit(0);
	}

	maxpc = load(argv[1]);
	printLoad(maxpc);

	while (pc <= maxpc)
	{
		fetch(instPtr);
		decode(instPtr);
		execute(instPtr);
		memory(instPtr);
		writeback(instPtr);
		print(instPtr,instnum++);
	}
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
    return;
}

/* print
 *
 * prints out the state of the simulator after each instruction
 */
void print(InstInfo *inst, int count)
{
	int i, j;
	int x = 0;
	printf("Cycle %d:\n",count);
	printf("Fetch instruction: %s\n",inst->string);
	printf("Decode instruction: %s\n", inst->string);
        printf("Memory instruction: %s\n", inst->string);
	printf("Writeback instruction: %s\n", inst->string);
	for(i=0;i<8;i++)
	{
		for(j=0;j<32;j+=8)
			printf("$%d: %4d ",i+j,regfile[i+j]);
		printf("\n");
	}
	printf("\n");
}

