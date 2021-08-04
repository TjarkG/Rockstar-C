/*
 * RockstarC.h
 *
 * programm to convert c to rockstar code
 *
 * Created: 02.08.2021 15:42:57
 *  Author: TjarkG
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "Parse.h"

#define MAXL 32

/*const char *ignore[] = {
    "auto",
    "register",
    "const",
    "unsigned",
    "signed",
    "void",
    "volatile",
    "main(void)",
    "{"
};*/

const char *foos[] = {
    "double",
    "int",
    "long",
    "float",
    "short",
    "char",
};

void rockstarConvert(FILE *, FILE *);
void removeEnding(const char*, char*);
void toCamelCase(char *);
void convert_to_words(char*, char*);
int getWord(FILE *, char*, char);

int main(int argc, char *argv[])
{
    FILE *fpIn;
    FILE *fpParse;
    FILE *fpOut;
    char *prog = argv[0];     // program name for errors

    if (argc == 1 ) /* no args: throw error */
    {
        fprintf(stderr, "%s: no files to interpile\n", prog);
        return -1;
    }
    while (--argc > 0)
    {
        ++argv;
        if ((fpIn = fopen(*argv, "r")) == NULL) 
        {
            fprintf(stderr, "%s: can't open %s\n", prog, *argv);
            exit(1);
        }
        else
        {
            char parseName[strlen(*argv)+4];
            removeEnding(*argv, parseName);
            strcpy(parseName+strlen(parseName), ".parse");
            fpParse = fopen(parseName, "r+");
            parseC(fpIn, fpParse);
            //fclose(fpParse);
            fclose(fpIn);

            char outName[strlen(*argv)+4];
            removeEnding(*argv, outName);
            strcpy(outName+strlen(outName), ".rock");
            fpOut = fopen(outName, "w");
            fseek(fpParse, 0, SEEK_SET);
            rockstarConvert(fpParse, fpOut);
            fclose(fpParse);
            fclose(fpOut);
        }
    }
    exit(0);
}

void rockstarConvert(FILE *ifp, FILE *ofp)
{
    char in[MAXL];
    while (getWord(ifp, in, MAXL) != EOF)
    {
        if(strcmp(in, "#include") == 0)
        {
            getWord(ifp, in, MAXL);
            if(strcmp(in, "<"))
                fprintf(stderr, "conversion Error: #include not followed by <");
            getWord(ifp, in, MAXL);
            getWord(ifp, in, MAXL);
            if(strcmp(in, ">"))
                fprintf(stderr, "conversion Error: #include not followed by >");
            continue;
        }
        fprintf(ofp,"%s\n",in);
    }  
}

void removeEnding(const char* in, char* out)
{
    char length = (strlen(in)-1);
    strcpy(out, in);
    for (char i = length; i >= 0; i--)
        if(in[i] == '.')
        {
            out[i] = '\0';
            return;
        }
}

void toCamelCase(char *name)
{
    char digits [4] = {'\0','\0','\0','\0'};
    char length = strlen(name)-1;
    name[0] = toupper(name[0]);
    for(char i = 0; i<=length; i++)
    {
        if(name[i] == '_')
        {
            for(char j = i; j<length; j++)
            {
                name[j] = name[j+1];
            }
            name[length] = '\0';
            name[i] = toupper(name[i]);
        }
        if(isdigit(name[i]))
        {
            for(char j = 0; (j<3) && (j<length-i+1); j++)
            {
                if(isdigit(name[j+i]))
                    digits[j]= name[j+i];
            }
            char number [MAXL];
            convert_to_words(number , digits);
            strncpy(name + i, number, strlen(number)+1);
        }
    }
}

void convert_to_words(char* out, char* num)
{
    int len = strlen(num); //Number of digits

    const char* single_digits[] = {"Zero", "One", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine"};
    const char* two_digits[]    = {"Ten", "Eleven", "Twelve", "Thirteen", "Fourteen", "Fifteen", "Sixteen", "Seventeen", "Eighteen", "Nineteen"};
    const char* tens_multiple[] = {"","Twenty", "Thirty", "Forty", "Fifty", "Sixty", "Seventy", "Eighty", "Ninety"};
    const char* tens_power[]    = {"hundred", "thousand"};
 
    // Single digit number
    if (len == 1) 
    {
        strcpy(out, single_digits[*num - '0']);
        return;
    }

    while (*num != '\0') {
 
        /* Code path for first 2 digits */
        if (len >= 3) {
            if (*num - '0' != 0) {
                strcpy(out, single_digits[*num - '0']);
                out += strlen(single_digits[*num - '0']);
                strcpy(out, tens_power[len - 3]);
                out += strlen(tens_power[len - 3]);
            }
            --len;
        }
 
        // Code path for last 2 digits
        else 
        {
            //10 - 19
            if (*num == '1')
            {
                strcpy(out, two_digits[*(num + 1) - '0']);
                return;
            }
 
            // 20 -99
            else {
                strcpy(out, tens_multiple[*num - '0'-1]);
                out += strlen(tens_multiple[*num - '0'-1]);
                ++num;
                if (*num != '0')
                {
                    strcpy(out, single_digits[*num - '0']);
                    out += strlen(single_digits[*num - '0']);
                }
            }
        }
        ++num;
    }
}

int getWord(FILE *ifp, char* out, char len)
{
    for (char i = 0; i < len; i++)
    {
        char in = getc(ifp);
        if(in == EOF)
            return EOF;
        else if(in == '\n')
        {
            out[i] = '\0';
            return 0;
        }
        else
            out[i] = in;
    }
    fprintf(stderr,"getWord: char* to short!\n");
    return -2;
}