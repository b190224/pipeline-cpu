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

// special cases
#define LDA 0x6

#define FLAG_RESUME(flag, bit) ((flag) |= ((bit) << 15))
#define FLAG_STALL(flag, bit) ((flag) |= ((bit) << 15))

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
			uint8_t op, copyOP;
			uint8_t imm;
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
		bool detectLoadDataHazard();
		void resumePipeline();
		void updateFlagRegister(bool);
	
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
		bool stall = false;
		uint16_t pc = 0;
		uint16_t flag = 0; // resume stalling after hazard handling, interrupt, carry, parirty, zero, overflow
		std::array<int16_t, NUM_OF_NONPIPELINED_REGISTERS> regs {  };
		std::array<PipelinedRegisters, NUM_OF_PIPEPLINED_REGISTERS> readPip { };
		std::array<PipelinedRegisters, NUM_OF_PIPEPLINED_REGISTERS> writePip { };
		std::array<uint16_t, MEMORY_SIZE> insMem {
			0xC00, 0x1200
		};
		std::array<int16_t, MEMORY_SIZE> dataMem {
			
		};
};

#endif
