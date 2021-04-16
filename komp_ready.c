#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include<stdbool.h>

#define MULTIPLY_FACTOR 3
#define EMPTY -1
#define OUTPUT 1
#define PUSH 2

typedef enum {
    NONE = -1,
    PUSH_0 = 0,
    PUSH_1 = 1,
    OUTPUT_0 = 2,
    OUTPUT_1 = 3,
    POP_BRANCH = 4,
    INPUT_BRANCH = 5,
    JUMP = 6,
    CALL = 7,
    RETURN = 8,
    HALT = 9

} code;

typedef struct
{
    code instruction_type;
    int address;
    int stack;

} instructionT;

typedef struct
{
    char letter;
    int address;

} functionT;

char *setup_functions(functionT function_array[27],int *instructions_count,int *functions_count);
bool ignore_char(char c)
{
    /*
    Funkcja pomocnicza pomijajaca komentarze i puste znaki
    */

    if(c != ' ' && c != '\n' && c != ';' && c!='\t')
        return false;
    return true;

}
char *setup_functions(functionT function_array[27],int *instructions_count,int *functions_count)
{

    /*
    Funkcja wczytuje kolejne znaki wpisujac je do tablicy dynamicznej i jednoczesnie
    wpisuje do tablicy struktur (function_array) litery funkcji i odpowiadajace im adresy
    */

    char *result=NULL;
    bool is_comment=false,option=false;
    int curr_function=0,curr_address=0,i=0,input_size=0,opened_brackets=0,previous;

    int c=getchar();
    while(c!=EOF)
    {
        assert(i<INT_MAX);
        if(c==';')
            is_comment=true;
        else if(c=='\n')
            is_comment=false;
        if(!ignore_char(c) && !is_comment)
        {
            if(i==input_size)
            {
                input_size=(input_size*MULTIPLY_FACTOR)+1;
                result=realloc(result,input_size * sizeof(*result));
                assert(result!=NULL);
            }
            result[i]=c;
            if(c=='$' || (c>='a' && c<='z'))
                option=true;

            if(c=='{' && previous>='A' && previous<='Z')
            {
                function_array[curr_function].letter=previous;
                function_array[curr_function].address=curr_address;
                curr_function++;
                option=false;
            }
            else if(c=='{' && opened_brackets==0)
            {
                curr_address++;
                function_array[curr_function].letter='.';
                function_array[curr_function].address=curr_address;
                curr_function++;
                option=false;
            }
            else if(((c=='+' || c=='-') && option)||(c=='{' && option) || (c>='A' && c<='Z'))
                curr_address++;

            if(c=='{')
                opened_brackets++;
            else if(c=='}')
                opened_brackets--;

            previous=c;
            i++;
        }

        c=getchar();
    }

    *instructions_count=curr_address;
    *functions_count=curr_function;

    return result;

}

