#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "vm.h"
#include "debug.h"

VM vm;

static void resetStack(void) {
	vm.stackTop = vm.stack;
}

void initVM(void) {
	resetStack();
}

void freeVM(void) {

}

static InterpretResult run() {
#define READ_BYTE()	(*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
// NOTE: Bob explains the rationale behind this macro;
// the way I see it, this shows why macros should be avoided in the first place.
#define BINARY_OP(op)									\
	do {												\
		double val = pop();								\
		*(vm.stackTop - 1) = *(vm.stackTop - 1) op val; \
	} while (false)

	for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
		printf("          ");
		for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
			printf("[ ");
			printValue(*slot);
			printf(" ]");
		}
		printf("\n");
		disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif
		uint8_t instruction;
		switch (instruction = READ_BYTE()) {
		case OP_CONSTANT: {
			Value constant = READ_CONSTANT();
			push(constant);
			break;
		}
		case OP_ADD:	  BINARY_OP(+); break;
		case OP_SUBTRACT: BINARY_OP(-); break;
		case OP_MULTIPLY: BINARY_OP(*); break;
		case OP_DIVIDE:   BINARY_OP(/); break;
		case OP_NEGATE:   *(vm.stackTop - 1) = -*(vm.stackTop - 1); break;
		case OP_RETURN: {
			printValue(pop());
			printf("\n");
			return INTERPRET_OK;
		}
		}
	}

#undef READ_BYTE
#undef READ_CONSTANT
}

InterpretResult interpret(const char* source) {
	Chunk chunk;
	initChunk(&chunk);

	if (!compile(source, &chunk)) {
		freeChunk(&chunk);
		return INTERPRET_COMPILE_ERROR;
	}

	vm.chunk = &chunk;
	vm.ip = vm.chunk->code;

	InterpretResult result = run();

	freeChunk(&chunk);
	return result;
}

void push(Value value) {
	*vm.stackTop = value;
	vm.stackTop++;
}

Value pop(void) {
	vm.stackTop--;
	return *vm.stackTop;
}