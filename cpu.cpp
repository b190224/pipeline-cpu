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
		
		stall = detectLoadDataHazard();
		
		if (stall) {
			FLAG_STALL(flag, true);
		}
		
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
	if (!stall) {
		readPip[IF_ID] = writePip[IF_ID];
		readPip[ID_EX] = writePip[ID_EX];
	}
	else {
		readPip[ID_EX].op = 0;
	}

	readPip[EX_MEM] = writePip[EX_MEM];	
	readPip[MEM_WB] = writePip[MEM_WB];
}

bool CPU::detectLoadDataHazard() {
	return ((readPip[ID_EX].op == LDA) && ((readPip[ID_EX].regDest == writePip[ID_EX].regSrcB) || (readPip[ID_EX].regDest == writePip[ID_EX].regSrcC)));
}

void CPU::resumePipeline() {
	
}

void CPU::updateFlagRegister(bool resume) {
	FLAG_RESUME(flag, resume);
}

void CPU::fetch() {
	if (stall) return;
	
	writePip[IF_ID].ins = insMem[pc];
	if (readPip[IF_ID].cs.pcWrite) ++pc;
}

void CPU::decode() {	
	if (stall) return;
	
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
		
		// sta
		case 0x26: {
				updateSignals(&writePip[ID_EX].cs, 
				true, // reg read
				false, // reg write
				true, // pc write (normal)
				true, // mem Write
				false, // mem read
				true // immsel
			);
			break;
		}
		
		// add
		case 0x9: {
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
		
		// sub
		case 0x29: {
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
		
		// and
		case 0x49: {
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
		
		// or
		case 0x69: {
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
		
		// add + imm
		case 0xB: {
			updateSignals(&writePip[ID_EX].cs, 
				true, // reg read
				true, // reg write
				true, // pc write (normal)
				false, // mem Write
				false, // mem read
				true // immsel
			);
			break;
		}
		
		// sub + imm
		case 0x2B: {
			updateSignals(&writePip[ID_EX].cs, 
				true, // reg read
				true, // reg write
				true, // pc write (normal)
				false, // mem Write
				false, // mem read
				true // immsel
			);
			break;
		}
		
		// and + imm
		case 0x4B: {
			updateSignals(&writePip[ID_EX].cs, 
				true, // reg read
				true, // reg write
				true, // pc write (normal)
				false, // mem Write
				false, // mem read
				true // immsel
			);
			break;
		}
		
		// or + imm
		case 0x6B: {
			updateSignals(&writePip[ID_EX].cs, 
				true, // reg read
				true, // reg write
				true, // pc write (normal)
				false, // mem Write
				false, // mem read
				true // immsel
			);
			break;
		}
		
		default: {
			break;
		}
	}
	
	if (writePip[ID_EX].cs.regRead) {
		writePip[ID_EX].opA = regs[(readPip[IF_ID].ins & 0x1C0) >> 6];
		writePip[ID_EX].opB = regs[(readPip[IF_ID].ins & 0x38) >> 3];
		writePip[ID_EX].opC = regs[readPip[IF_ID].ins & 0x7];
		writePip[ID_EX].regSrcB = (readPip[IF_ID].ins & 0x38) >> 3;
		writePip[ID_EX].regSrcC = readPip[IF_ID].ins & 0x7;
	}
	
	if (writePip[ID_EX].cs.regWrite) {
		writePip[ID_EX].regDest = (readPip[IF_ID].ins & 0x1C0) >> 6;
	}
	
	if (writePip[ID_EX].cs.immSel) {
		writePip[ID_EX].imm = readPip[IF_ID].ins & 0x7;
	}
}

void CPU::execute() {
	writePip[EX_MEM].cs = readPip[ID_EX].cs;
	writePip[EX_MEM].regSrcB = readPip[ID_EX].regSrcB;
	writePip[EX_MEM].regSrcC = readPip[ID_EX].regSrcC;
	
	switch (readPip[ID_EX].op) {
		case 0x0: {
			nop();
			break;
		}
		
		case 0x5: {
			mov();
			break;
		}
		
		case 0x6: {
			lda();
			break;
		}
		
		case 0x26: {
			sta();
			break;
		}
		
		case 0x9: {
			add();
			break;
		}
		
		case 0x29: {
			sub();
			break;
		}
		
		case 0x49: {
			andd();
			break;
		}
		
		case 0x69: {
			orr();
			break;
		}
		
		case 0xB: {
			adi();
			break;
		}
		
		case 0x2B: {
			sui();
			break;
		}
		
		case 0x4B: {
			ani();
			break;
		}
		
		case 0x6B: {
			ori();
			break;
		}
		
		default: {
			break;
		}
	}
}

void CPU::dma() {
	writePip[MEM_WB].cs = readPip[EX_MEM].cs;
	writePip[MEM_WB].result = readPip[EX_MEM].result;
	writePip[MEM_WB].regDest = readPip[EX_MEM].regDest;
	
	if (readPip[EX_MEM].cs.memRead) {
		writePip[MEM_WB].result = dataMem[readPip[EX_MEM].memDest];
		writePip[MEM_WB].regDest = readPip[EX_MEM].regDest;
	}
	
	if (readPip[EX_MEM].cs.memWrite) {
		dataMem[readPip[EX_MEM].memDest] = readPip[EX_MEM].result;
	}
}

void CPU::writeBack() {
	if (readPip[MEM_WB].cs.regWrite) {
		regs[readPip[MEM_WB].regDest] = readPip[MEM_WB].result;
	}
}

void CPU::nop() {
	return;
}

void CPU::mov() {
	writePip[EX_MEM].result = readPip[ID_EX].opB;
}

void CPU::lda() {
	writePip[EX_MEM].memDest = readPip[ID_EX].opB + readPip[ID_EX].imm;
}

void CPU::sta() {
	writePip[EX_MEM].result = readPip[ID_EX].opA;
}

void CPU::add() {
	int16_t opB = readPip[ID_EX].opB;
	int16_t opC = readPip[ID_EX].opC;
	
	if ((readPip[ID_EX].regSrcB == readPip[EX_MEM].regDest) && readPip[EX_MEM].cs.regWrite) {
		opB = readPip[EX_MEM].result;
	}
	if ((readPip[ID_EX].regSrcC == readPip[EX_MEM].regDest) && readPip[EX_MEM].cs.regWrite) {
		opC = readPip[EX_MEM].result;
	}
	
	if ((readPip[ID_EX].regSrcB == readPip[MEM_WB].regDest) && readPip[MEM_WB].cs.regWrite) {
		opB = readPip[MEM_WB].result;
	}
	if ((readPip[ID_EX].regSrcC == readPip[MEM_WB].regDest) && readPip[MEM_WB].cs.regWrite) {
		opC = readPip[MEM_WB].result;
	}
	
	writePip[EX_MEM].regDest = readPip[ID_EX].regDest;
	writePip[EX_MEM].result = opB + opC;
}
	
void CPU::sub() {	
	int16_t opB = readPip[ID_EX].opB;
	int16_t opC = readPip[ID_EX].opC;
	
	if ((readPip[ID_EX].regSrcB == readPip[EX_MEM].regDest) && readPip[EX_MEM].cs.regWrite) {
		opB = readPip[EX_MEM].result;
	}
	if ((readPip[ID_EX].regSrcC == readPip[EX_MEM].regDest) && readPip[EX_MEM].cs.regWrite) {
		opC = readPip[EX_MEM].result;
	}

	writePip[EX_MEM].regDest = readPip[ID_EX].regDest;
	writePip[EX_MEM].result = opB - opC;
}

void CPU::andd() {
	int16_t opB = readPip[ID_EX].opB;
	int16_t opC = readPip[ID_EX].opC;
	
	if ((readPip[ID_EX].regSrcB == readPip[EX_MEM].regDest) && readPip[EX_MEM].cs.regWrite) {
		opB = readPip[EX_MEM].result;
	}
	if ((readPip[ID_EX].regSrcC == readPip[EX_MEM].regDest) && readPip[EX_MEM].cs.regWrite) {
		opC = readPip[EX_MEM].result;
	}
	
	writePip[EX_MEM].regDest = readPip[ID_EX].regDest;
	writePip[EX_MEM].result = opB & opC;
}

void CPU::orr() {
	int16_t opB = readPip[ID_EX].opB;
	int16_t opC = readPip[ID_EX].opC;
	
	if ((readPip[ID_EX].regSrcB == readPip[EX_MEM].regDest) && readPip[EX_MEM].cs.regWrite) {
		opB = readPip[EX_MEM].result;
	}
	if ((readPip[ID_EX].regSrcC == readPip[EX_MEM].regDest) && readPip[EX_MEM].cs.regWrite) {
		opC = readPip[EX_MEM].result;
	}
	
	
	writePip[EX_MEM].regDest = readPip[ID_EX].regDest;
	writePip[EX_MEM].result = opB | opC;
}

void CPU::adi() {
	int16_t opB = readPip[ID_EX].opB;
	
	if ((readPip[ID_EX].regSrcB == readPip[EX_MEM].regDest) && readPip[EX_MEM].cs.regWrite) {
		opB = readPip[EX_MEM].result;
	}
	
	
	writePip[EX_MEM].regDest = readPip[ID_EX].regDest;
	writePip[EX_MEM].result = opB + readPip[ID_EX].imm;
}

void CPU::sui() {
	int16_t opB = readPip[ID_EX].opB;
	
	if ((readPip[ID_EX].regSrcB == readPip[EX_MEM].regDest) && readPip[EX_MEM].cs.regWrite) {
		opB = readPip[EX_MEM].result;
	}
	
	writePip[EX_MEM].regDest = readPip[ID_EX].regDest;
	writePip[EX_MEM].result = opB - readPip[ID_EX].imm;
}

void CPU::ani() {
	int16_t opB = readPip[ID_EX].opB;
	
	if ((readPip[ID_EX].regSrcB == readPip[EX_MEM].regDest) && readPip[EX_MEM].cs.regWrite) {
		opB = readPip[EX_MEM].result;
	}

	
	writePip[EX_MEM].regDest = readPip[ID_EX].regDest;
	writePip[EX_MEM].result = opB & readPip[ID_EX].imm;
}

void CPU::ori() {
	int16_t opB = readPip[ID_EX].opB;
	
	if ((readPip[ID_EX].regSrcB == readPip[EX_MEM].regDest) && readPip[EX_MEM].cs.regWrite) {
		opB = readPip[EX_MEM].result;
	}
	
	
	writePip[EX_MEM].regDest = readPip[ID_EX].regDest;
	writePip[EX_MEM].result = opB | readPip[ID_EX].imm;
}
