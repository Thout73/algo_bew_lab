#include <stdio.h>
#include <stdlib.h>
#include "functions_dpll_etc.h"

void print_assignment_2sat(int *assignment, int number_of_variables)
{
    for (int i = 0; i < number_of_variables; i++)
    {
        printf("%d", assignment[i] * (i + 1));

        if (i < number_of_variables - 1)
            printf(" ");
    }

    printf("\n");
}

static void print_stats(Stats *stats)
{
    printf("c time: %.3fs\n", stats->time);
    printf("c time in unit propagation: %.3fs\n", stats->time_in_unit_prop);
    printf("c peak memory: %d KB\n", stats->used_memory);
    printf("c unit propagations: %d\n", stats->num_of_unit_propagations);
    printf("c decisions: %d\n", stats->num_of_decisions);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("falsche eingabe\n");
    }

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

    Stats stats;
    int *assignment = malloc(number_of_variables * sizeof(int));

    int result = SAT2(
        number_of_variables,
        number_of_clauses,
        clauses,
        assignment,
        &stats);

    if (result == 20)
    {
        printf("s UNSATISFIABLE\n");
    }
    else
    {
        printf("s SATISFIABLE\n");
        printf("v ");
        print_assignment_2sat(assignment, number_of_variables);
    }

    print_stats(&stats);

    for (int i = 0; i < number_of_clauses; i++)
    {
        free(clauses[i].literals);
    }
    free(clauses);

    free(assignment);

    return 0;
}