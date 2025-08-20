#include "../0x_includes/glob.h"

#define USAGE   "Usage ./inf0 (ง'̀-'́)ง\n"

void    errors(err_code err, char *reason)
{
    (void)reason;
    if (err == ARGS)
        fprintf(stderr, USAGE);
}