#ifndef PRINT_FUNC_H
#define PRINT_FUNC_H

void print_free_default(const char *timestr, double percent_free);
void print_free_visual(const char *timestr, double percent_free);

typedef void (*print_func_t)(const char *, double);

#endif /* PRINT_FUNC_H */
