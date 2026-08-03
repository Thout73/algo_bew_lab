#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/resource.h>
#include "./cdcl/functions_cdcl.h"

static void print_stats(Stats *stats)
{
    printf("c time: %.3fs\n", stats->time);
    printf("c time in unit propagation: %.3fs\n", stats->time_in_unit_prop);
    printf("c peak memory: %d KB\n", stats->used_memory);
    printf("c unit propagations: %d\n", stats->num_of_unit_propagations);
    printf("c conflicts: %d\n", stats->num_of_confl);
    printf("c restarts: %d\n", stats->num_of_restarts);
    printf("c decisions: %d\n", stats->num_of_decisions);
}

int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        printf("%d, falsche eingabe\n", argc);
        return 4;
    }
    int number_of_variables, number_of_clauses, maximum_length;

    CDCL_Clause *clauses = parse(argv[1], &number_of_variables, &number_of_clauses, &maximum_length);

    Assignment *assignment = malloc(sizeof(*assignment) * number_of_variables);
    Stats stats;

    int result = CDCL(clauses, number_of_clauses, number_of_variables, assignment, &stats);

    if (result == 10)
    {
        printf("s SATISFIABLE\n");

        printf("v ");
        for (int i = 0; i < number_of_variables; i++)
        {
            printf("%d ", assignment[i].value * (i + 1));
        }
        printf("\n");
    }
    else
    {
        printf("s UNSATISFIABLE\n");
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