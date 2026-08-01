#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static void emit_source(FILE *f, int v, int nodes)
{
    fprintf(f, "%d %d 0\n", v, v + nodes);
}

static void emit_rule(FILE *f,
                      int left,
                      int right,
                      int v, int nodes)
{
    fprintf(f,
            "-%d -%d %d 0\n",
            left,
            right,
            v);
    fprintf(f,
            "-%d -%d %d 0\n",
            left + nodes,
            right,
            v);
    fprintf(f,
            "-%d -%d %d 0\n",
            left,
            right + nodes,
            v);
    fprintf(f,
            "-%d -%d %d 0\n",
            left + nodes,
            right + nodes,
            v);
}

static void emit_sink(FILE *f, int sink, int nodes)
{
    fprintf(f, "-%d 0\n", sink);
    fprintf(f, "-%d 0\n", sink + nodes);
}

int main(int argc, char **argv)
{

    int max_height = atoi(argv[1]);

    for (int height = 1; height <= max_height; height++)
    {

        int nodes = (height) * (height + 1) / 2;
        int internal = nodes - height;
        int leaves = height;

        char file_str[1024];
        sprintf(file_str, "cnf_files/pebbling%d.cnf", height);
        FILE *f = fopen(file_str, "w");

        if (!f)
        {
            perror("fopen");
            return 1;
        }

        int clauses =
            leaves +       /* Quellen */
            4 * internal + /* Regeln */
            2;             /* Senke */

        fprintf(f,
                "p cnf %d %d\n",
                2 * nodes,
                clauses);

        /* Blätter */
        for (int v = internal + 1; v <= nodes; ++v)
        {
            emit_source(f, v, nodes);
        }

        /* Innere Knoten */
        for (int v = internal; v >= 1; --v)
        {
            int tri = ceil((-1 + sqrt(1 + 8 * v)) / 2);
            int left = v + tri;
            int right = v + tri + 1;

            emit_rule(f,
                      left,
                      right,
                      v, nodes);
        }

        /* Wurzel negieren */
        emit_sink(f, 1, nodes);

        fclose(f);
    }

    return 0;
}