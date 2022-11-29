# getUSC - Course Registration Sys for USC

## Introduction

A course registration system for USC students to view their future course of study by using UNIX socket.


## Important Notice for FUTURE STUDENTS OF EE450 IN USC
YOU ARE NOT ALLOWED to take this project as a reference or look at any files under this directory.\
Please be aware that this project is recorded in Turnitin, and taking any code to your submission, will result in academic misconduct.\
The author of this project will NOT be responsible for any consequence.

## Developer/Author Name

Nicholas Shen

## Files Structure

`client.c:` For user login by entering correct username and password withing 3 attempts and then query any category of a specific course or ask for mutiple course infomation with serverM via TCP.

`serverM.c:` Receive user credentials via TCP and then encrypt those info to verify user's identity from serverC; Receive user query requests and then reply with results retrieved from correct department server(s) via UDP.

`serverC.c:` Receive user credentials from serverM and send feedback to serverM via UDP.

`serverEE.c:` Receive user query requests about EE department from serverM and send query results to serverM via UDP.

`serverCS.c:` Receive user query requests about CS department from serverM and send query results to serverM via UDP.

`header.h:` Header file: import libaries, define constants and data structures.

`Makefile:` Compile or clean executable files: client serverM serverC serverEE serverCS.

`README.md:` This file.

`Others:` To make servers run correctly, cred.txt, cs.txt, and ee.txt are needed.

## Formats of Messages Exchanged

| Message                  | Format                                                                                                                                                               | Who Use?                                     |
| ------------------------ | -------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------- |
| ServerM                  | struct ServerM{ int sd_tcp; int sd_udp; int connected_sd_tcp; struct sockaddr_in addr_ServerC; struct sockaddr_in addr_ServerEE; struct sockaddr_in addr_ServerCS; } | client <-> serverM <-> serverEE and serverCS |
| User_auth                | struct User_auth{ char userName[BUFFSIZE]; char userPsw[BUFFSIZE]; }                                                                                                 | client <-> serverM <-> serverC               |
| User_query               | struct User_auth{ char course[BUFFSIZECOURSE]; char category[BUFFSIZECOURSE]; }                                                                                      | client <-> serverM <-> serverEE and serverCS |
| authFeedback/fbcode/code | char authFeedback[FEEDBACKSIZE] ("101": Username does not exist; "102": Password does not match; "103": Authentication is successful)                                | client <-> serverM <-> serverC               |
| Muti query               | struct User_auth{ char course[BUFFSIZECOURSE]; char category == "!muti!"; }                                                                                          | client <-> serverM <-> serverEE and serverCS |
| Muti query lists         | char courselist[MUTIQUERYSIZE][courseinfosize]; }                                                                                                                    | serverM <- serverEE and serverCS             |
| Muti query results       | char courseinfos[MUTIQUERYSIZE][courseinfosize]; }                                                                                                                   | client <-> serverM <-> serverEE and serverCS |

## Idiosyncrasy of Program (f)

- Terminate any \[Server\*\] with continuing using \[client\] or other \[Server\*\]
- Though this program supports serverM reconnecting after offline(e.x. force quit), an error "Address family not supported by protocol family" may occured due to unsync between other normal servers(e.x. serverC) after serverM back to work. The serverM will 'rebind' with serverC and resend previous unsuccessful request if such issue(s) occured, and thus reduce such error occurence rate below 0.001%.(Only supports serverM credential request sending right now) If such error occured and not resolved by server itself, please restart the server. See first one of Idiosyncrasy of Program.
- Invalid Inputs, include but not limited to: 1) entering no inputs(Null); 2) entering more characters more than its limits. See [Usage] or header file.
- Designed port(s) is(are) not available. See [Usage] or header file.

## Reused Code (g)

N/A

## Usage

### Log In

[USER NAME] (5-50 characters) `enter`\
[USER PASSWORD] (5-50 characters)`enter`

### Query Course Info

[COURSECODE] (1-128 characters) `enter`\
[CATEGORY] (1-128 characters) `enter`

### Muti query

Please enter the course code to query: (2-10 course codes, followed by a single blank)\
CS100 CS310 CS561 CS356 EE450 CS435 EE658 EE608 EE604 EE520

### Database

See cs.txt for example, see serverM.c for how to encrypt passwords.

## Copytights Notice

If it is necessary or inadvertent to bring this project to the public (e.x. personal project demo), the author adhere followings:

1. WILL NOT SHARE with future students who take EE450 at USC.
2. WILL NOT ALLOW future students who take EE450 at USC take this project as reference (by noticing in README).

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
