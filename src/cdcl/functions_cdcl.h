#ifndef TRAIL_H
#define TRAIL_H
#include <stddef.h>

#define c 1.05

#define UNASSIGNED 0
#define FALSE -1
#define TRUE 1

typedef struct
{
    double time;
    double time_in_unit_prop;
    int used_memory;
    int num_of_unit_propagations;
    int num_of_confl;
    int num_of_restarts;
    int num_of_decisions;
} Stats;

typedef struct
{
    int size;
    int id;
    int *literals;
    int watch1;
    int watch2;
    int literal_block_distance;
    int under_assumption;
} CDCL_Clause;

typedef struct
{
    int literal;
    int value;
    CDCL_Clause *reason;
    int decision_lvl;
    double vsids_counter;
    int old_value;
    int is_assumption;
} Assignment;

typedef struct
{
    Assignment *data;
    size_t size;
    size_t capacity;
    double b;
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

CDCL_Clause *analyse_conflict(Trail *trail, LearnedClauses *learned, WatchDB *watch_DB, int *next_clause_id, Assignment *assignment, CDCL_Clause *confl, int *backtrack_level, int *UIP_lit, int num_vars, int decision_lvl);

int calc_lbd(CDCL_Clause *clause, Assignment *assignment, int decision_lvl);

void learned_delete(LearnedClauses *lc, int index);

void learned_destroy(LearnedClauses *lc);

void learned_add(LearnedClauses *lc, CDCL_Clause *clause);

void watchlist_remove(WatchList *wl, CDCL_Clause *clause);

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

void move_watch(WatchDB *db, int old_lit, int new_lit, CDCL_Clause *clause);

void watchlist_add(WatchList *wl, CDCL_Clause *clause);

void watchdb_destroy(WatchDB *db, int num_vars);

int sign(int x);

int clause_cmp_lbd_desc(const void *a, const void *b);

int is_locked(CDCL_Clause *clause, Assignment *assignment);

void remove_from_watchlists(WatchDB *watch_DB, CDCL_Clause *clause);

int getLubyElement(int n);

CDCL_Clause *Unitpropagation(int number_of_clauses, int number_of_variables, Trail *trail, WatchDB *watch_DB, Assignment *assignment, int decision_lvl, int start_qhead);

int backtrack(int target_lvl, Trail *trail, Assignment *assignment);

void decide(Assignment *assignment, Trail *trail, int decision_lvl, int number_of_variables);

CDCL_Clause *parse(char *file_str, int *number_of_variables, int *number_of_clauses, int *maximum_length);

#endif // TRAIL_H