#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define WORD_SIZE  4
#define MAX_DATA_NUM  100
#define MAX_TEXT_LABEL  100
#define MAX_INST_NUM  2000

/** data section var **/
int data_num=0;
int word_num=0;
char data_label[MAX_DATA_NUM][11]; // label of data
int data_label_index[MAX_DATA_NUM];
char data[MAX_DATA_NUM][33]; // pure list of given data
/** ================ **/

/** text section var **/
int text_label_num=0;
char text_label[MAX_TEXT_LABEL][11];
int text_label_index[MAX_TEXT_LABEL];

int inst_num=0;
char text_inst[MAX_INST_NUM][6];
char text_body[MAX_INST_NUM][3][11];
/** ================ **/

const char *R_INST_LIST[9] =
{
    "addu",     //0
    "and",      //1
    "jr",       //2
    "nor",      //3
    "or",       //4
    "sltu",     //5
    "sll",      //6
    "srl",      //7
    "subu"      //8
};

const char *I_INST_LIST[10] =
{
    "addiu",    //10
    "andi",     //11
    "beq",      //12
    "bne",      //13
    "lui",      //14
    "lw",       //15
    "ori",      //16
    "la",       //17
    "sltiu",    //18
    "sw"        //19
};

const char *J_INST_LIST[2] =
{
    "j",        //20
    "jal"       //21
};

int str2int(char *str)
{
    int len,val=0;
    len = strlen(str);

    if(str[0]=='0' && str[1]=='x')
    //hex case
    {
        for(int i=2;i<len;i++)
        {
            if(str[i]>=97 && str[i]<=102)
            {
                val+=( (str[i]-87) <<(4*(len-1-i)) );
            }
            else if(str[i]>=65 && str[i]<=70)
            {
                val+=( (str[i]-55) <<(4*(len-1-i)) );
            }
            else
            {
                val+=( (str[i]-'0') <<(4*(len-1-i)) );
            }
        }
    }
    else
    //dec case
    {
        val=atoi(str);
    }
    return val;
}

int printBin(int dec)
//gets decimal int and prints binary
{
    unsigned int mask=0x80000000;
    for(int i=0;i<32;i++)
    {
        printf("%d",!(!(dec&mask)) );
        mask = mask>>1;
    }
    //printf("\n");
    return 0;
}

int getInstType(char *inst)
{
    for(int i=0;i<9;i++)
    {
        if(!strcmp(inst,R_INST_LIST[i]))
        {
            return i;
        }
    }
    for(int i=0;i<10;i++)
    {
        if(!strcmp(inst,I_INST_LIST[i]))
        {
            return 10+i;
        }
    }
    for(int i=0;i<2;i++)
    {
        if(!strcmp(inst,J_INST_LIST[i]))
        {
            return 20+i;
        }
    }
    return -1;
}

int lookUpLabel(char label[][11], int *label_index, int table_size, char *target_label)
{
    for(int i=0;i<table_size;i++)
    {
        if(!strcmp(label[i],target_label))
            return label_index[i];
    }
    return -1;
}

