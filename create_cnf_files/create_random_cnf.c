#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Fisher-Yates Shuffle */
static void shuffle(int *array, int n)
{
    for (int i = n - 1; i > 0; --i)
    {
        int j = rand() % (i + 1);

        int tmp = array[i];
        array[i] = array[j];
        array[j] = tmp;
    }
}

static int compare_ints(const void *a, const void *b)
{
    return (*(const int *)a - *(const int *)b);
}

/* Prüft, ob Klausel bereits existiert */
static int clause_exists(
    int **cnf,
    int clause_count,
    int clause_len,
    const int *candidate)
{
    for (int i = 0; i < clause_count; ++i)
    {
        int equal = 1;

        for (int j = 0; j < clause_len; ++j)
        {
            if (cnf[i][j] != candidate[j])
            {
                equal = 0;
                break;
            }
        }

        if (equal)
            return 1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        fprintf(stderr,
                "Usage:\n"
                "%s <variables> <clauses> <clause_length> <outputfile>\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    const int num_vars = atoi(argv[1]);
    const int num_clauses = atoi(argv[2]);
    const int clause_len = atoi(argv[3]);
    const char *output_file = argv[4];

    if (num_vars <= 0 || num_clauses <= 0 || clause_len <= 0)
    {
        fprintf(stderr, "All parameters must be positive.\n");
        return EXIT_FAILURE;
    }

    if (clause_len > num_vars)
    {
        fprintf(stderr,
                "Clause length cannot exceed number of variables.\n");
        return EXIT_FAILURE;
    }

    srand((unsigned int)time(NULL));

    /* Variablenliste */
    int *variables = malloc(num_vars * sizeof(int));
    if (!variables)
        return EXIT_FAILURE;

    for (int i = 0; i < num_vars; ++i)
        variables[i] = i + 1;

    /* CNF-Speicher */
    int **cnf = malloc(num_clauses * sizeof(int *));
    if (!cnf)
    {
        free(variables);
        return EXIT_FAILURE;
    }

    for (int i = 0; i < num_clauses; ++i)
    {
        cnf[i] = malloc(clause_len * sizeof(int));
        if (!cnf[i])
        {
            while (--i >= 0)
                free(cnf[i]);

            free(cnf);
            free(variables);
            return EXIT_FAILURE;
        }
    }

    /* Klauseln erzeugen */
    int generated = 0;

    while (generated < num_clauses)
    {
        shuffle(variables, num_vars);

        int *candidate = malloc(clause_len * sizeof(int));

        for (int i = 0; i < clause_len; ++i)
            candidate[i] = variables[i];

        qsort(candidate,
              clause_len,
              sizeof(int),
              compare_ints);

        for (int i = 0; i < clause_len; ++i)
        {
            if (rand() % 2 == 0)
                candidate[i] = -candidate[i];
        }

        if (!clause_exists(cnf,
                           generated,
                           clause_len,
                           candidate))
        {
            memcpy(cnf[generated],
                   candidate,
                   clause_len * sizeof(int));

            ++generated;
        }

        free(candidate);
    }

    FILE *fp = fopen(output_file, "w");

    if (!fp)
    {
        perror("fopen");
        return EXIT_FAILURE;
    }

    fprintf(fp,
            "p cnf %d %d\n",
            num_vars,
            num_clauses);

    for (int i = 0; i < num_clauses; ++i)
    {
        for (int j = 0; j < clause_len; ++j)
        {
            fprintf(fp, "%d ", cnf[i][j]);
        }

        fprintf(fp, "0\n");
    }

    fclose(fp);

    for (int i = 0; i < num_clauses; ++i)
        free(cnf[i]);

    free(cnf);
    free(variables);

    return EXIT_SUCCESS;
}