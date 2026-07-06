#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "functions_cdcl.h"

#define UNASSIGNED 0
#define FALSE -1
#define TRUE 1

int sign(int x)
{
    if (x < 0)
        return -1;
    return 1;
}

CDCL_Clause *Unitpropagation(CDCL_Clause *clauses, int number_of_clauses, Trail *trail, WatchDB *watch_DB, Assignment *assignment, int decision_lvl, int start_qhead)
{
    int qhead = start_qhead;
    while (qhead < trail->size && trail->size > 0)
    {
        int literal = trail->data[qhead].literal;
        int literal_value = trail->data[qhead].value;

        CDCL_Clause *curr;
        WatchList *current_clauses;
        if (literal_value == FALSE)
        {
            current_clauses = &watch_DB->pos[literal];
        }
        else
        {
            current_clauses = &watch_DB->neg[literal];
        }

        int i = 0;
        while (i < current_clauses->size && current_clauses->data[i]->size > 1)
        {
            curr = current_clauses->data[i];
            // set watches
            int active_watch = -1;
            int inactive_watch = -1;
            int which_active = 0;
            if (curr->literals[curr->watch1] == -1 * (literal + 1) * literal_value)
            {
                active_watch = curr->watch1;
                inactive_watch = curr->watch2;
                which_active = 1;
            }
            if (curr->literals[curr->watch2] == -1 * (literal + 1) * literal_value)
            {
                active_watch = curr->watch2;
                inactive_watch = curr->watch1;
                which_active = 2;
            }
            int old_watch = active_watch;
            int curr_lit;
            int sat = 0;
            int unassigned = 0;

            // walk watch
            do
            {
                active_watch = (active_watch + 1) % curr->size;
                if (active_watch == inactive_watch)
                {
                    active_watch = (active_watch + 1) % curr->size;
                }

                curr_lit = curr->literals[active_watch];
                if (assignment[abs(curr_lit) - 1].value == sign(curr_lit) && assignment[abs(curr_lit) - 1].decision_lvl <= decision_lvl)
                {
                    sat = 1;
                }
                unassigned = (assignment[abs(curr_lit) - 1].value == 0);
            } while (unassigned == 0 && active_watch != old_watch && sat == 0); // stop when unassigend, true or loop

            if (active_watch != old_watch)
            {
                move_watch(watch_DB, curr->literals[old_watch], curr->literals[active_watch], curr);
            }

            if (which_active == 1)
            {
                curr->watch1 = active_watch;
            }
            else
            {
                curr->watch2 = active_watch;
            }

            if (curr->watch1 == curr->watch2)
            {
                printf("%d, %d\n", curr->id, decision_lvl);
            }

            // no further unassigend
            if (active_watch == old_watch)
            {
                if (assignment[abs(curr->literals[inactive_watch]) - 1].value == 0 || assignment[abs(curr->literals[inactive_watch]) - 1].decision_lvl > decision_lvl) // true
                {
                    curr_lit = curr->literals[inactive_watch];
                    // change assigenmt, push on trail

                    assignment[abs(curr_lit) - 1].value = sign(curr_lit);
                    assignment[abs(curr_lit) - 1].decision_lvl = decision_lvl;
                    assignment[abs(curr_lit) - 1].reason = curr;

                    trail_push(trail, abs(curr_lit) - 1, sign(curr_lit), curr, decision_lvl);
                }
                else
                {
                    if (assignment[abs(curr->literals[inactive_watch]) - 1].value != UNASSIGNED &&
                        assignment[abs(curr->literals[inactive_watch]) - 1].value != sign(curr->literals[inactive_watch]))
                    {
                        return curr; // conflict
                    }
                }
                i++;
            }
        }
        qhead++;
    }
    return NULL;
}

int backtrack(int target_lvl, Trail *trail, Assignment *assignment)
{
    while (trail->size > 0)
    {
        Assignment *a = &trail->data[trail->size - 1];
        int var = a->literal;

        // nur bis target_lvl zurückgehen
        if (a->decision_lvl <= target_lvl)
            break;

        assignment[var].value = UNASSIGNED;
        assignment[var].decision_lvl = -1;
        assignment[var].reason = NULL;

        trail->size--;
    }

    return target_lvl;
}
int apply_restart_policy() { return 0; } // for now false

// for now random
void decide(Assignment *assignment, Trail *trail, int decision_lvl, int number_of_variables)
{
    for (int i = 0; i < number_of_variables; i++)
    {
        if (assignment[i].value == UNASSIGNED)
        {

            assignment[i].value = (rand() % 2 == 0) ? TRUE : FALSE;
            assignment[i].decision_lvl = decision_lvl;
            assignment[i].reason = NULL;

            trail_push(trail, i, assignment[i].value, NULL, decision_lvl);
            return;
        }
    }
}

int CDCL(CDCL_Clause *clauses, int number_of_clauses, int number_of_variables)
{
    int decision_lvl = 0;
    Assignment *assignment = malloc(sizeof(Assignment) * number_of_variables);
    for (int i = 0; i < number_of_variables; i++)
    {
        assignment[i].decision_lvl = -1;
        assignment[i].literal = i;
        assignment[i].value = UNASSIGNED;
        assignment[i].reason = NULL;
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
            trail_push(&trail, var, sign(lit), NULL, decision_lvl);
        }
    }

    while (1)
    {
        CDCL_Clause *confl_clause = Unitpropagation(clauses, number_of_clauses, &trail, watchDB, assignment, decision_lvl, trail_lvl);
        while (confl_clause != NULL)
        {
            if (decision_lvl == 0)
            {
                return 0;
            }
            CDCL_Clause *learned_clause = analyse_conflict(&trail, &learned, watchDB, &next_claus_id, assignment, confl_clause, &backtrack_level, &UIP_lit, number_of_variables, decision_lvl);
            backtrack(backtrack_level, &trail, assignment);
            decision_lvl = backtrack_level;

            int UIP_var = abs(UIP_lit) - 1;
            assignment[UIP_var].value = sign(UIP_lit);
            assignment[UIP_var].decision_lvl = decision_lvl;
            assignment[UIP_var].reason = learned_clause;
            trail_push(&trail, UIP_var, sign(UIP_lit), learned_clause, decision_lvl);
            trail_lvl = trail.size - 1;
            confl_clause = Unitpropagation(clauses, number_of_clauses, &trail, watchDB, assignment, decision_lvl, trail_lvl);
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
            return 1;
        }
        // int restart = apply_restart_policy();
        decision_lvl++;
        decide(assignment, &trail, decision_lvl, number_of_variables);
        trail_lvl = trail.size - 1;
        printf("DECIDE lvl=%d: var=%d val=%d\n", decision_lvl, trail.data[trail.size - 1].literal + 1, trail.data[trail.size - 1].value);
    }
}

int main(int argc, char *argv[])
{
    int number_of_variables, number_of_clauses, maximum_length;

    CDCL_Clause *clauses = parse("debug.cnf", &number_of_variables, &number_of_clauses, &maximum_length);

    int result = CDCL(clauses, number_of_clauses, number_of_variables);
    printf("%d\n", result);

    free(clauses);

    return 1;
}