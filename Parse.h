/*
 * Parse.h
 *
 * Header to parse c to the format needed for the c to rockstar converter
 *
 * Created: 04.08.2021 19:40:05
 * Author: TjarkG
 */

#ifndef Parse_H_
#define Parse_H_

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
bool iscalc(char);

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
        else if(isspace(in) && ( isalpha(inLast) || iscalc(inLast) || isdigit(inLast)) && !inComment)
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

#endif /* Parse_H_ */