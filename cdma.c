#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CHIP_LENG 8 // chip length 8bit
#define BIT_LENG 5  // connection info 4bit, 1bit is extra
#define FRAME_LENGTH 33

// A = { } 
// B = { } 
// C = { }
// D = { }
// S1 = ABC'D
// S2 = A CD'
// S3 = A'B'C
// S4 = ABCD
// 프로세스 당 보내지는 순서
// A = 1101 
// B = 1 01
// C = 1111
// D = 00 1

//ex) A A A' A 로 보낼시 시퀀스는 11111111      11111111 -1-1-1-1-1-1-1-1   11111111
//ex) B 0 B` B 로 보낼시 시퀀스는 1111-1-1-1-1 -          1-1-1-11111       1111-1-1-1-1
//join 을 하게 되면 22220000 11111111 -2-2-2-20000 22220000
//join 시퀀스를 A 와 내적하고 크기(8) 로 나누면 1 1 1 1

void transSequence(seqA, bipolarA, chipA){

}

void joinSequence(seqJoin, seqA, seqB, seqC, seqD)
{}
void projection(seqJoin, chipA)
{}

void printChip(int chip);

int main(){
    //1111 1111
    int chipA[CHIP_LENG] = {1,1,1,1,1,1,1,1};
    //1111 0000
    int chipB[CHIP_LENG] = {1,1,1,1,-1,-1,-1};
    //0000 0000
    int chipC[CHIP_LENG] = {-1,-1,-1,-1,-1,-1,-1};
    //1010 1010
    int chipD[CHIP_LENG] = {1,-1,1,-1,1,-1,1,-1};

    //sender, receiver pid
    pid_t process_sender, process_receiver;

    int pipe_sender[2];
    int pipe_receiver[2];
    int pipe_joiner_with_sender[2];
    int pipe_joiner_with_receiver[2];

    int buffer[FRAME_LENGTH] = '\0';

    if(-1 == pipe(pipe_sender))
        exit(1);
    if(-1 == pipe(pipe_joiner_with_sender))
        exit(1);
    if(-1 == pipe(pipe_joiner_with_receiver))
        exit(1);
    if(-1 == pipe(pipe_receiver))
        exit(1);

    process_sender = fork();

    if(process_sender > 0){
        char bipolarA[BIT_LENG] = "1101";
        char bipolarB[BIT_LENG] = "1111";
        char bipolarC[BIT_LENG] = "0101";
        char bipolarD[BIT_LENG] = " 1 1";

        char seqA[FRAME_LENGTH];
        char seqB[FRAME_LENGTH];
        char seqC[FRAME_LENGTH];
        char seqD[FRAME_LENGTH];

        printf("각 프로세스의 chip Data\n");
        printChip('A', chipA);
        printChip('B', chipB);
        printChip('C', chipC);
        printChip('D', chipD);

        transSequence(seqA, bipolarA, chipA);
        printf("A >> %s", seqA);
        transSequence(seqB, bipolarB, chipB);
        printf("B >> %s", seqB);
        transSequence(seqC, bipolarC, chipC);
        printf("C >> %s", seqC);
        transSequence(seqD, bipolarD, chipD);
        printf("D >> %s", seqD);

        //use pipe to send
        sendPipe(buffer, pipe_sender[1], seqA);
        sendPipe(buffer, pipe_sender[1], seqB);
        sendPipe(buffer, pipe_sender[1], seqC);
        sendPipe(buffer, pipe_sender[1], seqD);
    }
    else{
        process_receiver = fork();

        if(process_receiver > 0){
            printf("\n\n//////////JOIN//////////\n\n");

            char seqA[FRAME_LENGTH];
            char seqB[FRAME_LENGTH];
            char seqC[FRAME_LENGTH];
            char seqD[FRAME_LENGTH];
            char seqJoin[FRAME_LENGTH];

            receivePipe(buffer, pipe_sender[2], seqA);
            receivePipe(buffer, pipe_sender[2], seqB);
            receivePipe(buffer, pipe_sender[2], seqC);
            receivePipe(buffer, pipe_sender[2], seqD);

            joinSequence(seqJoin, seqA, seqB, seqC, seqD);

            printf("\n%s\n", seqJoin);
            sendPipe(buffer, pipe_joiner_with_receiver[1], seqJoin);
            printf("\n\n//////////JOIN//////////\n\n");
        }
        else{
            char confirmA[BIT_LENG];
            char confirmB[BIT_LENG];
            char confirmC[BIT_LENG];
            char confirmD[BIT_LENG];

            char seqJoin[FRAME_LENGTH];
            receivePipe(buffer, pipe_joiner_with_receiver[0], seqJoin);
            printf("\n\n//////////RECIEVE//////////\n\n");
            projection(seqJoin, confirmA, chipA);
            projection(seqJoin, confirmB, chipB);
            projection(seqJoin, confirmC, chipC);
            projection(seqJoin, confirmD, chipD);

            printBipolar('A', confirmA);
            printBipolar('B', confirmB);
            printBipolar('C', confirmC);
            printBipolar('D', confirmD);
            printf("\n\n//////////RECIEVE//////////\n\n");
        }
    }

    return 0;
}

