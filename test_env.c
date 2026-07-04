#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "functions.h"

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

    FILE *f_gwsat_start = fopen("results_gwsat.csv", "w");
    fprintf(f_gwsat_start, "pigeons,time\n");
    fclose(f_gwsat_start);

    FILE *f_dpll = fopen("results_dpll.csv", "a");
    FILE *f_gwsat = fopen("results_gwsat.csv", "a");

    for (int iterator = 1; iterator <= 9; iterator++)
    {
        char file_str[1024];
        sprintf(file_str, "cnf_files/php%d.cnf", iterator);

        int number_of_variables;
        int number_of_clauses;
        int maximum_length;

        Clause *clauses = parse(file_str, &number_of_variables, &number_of_clauses, &maximum_length);

        struct timespec startDPLL, endDPLL;

        clock_gettime(CLOCK_MONOTONIC, &startDPLL);

        int is_satDPLL = DPLL_neu(number_of_variables, number_of_clauses, clauses);
        printf("DPLL %d, %d\n", iterator, is_satDPLL);

        clock_gettime(CLOCK_MONOTONIC, &endDPLL);

        double elapsed_DPLL = (endDPLL.tv_sec - startDPLL.tv_sec) + (endDPLL.tv_nsec - startDPLL.tv_nsec) / 1e9;

        printf("DPLL %d: (%.6f Sekunden)\n", iterator, elapsed_DPLL);

        fprintf(f_dpll, "%d,%.6f\n", iterator, elapsed_DPLL);

        struct timespec startGWSAT, endGWSAT;

        clock_gettime(CLOCK_MONOTONIC, &startGWSAT);

        int is_satGWSAT = GWSAT(number_of_variables, number_of_clauses, clauses, 500, 3 * number_of_variables, 0.6);
        printf("GWSAT %d, %d\n", iterator, is_satGWSAT);

        clock_gettime(CLOCK_MONOTONIC, &endGWSAT);

        double elapsed_GWSAT = (endGWSAT.tv_sec - startGWSAT.tv_sec) + (endGWSAT.tv_nsec - startGWSAT.tv_nsec) / 1e9;

        printf("GWSAT %d: (%.6f Sekunden)\n", iterator, elapsed_GWSAT);

        fprintf(f_gwsat, "%d,%.6f\n", iterator, elapsed_GWSAT);

        /*
            Speicher freigeben
        */
        for (int i = 0; i < number_of_clauses; i++)
        {
            free(clauses[i].literals);
        }

        free(clauses);
    }

    fclose(f_dpll);
    fclose(f_gwsat);

    return 0;
}