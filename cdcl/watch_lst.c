#include <stdlib.h>
#include <stdio.h>
#include "functions_cdcl.h"

WatchDB *watchdb_init(int num_vars)
{
    WatchDB *db = malloc(sizeof(WatchDB));

    db->pos = malloc(num_vars * sizeof(WatchList));
    db->neg = malloc(num_vars * sizeof(WatchList));

    for (int i = 0; i < num_vars; i++)
    {
        db->pos[i].size = 0;
        db->pos[i].cap = 4;
        db->pos[i].data = malloc(db->pos[i].cap * sizeof(CDCL_Clause *));

        db->neg[i].size = 0;
        db->neg[i].cap = 4;
        db->neg[i].data = malloc(db->neg[i].cap * sizeof(CDCL_Clause *));
    }

    return db;
}

void watchlist_add(WatchList *wl, CDCL_Clause *clause)
{
    if (wl->size == wl->cap)
    {
        wl->cap *= 2;
        wl->data = realloc(wl->data, wl->cap * sizeof(CDCL_Clause *));
    }

    wl->data[wl->size++] = clause;
}

void watchlist_remove(WatchList *wl, CDCL_Clause *clause)
{
    for (int i = 0; i < wl->size; i++)
    {
        if (wl->data[i] == clause)
        {
            wl->data[i] = wl->data[wl->size - 1];
            wl->size--;
            return;
        }
    }
}

void build_watchdb(WatchDB *db, CDCL_Clause *clauses, int n_clauses)
{
    for (int i = 0; i < n_clauses; i++)
    {
        CDCL_Clause *clause = &clauses[i];

        int lit1 = clauses[i].literals[clause->watch1];
        int lit2 = clause->literals[clause->watch2];

        int v1 = abs(lit1) - 1;
        int v2 = abs(lit2) - 1;

        if (lit1 > 0)
            watchlist_add(&db->pos[v1], clause);
        else
            watchlist_add(&db->neg[v1], clause);

        if (lit2 > 0)
            watchlist_add(&db->pos[v2], clause);
        else
            watchlist_add(&db->neg[v2], clause);
    }
}

void move_watch(WatchDB *db, int old_lit, int new_lit, CDCL_Clause *clause)
{
    int ov = abs(old_lit) - 1;
    int nv = abs(new_lit) - 1;

    // remove old
    if (old_lit > 0)
        watchlist_remove(&db->pos[ov], clause);
    else
        watchlist_remove(&db->neg[ov], clause);

    // add new
    if (new_lit > 0)
        watchlist_add(&db->pos[nv], clause);
    else
        watchlist_add(&db->neg[nv], clause);
}