int printInst(int index)
{
    int inst_type,k;
    inst_type = getInstType(text_inst[index]);
    unsigned int rs=0,rt=0,rd=0,shamt=0,im=0,addr=0;
    unsigned long long op=0;//printf(" ==> %d %d\n",index,inst_type);///
    switch(inst_type)
    {
    case 0://addu
        rs=atoi(text_body[index][1]+1);
        rt=atoi(text_body[index][2]+1);
        rd=atoi(text_body[index][0]+1);
        op=(rs<<21) + (rt<<16) + (rd<<11) + (shamt<<6) + 0x21;
        break;
    case 1://and
        rs=atoi(text_body[index][1]+1);
        rt=atoi(text_body[index][2]+1);
        rd=atoi(text_body[index][0]+1);
        op=(rs<<21) + (rt<<16) + (rd<<11) + (shamt<<6) + 0x24;
        break;
    case 2://jr
        rs=atoi(text_body[index][0]+1);
        op=(rs<<21) + (rt<<16) + (rd<<11) + (shamt<<6) + 0x08;
        break;
    case 3://nor
        rs=atoi(text_body[index][1]+1);
        rt=atoi(text_body[index][2]+1);
        rd=atoi(text_body[index][0]+1);
        op=(rs<<21) + (rt<<16) + (rd<<11) + (shamt<<6) + 0x27;
        break;
    case 4://or
        rs=atoi(text_body[index][1]+1);
        rt=atoi(text_body[index][2]+1);
        rd=atoi(text_body[index][0]+1);
        op=(rs<<21) + (rt<<16) + (rd<<11) + (shamt<<6) + 0x25;
        break;
    case 5://sltu
        rs=atoi(text_body[index][1]+1);
        rt=atoi(text_body[index][2]+1);
        rd=atoi(text_body[index][0]+1);
        op=(rs<<21) + (rt<<16) + (rd<<11) + (shamt<<6) + 0x2b;
        break;
    case 6://sll
        rd=atoi(text_body[index][0]+1);
        rt=atoi(text_body[index][1]+1);
        shamt=atoi(text_body[index][2]);
        op=(rs<<21) + (rt<<16) + (rd<<11) + (shamt<<6) + 0x00;
        break;
    case 7://srl
        rd=atoi(text_body[index][0]+1);
        rt=atoi(text_body[index][1]+1);
        shamt=atoi(text_body[index][2]);
        op=(rs<<21) + (rt<<16) + (rd<<11) + (shamt<<6) + 0x02;
        break;
    case 8://subu
        rs=atoi(text_body[index][1]+1);
        rt=atoi(text_body[index][2]+1);
        rd=atoi(text_body[index][0]+1);
        op=(rs<<21) + (rt<<16) + (rd<<11) + (shamt<<6) + 0x23;
        break;
    case 10://addiu
        rs=atoi(text_body[index][1]+1);
        rt=atoi(text_body[index][0]+1);
        im=str2int(text_body[index][2]);
        op=(rs<<21) + (rt<<16) + im + (0x09<<26);
        break;
    case 11://andi
        rs=atoi(text_body[index][1]+1);
        rt=atoi(text_body[index][0]+1);
        im=str2int(text_body[index][2]);
        op=(rs<<21) + (rt<<16) + im + (0x0c<<26);
        break;
    case 12://beq
        rs=atoi(text_body[index][0]+1);
        rt=atoi(text_body[index][1]+1);
        im=(lookUpLabel(text_label,text_label_index,text_label_num,text_body[index][2])-1 - index) & 0x0000ffff;
        op=(rs<<21) + (rt<<16) + im + (0x04<<26);
        break;
    case 13://bne
        rs=atoi(text_body[index][0]+1);
        rt=atoi(text_body[index][1]+1);
        im=(lookUpLabel(text_label,text_label_index,text_label_num,text_body[index][2])-1 - index) & 0x0000ffff;
        op=(rs<<21) + (rt<<16) + im + (0x05<<26);
        break;
    case 14://lui
        rt=atoi(text_body[index][0]+1);
        im=str2int(text_body[index][1]);
        op=(rs<<21) + (rt<<16) + im + (0x0f<<26);
        break;
    case 15://lw
        k=strlen(text_body[index][1]);
        for(int i=0;i<k;i++)
        {
            if(text_body[index][1][i]=='(')
            {
                rs=atoi(text_body[index][1]+2+i);
                break;
            }
        }
        rt=atoi(text_body[index][0]+1);
        im=atoi(text_body[index][1]) & 0x0000ffff;
        op=(rs<<21) + (rt<<16) + im + (0x23<<26);
        break;
    case 16://ori
        rs=atoi(text_body[index][1]+1);
        rt=atoi(text_body[index][0]+1);
        im=str2int(text_body[index][2]);
        op=(rs<<21) + (rt<<16) + im + (0x0d<<26);
        break;
    case 17://la
        //already changed to lui and ori.
        break;
    case 18://sltiu
        rs=atoi(text_body[index][1]+1);
        rt=atoi(text_body[index][0]+1);
        im=str2int(text_body[index][2]);
        op=(rs<<21) + (rt<<16) + im + (0x0b<<26);
        break;
    case 19://sw
        k=strlen(text_body[index][1]);
        for(int i=0;i<k;i++)
        {
            if(text_body[index][1][i]=='(')
            {
                rs=atoi(text_body[index][1]+2+i);
                break;
            }
        }
        rt=atoi(text_body[index][0]+1);
        im=atoi(text_body[index][1]) & 0x0000ffff;
        op=(rs<<21) + (rt<<16) + im + (0x2b<<26);
        break;
    case 20://j
        addr=0x400000/WORD_SIZE + lookUpLabel(text_label,text_label_index,text_label_num,text_body[index][0]);
        op=addr + (0x02<<26);
        break;
    case 21://jal
        addr=0x400000/WORD_SIZE + lookUpLabel(text_label,text_label_index,text_label_num,text_body[index][0]);
        op=addr + (0x03<<26);
        break;
    }
    printBin(op);
    return 0;
}


