#include <stdio.h>

int pc = 0;
int running = 1;
int indexReg;
int instrNum = 0;
int prog [];
// stack


int main (int argc, char **argv){
	run();   
	return 0;
}

int fetch() {

	// aaa como agarro el programa de **argv
	return prog[ pc++ ]; 
}

void  run(){
	while( running ){
		int instr = fetch();
   		decode( instr );
   		eval();   
	}	
}

void eval(){
	/*
	 *stack opertaions
	 * */
}

void decode(int str){
	/*
	 *switch para meterse  las intrucciones
	 * */
}