void manage_input(functionT *functions,instructionT *instruction,int *curr_address,char *input,int *it,int functions_count)
{

    /*
    Glowna funkcja programu ustawiajaca kolejne instrukcje programu
    */

    int option=EMPTY,stack;

    while(input[*it]!='}')
    {

        if(input[*it] == '-' && option==OUTPUT)     //Pisze bit 0 na wyjscie
        {
            instruction[*curr_address].instruction_type=OUTPUT_0;
            (*curr_address)++;
        }
        else if(input[*it] == '+' && option==OUTPUT)    //Pisze bit 0 na wyjscie
        {
            instruction[*curr_address].instruction_type=OUTPUT_1;
            (*curr_address)++;
        }
        else if(input[*it] == '-' && option==PUSH)  //Wklada bit 0 na stos stack
        {
            instruction[*curr_address].instruction_type=PUSH_0;
            instruction[*curr_address].stack=stack-'a';
            (*curr_address)++;
        }
        else if(input[*it] == '+' && option==PUSH)  //Wklada bit 1 na stos stack
        {
            instruction[*curr_address].instruction_type=PUSH_1;
            instruction[*curr_address].stack=stack-'a';
            (*curr_address)++;
        }
        else if(input[*it] == '$')
        {
            option=OUTPUT;
        }
        else if(input[*it] >= 'a' && input[*it] <= 'z')
        {
            stack=input[*it];
            option=PUSH;
        }
        else if(input[*it]>='A' && input[*it]<='Z')     //Wywolanie funkcji o danej literze
        {
            instruction[*curr_address].instruction_type=CALL;
            for(int k=0;k<functions_count;k++)
                if(functions[k].letter==input[*it])
                    instruction[*curr_address].address=functions[k].address;

            (*curr_address)++;
        }
        /*
        Ponizsze instrukcje wyboru zapamietuja wskaznik do aktualnego adresu w pomocniczej zmiennej
        Nastepnie iteruje po 1 instrukcji wyboru i ponownie pamieta wskaznik do aktualnego adresu
        i iteruje po 2 drugiej instrukcji wyboru
        Instrukcja wyboru opiera sie na rekurencyjnym wywolywaniu glownej funkcji (manage_input)
        */
        else if(input[*it]=='{' && option==OUTPUT)  //Instrukcja wyboru na podstawie bitu wczytanego z wejscia
        {
            option=EMPTY;
            int temp=*curr_address;
            instruction[temp].instruction_type=INPUT_BRANCH;
            (*curr_address)++;
            manage_input(functions,instruction,curr_address,input,it,functions_count);  //iteruje po 1 instrukcji wyboru
            instruction[temp].address=(*curr_address)+1;

            temp=*curr_address;
            instruction[temp].instruction_type=JUMP;
            (*it)++;
            (*curr_address)++;
            manage_input(functions,instruction,curr_address,input,it,functions_count);  //iteruje po 2 instrukcji wyboru
            instruction[temp].address=*curr_address;
        }
        else if(input[*it]=='{' && option==PUSH)    //Instrukcja wyboru na podstawie bitu zdjetego ze stosu stack
        {
            option=EMPTY;
            int temp=*curr_address;
            instruction[temp].instruction_type=POP_BRANCH;
            instruction[temp].stack=stack-'a';
            (*curr_address)++;
            manage_input(functions,instruction,curr_address,input,it,functions_count);  //iteruje po 1 instrukcji wyboru
            instruction[temp].address=(*curr_address)+1;

            temp=*curr_address;
            instruction[temp].instruction_type=JUMP;
            (*it)++;
            (*curr_address)++;
            manage_input(functions,instruction,curr_address,input,it,functions_count);  //iteruje po 2 instrukcji wyboru
            instruction[temp].address=*curr_address;
        }

        (*it)++;
    }

}

void empty_functions(functionT function_array[27])
{
    /*
    Funkcja ustawia domyslne wartosci tablicy struktur zawierajacej adresy i nazwy funkcji
    */

    for(int i=0;i<27;i++)
    {
        function_array[i].address=-1;
        function_array[i].letter=0;
    }

}

void empty_instructions(instructionT *instructions,int instructions_count)
{
    /*
    Funkcja ustawia domyslne wartosci glownej tablicy struktur
    */

    for(int i=0;i<=instructions_count;i++)
    {
        instructions[i].address=-1;
        instructions[i].stack=-1;
        instructions[i].instruction_type=NONE;
    }

}

void print_instructions(instructionT *instructions,int instructions_count)
{
    /*
    Funkcja wypisuje kolejne instrukcje programu
    */

    for(int i=0;i<=instructions_count;i++)
    {
        printf("%d",instructions[i].instruction_type);
        if(instructions[i].address!=-1)
            printf(" %d",instructions[i].address);
        if(instructions[i].stack!=NONE)
            printf(" %d",instructions[i].stack);
        putchar('\n');
    }

}

void set_first_address(functionT function_array[27],instructionT *instructions,int functions_count)
{
    /*
    Funkcja znajduje adres funkcji glownej, po czym ustawia go jako pierwsza instrukcje
    */

    for(int i=0;i<functions_count;i++)
    {
        if(function_array[i].letter=='.')
            {
                instructions[0].address=function_array[i].address;
                instructions[0].instruction_type=JUMP;
            }
    }

}

int main()
{

    instructionT *instructions;
    functionT function_array[27];
    empty_functions(function_array);

    int instructions_count=0,curr_address=1,functions_count;

    char *input=setup_functions(function_array,&instructions_count,&functions_count);

    instructions=malloc((instructions_count+1)*sizeof(*instructions));
    empty_instructions(instructions,instructions_count);
    set_first_address(function_array,instructions,functions_count);

    for(int j=0,it=1;j<functions_count;j++,it+=2)
    {
        manage_input(function_array,instructions,&curr_address,input,&it,functions_count);
        instructions[curr_address].instruction_type=RETURN;
        curr_address++;
    }
    instructions[instructions_count].instruction_type=HALT;

    print_instructions(instructions,instructions_count);

    free(input);
    free(instructions);

    return 0;

}
