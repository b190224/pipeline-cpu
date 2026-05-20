#include <iostream>
#include "cpu.h"

void CPU::cycle() {
	while (run) {
		if (pc>6)break;
		
		std::cout << int(readPip[ID_EX].ins) << ' '<< int(writePip[ID_EX].cs.regRead)  <<'\n';
		
		decode();				
		fetch();
		updatePipelineRegisters();
	}
}

void CPU::restoreControlSignalDefaults(ControlSignals* cs) {
	if (!cs) return;
	
	cs->pcWrite = true;
	cs->regRead = false;
	cs->regWrite = false;
	cs->memRead = false;
	cs->memWrite = false;
}

void CPU::updatePipelineRegisters() {
	readPip[IF_ID] = writePip[IF_ID];
	readPip[ID_EX] = writePip[ID_EX];
	readPip[EX_MEM] = writePip[EX_MEM];
	readPip[MEM_WB] = writePip[MEM_WB];
}

void CPU::fetch() {
	writePip[IF_ID].ins = insMem[pc];
	if (readPip[IF_ID].cs.pcWrite) ++pc;
}

void CPU::decode() {
	writePip[ID_EX].ins = readPip[IF_ID].ins;
	
	uint8_t op = (readPip[IF_ID].ins & 0xFE00) >> 9;
	
	switch (op) {
		case 0x0: {
			restoreControlSignalDefaults(&writePip[ID_EX].cs);
			break;
		}
		// mov
		case 0x5: {
			writePip[ID_EX].cs.regRead = true;
			writePip[ID_EX].cs.regWrite = true;
			break;
		}
		
		default: {
			break;
		}
	}
	
	if (writePip[ID_EX].cs.regRead) {
		writePip[ID_EX].opA = regs[(readPip[ID_EX].ins & 0x38) >> 3];
		writePip[ID_EX].opB = regs[readPip[ID_EX].ins & 0x7];
	}
}
