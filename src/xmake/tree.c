//********************************************************
// TREE.C - Routines to build and search a dependency tree
//********************************************************

//++++
// This software is in the public domain.  It may be freely copied and used
// for whatever purpose you see fit, including commerical uses.  Anyone
// modifying this software may claim ownership of the modifications, but not
// the complete derived code.  It would be appreciated if the authors were
// told what this software is being used for, but this is not a requirement.

//   THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR
//   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
//   OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
//   TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
//   USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//----

#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <CTYPE.H>
#include "XMAKE.H"

extern void o_precious(void), o_silent(void), o_ignore(void), dot_default(void);

// Index table for dot-commands in makefile

struct opttbl dottable[] =
{	{{"DEFAULT   "}, dot_default},
	{{"PRECIOUS  "}, o_precious},
	{{"SILENT    "}, o_silent},
	{{"IGNORE    "}, o_ignore}
};
int dotsize = (sizeof(dottable))/(sizeof(struct opttbl));

static char dtarget[255];	  	// Place to keep target atom

void buildtree(void)
{
    char *pnt;					// Pointer to first atom
    char *type;					// Pointer to atom which defines operation

    while ((pnt = getatom()) != 0)
    {
		if (*pnt == '.' && inslash(pnt) == 0) // Default rule or dot cmd
		{
			strcpy(dtarget, pnt);
			if ((type = getatom()) != 0 && *type == ':' && *(type + 1) == 0)
			{
				defltrule(dtarget); // Build a default rule
				continue;
			}
			if (type && *type == '\n')	// If must be a dot command
			{
				dotcmd(dtarget);
				continue;
			}
			else
				syntax();				// Display a syntax error on makefile
		}
		if (*pnt == '\t')
			syntax();
		if (*pnt == '\n')				// If null line in makefile
			continue;					//  Go around and get another atom

		// If we reached this point, we have either a dependency or a macro

		strcpy(dtarget, pnt);
		if ((type = getatom()) != 0) 	// Get atom which defines this operation
		{
			if (*type == '=' && *(type + 1) == 0) // If a macro definition
				macrodef(dtarget);
			else
			{
				if (*type == ':' && (*(type + 1) == 0 || (*(type + 1) == ':' &&
						*(type + 2) == 0)))
					depend(dtarget, *(type + 1)); // If a dependency definition
				else
					syntax();			// I don't know what you called it!
			}
		}
	}
	if (treepnt == NULL)
		fatal("No dependencies found in Makefile", NULL, 1);

	// We've read the entire makefile and built a temporary tree in memory.
	//  It's the responsiblity of another function to use that temporary tree.

}


void depend(
	char *name,
	char dtype)

