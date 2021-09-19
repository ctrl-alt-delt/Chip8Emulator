#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

// Implement the rest of the opcodes
// Implement the timers
// Use unistd for variables
// rearrange the random number code
// implement the keyboard

#define MEMSIZE      4096
#define DISPWIDTH    64
#define DISPHEIGHT   32
#define DISPSIZE     (DISPWIDTH * DISPHEIGHT)
#define STACKSIZE    100
#define NUMREGISTERS 16
#define FONTSIZE     80

typedef struct chip8 {
    unsigned char  memory[MEMSIZE];
    unsigned char  display[DISPSIZE];
    unsigned short pc;
    unsigned short iReg;
    unsigned short stack[STACKSIZE];
    unsigned short stackPtr;
    unsigned char  delayTimer;
    unsigned char  soundTimer;
    unsigned char  V[NUMREGISTERS];
    unsigned short opcode;
    bool           drawFlag;
    const char*    fileName;
    FILE*          romFile;
    unsigned char  fontset[FONTSIZE];
} chip8;

void printMemory(unsigned char* memory);
void showScreen(unsigned char* display);
void initMachine(chip8 *machine);
void loadRomFile(const char* fileName);
void parseArgs(int argc, char* argv[], chip8* machine);
void executeOpcode(chip8* machine);

int 
main(int argc, char* argv[])
{
    chip8 machine;

    // Declarations
    time_t t;

    parseArgs(argc, argv, &machine);

    initMachine(&machine);

    // Main Loop
    while (1)
    {
        executeOpcode(&machine);
        
        // Update timers

        // If the draw flag is set, update the screen
        if (machine.drawFlag)
        {
            showScreen(machine.display);
        }

        // Store key press state
    }
}

void
executeOpcode(chip8* machine)
{
    machine->opcode = (machine->memory[machine->pc] << 8) | machine->memory[machine->pc+1];
        int vx, vy, height, pixel;
        machine->drawFlag = false;

        switch (machine->opcode & 0xF000)
        {
            case 0x0000:
                switch (machine->opcode & 0x00FF)
                {
                    case 0x00E0:
                        memset(machine->display, 0, DISPSIZE * sizeof(machine->display[0]));
                        machine->pc += 2;
                        break;

                    case 0x00EE:
                        machine->pc = machine->stack[machine->stackPtr];
                        machine->stackPtr--;
                        break;
                };
                break;

            case 0x1000:
                machine->pc = (machine->opcode & 0x0FFF);
                break;

            case 0x2000:
                machine->stackPtr++;
                // TODO: put current PC on top of the stack
                //machine->stack[0] = machine->pc;
                machine->pc = (machine->opcode & 0x0FFF);
                break;

            case 0x3000:
                if (machine->V[(machine->opcode & 0x0F00) >> 8] == (machine->opcode & 0x0FF))
                {
                    machine->pc += 4;
                }
                else
                {
                    machine->pc += 2;
                }
                break;
            
            case 0x4000:
                if (machine->V[(machine->opcode & 0x0F00) >> 8] != (machine->opcode & 0x0FF))
                {
                    machine->pc += 4;
                }
                else
                {
                    machine->pc += 2;
                }
                break;
            
            case 0x5000:
                if (machine->V[(machine->opcode & 0x0F00) >> 8] == (machine->V[(machine->opcode & 0x00F0)]))
                {
                    machine->pc += 4;
                }
                else
                {
                    machine->pc += 2;
                }
                break;
            
            case 0x6000:
                machine->V[(machine->opcode & 0x0F00) >> 8] = (machine->opcode & 0x00FF);
                machine->pc += 2;
                break;

            case 0x7000:
                machine->V[(machine->opcode & 0x0F00) >> 8] += (machine->opcode & 0x00FF);
                machine->pc += 2;
                break;

            case 0x8000:
                switch (machine->opcode & 0x000F)
                {
                    case 0x0001:
                        machine->V[machine->opcode & 0x0F00] = machine->V[machine->opcode & 0x0F00] | machine->V[machine->opcode & 0x00F0];
                        break;
                    
                    case 0x0002:
                        machine->V[machine->opcode & 0x0F00] = machine->V[machine->opcode & 0x0F00] & machine->V[machine->opcode & 0x00F0];
                        break;
                    
                    case 0x0003:
                        machine->V[machine->opcode & 0x0F00] = machine->V[machine->opcode & 0x0F00] ^ machine->V[machine->opcode & 0x00F0];
                        break;
                    
                    case 0x0004:
                        machine->V[machine->opcode & 0x0F00] = machine->V[machine->opcode & 0x0F00] + machine->V[machine->opcode & 0x00F0];
                        // TODO: Values of Vx and Vy added together. If the result is greater than 8 bits (255), set VF to 1, else 0.
                        //       Only the lowest 8 bits are kept and stored in Vx.
                        break;
                    
                    case 0x0005:
                        // TODO: Make this ternary
                        if (machine->V[machine->opcode & 0x0F00] > machine->V[machine->opcode & 0x00F0])
                        {
                            machine->V[0xF] = 1;
                        }
                        else
                        {
                            machine->V[0xF] = 0;
                        }

                        machine->V[machine->opcode & 0x0F00] = machine->V[machine->opcode & 0x0F00] - machine->V[machine->opcode & 0x00F0];
                        break;
                    
                    case 0x0006:
                        if ((machine->V[machine->opcode & 0x0F00] & 0x000F) == 1)
                        {
                            machine->V[0xF] = 1;
                        }
                        else
                        {
                            machine->V[0xF] = 0;
                        }
                        machine->V[machine->opcode & 0x0F00] /= 2;
                        break;
                    
                    case 0x0007:
                        // TODO: Make this ternary
                        if (machine->V[machine->opcode & 0x00F0] > machine->V[machine->opcode & 0x0F00])
                        {
                            machine->V[0xF] = 1;
                        }
                        else
                        {
                            machine->V[0xF] = 0;
                        }

                        machine->V[machine->opcode & 0x0F00] = machine->V[machine->opcode & 0x00F0] - machine->V[machine->opcode & 0x0F00];
                        break;;
                    
                    case 0x000E:
                        // If the most significant bit of Vx is 1, then VF is 1, else 0. Then Vx is multiplied by 2.
                        if ((machine->V[machine->opcode & 0x0F00] & 0xF000) == 1)
                        {
                            machine->V[0xF] = 1;
                        }
                        else
                        {
                            machine->V[0xF] = 0;
                        }
                        machine->V[machine->opcode & 0x0F00] *= 2;
                        break;
                };
                break;

            case 0x9000:
                if (machine->V[(machine->opcode & 0x0F00) >> 8] != (machine->V[(machine->opcode & 0x00F0)]))
                {
                    machine->pc += 4;
                }
                else
                {
                    machine->pc += 2;
                }
                break;
                break;

            case 0xA000:
                machine->iReg = (machine->opcode & 0x0FFF);
                machine->pc += 2;
                break;

            case 0xB000:
                machine->pc = ((machine->opcode & 0x0FFF) + machine->V[0x0]);
                break;

            case 0xC000:
                machine->V[(machine->opcode & 0x0F00) >> 8] = (rand()) & (machine->opcode & 0x00FF);
                machine->pc += 2;
                break;

            case 0xD000:
                vx = machine->V[(machine->opcode & 0x0F00) >> 8];
                vy = machine->V[(machine->opcode & 0x00F0) >> 4];
                machine->V[0xF] = 0;
                height = machine->opcode & 0x000F;
                pixel;

                for (int y = 0; y < height; y++)
                {
                    pixel = machine->memory[machine->iReg + y];
                    for (int x = 0; x < 8; x++)
                    {
                        if ((pixel & (0x80 >> x)) != 0)
                        {
                            if (machine->display[x + vx + ((y + vy) * 64)])
                            {
                                machine->V[0xF] = 1;
                            }
                               
                            machine->display[x + vx + ((y + vy) * 64)] ^= 1;
                        }
                    }
                }
                machine->drawFlag = true;
                machine->pc += 2;
                break;
            
            case 0xE000:
                switch (machine->opcode & 0x000F)
                {
                    case 0x000E:
                        // Skips next instruction if key with value of Vx is pressed.
                        break;
                    
                    case 0x0001:
                        // Skip next instruction if key with value Vx is not pressed.
                        break;

                };
                break;
            
            default:
                //printf("%x: unimplemented.\n", machine->opcode);
                break;
        };
}

