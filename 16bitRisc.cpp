// ConsoleApplication9.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <string>
#include <utility>
#include <tuple>
using namespace std;

// gera og annað  ///////////////////////////////////////////////////////
// lesa um pipelineing chapt 6
// implementa 3 stage piplineing í kóðan
// breyta recursive algriminnu
// breyta stackinnu
// fynna leið til þess að stopa forritið
/////////////////////////////////////////////////////////////////////////

// eiga að vera global breytur
short ram[64]; //ram
short regNr[16]; // registerar

short stack[128]; //stack 
int PC = 0; // program counter
int PCN = 0; // hafa refresaðan program counter?
int stackpointer = 0; //stack pointer
int IR = 0; //instruction register
			///// status bits
string flagRegister[6] = { "overflow", "underflow", "carryOut", "registerOverFlow", "ramOverFlow", "algorithmSelectionError" };
short flagRegisterPointer = 0;
/* pæla í status bita; iterup-bits; stack; zero,carry,overflow-bits;  */
short zero_reg = 0; // !!! breyta reg1 í zero Reg. hægt að hafa regNr[17] -> 16 vera zero reg ??

					// instrcution stack
short instIterative[] =
{
	0x5101, 0x4000, 0x4010, 0x4002, 0x4011, 0x4201, 0x4320, 0x4432, 0x4543, 0x4654, 0x4765, 0x4876, 0x4987, 0x4a98, 0x4ba9, 0x4cba,
	0x4dcb, 0x4edc, 0x4fed, 0x40fe, 0x410f, 0x4210, 0x4321, 0x4432, 0x4543, 0x4654 // seinasta break koðan og endar forritið 
};
int stackJAL = 0;
short instRecursive[] =
{
	/*0x5004, 0xdf04, 0x100d, 0x4002, 0x5101, 0x9001, 0xcc0b, 0x8000, 0xcc1c, 0x5201, 0xef00, 0x6c00, 0xa000,
	0xaf01, 0xf001, 0xdf04, 0xb001, 0x4001, 0xba02, 0x6c00, 0x1fdf, 0x4faf, 0xef00, 0xa200, 0xb001, 0xf002,
	0xdf04, 0x5000, 0xef00*/
	// test 2
	/*0x5004, 0xdf04, 0x100d, 0x4002, 0x5101, 0x9001, 0xcc0b, 0x8000, 0xcc1b, 0x5000, 0xef00, 0x6c00, 0xa000, 0xaf01, 0xf001, 0xdf04,
	0xa200, 0xb001, 0xf002, 0xdf04, 0xb001, 0x4001, 0xba02, 0x6c00, 0x1fdf, 0x4faf, 0xef00, 0x5201, 0xef00*/
	/* virkar kannski
	0x5004, 0xdf04, 0x40dd, 0x4002, 0x5101, 0x9c01, 0xcc0b, 0x8000, 0xcc19, 0x4200, 0xdf00, 0x6c00, 0xaf00, 0xa001, 0xf001, 0xdf04,
	0xa202, 0xb001, 0xf002, 0xdf04, 0xb102, 0x4010, 0xbf00, 0x6c00, 0xdf00, 0x5201, 0xdf00*/
	0x5006, 0xdf05, 0x40dd, 0x4002, 0x0000, 0x5101, 0x9c01, 0xcc0c, 0x8000, 0xcc1a, 0x4200, 0xef00, 0x6c00, 0xaf02, 0xa001, 0xf001, 0xdf05,
	0xa200, 0xb001, 0xf002, 0xdf05, 0xb100, 0x4212, 0xbf02, 0x7c00, 0xef00, 0x5201, 0xef00
};
//breyta 7.... breyti stackpointer =.... +2 i 0x6c00 .... breytti cmp skip if = 1

/*			// fib her = 04,,, compare talan meira en 1 = 0B,,,,   brance tala != 0 = 1B //1b
registerar
0 = fibtalan oft
1 = compare to
2 = talan 1 ef minnus endar i 1
A = retune register fyrir ram pointer
C = dont care
D = zero reg
F = jal pointer
*/