{
    char     *atom;				// Pointer to an atom
    TREENODE *dpnt;
    TREENODE *xpnt;

    dpnt = tsearch(name, treepnt);	// Check for existing dependency
    if (dpnt != NULL && dpnt->type != dtype)
	{									// If wrong kind of dependency
		fprintf(stderr, "? XMAKE: %s @%d, Mixing single and double-colon "
				"depenencies for the\n\t\t\tsame target not allowed\n",
                xmakex, mline);
		showerror();
		exit(1);
	}
	if (dpnt == NULL || (dtype == ':' && dpnt == NULL))
	{
		dpnt = (TREENODE *)getmemory(sizeof(TREENODE));
		dpnt->shell_list = NULL;
		dpnt->tree_add   = NULL;
		dpnt->tree_list  = NULL;
		dpnt->next       = NULL;
		dpnt->side       = dpnt->side_add = NULL;
		dpnt->type       = dtype;
		dpnt->file       = fnodemake(name); // Get filenode for this file
		dpnt->level      = 0;
		if (treeadd == NULL)			// If no blocks in list
			treeadd = treepnt = dpnt;	// Place first block in list
		else
		{
			treeadd->next = dpnt;		// Previous block points to this
			treeadd       = dpnt;		// This is now the last block
		}
    }
    else if (dtype == ':' && dpnt != NULL)
    {
		xpnt = (TREENODE *)getmemory((long)(sizeof(TREENODE)));
		xpnt->shell_list = NULL;
		xpnt->tree_add   = NULL;
		xpnt->tree_list  = NULL;
		xpnt->next       = NULL;
		xpnt->side       = xpnt->side_add = NULL;
		xpnt->type       = dtype;
		xpnt->level      = 0;
		xpnt->file       = fnodemake(name); // Get filenode for this file
		if (dpnt->side_add == NULL)
			dpnt->side_add = dpnt->side = xpnt;
		else
		{
			dpnt->side_add->side = xpnt;
			dpnt->side_add = xpnt;
		}
		dpnt = xpnt;
    	}

	if (OPT_debug && dpnt->tree_list == NULL)
		printf("Dependency:%c %s -> ", dtype, name);
    if (OPT_debug && dpnt->tree_list != NULL)
		printf("Extending Dependency: %s -> ", name);

	while (*(atom = getatom()) != '\n') // Until end of dependency list
	{
		if (OPT_debug)
			printf("%s ", atom);
		xpnt = (TREENODE *)getmemory(sizeof(TREENODE));
		if (dpnt->tree_add == NULL)
			dpnt->tree_add = dpnt->tree_list = xpnt;
		else
		{
			dpnt->tree_add->next = xpnt;
			dpnt->tree_add = xpnt;
		}
		xpnt->file = fnodemake(atom);	// Get filenode for this file
		xpnt->shell_list = NULL;
		xpnt->tree_add   = NULL;
		xpnt->next       = NULL;
		xpnt->tree_list  = NULL;		// Clear all pointers
		xpnt->level      = 0;
		xpnt->side = xpnt->side_add = NULL;
    }

///	if (dpnt->tree_list == NULL)		// If no dependents found
///	{
///		fprintf(stderr, "? XMAKE: %s @%d, Dependancy %s doesn't have any "
///				"dependents\n", xmakex, mline, name);
///		showerror();
///		exit(1);
///	}

    if (OPT_debug)
		fputc('\n', stderr);
    if (dtype == ':' || dpnt->shell_list == NULL)
		dpnt->shell_list = getcmd();	// Get any commands specified
    else
    {
		if (getcmd() != NULL)			// Must not specify a command
		{
			fprintf(stderr, "? XMAKE: %s @%d, Only one single-colon dependency "
					"may have a command list\n", xmakex, mline);
			showerror();
			exit(1);
		}
    }
}


/*
 * Function to define (or redefine) a macro
 */

void macrodef(
	char *name)

{
    MACRONODE *search = macpnt;	// Pointer for macro list search

    if (OPT_debug)
		printf("Macro %s -> ", name);
    while (search != NULL && stricmp(name, search->token) != 0)
		search = search->next;			// Search the list
    if (search == NULL)					// If no existing definition found
    {
		search = (MACRONODE *)getmemory(sizeof(MACRONODE));
										// Allocate a new node
        search->token    = getmemory(strlen(name) + 1);
		strcpy(search->token, name);
		search->lock     = 0;			// This macro is NOT locked
		search->nested   = NULL;		// New block cannot be in use
		search->position = (char *)EOF;	// Mark as in use until we finish
		search->next     = macpnt;		// Link into macro list
		macpnt           = search;
    }
    else
    {
		if (search->lock == 0)			// Cannot re-define locked macros
		{
///			free(search->value);		// Release old definition
// Cannot release memory when allocating under sbrk
			if (search->position != NULL)
			{
				fprintf(stderr, "? XMAKE: %s @%d, macro %s cannot re-define "
						"itself\n", xmakex, mline, search->token);
				showerror();
				exit(1);
			}
		}
    }

    if (search->lock == 0)				// Cannot re-define locked macros
    {
		search->vsize = strlen(search->value = getmacro()) + 1;
										// Allocate memory, copy macro body
    }									//   to memory, and set up MACRONODE
    search->position = NULL;			// Macro definition now complete
    if (OPT_debug)
    {
		fputs(search->value, stderr);
		fputc('\n', stderr);
    }
}

