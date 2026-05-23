#pragma once
#ifndef CPU_H
#define CPU_H

#include <array>
#include <cstdint>

#define MEMORY_SIZE 65535
#define NUM_OF_PIPEPLINED_REGISTERS 4
#define NUM_OF_NONPIPELINED_REGISTERS 8

#define IF_ID 0
#define ID_EX 1
#define EX_MEM 2
#define MEM_WB 3

class CPU {
	private:
		struct ControlSignals {
			bool pcWrite = true;
			bool regRead = false;
			bool regWrite = false;
			bool memRead = false;
			bool memWrite = false;
			bool immSel = false;
		};
	
		struct PipelinedRegisters {
			ControlSignals cs;
			uint16_t ins;
			uint16_t memDest;
			int16_t opA, opB, opC, result;
			uint8_t regDest, regSrcB, regSrcC;
			uint8_t op;
			int8_t imm;
		};

	public:
		CPU() = default;	
		
		void cycle();
		void fetch();
		void decode();
		void execute();
		void dma();
		void writeBack();
		void updatePipelineRegisters();
		void restoreControlSignalDefaults(ControlSignals*);
		void updateSignals(ControlSignals*, bool, bool, bool, bool, bool, bool);
	
	private:
		void nop();
		void hlt();
		
		void mov();
		void lda();
		void sta();
		
		void add();
		void sub();
		void andd();
		void orr();
		void adi();
		void sui();
		void ani();
		void ori();
	
	private:
		bool run = true;
		uint16_t pc = 0;
		uint16_t flag = 0;
		std::array<int16_t, NUM_OF_NONPIPELINED_REGISTERS> regs { 10 };
		std::array<PipelinedRegisters, NUM_OF_PIPEPLINED_REGISTERS> readPip { };
		std::array<PipelinedRegisters, NUM_OF_PIPEPLINED_REGISTERS> writePip { };
		std::array<uint16_t, MEMORY_SIZE> insMem {
			0x5603
		};
		std::array<int16_t, MEMORY_SIZE> dataMem {
			
		};
};

#endif
