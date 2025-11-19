#include "kilo.h"

// #===File I/O===#
void editorOpen(char *fileName)
{
    free(E.fileName);
    E.fileName = strdup(fileName); //Store file name

    editorSelectSyntaxHighlight();

    FILE *fp = fopen(fileName, "r"); //Open file for reading, if fails then call 'die'
    if(!fp) die("fopen");

    char *line = NULL;
    size_t lineCap = 0;
    ssize_t lineLen;
    while ((lineLen = getline(&line, &lineCap, fp)) != -1) //Read each line
    {
        while(lineLen > 0 && (line[lineLen - 1] == '\n' || line[lineLen - 1] == '\r')) //Trim newline characters
        {
            lineLen--; 
        }
        editorInsertRow(E.numRows, line, lineLen);//Append row to editor
    } 
    free(line);
    fclose(fp); //Close file
    E.dirty = 0;
}

char *editorRowsToString(int *bufferlen)
{
    int totalLen = 0;
    int i;
    for(i = 0; i < E.numRows; i++) totalLen += E.row[i].size + 1; //Calculate total length including newlines
    *bufferlen = totalLen;

    char *buffer = malloc(totalLen);
    char *p = buffer;

    for(i = 0; i < E.numRows; i++) //For each row
    {
        memcpy(p, E.row[i].chars, E.row[i].size); //Copy row characters
        p += E.row[i].size; //Move pointer forward
        *p = '\n'; //Add newline
        p++;
    }
    
    return buffer;
}

void editorSave()
{
    if(E.fileName == NULL)
    {
        E.fileName = editorPrompt("Save as: %s (ESC to cancel)", NULL);

        if(E.fileName == NULL)
        {
            editorStatusMessage("Save aborted!");
            return;
        }
        editorSelectSyntaxHighlight();
    }

    int len;
    char *buffer = editorRowsToString(&len); //Convert rows to string

    int fd = open(E.fileName, O_RDWR | O_CREAT, 0644); //Open file for reading and writing, create if it doesn't exist
    if(fd != -1)
    {
        if(ftruncate(fd, len) != -1) //Truncate file to new length
        {
            if(write(fd, buffer, len) == len) //write buffer to file
            {
                close(fd);
                free(buffer);
                E.dirty = 0;
                editorStatusMessage("%d bytes written into disk", len);
                return;
            }
        }
        close(fd);
    }
    free(buffer);
    editorStatusMessage("Cant Save! I/O error: %s", strerror(errno));
}
