/*
 * RockstarC.c
 *
 * programm to convert c to rockstar code
 *
 * Created: 02.08.2021 15:42:57
 * Author: TjarkG
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "Parse.h"

#define MAXL 32     //maximum lenght of a keyword or name
#define MAXCMT 120  //maximum lenght of a comment line
#define MAXW 255    //maximum number of instructions in a block

struct operator
{
    char c[3];
    char rock[16];
};

const struct operator operators[] = {
    {"+=", "plus "          },
    {"-=", "minus "         },
    {"*=", "times "         },
    {"/=", "over "          },
    {"+", " plus "          },
    {"-", " minus "         },
    {"*", " times "         },
    {"/", " over "          },
    {"==", " is "            },
    {">" , " is higher than "},
    {"<" , " is lower than " },
    {">=", " is as high as " },
    {"<=", " is as low as "  },
    {"(", " taking "        },
    {",", ", "           }
};

const char unsuported[] = {
    '%',
    '|',
    '&',
    '<',
    '>',
    '^'
};

const char *dataType[] = {
    "auto",
    "register",
    "const",
    "unsigned",
    "signed",
    "void",
    "volatile",
    "double",
    "int",
    "long",
    "float",
    "short",
    "char"
};

const char *keyword[] = {
    "auto",
    "register",
    "const",
    "unsigned",
    "signed",
    "void",
    "volatile",
    "double",
    "int",
    "long",
    "float",
    "short",
    "char",
    "break",
    "case",
    "continue",
    "default",
    "else",
    "enum",
    "extern",
    "for",
    "goto",
    "if",
    "inline",
    "return",
    "restrict",
    "sizeof",
    "static",
    "struct",
    "switch",
    "typedef",
    "union",
    "while",
    "_Bool",
    "_Complex",
    "_Generic",
    "_Imaginary",
    "bool",
    "complex",
    "imaginary",
    ";"
};

#define DATATYPE (sizeof(dataType)/sizeof(dataType[0]))

void rockstarConvert(FILE *, FILE *);
void removeEnding(const char*, char*);
void toCamelCase(char *);
void convert_to_words(char*, char*);
int getWord(FILE *, char*, char);
bool convComment(char *, FILE *, FILE *);
bool isInArray(char *, const char**, char);
bool convVarDec(char *, FILE *, FILE *);
bool convFuncDec(char *, FILE *, FILE *);
void convStatment(char *, FILE *, FILE *);
bool convPrintf(char *, FILE *, FILE *);
bool convReturn(char *, FILE *, FILE *);
void _trim(char *, bool);
bool convAssigment(char*, FILE *, FILE *);
void convExpression(FILE *, FILE *, char);

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
            fpParse = fopen(parseName, "w+");
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
            continue;
        }
        else if(convComment(in, ifp, ofp))
            continue;
        else if(convVarDec(in, ifp, ofp))
            continue;
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
            /*for(char j = i; j<length; j++)
            {
                name[j] = name[j+1];
            }
            name[length] = '\0';*/
            name[i] = ' ';
            name[i+1] = toupper(name[i+1]);
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
    for (int i = 0; i < len; i++)
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

bool convComment(char *in, FILE *ifp, FILE *ofp)
{
    char temp[MAXCMT];
    if(strcmp(in, "//") == 0)
    {
        getWord(ifp, temp, MAXCMT);
        fprintf(ofp, "(%s)\n", temp);
        getWord(ifp, temp, MAXCMT);
        if(strcmp(temp, ""))
                fprintf(stderr, "conversion Error: single line comment not followed by \\n\n");
        return 1;
    }
    else if(strcmp(in, "/*") == 0)
    {
        while (1)
        {
            getWord(ifp, temp, MAXCMT);
            if(strcmp(temp, "/") == 0)
                break;
            _trim(temp, true);
            fprintf(ofp, "(%s)\n", temp);
        }
        
        return 1;
    }
    return 0;
}

void _trim(char * s, bool astriks) {
    char * p = s;
    int l = strlen(p);

    while(isspace(p[l - 1]) || (p[l-1] == '*' && astriks))
        p[--l] = 0;
    while(* p && (isspace(*p) || (*p == '*' && astriks)))
        ++p, --l;

    memmove(s, p, l + 1);
}

bool isInArray(char *c, const char **array, char lenght)
{
    for (char i = 0; i < lenght; i++)
    {
        if(strcmp(c, array[i]) == 0)
            return true;
    }
    return false;
}

