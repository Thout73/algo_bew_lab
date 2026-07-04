test_env:
	gcc test_env.c 2SAT.c DPLL.c DPLL_neu.c GWSAT.c parse_cnf.c -o test.exe -O3
	./test.exe
	python plot_graph.py

test_env_2SAT:
	gcc test_env_2SAT.c 2SAT.c DPLL.c DPLL_neu.c GWSAT.c parse_cnf.c -o test.exe -O3
	./test.exe
	python plot_graph_2SAT.py


clean_exe:
	rm -f *.exe

clean_cnf:
	rm *.cnf

clean_csv:
	rm *.csv

clean:
	make clean_csv
	make clean_exe

plot: 
	python plot_graph.py

dpll:
	gcc run_dpll.c parse_cnf.c DPLL.c -o dpll_run.exe -O3
	./dpll_run.exe

dpll_neu:
	gcc run_dpll_neu.c parse_cnf.c DPLL_neu.c -o dpll_run_neu.exe -O3 -fsanitize=address
	./dpll_run_neu.exe
	python plot_dpll.py

cdcl:
	gcc CDCL.c parse_cdcl_cnf.c watch_lst.c trail_cdcl.c -fsanitize=address -o cdcl.exe -O3
	./cdcl.exe