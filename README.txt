These are compilation lines for the implemented files, (I recommend starting with main as its the easiest to understand the libraries functions):

============main.cpp:==================

g++ para-pat/Farm.cpp para-pat/Pipe.cpp para-pat/Pipeline.cpp main.cpp -o main -lpthread 

==========convolution_parapat.cpp (Convolution implementation):============

g++ para-pat/Farm.cpp para-pat/Pipe.cpp para-pat/Pipeline.cpp convolution_parapat.cpp -o conv_parapat -lpthread -lpng

================fibonacci_parapat (Para-Pat implementation):==========

g++ para-pat/Farm.cpp para-pat/Pipe.cpp para-pat/Pipeline.cpp fibonacci_parapat.cpp -o fibonacci_parapat -lpthread

===============fibonacci_ff (FastFlow implementation):====================

g++ -I/home/chaysse/fastflow -o fibonacci_ff fibonacci_ff.cpp -lpthread