void printChip(char name, int process){
    printf("%c >> ", name);
    for(int i = 0;i<CHIP_LENG;i++){
        printf("%d ", process[i]);
    }
    printf("\n");
}

void printBipolar(char name, char bipolar){
    printf("%c >> %s\n", name, bipolar);
}

void sendPipe(int buffer, int pipe, int seq){
    sprintf(buffer, "%s", seq);
    write(pipe, buffer, strlen(buffer));
    memset(buffer, 0, FRAME_LENGTH -1);
}

void receivePipe(int buffer, int pipe, int seq){
    read(pipe, buffer, FRAME_LENGTH -1);
    buffer[FRAME_LENGTH] = '\0';
    strcpy(seq, buffer);
    memset(buffer, 0, FRAME_LENGTH);
}

void projection(char seqJoin[FRAME_LENGTH], char bipolar[BIT_LENG], int chip[CHIP_LENG]){
    for( int i = 0;i<BIT_LENG;i++){
        int sum = 0;
        for (int j = 0;j<CHIP_LENG;j++){
            sum += CtoI(seqJoin[i*CHIP_LENG + j]) * chip[j];
        }
        bipolar[i] = checkRecieve(sum);
    }
    bipolar[4] = '\0';
}

int CtoI (char num){
    switch(num){
        case '0':
            return 0;
        case '1':
            return 1;
        case '2':
            return 2;
        case '3':
            return 3;
        case '4':
            return 4;
        case 'A':
            return -1;
        case 'B':
            return -2;
        case 'C':
            return -3;
        case 'D':
            return -4;
    }
}

char ItoC (int num){
    switch(num){
        case 0:
            return '0';
        case 1:
            return '1';
        case 2:
            return '2';
        case 3:
            return '3';
        case 4:
            return '4';
        case -1:
            return 'A';
        case -2:
            return 'B';
        case -3:
            return 'C';
        case -4:
            return 'D';
    }
}

char checkRecieve(int bit){
    bit = bit/8;
    if(bit == 1)
        return '1';
    else if(bit == 0)
        return '0';
    else if(bit == -1)
        return 'A';
}

int chipToBipolar(char bit){
    
    if(bit == '1')
        return 1;
    else if(bit == '0')
        return 0;
    else if(bit == 'A')
        return -1;
}

void transSequence(char seq[FRAME_LENGTH], char bipolar[BIT_LENG], int chip[BIT_LENG])
{
    for(int i = 0;i<BIT_LENG;i++)
    {
        int chipNum = chipToBipolar(chip[i]);
        for(int j = 0;j<CHIP_LENG;j++)
        {
            seq[i * CHIP_LENG + j] = ItoC(chip[i] * chipNum);
        }
    }
    seq[32] = '\0';
}