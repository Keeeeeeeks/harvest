/* Build-only compatibility for Lucid libc's legacy vsyscall path under QEMU.
 * Use the x86-64 Linux syscalls while installing packages; this library is removed
 * before the final compiler image is used. No compiler/runtime package is patched.
 */
long time(long *out)
{
    long result;
    __asm__ volatile("syscall" : "=a"(result) : "a"(201), "D"(out) : "rcx", "r11", "memory");
    return result;
}

int gettimeofday(void *tv, void *tz)
{
    long result;
    __asm__ volatile("syscall" : "=a"(result) : "a"(96), "D"(tv), "S"(tz) : "rcx", "r11", "memory");
    return (int)result;
}

int clock_gettime(int clock, void *tp)
{
    long result;
    __asm__ volatile("syscall" : "=a"(result) : "a"(228), "D"((long)clock), "S"(tp) : "rcx", "r11", "memory");
    return (int)result;
}
