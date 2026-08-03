#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "functions_cdcl.h"

int parse_literal_list(char *args, int *out, int max)
{
    int n = 0;
    char *tok = strtok(args, " \t");
    while (tok != NULL && n < max)
    {
        out[n++] = atoi(tok);
        tok = strtok(NULL, " \t");
    }
    return n;
}

int parse_dimacs_clause(char *args, int *out, int max)
{
    int n = 0;
    char *tok = strtok(args, " \t");
    while (tok != NULL && n < max)
    {
        int v = atoi(tok);
        if (v == 0)
            break;
        out[n++] = v;
        tok = strtok(NULL, " \t");
    }
    return n;
}

void handle_assume_command(char *args, Assignment *assignment, Trail *trail)
{
    int lits[1024];
    int n = parse_literal_list(args, lits, 1024);

    for (int i = 0; i < n; i++)
    {
        int lit = lits[i];
        int var = abs(lit) - 1;
        int val = sign(lit);

        if (assignment[var].value == UNASSIGNED)
        {
            assignment[var].value = val;
            assignment[var].decision_lvl = 0;
            assignment[var].reason = NULL;
            assignment[var].is_assumption = 1;
            trail_push(trail, var, val, NULL, 0);
        }
        else if (assignment[var].value != val)
        {
            printf("Widerspruch: Variable %d ist bereits gegenteilig belegt (assumption ignoriert).\n", var + 1);
        }
    }
}

void purge_assumption_clauses(LearnedClauses *list, WatchDB *watchDB)
{
    for (int i = list->size - 1; i >= 0; i--)
    {
        CDCL_Clause *curr = list->data[i];

        if (!curr->under_assumption)
            continue;

        if (curr->size > 1)
        {
            int lit1 = curr->literals[curr->watch1];
            int v1 = abs(lit1) - 1;
            if (lit1 > 0)
                watchlist_remove(&watchDB->pos[v1], curr);
            else
                watchlist_remove(&watchDB->neg[v1], curr);

            int lit2 = curr->literals[curr->watch2];
            int v2 = abs(lit2) - 1;
            if (lit2 > 0)
                watchlist_remove(&watchDB->pos[v2], curr);
            else
                watchlist_remove(&watchDB->neg[v2], curr);
        }

        free(curr->literals);
        free(curr);
        learned_delete(list, i); // size--, swap letztes Element an Position i
    }
}

void clear_assumptions(Trail *trail, Assignment *assignment, size_t baseline_trail_size, LearnedClauses *learned, WatchDB *watchDB)
{
    // 1. Trail/Belegung zurücksetzen
    while (trail->size > baseline_trail_size)
    {
        Assignment *a = &trail->data[trail->size - 1];
        int var = a->literal;

        assignment[var].value = UNASSIGNED;
        assignment[var].decision_lvl = -1;
        assignment[var].reason = NULL;
        assignment[var].is_assumption = 0;

        trail->size--;
    }
    purge_assumption_clauses(learned, watchDB);
}

void add_permanent_clause(char *args, WatchDB *watchDB, LearnedClauses *permanent_clauses, int *next_clause_id, Assignment *assignment, Trail *trail)
{
    int lits[1024];
    int n = parse_dimacs_clause(args, lits, 1024);

    if (n == 0)
    {
        printf("Leere/ungueltige Klausel ignoriert.\n");
        return;
    }

    CDCL_Clause *curr = malloc(sizeof(CDCL_Clause));
    curr->literals = malloc(n * sizeof(int));
    memcpy(curr->literals, lits, n * sizeof(int));
    curr->size = n;
    curr->id = (*next_clause_id)++;
    curr->literal_block_distance = 0;
    curr->under_assumption = 0; // permanent -- haengt an keiner assume ab
    curr->watch1 = 0;
    curr->watch2 = (n > 1) ? 1 : 0;

    if (n > 1)
    {
        int lit1 = curr->literals[curr->watch1];
        int v1 = abs(lit1) - 1;
        if (lit1 > 0)
            watchlist_add(&watchDB->pos[v1], curr);
        else
            watchlist_add(&watchDB->neg[v1], curr);

        int lit2 = curr->literals[curr->watch2];
        int v2 = abs(lit2) - 1;
        if (lit2 > 0)
            watchlist_add(&watchDB->pos[v2], curr);
        else
            watchlist_add(&watchDB->neg[v2], curr);
    }

    learned_add(permanent_clauses, curr);
}

