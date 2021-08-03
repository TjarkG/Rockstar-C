#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAXL 32

const char *ignore[] = {
    "auto",
    "register",
    "const",
    "unsigned",
    "signed",
    "void",
    "volatile",
    "main(void)",
    "{"
};

const char *foos[] = {
    "double",
    "int",
    "long",
    "float",
    "short",
    "char",
};
const char separators[] = {
    '(',
    ')',
    '{',
    '}',
    '[',
    ']',
    ';'
};
const char calcs[] = {
    '+',
    '-',
    '*',
    '/',
    '%',
    '=',
    ',',
    '<',
    '>',
    '|',
    '&',
    '~',
    '?',
    '!'
};

void parseC(FILE *, FILE *);
void removeEnding(const char*, char*);
void toCamelCase(char *);
void convert_to_words(char*, char*);
bool iscalc(char);

int main(int argc, char *argv[])
{
    FILE *fpIn;
    FILE *fpParse;
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
            strcpy(parseName+strlen(parseName)-1, ".parse");
            fpParse = fopen(parseName, "w");
            parseC(fpIn, fpParse);
            fclose(fpParse);
            fclose(fpIn);
        }
    }
    if (ferror(stdout))
    {
        fprintf(stderr, "%s: error writing stdout\n", prog);
        exit(2);
    }
    exit(0);
}

void parseC(FILE *ifp, FILE *ofp)
{
    char in;
    char inLast = '\0';
    bool isSep;
    bool wasSep = false;
    char inComment = 0;     // 0:no comment, 1: one line comment, 2: multi Line comment, 3: Include, 4: String Literal 5: Char Literal
    while ((in = getc(ifp)) != EOF)
    {
        isSep = false;
        char cmtOld = inComment;
        if(inLast == '/' && in == '/')
            inComment = 1;
        else if(inLast == '/' && in == '*')
            inComment = 2;
        else if(in == '<')
            inComment = 3;
        else if(in == '"' && inComment != 4)
            inComment = 4;
        else if(in == '\'' && inComment != 4)
            inComment = 5;
        else if(inComment == 1 && in == '\n')
            inComment = 0;
        else if(inComment == 2 && in == '/' && inLast == '*')
            inComment = 0;
        else if(inComment == 3 && in == '>')
            inComment = 0;
        else if(inComment == 4 && in == '"' && inLast != '\\')
            inComment = 0;
        else if(inComment == 5 && in == '\'' && inLast != '\\')
            inComment = 0;
        
        if(inComment < cmtOld)
            fprintf(ofp, "\n");

        if(!inComment)
        {
            for (char i = 0; i < sizeof(separators); i++)
            {
                if(in == separators[i])
                {
                    isSep = true;
                    break;
                }
            }
        }
        if((isSep && wasSep) || (isSep && (inLast == '\n')))
            fprintf(ofp, "%c\n", in);
        else if(isSep)
            fprintf(ofp, "\n%c\n", in);
        else if(in == '\n' && wasSep)
            ;
        else if(isspace(in) && isspace(inLast) && !inComment)
            ;
        else if(isspace(in) && ( isalpha(inLast) || iscalc(inLast)) && !inComment)
            fprintf(ofp, "\n");
        else
            fprintf(ofp, "%c", in);

        if(inComment > cmtOld)
            fprintf(ofp, "\n");
        inLast = in;
        wasSep = isSep;
    }
}

bool iscalc(char c)
{
    for (char i = 0; i < sizeof(calcs); i++)
        {
            if(c == calcs[i])
                return true;
        }
    return false;
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