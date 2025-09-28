void new_sd_get_seats(void);
__asm__(".symver new_sd_get_seats,sd_get_seats@LIBSYSTEMD_209");
#if defined __powerpc64__ && defined _CALL_AIXDESC && !defined _CALL_LINUX
__asm__(".symver .new_sd_get_seats,.sd_get_seats@LIBSYSTEMD_209");
#endif
void (*resolve_sd_get_seats(void)) (void) __asm__ ("sd_get_seats");
void (*resolve_sd_get_seats(void)) (void) {
        return new_sd_get_seats;
}
__asm__(".type sd_get_seats, %gnu_indirect_function");