bool convVarDec(char *in, FILE *ifp, FILE *ofp)
{
    if(!isInArray(in, dataType, DATATYPE))
        return 0;
    char tempName[MAXL];
    char temp[MAXL];
    while (1)
    {
        getWord(ifp, tempName, MAXL);
        if(!isInArray(tempName, dataType, DATATYPE))
            break;
    }
    getWord(ifp, temp, MAXL);
    if(strcmp(temp, ",") == 0)
        while (1)
        {
            getWord(ifp, temp, MAXL);
            if(strcmp(temp, ";") == 0)
                break;
        }
    else if(strcmp(temp, ";") == 0)
        ;
    else if(strcmp(temp, "[") == 0)
        ;
    else if(strcmp(temp, "(") == 0)
        convFuncDec(tempName, ifp, ofp);
    else if(strcmp(temp, "=") == 0)
    {
        toCamelCase(tempName);
        fprintf(ofp, "%s is", tempName);
        getWord(ifp, temp, MAXL);
        fprintf(ofp, " %s\n", temp);
        getWord(ifp, temp, MAXL);
        if(strcmp(temp, ";"))
            fprintf(stderr, "conversion Error: variable declaration not followed by ;\n");
    }
    return 1;
}

bool convFuncDec(char *name, FILE *ifp, FILE *ofp)
{
    if(strcmp(name, "main") == 0)
    {
        char temp[MAXL];
        while (1)
        {
            getWord(ifp, temp, MAXL);
            if(strcmp(temp, "{") == 0)
            {
                convStatment("{", ifp, ofp);
                break;
            }
        }
        return 1;
    }
    else
    {
        char temp[MAXL];
        toCamelCase(name);
        fprintf(ofp, "%s takes ", name);
        while (1)
        {
            getWord(ifp, temp, MAXL);
            if(strcmp(temp, ")") == 0)
            {
                getWord(ifp, temp, MAXL);
                convStatment(temp, ifp, ofp);
                break;
            }
            else if(isInArray(temp, dataType, DATATYPE))
                continue;
            else if(temp[0] == ',')
            {
                fprintf(ofp, "and ");
                continue;
            }
            else
            {
                toCamelCase(temp);
                fprintf(ofp, "%s ", temp);
            }
        }
    }
}

void convSingleStatment(char *in, FILE *ifp, FILE *ofp)
{
    char temp[MAXL];
    if(strlen(in) == 0)
        return;
    else if(strcmp(in, "while") == 0)
    {
        for (int i = 0; i < MAXW; i++)
        {
            getWord(ifp, temp, MAXL);
            if(strcmp(temp, "(") == 0)
                break;
        }
        fprintf(ofp, "While ");
        convExpression(ifp, ofp, ')');
        fprintf(ofp, "\n");
        for (int i = 0; i < MAXW; i++)
        {
            getWord(ifp, temp, MAXL);
            if(strcmp(temp, "{") == 0)
                break;
        }
        for (int i = 0; i < MAXW; i++)
        {
            getWord(ifp, temp, MAXL);
            if(temp[0] == '}')
                break;
            else
                convSingleStatment(temp, ifp, ofp);
        }
        fprintf(ofp, "\n");
        return;
    }
    else if(strcmp(in, "if") == 0)
    {
        for (int i = 0; i < MAXW; i++)
        {
            getWord(ifp, temp, MAXL);
            if(strcmp(temp, "(") == 0)
                break;
        }
        fprintf(ofp, "If ");
        convExpression(ifp, ofp, ')');
        fprintf(ofp, "\n");
        for (int i = 0; i < MAXW; i++)
        {
            getWord(ifp, temp, MAXL);
            if(strcmp(temp, "{") == 0)
                break;
        }
        for (int i = 0; i < MAXW; i++)
        {
            getWord(ifp, temp, MAXL);
            if(temp[0] == '}')
                break;
            else
                convSingleStatment(temp, ifp, ofp);
        }
        fprintf(ofp, "\n");
        return;
    }
    else if(strcmp(in, "continue") == 0)
    {
        fprintf(ofp, "Take it to the top\n");
        getWord(ifp, temp, MAXL);
        if(strcmp(temp, ";"))
            fprintf(stderr, "conversion Error: continue not followed by ;\n");  
        return;
    }
    else if(strcmp(in, "break") == 0)
    {
        fprintf(ofp, "Break it down\n");
        getWord(ifp, temp, MAXL);
        if(strcmp(temp, ";"))
            fprintf(stderr, "conversion Error: break not followed by ;\n");  
        return;
    }
    else if(convComment(in, ifp, ofp))
        return;
    else if(convVarDec(in, ifp, ofp))
        return;
    else if(convPrintf(in, ifp, ofp))
        return;
    else if(convReturn(in, ifp, ofp))
        return;
    else if(convAssigment(in, ifp, ofp))
        return;
}

