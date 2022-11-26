# Author: Niholas Shen https://shen.dev
# University of Southern California. EE450 Nov.28.2022

all: client serverM serverC serverEE serverCS

client:
	gcc client.c -o client

serverM:
	gcc serverM.c -o serverM

serverC:
	gcc serverC.c -o serverC

serverEE:
	gcc serverEE.c -o serverEE

serverCS:
	gcc serverCS.c -o serverCS

clean:
	rm -rf client serverM serverC serverEE serverCS
	