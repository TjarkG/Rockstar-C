#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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

void filecopy(FILE *, FILE *);
void toCamelCase(char *);
void convert_to_words(char*, char*);

int main(int argc, char *argv[])
{
    FILE *fp;
    char *prog = argv[0];     // program name for errors

    if (argc == 1 ) /* no args: throw error */
    {
        fprintf(stderr, "%s: no files to interpile\n", prog);
        return -1;
    }
    while (--argc > 0)
    {
        if ((fp = fopen(*++argv, "r")) == NULL) 
        {
            fprintf(stderr, "%s: can't open %s\n", prog, *argv);
            exit(1);
        }
        else
        {
            filecopy(fp, stdout);
            fclose(fp);
        }
    }
    char c[] = "test_name12";
    toCamelCase(c);
    printf("%s\n", c);
    char d[] = "testtest30_name";
    toCamelCase(d);
    printf("%s\n", d);
    char e[] = "20";
    toCamelCase(e);
    printf("%s\n", e);
    if (ferror(stdout))
    {
        fprintf(stderr, "%s: error writing stdout\n", prog);
        exit(2);
    }
    exit(0);
}

void filecopy(FILE *ifp, FILE *ofp)
{
    char c[MAXL];
    char indent = 0;
    while ( fscanf(ifp, "%s[a-z]", c) != EOF)
    {
        for (char i = 0; i < (sizeof(ignore) / sizeof(char *)); i++)        //typs get ignored
            if(strcmp(c, ignore[i]) == 0)
            {
                c[0] = '\0';
                break;
            }
        
        for (char i = 0; i < (sizeof(foos) / sizeof(char *)); i++)
        {
            if(strcmp(c, foos[i]) == 0)
            {
                if(!indent)
                {
                    c[0] = '\0';
                    indent ++;
                }
                else
                {
                    fscanf(ifp, "%s ", c);
                    for (char j = 0; j < (sizeof(foos) / sizeof(char *)); j++)
                    {
                        if(strcmp(c, foos[j]) == 0)
                        {
                            fscanf(ifp, "%s ", c);
                            break;
                        }
                    }
                    
                    char temp;
                    if(temp = getc(ifp) == '=')
                    {
                        fprintf(ofp,"%s is ", c);
                        fscanf(ifp, "%s[a-z]", c);
                    }
                    else
                    {
                        fscanf(ifp, "%s ", c+1);
                        c[0] = temp;
                    }
                }
                //fprintf(ofp," \n%d\n", i);
                //fprintf(ofp," %s\n", c);
                break;
            }
        }

        if(strcmp(c, "return") == 0)
        {
            fprintf(ofp,"Give Back ");
            fscanf(ifp, " %s ;", c);
            if(c[strlen(c)-1] == ';')
            {
                c[strlen(c)-1] = '\0';
            }
        }
        else if(strcmp(c, "#include") == 0)
        {
            fscanf(ifp, " <%[a-z]s", c);
            fscanf(ifp, ".h>");
            c[0] ='\0';
        }
        else if(strcmp(c, "}") == 0)
        {
            indent--;
            c[0] ='\0';
        }
        else if(strncmp(c, "printf", (sizeof("printf")-1)) == 0)
        {
            if(c[strlen(c)-1] != ';')
            {
                char in;
                int lenght = strlen(c);
                c[lenght++] = ' ';
                while ((in = getc(ifp)) != ')')
                    c[lenght++] = in;
                c[lenght] = '\0';
            }
            else
            {
                c[strlen(c)-2] = '\0';
            }
            fprintf(ofp,"Say %s\n", c+sizeof("printf"));
            c[0] = '\0';
        }
        else if(strncmp(c, "//", (sizeof("//")-1)) == 0)
        {
            strcpy(c, c+1);
            c[0] = '(';
            char in;
            int lenght = strlen(c);
            c[lenght++] = ' ';
            while ((in = getc(ifp)) != '\n')
                c[lenght++] = in;
            c[lenght++] = ')';
            c[lenght] = '\0';
        }
        else if(strncmp(c, "/*", (sizeof("/*")-1)) == 0)
        {
            strcpy(c, c+1);
            c[0] = '(';
            char in;
            int lenght = strlen(c);
            c[lenght++] = ' ';
            do
            {
                in = getc(ifp);
                c[lenght] = in;
                lenght++;
            } while ( strncmp("*/",c+lenght-2, 2) && lenght < MAXL);

            c[lenght-2] = ')';
            c[lenght-1] = '\0';
        }

        if(c[strlen(c)-1] == ';')
        {
            c[strlen(c)-1] = '\0';
        }
        if(!c[0] == '\0')
            fprintf(ofp,"%s\n", c);
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