int main(int argc, char* argv[]){

	if(argc != 2){
		printf("Usage: ./runfile <assembly file>\n"); //Example) ./runfile /sample_input/example1.s
		printf("Example) ./runfile ./sample_input/example1.s\n");
		exit(0);
	}
	else
	{

		// To help you handle the file IO, the deafult code is provided.
		// If we use freopen, we don't need to use fscanf, fprint,..etc.
		// You can just use scanf or printf function
		// ** You don't need to modify this part **
		// If you are not famailiar with freopen,  you can see the following reference
		// http://www.cplusplus.com/reference/cstdio/freopen/

		//For input file read (sample_input/example*.s)

		char *file=(char *)malloc(strlen(argv[1])+3);
		strncpy(file,argv[1],strlen(argv[1]));

		if(freopen(file, "r",stdin)==0){
			printf("File open Error!\n");
			exit(1);
		}
		//From now on, if you want to read string from input file, you can just use scanf function.



        char str[50]; // currently reading string
        char temp_str[50];
        int index;
        int len;

        /**data section starts**/
        while(scanf("%s",str) != EOF )
        {
            if(str[strlen(str)-1]==':')//label case
            {
                str[strlen(str)-1]='\0';
                strcpy(data_label[data_num],str);
                data_label_index[data_num]=word_num;
                data_num++;
            }
            else//.word case
            {
                if(!strcmp(str,".text"))
                    break;
                if(!strcmp(str,".data"))
                    continue;
                scanf("%s",data[word_num]);
                word_num++;
            }
        }
        /** data section ends **/

        /** text section starts **/
        while(scanf("%s",str) != EOF)
        {
            len = strlen(str);
            if(str[len-1] == ':')
            // label addition
            {
                str[len-1] ='\0';
                strcpy(text_label[text_label_num],str);
                text_label_index[text_label_num]=inst_num;
                text_label_num++;
            }
            else
            // save instructions, converts "la" while saving
            {
                if(!strcmp(str,"la"))
                {
                    scanf("%s",temp_str);
                    temp_str[strlen(temp_str)-1]='\0';
                    scanf("%s",str);

                    strcpy(text_inst[inst_num],"lui");
                    strcpy(text_body[inst_num][0],temp_str);
                    strcpy(text_body[inst_num][1],"0x1000");

                    index = lookUpLabel(data_label,data_label_index,data_num,str);
                    if(index != 0)
                    {
                        inst_num+=1;
                        strcpy(text_inst[inst_num],"ori");
                        strcpy(text_body[inst_num][0],temp_str);
                        strcpy(text_body[inst_num][1],temp_str);
                        sprintf(str,"0x%x",index*WORD_SIZE);
                        strcpy(text_body[inst_num][2],str);
                    }

                    inst_num++;
                }
                else if(!strcmp(str,"j") || !strcmp(str,"jal") || !strcmp(str,"jr") )
                // body length == 1
                {
                    strcpy(text_inst[inst_num],str);
                    scanf("%s",str);
                    strcpy(text_body[inst_num][0],str);
                    inst_num++;
                }
                else if(!strcmp(str,"lui") || !strcmp(str,"lw") || !strcmp(str,"la") || !strcmp(str,"sw"))
                //body length == 2
                {
                    strcpy(text_inst[inst_num],str);
                    scanf("%s",str);
                    str[strlen(str)-1]='\0';
                    strcpy(text_body[inst_num][0],str);
                    scanf("%s",str);
                    strcpy(text_body[inst_num][1],str);
                    inst_num++;
                }
                else
                //body length == 3
                {
                    strcpy(text_inst[inst_num],str);
                    scanf("%s",str);
                    str[strlen(str)-1]='\0';
                    strcpy(text_body[inst_num][0],str);
                    scanf("%s",str);
                    str[strlen(str)-1]='\0';
                    strcpy(text_body[inst_num][1],str);
                    scanf("%s",str);
                    strcpy(text_body[inst_num][2],str);
                    inst_num++;
                }
            }
        }

        /** text section ends **/

        /*for(int i=0;i<inst_num;i++)
        {
            printf("%s %s %s %s\n",text_inst[i],text_body[i][0], text_body[i][1], text_body[i][2]);
        }*/




		// For output file write
		// You can see your code's output in the sample_input/example#.o
		// So you can check what is the difference between your output and the answer directly if you see that file
		// make test command will compare your output with the answer
		file[strlen(file)-1] ='o';
		freopen(file,"w",stdout);

		//If you use printf from now on, the result will be written to the output file.
        /** print section sizes **/
        printBin(inst_num*WORD_SIZE);
        printBin(word_num*WORD_SIZE);

        /** print data section **/
        for(int i=0;i<inst_num;i++)
        {
            printInst(i);
        }

        /** print word section **/
        for(int i=0;i<word_num;i++)
        {
            printBin(str2int(data[i]));
        }

	}
	return 0;
}


/*const char *HEX2BIN[15] =
{
    "0000", "0001", "0010", "0011",
    "0100", "0101", "0110", "0111",
    "1000", "1001", "1010", "1011",
    "1100", "1101", "1110", "1111",
};*/
