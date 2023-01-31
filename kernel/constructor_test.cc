void terminal_writestring(const char* data);

__attribute__((constructor)) void this_is_constructor()
{
    //terminal_writestring("If this printed, constructor works!\nit can be used for initializating modules without calling each method, or other(like auto initializating gdt).\nok, test passed.\n(i like c/c++).\n");
}