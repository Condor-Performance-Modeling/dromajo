
void custom_asm() {
    __asm__ volatile (
        "addi sp, sp, -16\n\t"       // Adjust the stack pointer
        "li a0, 8\n\t"               // Load immediate value 8 into a0
        "sd ra, 8(sp)\n\t"           // Save return address to stack
        "auipc a5, 0x91\n\t"         // Load upper immediate with PC-relative offset
        "ld a5, -1018(a5)\n\t"       // Load a value into a5
        "addi a5, a5, 16\n\t"        // Add 16 to a5
        "sd a5, 0(a0)\n\t"           // Store a5 into memory at address in a0
        "auipc a2, 0x91\n\t"         // Load upper immediate with PC-relative offset
        "ld a2, -1502(a2)\n\t"       // Load a value into a2
        "auipc a1, 0x91\n\t"         // Load upper immediate with PC-relative offset
        "ld a1, -1830(a1)\n\t"       // Load a value into a1
    );
}

int main() {
    custom_asm();
    return 0;
}
