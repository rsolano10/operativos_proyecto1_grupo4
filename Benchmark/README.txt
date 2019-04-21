1. For execute the benchmark, run the commands as follows:

~$ gcc -pthread -o bclient benchmarkTool.c -lcurl -fopenmp

2. If you want run the benchmar as command line tool:

~$ export PATH=<path-to-bclient-file>:$PATH

<path-to-bclient-file> => path to compiled benchmarkTool.c file (bclient) 

3. Run the tool

if you run 2.
~$ bclient <machine> <port> <file> <N-threads> <N-cycles> 

else
~$ ./bclient <machine> <port> <file> <N-threads> <N-cycles> 

You will see the stats.csv file in the same path of bclient file.


