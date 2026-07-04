#ifndef TRAIL_H
#define TRAIL_H
#include <stddef.h>

typedef struct
{
    int size;
    int id;
    int *literals;
    int watch1;
    int watch2;
} CDCL_Clause;

typedef struct
{
    int literal;
    int value;
    CDCL_Clause *reason;
    int decision_lvl;
} Assignment;

typedef struct
{
    Assignment *data;
    size_t size;
    size_t capacity;
} Trail;

typedef struct
{
    CDCL_Clause **data;
    int size;
    int cap;
} WatchList;

typedef struct
{
    WatchList *pos; // literal i ist TRUE
    WatchList *neg; // literal i ist FALSE
} WatchDB;

typedef struct
{
    CDCL_Clause **data;
    int size;
    int capacity;
} LearnedClauses;

void learned_init(LearnedClauses *lc);
CDCL_Clause *analyse_conflict(Trail *trail, LearnedClauses *learned, WatchDB *watch_DB, int *next_clause_id, Assignment *assignment, int conflict_qhead, int *backtrack_level, int num_vars);

void trail_init(Trail *trail);

void trail_push(Trail *trail,
                int literal,
                int value,
                CDCL_Clause *reason,
                int decision_lvl);

void trail_backtrack(Trail *trail,
                     size_t target_size);

void trail_destroy(Trail *trail);

void build_watchdb(WatchDB *db, CDCL_Clause *clauses, int n_clauses);

WatchDB *watchdb_init(int num_vars);

void move_watch(WatchDB *db, int old_lit, int new_lit, CDCL_Clause *c);

void watchlist_add(WatchList *wl, CDCL_Clause *c);

CDCL_Clause *parse(char *file_str, int *number_of_variables, int *number_of_clauses, int *maximum_length);

#endif // TRAIL_H