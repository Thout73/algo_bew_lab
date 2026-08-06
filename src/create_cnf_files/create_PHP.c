#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage:\n %s <n> <output-file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *endptr;
    long n = strtol(argv[1], &endptr, 10);

    if (*argv[1] == '\0' || *endptr != '\0' || n < 1)
    {
        fprintf(stderr, "Error: <n> must be a positive integer.\n\n");
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    FILE *f = fopen(argv[2], "w");

    if (f == NULL)
    {
        perror("Error opening output file");
        return EXIT_FAILURE;
    }

    long num_var = n * (n + 1);
    long num_clauses = (n + 1) + ((n + 1) * n / 2) * n;

    fprintf(f, "p cnf %ld %ld\n", num_var, num_clauses);

    /* Every pigeon must be placed in at least one hole. */
    for (long i = 1; i <= n + 1; i++)
    {
        for (long j = 1; j <= n; j++)
        {
            fprintf(f, "%ld ", (i - 1) * n + j);
        }

        fprintf(f, "0\n");
    }

    /* No two pigeons may occupy the same hole. */
    for (long i1 = 1; i1 <= n + 1; i1++)
    {
        for (long i2 = i1 + 1; i2 <= n + 1; i2++)
        {
            for (long j = 1; j <= n; j++)
            {
                fprintf(f,
                        "-%ld -%ld 0\n",
                        (i1 - 1) * n + j,
                        (i2 - 1) * n + j);
            }
        }
    }

    fclose(f);

    return EXIT_SUCCESS;
}