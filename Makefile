# Author: Niholas Shen https://shen.dev
# University of Southern California. EE450 Nov.28.2022

all: client serverM

client:
	gcc client.c -o client

server:
	gcc serverM.c -o serverM

clean:
	rm -rf client serverM
	