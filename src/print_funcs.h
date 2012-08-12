#ifndef PRINT_FUNC_H
#define PRINT_FUNC_H

void print_free_default(double percent_free);
void print_free_visual(double percent_free);

typedef void (*print_func_t)(double);

#endif /* PRINT_FUNC_H */
