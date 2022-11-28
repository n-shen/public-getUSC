# getUSC - Course Registration Sys for USC

## Introduction

A course registration system for USC students to view their future course of study by using UNIX socket.

## Developer/Author Name (a)

Nan Shen

## Student ID (b)

1905334448

## Completed Assignments' List (c)

- Phase 0
- Phase 1
- Phase 2
- Phase 3
- Phase 4
- Extra Credits

## Files Structure (d)

- client.c: For user login by entering correct username and password withing 3 attempts and then query any category of a specific course or ask for mutiple course infomation with serverM via TCP.
- serverM.c: Receive user credentials via TCP and then encrypt those info to verify user's identity from serverC; Receive user query requests and then reply with results retrieved from correct department server(s) via UDP.
- serverC.c: Receive user credentials from serverM and send feedback to serverM via UDP.
- serverEE.c: Receive user query requests about EE department from serverM and send query results to serverM via UDP.
- serverCS.c: Receive user query requests about CS department from serverM and send query results to serverM via UDP.
- header.h: Header file: import libaries, define constants and data structures.
- Makefile: Compile or clean executable files: client serverM serverC serverEE serverCS.
- README.md: This file.

To make servers run correctly, cred.txt, cs.txt, and ee.txt are needed.

## Formats of Messages Exchanged (e)

## Idiosyncrasy of Program (f)

- Terminate any \[Server\*\] with continuing using \[client\] or other \[Server\*\]
- Invalid Inputs, include but not limited to: 1) entering no inputs(Null); 2) entering more characters more than its limits. See [Usage] or header file.
- Designed port(s) is(are) not available. See [Usage] or header file.

## Reused Code (g)

N/A

## Usage

### muti query

Please enter the course code to query: CS100 CS310 CS561 CS356 EE450 CS435 EE658 EE608 EE604 EE520
