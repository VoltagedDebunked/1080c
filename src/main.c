#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 100
#define MAX_INSTRUCTIONS 256

const char *instructions[] = {
    "ADD", "SUB", "MOV", "CMP", "HLT", "AND", "JMP", "SWT"
};

unsigned char encode_instruction(const char *instr, int *operand1, int *operand2) {
    for (int i = 0; i < 8; i++) {
        if (strcmp(instr, instructions[i]) == 0) {
            if (i == 2) { // MOV instruction
                sscanf(instr, "MOV %d, %d", operand1, operand2);
            } else if (i < 4) { // ADD, SUB, CMP
                sscanf(instr, "%s %d, %d", instr, operand1, operand2);
            } else if (i == 5) { // AND instruction
                sscanf(instr, "AND %d, %d", operand1, operand2);
            } else if (i == 6) { // JMP instruction
                sscanf(instr, "JMP %d", operand1);
            } else if (i == 7) { // SWT instruction
                sscanf(instr, "SWT %d", operand1);
            }
            return (unsigned char)i;
        }
    }
    return 0xFF;
}

void compile(const char *input_file, const char *output_file) {
    FILE *input = fopen(input_file, "r");
    FILE *output = fopen(output_file, "wb");

    if (!input || !output) {
        perror("File opening failed");
        return;
    }

    char line[MAX_LINE_LENGTH];
    unsigned char machine_code[MAX_INSTRUCTIONS];
    int instruction_count = 0;

    // Label management
    char labels[MAX_INSTRUCTIONS][10];
    int label_addresses[MAX_INSTRUCTIONS];
    int label_count = 0;

    // First pass to find labels
    while (fgets(line, sizeof(line), input)) {
        line[strcspn(line, "\n")] = 0;  // Remove newline

        if (strchr(line, ':')) { // Check for labels
            char *label = strtok(line, ":");
            strcpy(labels[label_count], label);
            label_addresses[label_count++] = instruction_count;
            continue;
        }

        char instr[10];
        int operand1 = 0, operand2 = 0;
        if (sscanf(line, "%s", instr) > 0) {
            unsigned char encoded = encode_instruction(instr, &operand1, &operand2);
            if (encoded != 0xFF) {
                machine_code[instruction_count++] = encoded;
                if (encoded == 2) { // MOV instruction
                    machine_code[instruction_count++] = (unsigned char)operand1;
                    machine_code[instruction_count++] = (unsigned char)operand2;
                } else if (encoded < 4) { // ADD, SUB, CMP
                    machine_code[instruction_count++] = (unsigned char)operand1;
                    machine_code[instruction_count++] = (unsigned char)operand2;
                } else if (encoded == 6) { // JMP instruction
                    // Store the address for later resolution
                    machine_code[instruction_count++] = (unsigned char)operand1;
                }
            }
        }
    }

    // Second pass to resolve labels
    for (int i = 0; i < instruction_count; i++) {
        if (machine_code[i] == 6) { // JMP instruction
            unsigned char label_index = machine_code[i + 1];
            machine_code[i + 1] = label_addresses[label_index];
        }
    }

    fwrite(machine_code, sizeof(unsigned char), instruction_count, output);

    fclose(input);
    fclose(output);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s input.s output.o\n", argv[0]);
        return 1;
    }

    compile(argv[1], argv[2]);
    return 0;
}