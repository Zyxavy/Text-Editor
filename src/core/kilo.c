#include "kilo.h"

//C / C++
char *C_HL_extension[] = {".c", ".h", ".cpp", ".hpp", NULL };
char *C_HL_keywords[] = 
{
  "switch", "if", "while", "for", "break", "continue", "return", "else",
  "struct", "union", "typedef", "static", "enum", "class", "case",
  "int|", "long|", "double|", "float|", "char|", "unsigned|", "signed|",
  "void|", NULL
};
struct editorSyntax HLDB[] = 
{
    {
        "c", C_HL_extension, C_HL_keywords, "//", "/*", "*/", HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
    },
    {
        "py", PY_HL_extension, PY_HL_keywords, "#", "''", "''", HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
    },
};

//Python
char *PY_HL_extension[] = {".py", NULL };
char *PY_HL_keywords[] = 
{
  // Keywords
  "False", "None", "True",
  "and", "as", "assert",
  "async", "await",
  "break", "class", "continue",
  "def", "del",
  "elif", "else", "except",
  "finally", "for", "from",
  "global",
  "if", "import", "in", "is",
  "lambda",
  "nonlocal", "not",
  "or",
  "pass",
  "raise", "return",
  "try",
  "while", "with", "yield",

  // Common builtins/types (with | suffix to mark types)
  "self",
  "__init__",
  "print",
  "range",
  "len",
  "str|", "int|", "float|", "bool|",
  "list|", "dict|", "set|", "tuple|", "object|",

  NULL
};

struct editorConfig E;
const unsigned int HLDB_ENTRIES = sizeof(HLDB) / sizeof(HLDB[0]);

//  #===Init===#
void initEditor()
{
    //Initialize editor state to 0 or NULL
    E.curX = 0;
    E.curY = 0;
    E.renderX = 0;
    E.rowOffset = 0;
    E.colOffset = 0;
    E.numRows = 0;
    E.row = NULL;
    E.fileName = NULL;
    E.statusMsg[0] = '\0';
    E.statusMsgTime = 0;
    E.dirty = 0;
    E.syntax = NULL;

    if(getWindowSize(&E.screenRows, &E.screenCols) == -1) die("getWindowSize");//Get terminal size
    E.screenRows -= 2;
}

int main(int argc, char*argv[])
{
    enableRawMode();//Enable raw mode to disable echoing
    initEditor();
    if(argc >= 2)
    {
        editorOpen(argv[1]);
    }

    editorStatusMessage("HELP: Ctrl+S = Save | Ctrl+Q = Quit | Ctrl+F = Find");

    while (1)
    {
        editorRefreshScreen();
        editorProcessKeypress();
    } 
    return 0;
}
