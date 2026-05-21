#include <iostream>
#include "cpu.h"

void CPU::cycle() {
	while (run) {
		if (pc>6)break;
		
		writeBack();
		dma();
		execute();
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
	cs->immSel = false;
}

void CPU::updateSignals(ControlSignals* cs, bool regRead, bool regWrite, bool pcWrite, bool memWrite, bool memRead, bool immSel) {
	if (!cs) return;
	
	cs->regRead = regRead;
	cs->regWrite = regWrite;
	cs->pcWrite = pcWrite;
	cs->memWrite = memWrite;
	cs->memRead = memRead;
	cs->immSel = immSel;
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
	writePip[ID_EX].op = (readPip[IF_ID].ins & 0xFE00) >> 9;
	
	switch (writePip[ID_EX].op) {
		case 0x0: {
			restoreControlSignalDefaults(&writePip[ID_EX].cs);
			break;
		}
		
		// mov
		case 0x5: {			
			updateSignals(&writePip[ID_EX].cs, 
				true, // reg read
				true, // reg write
				true, // pc write (normal)
				false, // mem Write
				false, // mem read
				false // immsel
			);
			break;
		}
		
		// lda
		case 0x6: {					
			updateSignals(&writePip[ID_EX].cs, 
				true, // reg read
				true, // reg write
				true, // pc write (normal)
				false, // mem Write
				true, // mem read
				true // immsel
			);
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
	
	if (writePip[ID_EX].cs.regWrite) {
		writePip[ID_EX].regDest = (readPip[ID_EX].ins & 0x1C0) >> 6;
	}
	
	if (writePip[ID_EX].cs.immSel) {
		writePip[ID_EX].imm = readPip[ID_EX].ins & 0x7;
	}
}

void CPU::execute() {
	writePip[EX_MEM].cs = readPip[ID_EX].cs;
	
	switch (readPip[ID_EX].op) {
		case 0x5: {
			mov();
			break;
		}
		
		case 0x6: {
			lda();
			break;
		}
		
		default: {
			break;
		}
	}
}

void CPU::dma() {
	writePip[MEM_WB].cs = readPip[EX_MEM].cs;
	
	if (readPip[EX_MEM].cs.memRead) {
		writePip[MEM_WB].result = dataMem[readPip[EX_MEM].memDest];
		writePip[MEM_WB].regDest = readPip[EX_MEM].regDest;
		std::cout << writePip[MEM_WB].result;
	}
	
	if (readPip[EX_MEM].cs.memWrite) {
		dataMem[readPip[EX_MEM].memDest] = writePip[MEM_WB].result;
	}
}

void CPU::writeBack() {
	if (readPip[MEM_WB].cs.regWrite) {
		regs[writePip[MEM_WB].regDest] = writePip[MEM_WB].result;
	}
}

void CPU::mov() {
	writePip[EX_MEM].result = readPip[ID_EX].opA;
}

void CPU::lda() {
	writePip[EX_MEM].memDest = readPip[ID_EX].opA + readPip[ID_EX].imm;
}