void convStatment(char *in, FILE *ifp, FILE *ofp)
{
    char temp[MAXL];
    if(strcmp(in, "{") == 0)
    {
        fprintf(ofp, "\n");
        for (int i = 0; i < MAXW; i++)
        {
            getWord(ifp, temp, MAXL);
            if(strcmp(temp, "}") == 0)
                break;
            else
                convSingleStatment(temp, ifp, ofp);
        }
        return;
    }
    else
        convSingleStatment(temp, ifp, ofp);
}

bool convPrintf(char *in, FILE *ifp, FILE *ofp)
{
    if(strcmp(in, "printf") == 0)
    {
        char temp[MAXL];
        getWord(ifp, temp, MAXCMT);
        if(strcmp(temp, "("))
                fprintf(stderr, "conversion Error: printf not followed by (\n");
        fprintf(ofp, "Say ");
        char c;
        while (1)
        {
            c = getc(ifp);
            if(c == ')')
            {
                fprintf(ofp, "\n");
                break;
            }
            if(c == '\n')
            {
                continue;
            }
            if(c == '\\')
            {
                c = getc(ifp);
                continue;
            }
            else
                fprintf(ofp, "%c", c);
        }
        getc(ifp);
        getWord(ifp, temp, MAXL);
        if(strcmp(temp, ";"))
            fprintf(stderr, "conversion Error: printf() not followed by ;\n");  
        return 1;
    }
    return 0;
}

bool convReturn(char *in, FILE *ifp, FILE *ofp)
{
    if(strcmp(in, "return") == 0)
    {
        char temp[MAXL];
        fprintf(ofp, "Give Back ");
        while (1)
        {
            getWord(ifp, temp, MAXL);
            if(strcmp(temp, ";") == 0)
            {
                fprintf(ofp, "\n");
                break;
            }
            else
                fprintf(ofp, "%s", temp);
        }
        return 1;
    }
    return 0;
}

bool convAssigment(char* in, FILE *ifp, FILE *ofp)
{
    if(isInArray(in, keyword, (sizeof(keyword)/sizeof(keyword[0]))))
    {
        fprintf(stderr, "convAssigmnet called with %s as an argument\n", in);
        return 0;
    }
    char temp[MAXL];
    bool found = false;
    toCamelCase(in);
    getWord(ifp, temp, MAXL);
    if(strcmp(temp, "++") == 0)
    {
        fprintf(ofp, "Build %s up\n", in);
        convExpression(ifp, ofp, ';');
        return 1;
    }
    else if(strcmp(temp, "--") == 0)
    {
        fprintf(ofp, "Knock %s down\n", in);
        convExpression(ifp, ofp, ';');
        return 1;
    }
    fprintf(ofp, "Let %s be ", in);
    for (char i = 0; i < (sizeof(operators)/sizeof(operators[0])); i++)
        if(strcmp(operators[i].c, temp) == 0)
        {
            fprintf(ofp, "%s", operators[i].rock);
            found = true;
            break;
        }
    if(!found)
        fprintf(stderr, "%s not an operator in rockstar\n", temp);
    convExpression(ifp, ofp, ';');
    fprintf(ofp, "\n");
    return 1;
}

void convExpression(FILE *ifp, FILE *ofp, char end)
{
    char temp[MAXL];
    char indent = 0;
    bool found = false;
    while (1)
    {
        getWord(ifp, temp, MAXL);
        _trim(temp, false);
        found = false;
        for (char i = 0; i < (sizeof(operators)/sizeof(operators[0])); i++)
            if(strcmp(operators[i].c, temp) == 0)
            {
                fprintf(ofp, "%s", operators[i].rock);
                found = true;
                break;
            }
        if(temp[0] == '(')
            indent++;
        if(found)
            continue;
        else if(temp[0] == end && indent == 0)
            return;
        else if(temp[0] == ')')
            indent--;
        else if(isdigit(temp[0]))
        {
            fprintf(ofp, "%s", temp);
        }
        else
        {
            toCamelCase(temp);
            fprintf(ofp, "%s", temp);
        }
    } 
}