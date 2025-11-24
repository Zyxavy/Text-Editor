#include "kilo.h"


// #===Syntax Highlight===#

void editorUpdateSyntax(erow *row)
{
    row->highlight = realloc(row->highlight, row->rSize); 
    memset(row->highlight, HL_NORMAL, row->rSize); //Default all to normal

    if(E.syntax == NULL) return;

    char **keywords = E.syntax->keywords;

    char *scs = E.syntax->singleLineCommentStart; 
    char *mcs = E.syntax->multilineCommentStart;
    char *mce = E.syntax->multilineCommentEnd;

    int scsLen = scs ? strlen(scs) : 0;
    int mcsLen = mcs ? strlen(mcs) : 0;
    int mceLen = mce ? strlen(mce) : 0;

    int prevSep = 1;
    int inString = 0;
    int inComment = (row->index > 0 && E.row[row->index - 1].hlOpenComment); //Check if previous row was in multi-line comment

    int i = 0;
    while(i < row->rSize)
    {
        char c = row->render[i];
        unsigned char prevHL = (i > 0) ? row->highlight[i-1] : HL_NORMAL; //Get previous highlight

        if(scsLen && !inString && !inComment) //If single-line comment delimiter is defined and not in string or comment
        {
            if (scsLen && strncmp(&row->render[i], scs, scsLen) == 0)
            {
                memset(&row->highlight[i], HL_COMMENT, row->rSize - i);
                break;
            }
        }

        if(mcsLen && mceLen && !inString) //If multi-line comment delimiters are defined and not in a string
        {
            if(inComment)
            {
                row->highlight[i] = HL_MLCOMMENT;
                if(!strncmp(&row->render[i], mce, mceLen)) //If end of multi-line comment found
                {
                    memset(&row->highlight[i], HL_MLCOMMENT, mceLen); //Highlight multi-line 
                    i += mceLen;
                    inComment = 0;
                    prevSep = 1;
                    continue;
                }
                else
                {
                    i++;
                    continue;
                }
            }
            else if(!strncmp(&row->render[i], mcs, mcsLen)) //If start of multi-line comment found
            {
                memset(&row->highlight[i], HL_MLCOMMENT, mcsLen);
                i += mcsLen;
                inComment = 1;
                continue;
            }
        }
        
        if(E.syntax->flags & HL_HIGHLIGHT_STRINGS) //If syntax highlighting for strings is enabled
        {
            if(inString) 
            {
                row->highlight[i] = HL_STRING;
                if(c == '\\' && i + 1 < row->rSize) //Escape sequence
                {
                    row->highlight[i+1] = HL_STRING;
                    i += 2;
                    continue;
                }
                if (c == inString) inString = 0;
                i++;
                prevSep = 1;
                continue;
            }
            else
            {
                if(c == '"' || c == '\'')
                {
                    inString = c;
                    row->highlight[i] = HL_STRING;
                    i++;
                    continue;
                }
            }
        }

        if(E.syntax->flags & HL_HIGHLIGHT_NUMBERS) //If syntax highlighting for numbers is enabled
        { 
            if((isdigit(c) && (prevSep || prevHL == HL_NUMBER)) || 
                (c == '.' && prevHL == HL_NUMBER)) //If digit or part of a number
            {
                row->highlight[i] = HL_NUMBER; //Highlight numbers
                i++;
                prevSep = 0;
                continue;
            }
        }

        if(prevSep)
        {
            int j;
            for(j = 0; keywords[j]; j++)
            {
        
                int keyLen = strlen(keywords[j]); 
                int keyword2 = keywords[j][keyLen - 1] == '|';
                if(keyword2) keyLen--;

                if(!strncmp(&row->render[i], keywords[j], keyLen) && isSeparator(row->render[i + keyLen])) //If keyword matches and is followed by a separator
                {
                    memset(&row->highlight[i], keyword2 ? HL_KEYWORD2 : HL_KEYWORD1, keyLen); //Highlight keyword
                    i += keyLen;
                    break;
                }
            }
            if(keywords[j] != NULL)
            {
                prevSep = 0;
                continue;
            }
        }

        prevSep = isSeparator(c); //Check if current character is a separator
        i++;
    }

    int changed = (row->hlOpenComment != inComment); //Check if multi-line comment state changed
    row->hlOpenComment = inComment;
    if(changed && row->index + 1 < E.numRows) editorUpdateSyntax(&E.row[row->index + 1]); //Update next row if comment state changed
}

int editorSyntaxToColor(int highlight)
{
    switch (highlight)
    {
        case HL_MLCOMMENT:
        case HL_COMMENT: return 36;
        case HL_KEYWORD1: return 33;
        case HL_KEYWORD2: return 32;
        case HL_STRING: return 35;
        case HL_NUMBER: return 31;
        case HL_MATCH: return 34;
        default: return 37;
    }
}

int isSeparator(int c)
{
    return isspace(c) || c == '\0' || strchr(",.()+-/*=~%<>[];", c) != NULL;
}

void editorSelectSyntaxHighlight()
{
    E.syntax = NULL;
    if(E.fileName == NULL) return;

    char *extension = strrchr(E.fileName, '.'); //Get file extension

    for(unsigned int i = 0; i < HLDB_ENTRIES; i++)
    {
        struct editorSyntax *s = &HLDB[i];

        for (unsigned int j = 0; s->filematch[j]; j++) 
        {
            int isExtension = (s->filematch[j][0] == '.'); 

            // Check for match
            if ((isExtension && extension &&
                 !strcmp(extension, s->filematch[j])) || 
                (!isExtension && strstr(E.fileName, s->filematch[j])))
            {
                // Match found
                E.syntax = s;

                for (int row = 0; row < E.numRows; row++)
                {
                    editorUpdateSyntax(&E.row[row]);
                }
                return;
            }
        }
        
    }
}
