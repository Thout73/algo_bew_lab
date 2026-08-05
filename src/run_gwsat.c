#include <stdio.h>
#include <stdlib.h>
#include "functions_dpll_etc.h"

int sign(int x)
{
    if (x < 0)
        return -1;
    return 1;
}

void print_assignment(Variable_GWSAT *assignment, int number_of_variables)
{
    for (int i = 0; i < number_of_variables; i++)
    {
        printf("%d", assignment[i].value * (i + 1));

        if (i < number_of_variables - 1)
            printf(" ");
    }

    printf("\n");
}

static void print_stats(Stats_GWSAT *stats)
{
    printf("c time: %.3fs\n", stats->time);
    printf("c peak memory: %d KB\n", stats->used_memory);
    printf("c tries used: %d\n", stats->number_of_used_tries);
    printf("c steps used: %d\n", stats->number_of_used_steps);
}
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr,
                "Usage: %s <cnf-file> [MAX_TRIES] [MAX_STEPS] [PROBABILITY]\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    int max_tries = 100;
    int max_steps = 1000;
    double probability = 0.5;

    if (argc > 2)
        max_tries = atoi(argv[2]);
    if (argc > 3)
        max_steps = atoi(argv[3]);
    if (argc > 4)
        probability = atof(argv[4]);

    int number_of_variables;
    int number_of_clauses;
    int maximum_length;

    Clause *clauses = parse(
        argv[1],
        &number_of_variables,
        &number_of_clauses,
        &maximum_length);

    if (clauses == NULL)
    {
        printf("Fehler beim Parsen\n");
        return 1;
    }

    Variable_GWSAT *assignment = malloc(number_of_variables * sizeof(Variable_GWSAT));

    for (int i = 0; i < number_of_variables; i++)
    {
        assignment[i].pos_clauses = malloc(number_of_clauses * sizeof(int));
        assignment[i].neg_clauses = malloc(number_of_clauses * sizeof(int));
    }

    Stats_GWSAT stats;
    int result = GWSAT(number_of_variables, number_of_clauses, clauses, max_tries, max_steps, probability, assignment, &stats);

    if (result == 0)
    {
        printf("s UNKNWON\n");
    }
    else
    {
        printf("s SATISFIABLE\n");
        printf("v ");
        print_assignment(assignment, number_of_variables);
    }

    print_stats(&stats);

    for (int i = 0; i < number_of_clauses; i++)
        free(clauses[i].literals);

    free(clauses);

    for (int i = 0; i < number_of_variables; i++)
    {
        free(assignment[i].pos_clauses);
        free(assignment[i].neg_clauses);
    }
    free(assignment);

    if (result == 10)
        return 10;
    return 0;
}