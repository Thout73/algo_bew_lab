#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/resource.h>
#include "./cdcl/functions_cdcl.h"

static void print_stats(Stats_CDCL *stats)
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
    int number_of_variables, number_of_clauses, maximum_length;

    if (argc < 2)
    {
        fprintf(stderr,
                "Usage: %s <cnf-file> [DELETE_CLAUSES] [PROOF_LOG] "
                "[RESTARTS] [VSIDS] [DELETE_AFTER] "
                "[DELETE_STEP] [RESTART_AFTER]\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    CDCL_Clause *clauses = parse_cdcl(argv[1],
                                      &number_of_variables,
                                      &number_of_clauses,
                                      &maximum_length);

    CDCL_options options = {
        .use_delete_clauses = 1,
        .use_proof_log = 1,
        .use_restarts = 1,
        .use_vsids = 1,
        .delete_after = 2000,
        .delete_step = 50,
        .restart_after = 40};

    if (argc > 2)
        options.use_delete_clauses = atoi(argv[2]);
    if (argc > 3)
        options.use_proof_log = atoi(argv[3]);
    if (argc > 4)
        options.use_restarts = atoi(argv[4]);
    if (argc > 5)
        options.use_vsids = atoi(argv[5]);
    if (argc > 6)
        options.delete_after = atoi(argv[6]);
    if (argc > 7)
        options.delete_step = atoi(argv[7]);
    if (argc > 8)
        options.restart_after = atoi(argv[8]);

    Assignment *assignment = malloc(sizeof(*assignment) * number_of_variables);
    Stats_CDCL stats;

    int result = CDCL(clauses, number_of_clauses, number_of_variables, assignment, &stats, &options);

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