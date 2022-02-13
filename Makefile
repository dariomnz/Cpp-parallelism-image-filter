#!/bin/bash
CC = g++
FLAGS = -std=c++17 -Wall -Wextra -Wno-deprecated -Werror -pedantic -pedantic-errors
FILE = image-seq.cpp
OUTFILE = image-seq

OPTIMIZACION = -O3 -DNDEBUG
NO_OPTIMIZACION = -O0

PARALEFLAGS = -std=c++17 -Wall -Wextra -Wno-deprecated -Werror -pedantic -pedantic-errors -fopenmp

PARALEFILE = image-par.cpp
PARALEOUTFILE = image-par


100numbers = 1	2	3	4	5	6	7	8	9	10 11	12	13	14	15	16	17	18	19	20 21	22	23	24	25	26	27	28	29	30 31	32	33	34	35	36	37	38	39	40 41	42	43	44	45	46	47	48	49	50 51	52	53	54	55	56	57	58	59	60 61	62	63	64	65	66	67	68	69	70 71	72	73	74	75	76	77	78	79	80 81	82	83	84	85	86	87	88	89	90 91	92	93	94	95	96	97	98	99	100

default:
	@${CC} ${FLAGS} -o ${OUTFILE} ${FILE} 
	@${CC} ${PARALEFLAGS} -o ${PARALEOUTFILE} ${PARALEFILE} 

optimizacion:
	@${CC} ${FLAGS} ${OPTIMIZACION} -o ${OUTFILE} ${FILE} 
	@${CC} ${PARALEFLAGS} ${OPTIMIZACION} -o ${PARALEOUTFILE} ${PARALEFILE} 

no_optimizacion:
	@${CC} ${FLAGS} ${NO_OPTIMIZACION} -o ${OUTFILE} ${FILE} 
	@${CC} ${PARALEFLAGS} ${NO_OPTIMIZACION} -o ${PARALEOUTFILE} ${PARALEFILE} 

medias_seq:
	@${CC} ${FLAGS} ${OPTIMIZACION} -o ${OUTFILE} ${FILE} 
	@./${OUTFILE} copy indir outdir > output.txt
	@for i in ${100numbers} ; do ./${OUTFILE} copy indir outdir >> output.txt ; done
		
	@echo ---------------------------------------Copy seq optimizado: ---------------------------------------
	@python3 medias.py output.txt

	@./${OUTFILE} gauss indir outdir > output.txt
	@for i in ${100numbers} ; do ./${OUTFILE} gauss indir outdir >> output.txt ; done
		
	@echo ---------------------------------------gauss seq optimizado: ---------------------------------------
	@python3 medias.py output.txt

	@./${OUTFILE} sobel indir outdir > output.txt
	@for i in ${100numbers} ; do ./${OUTFILE} sobel indir outdir >> output.txt ; done
		
	@echo ---------------------------------------sobel seq optimizado: ---------------------------------------
	@python3 medias.py output.txt


	@${CC} ${FLAGS} ${NO_OPTIMIZACION} -o ${OUTFILE} ${FILE} 
	@./${OUTFILE} copy indir outdir > output.txt
	@for i in ${100numbers} ; do ./${OUTFILE} copy indir outdir >> output.txt ; done
		
	@echo ---------------------------------------Copy seq no optimizado: ---------------------------------------
	@python3 medias.py output.txt

	@./${OUTFILE} gauss indir outdir > output.txt
	@for i in ${100numbers} ; do ./${OUTFILE} gauss indir outdir >> output.txt ; done
		
	@echo ---------------------------------------gauss seq no optimizado: ---------------------------------------
	@python3 medias.py output.txt

	@./${OUTFILE} sobel indir outdir > output.txt
	@for i in ${100numbers} ; do ./${OUTFILE} sobel indir outdir >> output.txt ; done
		
	@echo ---------------------------------------sobel seq no optimizado: ---------------------------------------
	@python3 medias.py output.txt

medias_par:
	@${CC} ${PARALEFLAGS} ${OPTIMIZACION} -o ${PARALEOUTFILE} ${PARALEFILE} 
	@./${PARALEOUTFILE} copy indir outdir > output.txt
	@for i in ${100numbers} ; do ./${PARALEOUTFILE} copy indir outdir >> output.txt ; done

	@echo ---------------------------------------copy par optimizado: ---------------------------------------
	@python3 medias.py output.txt

	@./${PARALEOUTFILE} gauss indir outdir > output.txt
	@for i in ${100numbers} ; do ./${PARALEOUTFILE} gauss indir outdir >> output.txt ; done

	@echo ---------------------------------------gauss par optimizado: ---------------------------------------
	@python3 medias.py output.txt

	@./${PARALEOUTFILE} sobel indir outdir > output.txt
	@for i in ${100numbers} ; do ./${PARALEOUTFILE} sobel indir outdir >> output.txt ; done

	@echo ---------------------------------------sobel par optimizado: ---------------------------------------
	@python3 medias.py output.txt