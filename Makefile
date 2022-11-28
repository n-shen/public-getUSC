# Author: Niholas Shen https://shen.dev
# University of Southern California. EE450 Nov.28.2022

all: client serverM serverC serverEE serverCS

client:
	gcc -o client client.c

serverM:
	gcc -o serverM serverM.c

serverC:
	gcc -o serverC serverC.c

serverEE:
	gcc -o serverEE serverEE.c

serverCS:
	gcc -o serverCS serverCS.c

clean:
	rm -rf client serverM serverC serverEE serverCS
	