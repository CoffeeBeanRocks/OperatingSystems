#include <xinu.h>

#define TY_ESC   0x1b
#define TY_BRACE 0x5b
#define TY_A     0x41

#define SKIP     0
#define DO_PRINT 1

local void echo(char ch, struct ttycblk* typtr, struct uart_csreg* csrptr);
local void clearline(struct ttycblk* typtr, struct uart_csreg* csrptr);
/*
 * ttydiscipline - Handles special character interactions.
 * 
 * Returns - Whether to print character (SKIP or DO_PRINT)
 */
int ttydiscipline(char ch,
		  struct ttycblk *typtr,
		  struct uart_csreg* csrptr) {

  if (ch == TY_NEWLINE || ch == TY_RETURN) {
    /* 
     * Copy the contents of the 'tyibuff' buffer from the 'tyihead' through 'tyitail'
     *     into the 'typrev' buffer.
     */ 

	  char* headOriginal = typtr->tyihead;
	  int i=0;
	  while(typtr->tyihead != typtr->tyitail && i<TY_IBUFLEN-1)
	  {
		typtr->typrev[i++] = *typtr->tyihead;
		typtr->tyihead++;
		if((int)typtr->tyihead == TY_IBUFLEN)
			typtr->tyihead = typtr->tyibuff;
	  }
	  typtr->typrev[i] = '\0';

	  typtr->tyihead = headOriginal;
  }
  
  /*
   * Check if the up key was pressed:
   * Use 'tycommand' to indicate if a command is in progress and the previous character
   *     If the characters appear in the sequence TY_ESC, then TY_BRACE, then TY_A
   *     the up key was sent
   */
 
  if(ch == TY_ESC || ch == TY_BRACE || ch == TY_A)
  {
  	char prevCommand = typtr->tycommand;
	if(ch == TY_ESC)
		typtr->tycommand = TY_ESC;
	else if(prevCommand == TY_ESC && ch == TY_BRACE)
		typtr->tycommand = TY_BRACE;
	else if(prevCommand == TY_BRACE && ch == TY_A)
	{
		typtr->tycommand = NULL;
		
		//UP KEY DETECTED
		clearline(typtr, csrptr);

		char* typrev_head = typtr->typrev;
		while(*(typrev_head) != '\0')
		{
			strcpy(typtr->tyitail++, typrev_head);
			echo(*(typrev_head++), typtr, csrptr);
			typtr->tyicursor++;
		}
	}
	else
		typtr->tycommand = NULL;

	return SKIP;
  }
  else
  {
  	typtr->tycommand = NULL;
  } 

  // If the up key is detected (as above)
      /*
       * Clear the current input with the 'clearline' function and copy the contents of 
       *     'typrev' buffer into the 'tyibuff' buffer
       *     remember to reset the 'tyicursor' as well
       *  Call 'echo' on each character to display it to the screen
       */
  return DO_PRINT;
}

local void echo(char ch, struct ttycblk* typtr, struct uart_csreg* csrptr) {
  *typtr->tyetail++ = ch;

  /* Wrap around buffer, if needed */
  if (typtr->tyetail >= &typtr->tyebuff[TY_EBUFLEN])
    typtr->tyetail = typtr->tyebuff;
  ttykickout(csrptr);
  return;
}

local void clearline(struct ttycblk* typtr, struct uart_csreg* csrptr) {
  int i, j;
  while (typtr->tyitail != typtr->tyihead)
    if ((--typtr->tyitail) < typtr->tyibuff)
      typtr->tyitail += TY_IBUFLEN;

  for (i=0; i < typtr->tyicursor; i++) {
    echo(TY_BACKSP, typtr, csrptr);
    echo(TY_BLANK, typtr, csrptr);
    echo(TY_BACKSP, typtr, csrptr);
    for (j=0; j<3; j++)
      ttyhandle_out(typtr, csrptr);
  }
  typtr->tyicursor = 0;
}
