#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/resource.h>
#include "functions_cdcl.h"

static CDCL_Clause *unitprop_timed(int number_of_clauses, int number_of_variables, Trail *trail,
                                   WatchDB *watchDB, Assignment *assignment, int decision_lvl,
                                   int start_qhead, Stats_CDCL *stats)
{
    size_t trail_before = trail->size;
    clock_t t0 = clock();

    CDCL_Clause *result = Unitpropagation(number_of_clauses, number_of_variables, trail, watchDB,
                                          assignment, decision_lvl, start_qhead);

    clock_t t1 = clock();
    stats->time_in_unit_prop += (double)(t1 - t0) / CLOCKS_PER_SEC;
    stats->num_of_unit_propagations += (int)(trail->size - trail_before);

    return result;
}

int CDCL(CDCL_Clause *clauses, int number_of_clauses, int number_of_variables, Assignment *assignment, Stats_CDCL *stats)
{
    FILE *proof_log = fopen("./src/cdcl/proof_log.cnf", "a");

    clock_t cdcl_start = clock();
    memset(stats, 0, sizeof(Stats_CDCL));

    int decision_lvl = 0;
    for (int i = 0; i < number_of_variables; i++)
    {
        assignment[i].decision_lvl = -1;
        assignment[i].literal = i;
        assignment[i].value = UNASSIGNED;
        assignment[i].reason = NULL;
        assignment[i].old_value = 0;
        assignment[i].is_assumption = 0;
    }
    Trail trail;
    trail_init(&trail);

    LearnedClauses learned;
    learned_init(&learned);

    WatchDB *watchDB = watchdb_init(number_of_variables);
    build_watchdb(watchDB, clauses, number_of_clauses);

    int next_claus_id = number_of_clauses + 1;
    int backtrack_level;
    int UIP_lit;

    int trail_lvl = 0;

    int restart_after = 40;
    int num_of_restarts = 0;
    int num_of_confl = 0;

    int delete_after = 3000;
    int delete_step = 500;

    // decide all unit clauses
    for (int i = 0; i < number_of_clauses; i++)
    {
        if (clauses[i].size == 1)
        {
            int lit = clauses[i].literals[0];
            int var = abs(lit) - 1;
            assignment[var].value = sign(lit);
            assignment[var].decision_lvl = decision_lvl;
            assignment[var].reason = NULL;
            assignment[var].vsids_counter = 0;
            trail_push(&trail, var, sign(lit), NULL, decision_lvl);
        }
    }

    while (1)
    {
        CDCL_Clause *confl_clause = unitprop_timed(number_of_clauses, number_of_variables, &trail, watchDB, assignment, decision_lvl, trail_lvl, stats);
        while (confl_clause != NULL)
        {
            num_of_confl++;
            stats->num_of_confl++;

            if (decision_lvl == 0)
            {
                // free memory
                trail_destroy(&trail);
                learned_destroy(&learned);
                watchdb_destroy(watchDB, number_of_variables);

                fprintf(proof_log, "0");
                fclose(proof_log);

                stats->time = (double)(clock() - cdcl_start) / CLOCKS_PER_SEC;
                struct rusage usage;
                getrusage(RUSAGE_SELF, &usage);
                stats->used_memory = (int)usage.ru_maxrss; // in KB

                return 20;
            }
            CDCL_Clause *learned_clause = analyse_conflict(&trail, &learned, watchDB, &next_claus_id, assignment, confl_clause, &backtrack_level, &UIP_lit, number_of_variables, decision_lvl);

            for (int i = 0; i < learned_clause->size; i++)
            {
                fprintf(proof_log, "%d ", learned_clause->literals[i]);
            }
            fprintf(proof_log, "0 \n");

            backtrack(backtrack_level, &trail, assignment);
            trail.b = trail.b * c;
            decision_lvl = backtrack_level;

            int UIP_var = abs(UIP_lit) - 1;
            assignment[UIP_var].value = sign(UIP_lit);
            assignment[UIP_var].decision_lvl = decision_lvl;
            assignment[UIP_var].reason = learned_clause;
            trail_push(&trail, UIP_var, sign(UIP_lit), learned_clause, decision_lvl);
            trail_lvl = trail.size - 1;
            confl_clause = unitprop_timed(number_of_clauses, number_of_variables, &trail, watchDB, assignment, decision_lvl, trail_lvl, stats);
        }
        if (trail.size == number_of_variables)
        {
            // free memory
            trail_destroy(&trail);
            learned_destroy(&learned);
            watchdb_destroy(watchDB, number_of_variables);

            fclose(proof_log);

            stats->time = (double)(clock() - cdcl_start) / CLOCKS_PER_SEC;
            struct rusage usage;
            getrusage(RUSAGE_SELF, &usage);
            stats->used_memory = (int)usage.ru_maxrss; // in KB

            return 10;
        }

        // restart
        if (num_of_confl > restart_after)
        {
            num_of_restarts++;
            stats->num_of_restarts++;
            restart_after = getLubyElement(num_of_restarts) * 100;
            num_of_confl = 0;
            backtrack(0, &trail, assignment);
            decision_lvl = 0;
        }

        // delete clauses
        if (learned.size > delete_after)
        {
            delete_after += delete_step;

            qsort(learned.data, learned.size, sizeof(CDCL_Clause *), clause_cmp_lbd_desc);

            int num_of_all = learned.size;
            int target = num_of_all / 2;

            for (int i = target - 1; i >= 0; i--)
            {
                CDCL_Clause *curr = learned.data[i];

                if (is_locked(curr, assignment))
                    continue;

                fprintf(proof_log, "d ");
                for (int j = 0; j < curr->size; j++)
                {
                    fprintf(proof_log, "%d ", curr->literals[j]);
                }
                fprintf(proof_log, "0 \n");

                remove_from_watchlists(watchDB, curr);
                free(curr->literals);
                free(curr);
                learned_delete(&learned, i);
            }
        }

        decision_lvl++;
        decide(assignment, &trail, decision_lvl, number_of_variables);
        stats->num_of_decisions++;
        trail_lvl = trail.size - 1;
    }
}
