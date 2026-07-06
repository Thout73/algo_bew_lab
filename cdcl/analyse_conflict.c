#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "functions_cdcl.h"

void sorted_insert_unique(int *list, int *size, int value)
{
    // Binaere Suche nach der Einfuegeposition
    int lo = 0, hi = *size;
    while (lo < hi)
    {
        int mid = (lo + hi) / 2;
        if (abs(list[mid]) == abs(value))
            return; // bereits vorhanden, nichts tun
        if (abs(list[mid]) < abs(value))
            lo = mid + 1;
        else
            hi = mid;
    }
    // lo ist jetzt die Einfuegeposition
    // alles ab lo um eine Stelle nach rechts schieben
    for (int i = *size; i > lo; i--)
        list[i] = list[i - 1];
    list[lo] = value;
    (*size)++;
}

void learned_init(LearnedClauses *lc)
{
    lc->size = 0;
    lc->capacity = 8;
    lc->data = malloc(lc->capacity * sizeof(CDCL_Clause *));
    if (lc->data == NULL)
    {
        fprintf(stderr, "malloc failed\n");
        exit(EXIT_FAILURE);
    }
}

static void learned_add(LearnedClauses *lc, CDCL_Clause *c)
{
    if (lc->size == lc->capacity)
    {
        lc->capacity *= 2;
        CDCL_Clause **tmp = realloc(lc->data, lc->capacity * sizeof(CDCL_Clause *));
        if (tmp == NULL)
        {
            fprintf(stderr, "realloc failed\n");
            exit(EXIT_FAILURE);
        }
        lc->data = tmp;
    }
    lc->data[lc->size++] = c;
}

CDCL_Clause *analyse_conflict(Trail *trail, LearnedClauses *learned, WatchDB *watch_DB, int *next_clause_id, Assignment *assignment, CDCL_Clause *confl, int *backtrack_level, int *UIP_lit, int num_vars, int decision_lvl)
{
    int *learned_lits = malloc(num_vars * sizeof(int));
    int learned_size = 0;

    int *learn_queue = calloc(num_vars, sizeof(int));
    int learn_queue_head = 0;
    int learn_queue_size = 0;

    int *seen = calloc(num_vars, sizeof(int));

    for (int i = 0; i < confl->size; i++)
    {
        int lit = confl->literals[i];
        int var = abs(lit) - 1;
        if (seen[var] == 1)
            continue;
        seen[var] = 1;
        if (assignment[var].decision_lvl == decision_lvl)
            learn_queue[learn_queue_size++] = lit;
        else
            sorted_insert_unique(learned_lits, &learned_size, lit);
    }

    while (learn_queue_size - learn_queue_head != 1)
    {
        int curr_lit = learn_queue[learn_queue_head++];
        int curr_var = abs(curr_lit) - 1;
        CDCL_Clause *curr_reason = assignment[curr_var].reason;

        if (curr_reason != NULL)
        {
            for (int i = 0; i < curr_reason->size; i++)
            {
                int lit = curr_reason->literals[i];
                int var = abs(lit) - 1;

                if (var == curr_var)
                    continue;
                if (seen[var] == 1)
                    continue;

                seen[var] = 1;

                if (assignment[var].decision_lvl == decision_lvl)
                    learn_queue[learn_queue_size++] = lit;
                else
                    sorted_insert_unique(learned_lits, &learned_size, lit);
            }
        }
    }

    // UIP selbst noch hinzufügen
    sorted_insert_unique(learned_lits, &learned_size, learn_queue[learn_queue_head]);
    *UIP_lit = learn_queue[learn_queue_head];

    CDCL_Clause *new_clause = malloc(sizeof(CDCL_Clause));
    new_clause->literals = malloc(learned_size * sizeof(int));
    new_clause->size = learned_size;
    new_clause->id = (*next_clause_id)++;
    memcpy(new_clause->literals, learned_lits, learned_size * sizeof(int));
    free(learned_lits);
    free(learn_queue);
    free(seen);

    if (learned_size == 1)
    {
        // print new clause
        printf("LEARN: ");
        for (int i = 0; i < new_clause->size; i++)
        {
            printf("%d ", new_clause->literals[i]);
        }
        printf("Decisionlvl: %d Backtrack: %d\n", decision_lvl, *backtrack_level);
        *backtrack_level = 0;
        return new_clause;
    }

    int second_highest = -1;
    int watch2 = 0;
    for (int i = 0; i < new_clause->size; i++)
    {
        int curr_lit = new_clause->literals[i];
        if (assignment[abs(curr_lit) - 1].decision_lvl == decision_lvl)
        {
            new_clause->watch1 = i;
        }
        else
        {
            if (assignment[abs(curr_lit) - 1].decision_lvl > second_highest)
            {
                second_highest = assignment[abs(curr_lit) - 1].decision_lvl;
                watch2 = i;
            }
        }
    }

    new_clause->watch2 = watch2;

    *backtrack_level = second_highest;

    learned_add(learned, new_clause);

    if (new_clause->size > 1)
    {
        int lit1 = new_clause->literals[new_clause->watch1];
        int v1 = abs(lit1) - 1;
        if (lit1 > 0)
            watchlist_add(&watch_DB->pos[v1], new_clause);
        else
            watchlist_add(&watch_DB->neg[v1], new_clause);

        int lit2 = new_clause->literals[new_clause->watch2];
        int v2 = abs(lit2) - 1;
        if (lit2 > 0)
            watchlist_add(&watch_DB->pos[v2], new_clause);
        else
            watchlist_add(&watch_DB->neg[v2], new_clause);
    }

    // print new clause
    printf("LEARN: ");
    for (int i = 0; i < new_clause->size; i++)
    {
        printf("%d ", new_clause->literals[i]);
    }
    printf("Decisionlvl: %d Backtrack: %d\n", decision_lvl, *backtrack_level);

    return new_clause;
}