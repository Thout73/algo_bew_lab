#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "functions_cdcl.h"

int CDCL(CDCL_Clause *clauses, int number_of_clauses, int number_of_variables)
{
    FILE *proof_log = fopen("./cdcl/proof_log.cnf", "a");

    int decision_lvl = 0;
    Assignment *assignment = malloc(sizeof(Assignment) * number_of_variables);
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

    int restart_after = 200;
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
        CDCL_Clause *confl_clause = Unitpropagation(number_of_clauses, number_of_variables, &trail, watchDB, assignment, decision_lvl, trail_lvl);
        while (confl_clause != NULL)
        {
            num_of_confl++;
            if (decision_lvl == 0)
            {
                // free memory
                free(assignment);
                trail_destroy(&trail);
                learned_destroy(&learned);
                watchdb_destroy(watchDB, number_of_variables);

                fprintf(proof_log, "0");

                fclose(proof_log);

                return 0;
            }
            CDCL_Clause *learned_clause = analyse_conflict(&trail, &learned, watchDB, &next_claus_id, assignment, confl_clause, &backtrack_level, &UIP_lit, number_of_variables, decision_lvl);
            // proof log
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
            confl_clause = Unitpropagation(number_of_clauses, number_of_variables, &trail, watchDB, assignment, decision_lvl, trail_lvl);
        }
        if (trail.size == number_of_variables)
        {
            // print assignment
            printf("Assignment\n");
            for (int i = 0; i < number_of_variables; i++)
            {
                printf("%d ", assignment[i].value * (i + 1));
            }
            printf("\n");

            // free memory
            free(assignment);
            trail_destroy(&trail);
            learned_destroy(&learned);
            watchdb_destroy(watchDB, number_of_variables);

            fclose(proof_log);

            return 1;
        }

        // restart
        if (num_of_confl > restart_after)
        {
            num_of_restarts++;
            restart_after = getLubyElement(num_of_restarts) * 100;
            num_of_confl = 0;
            backtrack(0, &trail, assignment);
            decision_lvl = 0;
            printf("RESTART, %d\n", num_of_restarts);
        }

        // delete clauses
        if (learned.size > delete_after)
        {
            delete_after += delete_step;

            qsort(learned.data, learned.size, sizeof(CDCL_Clause *), clause_cmp_lbd_desc);

            int num_of_all = learned.size;
            int target = num_of_all / 2;
            int num_of_del = 0;

            for (int i = target - 1; i >= 0; i--)
            {
                CDCL_Clause *curr = learned.data[i];

                if (is_locked(curr, assignment))
                    continue; // aktiv als reason genutzt -> nicht anfassen

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
                num_of_del++;
            }
        }

        decision_lvl++;
        decide(assignment, &trail, decision_lvl, number_of_variables);
        trail_lvl = trail.size - 1;
    }
}

int main(int argc, char *argv[])
{
    int number_of_variables, number_of_clauses, maximum_length;

    CDCL_Clause *clauses = parse("./cdcl/debug.cnf", &number_of_variables, &number_of_clauses, &maximum_length);

    int result = CDCL(clauses, number_of_clauses, number_of_variables);
    printf("%d\n", result);

    for (int i = 0; i < number_of_clauses; i++)
    {
        free(clauses[i].literals);
    }

    free(clauses);

    return 0;
}