// lata 4 inni reg0-																				0x5004
// jal geyma PC + 1 i f, og offset i "fib her" -													0xdf##
//																									/////
// cleara regNr 0 -																					0x100d
// lata hlutinn i reg 2 i reg 0 -																	0x4002
//////
///// fib her ////																					//////
// addi i reg1 tölunna 1 -																			0x5101 // kannski
// compare ef talan er ==  1 ef ekki hopa 2 -														0x9X01
// jmp i "compare ef talan er meira en 1" -															0xcX##
// branca ef talan er ekki jafnt og 0  ef ekki hoppa 2-												0x8X01
// jmp i "brance talan != 0" -																		0xcX##
// lata 0 i reg0, - addi																			0x5000
// jump return -																					0xef00
//////
///////compare ef talan meira en 1 ////////															//////
// hreyfa ram/stack pointerinn með addi, 0x5SP2  NOTA stackpointer = stackpoint + 2 nuna			0x6c00
// save current fib tölu -																			0xa000
// save gamal return (sem er enþá í regF) -															0xaf01 // offsetið verður minus
// minusa 1 ur tölu -																				0xf001
// jal i "fib her", geyma pc + 1 i stack,															0xdf##
//////
// save-a dotið ur reg2 i ram billinu sem eftir verður -	fundið steps							0xa200
// load-a data ur seinasta calli -																	0xb001
// minusa 2 ur reg0, -																				0xf002
// jal i "fib her" -																				0xdf##															
//////
// load ur current ram pointer ?? lata það i reg2 sem fer i reg0 -									0xb001
// add ur reg0 og reg1 og geyma það i reg0 -														0x4001
// loada return PC ur ram - 2 og geyma það i A -													0xba02
// minka ram pointer um 2 -																			0x6c00
// cleara jump return svo hægt se að loada nyju returni í það með add -								0x1fdf
// add-a return pc ur ram i JR reg F -																0x4faf
// jump return																						0xef00
//////
//////// brance talan != 0///////																	//////
// add-a 1 i register 2, þetta kemur upp þegar compare ef.. er buinn ap minusa tölunna niður i 1 -	0x5201
// jump return, -	fer i fundið steps																0xef00


int fibonatci(int n)
{
	if (n == 0)
		return 0;
	else if (n == 1)
		return 1;
	else
		return fibonatci(n - 1) + fibonatci(n - 2);
}

void stackListPush(int programCounter, int data)
{
	stack[stackpointer] = programCounter;
	stackpointer++;
	stack[stackpointer] = data; // í ram
	stackpointer++;
}

pair<int, int> stackListPop()
{
stackpointer--;
int data = stack[stackpointer];
stackpointer--;
int programCounter = stack[stackpointer];
return make_pair(data, programCounter);

}

