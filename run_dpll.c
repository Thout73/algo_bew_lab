#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "functions.h"

struct CDCL_Clause
{
    int *literals;
    int size;
    int sat;
    int true_count;
};

int sign(int x)
{
    if (x < 0)
    {
        return -1;
    }
    return 1;
}

int main(int argc, char **argv)
{
    FILE *f_dpll_start = fopen("results_dpll.csv", "w");
    fprintf(f_dpll_start, "pigeons,time\n");
    fclose(f_dpll_start);

    // for (int iterator = 1; iterator <= 15; iterator++)
    // {
    char file_str[1024];
    sprintf(file_str, "cnf_files/php%d.cnf", 7);

    int number_of_variables;
    int number_of_clauses;
    int maximum_length;

    Clause *clauses = parse(file_str, &number_of_variables, &number_of_clauses, &maximum_length);
    struct timespec startDPLL, endDPLL;

    clock_gettime(CLOCK_MONOTONIC, &startDPLL);

    int is_satDPLL = DPLL(number_of_variables, number_of_clauses, clauses);
    printf("Ergebnis: %d\n", is_satDPLL);

    clock_gettime(CLOCK_MONOTONIC, &endDPLL);

    double elapsed_DPLL = (endDPLL.tv_sec - startDPLL.tv_sec) + (endDPLL.tv_nsec - startDPLL.tv_nsec) / 1e9;

    printf("DPLL (%.6f Sekunden)\n", elapsed_DPLL);
    FILE *f_dpll = fopen("results_dpll.csv", "a");

    fprintf(f_dpll, "%d,%.6f\n", 1, elapsed_DPLL);

    fclose(f_dpll);

    for (int i = 0; i < number_of_clauses; i++)
    {
        free(clauses[i].literals);
    }

    free(clauses);
    //}
}