int CDCL_inkrementell(CDCL_Clause *clauses, int number_of_clauses, int number_of_variables)
{
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

    LearnedClauses permanent_clauses;
    learned_init(&permanent_clauses);

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

    // set baseline
    int trail_baseline = trail.size;

    while (1)
    {
        CDCL_Clause *confl_clause = Unitpropagation(number_of_clauses, number_of_variables, &trail, watchDB, assignment, decision_lvl, trail_lvl);
        while (confl_clause != NULL)
        {
            num_of_confl++;
            if (decision_lvl == 0)
            {
                printf("s UNSATISFIABLE\nBeende...\n");
                // free memory
                free(assignment);
                trail_destroy(&trail);
                learned_destroy(&learned);
                learned_destroy(&permanent_clauses);
                watchdb_destroy(watchDB, number_of_variables);

                return 0;
            }
            CDCL_Clause *learned_clause = analyse_conflict(&trail, &learned, watchDB, &next_claus_id, assignment, confl_clause, &backtrack_level, &UIP_lit, number_of_variables, decision_lvl);

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
            printf("s SATISFIABLE\n");
            printf("v ");
            for (int i = 0; i < number_of_variables; i++)
            {
                printf("%d ", assignment[i].value * (i + 1));
            }
            printf("\n");

            // hier dann auf neue eingaben warten
            // bei klausel einfach hinzufügen

            backtrack(0, &trail, assignment);
            decision_lvl = 0;

            printf("Neue Eingaben:\n");

            char line[256];
            int done = 0;

            clear_assumptions(&trail, assignment, trail_baseline, &learned, watchDB);

            while (!done)
            {
                printf("> ");
                fflush(stdout);

                if (fgets(line, sizeof(line), stdin) == NULL)
                    break; // EOF (z.B. Ctrl+D) -> Schleife beenden

                // Zeilenumbruch entfernen
                line[strcspn(line, "\n")] = '\0';

                // leere Eingabe ignorieren
                if (line[0] == '\0')
                    continue;

                // erstes Wort (Befehl) und Rest (Argumente) trennen
                char command[64];
                char args[192] = {0};
                int matched = sscanf(line, "%63s %191[^\n]", command, args);

                if (matched < 1)
                    continue;

                if (strcmp(command, "exit") == 0 || strcmp(command, "quit") == 0)
                {
                    printf("Beende...\n");

                    // free memory
                    free(assignment);
                    trail_destroy(&trail);
                    learned_destroy(&learned);
                    learned_destroy(&permanent_clauses);
                    watchdb_destroy(watchDB, number_of_variables);

                    return 1;
                }
                else if (strcmp(command, "solve") == 0)
                {
                    // unit klauseln aus permanenten lernen
                    for (int i = 0; i < permanent_clauses.size; i++)
                    {
                        if (permanent_clauses.data[i]->size == 1)
                        {
                            int var = abs(permanent_clauses.data[i]->literals[0]) - 1;
                            int val = sign(permanent_clauses.data[i]->literals[0]);

                            if (assignment[var].value == UNASSIGNED)
                            {
                                assignment[var].value = val;
                                assignment[var].decision_lvl = 0;
                                assignment[var].reason = NULL;
                                trail_push(&trail, var, val, NULL, 0);
                            }
                            else if (assignment[var].value != val)
                            {
                                printf("Widerspruch: neue Unit-Klausel steht im Konflikt mit bestehender Belegung.\n");
                                return 4;
                            }
                        }
                    }

                    done = 1;
                }
                else if (strcmp(command, "clause") == 0)
                {
                    add_permanent_clause(args, watchDB, &permanent_clauses, &next_claus_id, assignment, &trail);
                }
                else if (strcmp(command, "assume") == 0)
                {
                    handle_assume_command(args, assignment, &trail);
                }
                else if (strcmp(command, "help") == 0)
                {
                    printf("Verfuegbare Befehle: solve, assume, clause, help, exit\n");
                }
                else
                {
                    printf("Unbekannter Befehl: %s (siehe 'help')\n", command);
                }
            }
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