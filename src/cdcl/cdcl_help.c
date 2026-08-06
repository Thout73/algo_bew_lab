#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "functions_cdcl.h"

// diverse Hilfs-Funktionen für den CDCL-Solver

int sign(int x)
{
    if (x < 0)
        return -1;
    return 1;
}

int clause_cmp_lbd_desc(const void *a, const void *b)
{
    CDCL_Clause *ca = *(CDCL_Clause *const *)a;
    CDCL_Clause *cb = *(CDCL_Clause *const *)b;
    return cb->literal_block_distance - ca->literal_block_distance; // absteigend
}

int is_locked(CDCL_Clause *clause, Assignment *assignment)
{
    int v1 = abs(clause->literals[clause->watch1]) - 1;
    int v2 = abs(clause->literals[clause->watch2]) - 1;
    return assignment[v1].reason == clause || assignment[v2].reason == clause;
}

void remove_from_watchlists(WatchDB *watch_DB, CDCL_Clause *clause)
{
    int lit1 = clause->literals[clause->watch1];
    int v1 = abs(lit1) - 1;
    if (lit1 > 0)
        watchlist_remove(&watch_DB->pos[v1], clause);
    else
        watchlist_remove(&watch_DB->neg[v1], clause);

    int lit2 = clause->literals[clause->watch2];
    int v2 = abs(lit2) - 1;
    if (lit2 > 0)
        watchlist_remove(&watch_DB->pos[v2], clause);
    else
        watchlist_remove(&watch_DB->neg[v2], clause);
}

int getLubyElement(int n)
{
    while (1)
    {
        if ((n & (n + 1)) == 0)
        {
            return (n + 1) / 2;
        }

        int k = 1;
        while (k * 2 - 1 < n)
        {
            k *= 2;
        }
        n = n - (k - 1);
    }
}

CDCL_Clause *Unitpropagation(int number_of_clauses, int number_of_variables, Trail *trail, WatchDB *watch_DB, Assignment *assignment, int decision_lvl, int start_qhead)
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
        while (i < current_clauses->size)
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
                    assignment[abs(curr_lit) - 1].old_value = sign(curr_lit);

                    trail_push(trail, abs(curr_lit) - 1, sign(curr_lit), curr, decision_lvl);

                    // calc new lbd
                    if (curr->literal_block_distance != 0)
                    {
                        int new_lbd = calc_lbd(curr, assignment, number_of_variables);
                        if (curr->literal_block_distance > new_lbd)
                        {
                            curr->literal_block_distance = new_lbd;
                        }
                    }
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

void decide(Assignment *assignment, Trail *trail, int decision_lvl, int number_of_variables)
{

    double max_counter = -1;
    int max_counter_index = -1;
    for (int i = 0; i < number_of_variables; i++)
    {
        if (assignment[i].value == UNASSIGNED)
        {
            if (assignment[i].vsids_counter >= max_counter)
            {
                max_counter = assignment[i].vsids_counter;
                max_counter_index = i;
            }
        }
    }

    if (assignment[max_counter_index].old_value != 0)
    {
        assignment[max_counter_index].value = assignment[max_counter_index].old_value;
    }
    else
    {
        assignment[max_counter_index].value = -1;
    }
    assignment[max_counter_index].old_value = assignment[max_counter_index].value;
    assignment[max_counter_index].decision_lvl = decision_lvl;
    assignment[max_counter_index].reason = NULL;

    trail_push(trail, max_counter_index, assignment[max_counter_index].value, NULL, decision_lvl);
    return;
}

void decide_random(Assignment *assignment, Trail *trail, int decision_lvl, int number_of_variables)
{
    int chosen_index = -1;
    int unassigned_count = 0;

    for (int i = 0; i < number_of_variables; i++)
    {
        if (assignment[i].value == UNASSIGNED)
        {
            unassigned_count++;
            // mit Wahrscheinlichkeit 1/unassigned_count die aktuelle Variable wählen
            if (rand() % unassigned_count == 0)
            {
                chosen_index = i;
            }
        }
    }

    int val = (rand() % 2 == 0) ? 1 : -1;

    assignment[chosen_index].value = val;
    assignment[chosen_index].old_value = val;
    assignment[chosen_index].decision_lvl = decision_lvl;
    assignment[chosen_index].reason = NULL;

    trail_push(trail, chosen_index, val, NULL, decision_lvl);
    return;
}