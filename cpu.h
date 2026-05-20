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
		};
	
		struct PipelinedRegisters {
			ControlSignals cs;
			uint16_t ins;
			uint16_t dest;
			int16_t opA, opB;
		};

	
	public:
		CPU() = default;	
		
		void cycle();
		void fetch();
		void decode();
		void updatePipelineRegisters();
		void restoreControlSignalDefaults(ControlSignals*);
	
	private:
		bool run = true;
		uint16_t pc = 0;
		uint16_t flag = 0;
		std::array<int16_t, NUM_OF_NONPIPELINED_REGISTERS> regs {};
		std::array<PipelinedRegisters, NUM_OF_PIPEPLINED_REGISTERS> readPip {};
		std::array<PipelinedRegisters, NUM_OF_PIPEPLINED_REGISTERS> writePip {};
		std::array<uint16_t, MEMORY_SIZE> insMem {
			0xA00, 0xA00
		};
		std::array<int16_t, MEMORY_SIZE> dataMem {};
};

#endif