void
parseArgs(int argc, char* argv[], chip8* machine)
{
    if (argc != 2)
    {
        printf("No ROM file specified.\n");
        exit(0);
    }
    else
    {
        machine->fileName = argv[1];
    }

    machine->romFile = fopen(machine->fileName, "r");

    if (NULL == machine->romFile)
    {
        printf("%s does not exist.\n", machine->fileName);
        exit(0);
    }
}

void
initMachine(chip8* machine)
{
    // Initialize machine values.
    machine->pc       = 0x200;
    machine->opcode   = 0;
    machine->iReg     = 0;
    machine->stackPtr = 0;
    machine->drawFlag = false;
    memset(machine->display,   0, DISPSIZE     * sizeof(machine->display[0]));
    memset(machine->stack,     0, STACKSIZE    * sizeof(machine->stack[0]));
    memset(machine->V,         0, NUMREGISTERS * sizeof(machine->V[0]));
    memset(machine->memory,    0, MEMSIZE      * sizeof(machine->memory[0]));

    // Load font into memory.
    unsigned char fontToLoad[FONTSIZE] = 
    {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };
    memcpy(machine->fontset, fontToLoad,       sizeof(fontToLoad));
    memcpy(machine->memory,  machine->fontset, sizeof(machine->fontset));

    // Read the rom into memory
    fread((machine->memory + 0x200), 1, (MEMSIZE - 0x200), machine->romFile);
}

// DEBUG FUNCTION
void
printMemory(unsigned char *memory)
{
    int col = 0;
    const int numCols = 8;
    for (int addr = 0; addr < MEMSIZE; addr++)
    {
        if ((col != 0) && ((col % numCols) == 0))
        {
            printf("\n");
        }

        printf("%x ", memory[addr]);
        memory[addr] = 0;
        col++;
    }
}

void
showScreen(unsigned char *display)
{
    for (int pixel = 0; pixel < DISPSIZE; pixel++)
    {
        if (pixel != 0 && ((pixel % DISPWIDTH) == 0))
        {
            printf("\n");
        }

        printf("%d ", display[pixel]);
    }

    printf("\n\n");
}
