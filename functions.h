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

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

int SAT2(int number_of_variables, int number_of_clauses, Clause *clauses);
int DPLL(int number_of_variables, int number_of_clauses, Clause *clauses);
int DPLL_neu(int number_of_variables, int number_of_clauses, Clause *clauses);
int GWSAT(int number_of_variables, int number_of_clauses, Clause *clauses, int max_tries, int max_steps, double propability);
int sign(int x);
Clause *parse(char *file_str, int *number_of_variables, int *number_of_clauses, int *maximum_length);

#endif
