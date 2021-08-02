#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXL 64

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