// Function to define a rule

void defltrule(
	char *name)

{
    RULENODE *definition;		// Pointer to new rule node
    char     *temp = name;		// Pointer for character count
    int       i;

    definition = (RULENODE *)getmemory(sizeof(RULENODE));

    ++temp;								// Move past first period
    i = 1;
    while (*temp != '.' && *temp != 0)
    {
		++temp;							// Advance to end of this entry
		++i;
    }
    definition->srctype = getmemory((long)i + 1);
    strncpy(definition->srctype, name, i);
    *(definition->srctype + i) = 0;
    if (OPT_debug)
		printf("Default rule %s:\n", name);
    if (*temp == 0)						// If no destination type
		definition->desttype = NULL;	// Indicate this
    else
    {
		name = temp;			/* Set to the beginning of this item */
		i = 0;
		while (*temp++ != 0)		/* Move to the end of this item */
			++i;
		definition->desttype = getmemory((long)i + 1);
		strncpy(definition->desttype, name, i);
		*(definition->desttype + i) = 0;
    }
    if (*(getatom()) != '\n') 
    {					/* If unexpected atom returned */
		fprintf(stderr, "? XMAKE: %s @%d, Syntax Error in definition of a "
				"default rule\n", xmakex, mline);
		showerror();
		exit(1);
    }
    if ((definition->command = getcmd()) == NULL) // If no commands
    {
		fprintf(stderr, "? XMAKE: %s @%d, No commands specified for a default "
				"rule\n", xmakex, mline);
		showerror();
		exit(1);
    }
    if (ruleadd == NULL)				// If no rules in list
    {
		ruleadd = rulepnt = definition;	// Add first block to rule list
		definition->next  = NULL;		// No next block in list yet
    }
    else
    {
		ruleadd->next = definition;		// This is now the next block
		ruleadd       = definition;		// This is now the last block
		ruleadd->next = NULL;			// This doesn't have a next block yet
    }
}

void dotcmd(name)
char *name;
{
    char symbuf[11];
    char *dest = symbuf;
    struct opttbl *dotpnt;

    ++name;								// Advance past the dot
    strcpy(symbuf, "          ");
    while (*name != 0)
		*dest++ = *name++;
    if (OPT_debug)
		printf("Searching for .%s command\n", symbuf);
    if ((dotpnt=(struct opttbl *)srchtbl(symbuf, dottable, dotsize)) != NULL)
		(*dotpnt->op_rout)();
    if (!dotpnt)
    {
		fprintf(stderr, "? XMAKE: No such dot-command .%s\n", symbuf);
		exit(1);						// Error - fail
    }
}


void dot_default(void)

{
    default_cmd = getcmd();
    if (default_cmd == 0)				// If no commands specified
    {
		printf("? XMAKE: %s @%d, .DEFAULT must be followed by a command list\n",
		xmakex, mline);
		exit(1);
    }
}

/*
 * Utility routines for manipulating the various data blocks
 */


// Function to return a filenode for a file

FILENODE *fnodemake(
	char *name)					// Pointer to filename to use for this search

{
    FILENODE *fpnt;

///	printf("### looking for file |%s|\n", name);

    if ((fpnt = filesrch(name)) == NULL)
	{									// If this name isn't on file
        fpnt = (FILENODE *)getmemory((long)(sizeof(FILENODE)));
										// Get new block
        fpnt->chain = filepnt;
        filepnt = fpnt;					// Add this file into node list
        fpnt->fname = getmemory(strlen(name) + 1);
										// Get length of filename
        strcpy(fpnt->fname, name);		// Place filename in block
		fpnt->fdatetime = 0xFFFFFFFFL;	// Time/date not obtained
    }
    return (fpnt);
}


// Function to return existing file node
//   for a given filename

FILENODE *filesrch(
	char *name)

{
    FILENODE *spnt = filepnt;

    while (spnt != NULL && stricmp(name, spnt->fname) != 0) 
        spnt = spnt->chain;				// Until we run out of nodes, or have
										//   a match
    return (spnt);						// Return whatever we have
}
