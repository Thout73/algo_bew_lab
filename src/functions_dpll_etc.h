typedef struct
{
    int *literals;
    int size;
    int sat;
    int true_count;
    int number_of_unassigned;
    int id;
    int watched_lit1;
    int watched_lit2;
} Clause;

typedef struct
{
    int value;

    int *pos_clauses;
    int pos_size;

    int *neg_clauses;
    int neg_size;
} Variable_DPLL;

typedef struct
{
    int value;

    int *pos_clauses;
    int pos_size;

    int *neg_clauses;
    int neg_size;

    int make_score;
    int break_score;
} Variable_GWSAT;

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

int SAT2(int number_of_variables, int number_of_clauses, Clause *clauses);
int DPLL(int number_of_variables, int number_of_clauses, Clause *clauses, Variable_DPLL *assignment);
int GWSAT(int number_of_variables, int number_of_clauses, Clause *clauses, int max_tries, int max_steps, double propability, Variable_GWSAT *assignment);
int sign(int x);
Clause *parse(char *file_str, int *number_of_variables, int *number_of_clauses, int *maximum_length);

#endif