int main()
{
	system("chcp 1252 > nul");   // aukapaki til að geta notað íslenska stafi

	unsigned short instructMask = 0xf000;
	unsigned short dirMask = 0x0f00;
	unsigned short sourceMask = 0x00f0;
	unsigned short targetMask = 0x000f;
	unsigned short immedMask = 0x00ff;  // immed number
	unsigned short jmpOffsetMask = 0x00ff;
	unsigned short ramNrMask = 0x00ff;

	unsigned short immed = 0x0000;
	unsigned short jmpOffset = 0x0000;
	unsigned short ramNr = 0x0000;

	unsigned short OPC = 0x0000;
	unsigned short Rd = 0x0000;
	unsigned short Rs = 0x0000;
	unsigned short Rt = 0x0000;

	unsigned short flagRegisterPointer = 0x0000;

	/*unsigned short Mopc = 0xf000;
	unsigned short Mdr = 0x0f00;
	unsigned short Msr = 0x00f0;
	unsigned short Mtr = 0x000f;*/

	//short immed, jmpOffset, ramNr;
	//short OPC, DR, SR, TR; // eftir maska
	unsigned short Mopc, Mdr, Msr, Mtr; // fyrir maska




start:
	while (1) {
		int PH4; // breyta i registera

				 ///junk fyrir neðan
				 //regNr[15] = 32600;
		int jumpcountererror = 0;
		int jalerror = 0;
		int hringur = 0;
		//////////////////////////////////////////////	
		cout << "start of program\nselect 1. for Iterative and 2. for Recursive\n" << endl;
		int velja = 0;
		cin >> velja;
		cout << "\n\n" << endl;
		if (velja == 1 || velja == 2)
			goto cont;
		else {
			cout << "invalid option" << endl;
			goto start;
		}
	cont:
		//cout << "hringur nr: " << hringur << endl;
		cout << " " << endl;
		hringur++;
		//if (hringur > 105)
		//goto error_stop;
		//cin >> hex >> inst[PC]; // debuging
		// fetch 
		//IR = instIterative[PC];
		if (velja == 1)
			IR = instIterative[PC];
		else if (velja == 2)
			IR = instRecursive[PC];
		else {
			flagRegisterPointer = 5;
			goto error_stop;
		}
		//decode
		Mopc = instructMask & IR;
		Mdr = dirMask & IR;
		Msr = sourceMask & IR;
		Mtr = targetMask & IR;
		immed = immedMask & IR;
		jmpOffset = jmpOffsetMask & IR;
		ramNr = ramNrMask & IR;
		OPC = Mopc >> 12;
		Rd = Mdr >> 8;
		Rs = Msr >> 4;
		Rt = Mtr >> 0;

		//compute
		switch (OPC) // löguðust errorar við að hafa a,b,c,d dno why
		{
		case 0x000a:   // or
			regNr[Rd] = regNr[Rs] || regNr[Rt];
			cout << "OR" << endl;
			PC++;
			goto cont;

			if (regNr[Rs] || regNr[Rt] != 0){
				regNr[Rd] = 1;
			}
			else {
				regNr[Rd] = 0;
			}
			cout << regNr[Rd] << endl;
			PC++;
			goto cont;
			break;


			// Þarf if ? per se ja
						  /*cout << "or " << endl;
						  if (regNr[SR] || regNr[TR] != 0)
						  regNr[DR] = 1;
						  else
						  regNr[DR] = 0;
						  cout << regNr[DR] << endl;
						  PC++;
						  goto cont;
						  break;*/

			/*errorStp:	cout << "end of program: " << regNr[0] << endl;
			getchar();
			getchar();
			goto endOfProgram;*/
			//break;

		case 0x000b:		//and
			cout << "AND" << endl;
			if (regNr[Rs] && regNr[Rt] != 0) {
				regNr[Rd] = 1;
			}
			else {
				regNr[Rd] = 0;
			}
			cout << regNr[Rd] << endl;
			PC++;
			goto cont;
			break;

		case 0x000c:		//not
			cout << "NOT" << endl;
			if (regNr[Rs] == true) {
				regNr[Rd] = 0;
			}
			else {
				regNr[Rd] = 1;
			}
			cout << regNr[Rd] << endl;
			PC++;
			goto cont;
			break;

		case 0x000d:		//xor
			cout << "XOR " << endl;
			if (((regNr[Rs] == false) && (regNr[Rt] == false)) || ((regNr[Rs] == true) && (regNr[Rt] == true))) {
				regNr[Rd] = 0;
			}
			else {
				regNr[Rd] = 1;
			}
			cout << regNr[Rd] << endl;
			PC++;
			goto cont;
			break;
		case 0x0001:		//add
			//cout << "add " << endl;
			//if (regNr[Rs] + regNr[Rt] >= 32678) {
			//	flagRegisterPointer = 0;
			//	// tekka hvort það eigi að vera unsigned og >= 65536
			//	goto error_stop;
			//}
			//else
			//	regNr[Rd] = regNr[Rs] + regNr[Rt];
			////cout << regNr[DR] << endl;
			//PC++;
			//goto cont;
			//break;
			cout << "ADD " << endl;
			if (regNr[Rs] + regNr[Rt] >= 32678) {
				flagRegisterPointer = 0;
				goto error_stop;
			}
			if	(regNr[Rd] = regNr[Rs] + regNr[Rt]){
				cout << regNr[Rd] << endl;
				PC++;
				goto cont;
				break;
			}
			else {
				goto error_stop;
			}

		case 0x0002:		//addi
			cout << "addi " << endl;
			regNr[Rd] = immed;
			//cout << "testing::" << regNr[2] << endl;
			PC++;
			goto cont;
			break;

		case 0x0003:		//sla !!! //shiftar einum bita til left
			cout << "stackpointer++ \n" << endl;
			/*PH4 = regNr[SR];
			for (int i = 0; i < immed; i++)
			{
			// << vinstri hækkar tölu, >> lækkar tölu
			PH4 = PH4 << 1;
			}
			regNr[SR] = PH4;
			cout << regNr[SR] << endl;*/
			//stackpointer = stackpointer + 3;
			regNr[Rd] = regNr[Rs] << Rt;
			cout << "SLA " << endl;
			PC++;
			goto cont;
			break;

		case 0x0004:		//srl !!! // á að vera SRA? og shift til hægri
			cout << "stackpointer -- \n" << endl;
			/*PH4 = regNr[SR];
			for (int i = 0; i < immed; i++)
			{
			// << vinstri hækkar tölu, >> lækkar tölu
			PH4 = PH4 >> 1;
			}
			regNr[SR] = PH4;
			cout << regNr[SR] << endl;*/
			//stackpointer = stackpointer - 3;
			regNr[Rd] = regNr[Rs] >> Rt;
			cout << "SRL" << endl;
			PC++;
			goto cont;
			break;

		case 0x0005: //biz = If all the bits in register Rs are zero than the current
								//Program Count (PC + 1) is offset to PC + 1 + Offset.
			cout << "biz " << endl;
			//if (regNr[Rd] == 0) {
			//	//PC = 9;
			//	//goto cont;
			//	PC = PC + 2;
			//	goto cont;
			//}
			if (regNr[Rs] == 0) {
				PC = (PCN + 1) + jmpOffset;
			}
			else {
				PCN++;
			}
			goto cont;
			break;

		case 0x0006:	//bnz -- nytt cmpst compare skip if false 
						//If all the bits in register Rs are not zero than the current Program Count (PC + 1) is offset
						//to PC + 1 + Offset.
			cout << "const " << endl;
			/*	if (regNr[DR] == 1) {
			//PC = immed;
			goto cont;
			//!!!!! SKOÐA BETUR HVORT ÞAÐ ÞURFI AÐ VERA BRANCH IF == 1
			}
			PC++;
			goto cont;*/
			// skoða hvað reg á að fara i þetta
			// ef tala == 1 skip
			if (regNr[Rs] <= regNr[Rt]) {
				PC = PC + 2;
				goto cont;
			}
			else {
				PC = PC++;
				goto cont;
			}
			break;

		case 0x0007:		//save !!!
			cout << "save " << endl;
			/*if (ramNr < 15) {
			flagRegisterPointer = 4;
			goto error_stop;
			}
			ram[ramNr] = regNr[DR];
			PC++;
			goto cont;
			break;*/
			ram[stackpointer - immed] = regNr[Rd];
			PC++;
			goto cont;
			break;

		case 0x0008:		//load  !!!
			cout << "load " << endl;
			/*	regNr[DR] = ram[ramNr];
			PC++;
			goto cont;
			break;*/
			regNr[Rd] = ram[stackpointer - immed];
			PC++;
			goto cont;
			break;

		case 0x0009:		//jmp 
		jump:	cout << "jump " << endl;
			//jumpcountererror++;
			if (jumpcountererror > 20)
				goto error_stop;
			PC = jmpOffset;
			goto cont;
			//goto PC;
			break;
		case 0x0010: //JAL 
		jal:	cout << "jal " << endl;
			//jalerror++;
			if (jalerror > 10)
				goto error_stop;
			//stackListPush(PC, regNr[DR]);
			regNr[Rd] = PC + 1; // nota reg #F sem dedcated stackpointer
			PC = jmpOffset;
			goto cont;
			//goto PC;
			break;


		case 0x0011:		//jr !!!
			cout << "jr " << endl;
			/*int JRdata, JRpc;  // registerar !!!
			tie(JRdata, JRpc) = stackListPop();
			PC = JRpc;
			regNr[DR] = JRdata;
			cout << JRdata << "....." << JRpc << endl;
			goto cont;
			break;*/
			PC = regNr[Rd];
			goto cont;
			break;

		case 0x0012:  // ???
			cout << "sub" << endl;
			if ((regNr[Rd] - immed) <= 0)
				regNr[Rd] = 0; // veit ekki hvort má !!!!!
			else
				regNr[Rd] = regNr[Rd] - immed;
			PC++;
			goto cont;

		}

	error_stop:
		cout << "operation stoped becasue " << flagRegister[flagRegisterPointer] << " happend \n" << endl;
		getchar();
		getchar();
		goto endOfProgram;
		//goto start; // eða goto cont þarf að pæla

	}
endOfProgram:
	return 0;
}