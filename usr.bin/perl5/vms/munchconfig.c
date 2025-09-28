/* munchconfig.c

   A very, very (very!) simple program to process a config_h.sh file on
   non-unix systems.

   usage:
   munchconfig config.sh config_h.sh [-f file] [foo=bar [baz=xyzzy [...]]] >config.h

   which is to say, it takes as its first parameter a config.sh (or
   equivalent), as its second a config_h.sh (or equivalent), an optional file
   containing tag=value pairs (one on each line), and an optional list of
   tag=value pairs on the command line.

   It spits the processed config.h out to STDOUT.

   */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

/* The failure code to exit with */
#ifndef EXIT_FAILURE
#ifdef VMS
#define EXIT_FAILURE 0
#else
#define EXIT_FAILURE -1
#endif
#endif

/* The biggest line we can read in from a file */
#define LINEBUFFERSIZE 1024
#define NUMTILDESUBS 30
#define NUMCONFIGSUBS 1500
#define TOKENBUFFERSIZE 80

typedef struct {
  char Tag[TOKENBUFFERSIZE];
  char Value[512];
} Translate;

void tilde_sub(char [], Translate [], int);

int
main(int argc, char *argv[])
{
  int c, i;
  char *ifile = NULL;
  char WorkString[LINEBUFFERSIZE]; 
  FILE *ConfigSH, *Config_H, *Extra_Subs;
  char LineBuffer[LINEBUFFERSIZE], *TempValue, *StartTilde, *EndTilde;
  char SecondaryLineBuffer[LINEBUFFERSIZE], OutBuf[LINEBUFFERSIZE];
  char TokenBuffer[TOKENBUFFERSIZE];
  int LineBufferLength, TempLength, DummyVariable, LineBufferLoop;
  int TokenBufferLoop, ConfigSubLoop, GotIt, OutBufPos;
  Translate TildeSub[NUMTILDESUBS];    /* Holds the tilde (~FOO~) */
                                       /* substitutions */
  Translate ConfigSub[NUMCONFIGSUBS];  /* Holds the substitutions from */
                                       /* config.sh */
  int TildeSubCount = 0, ConfigSubCount = 0; /* # of tilde substitutions */
                                             /* and config substitutions, */
                                             /* respectively */
  if (argc < 3) {
    printf("Usage: munchconfig config.sh config_h.sh [-f file] [foo=bar [baz=xyzzy [...]]]\n");
    exit(EXIT_FAILURE);
  }

  optind = 3;    /* skip config.sh and config_h.sh */
  while ((c = getopt(argc, argv, "f:")) != -1) {
      switch (c) {
        case 'f':
            ifile = optarg;
            break;
        case ':':
            fprintf(stderr, "Option -%c requires an operand\n", optopt);
            break;
        case '?':
            fprintf(stderr,"Unrecognised option: -%c\n", optopt);
      }
  }

  /* First, open the input files */
  if (NULL == (ConfigSH = fopen(argv[1], "r"))) {
    printf("Error %i trying to open config.sh file %s\n", errno, argv[1]);
    exit(EXIT_FAILURE);
  }
  
  if (NULL == (Config_H = fopen(argv[2], "r"))) {
    printf("Error %i trying to open config_h.sh file %s\n", errno, argv[2]);
    exit(EXIT_FAILURE);
  }

  if (ifile != NULL && NULL == (Extra_Subs = fopen(ifile, "r"))) {
    printf("Error %i trying to open extra substitutions file %s\n", errno, ifile);
    exit(EXIT_FAILURE);
  }

  /* Any tag/value pairs on the command line? */
  if (argc > optind) {
    for (i=optind; i < argc && argv[i]; i++) {
      /* Local copy */
      strcpy(WorkString, argv[i]);
      /* Stick a NULL over the = */
      TempValue = strchr(WorkString, '=');
      *TempValue++ = '\0';

      /* Copy the tag and value into the holding array */
      strcpy(TildeSub[TildeSubCount].Tag, WorkString);
      strcpy(TildeSub[TildeSubCount].Value, TempValue);
      TildeSubCount++;
    }
  }

  /* Now read in the tag/value pairs from the extra substitutions file, if any */
  while(ifile && fgets(LineBuffer, LINEBUFFERSIZE - 1, Extra_Subs)) {
    /* Force a trailing null, just in case */
    LineBuffer[LINEBUFFERSIZE - 1] = '\0';
    LineBufferLength = strlen(LineBuffer);

    /* Chop trailing control characters */
    while((LineBufferLength > 0) && (LineBuffer[LineBufferLength-1] < ' ')) {
      LineBuffer[LineBufferLength - 1] = '\0';
      LineBufferLength--;
    }

    /* If it's empty, then try again */
    if (!*LineBuffer)
      continue;

    /* Local copy */
    strcpy(WorkString, LineBuffer);
    /* Stick a NULL over the = */
    TempValue = strchr(WorkString, '=');
    *TempValue++ = '\0';

    /* Copy the tag and value into the holding array */
    strcpy(TildeSub[TildeSubCount].Tag, WorkString);
    strcpy(TildeSub[TildeSubCount].Value, TempValue);
    TildeSubCount++;
  }


  /* Now read in the config.sh file. */
  while(fgets(LineBuffer, LINEBUFFERSIZE - 1, ConfigSH)) {
    /* Force a trailing null, just in case */
    LineBuffer[LINEBUFFERSIZE - 1] = '\0';

    LineBufferLength = strlen(LineBuffer);

    /* Chop trailing control characters */
    while((LineBufferLength > 0) && (LineBuffer[LineBufferLength-1] < ' ')) {
      LineBuffer[LineBufferLength - 1] = '\0';
      LineBufferLength--;
    }

    /* If it's empty, then try again */
    if (!*LineBuffer)
      continue;

    /* If the line begins with a '#' or ' ', skip */
    if ((LineBuffer[0] == ' ') || (LineBuffer[0] == '#'))
      continue;

    /* We've got something. Guess we need to actually handle it */
    /* Do the tilde substitution */
    tilde_sub(LineBuffer, TildeSub, TildeSubCount);

    /* Stick a NULL over the = */
    TempValue = strchr(LineBuffer, '=');
    *TempValue++ = '\0';
    /* And another over the leading ', which better be there */
    *TempValue++ = '\0';
    
    /* Check to see if there's a trailing ' or ". If not, add a newline to
       the buffer and grab another line. */
    TempLength = strlen(TempValue);
    while ((TempValue[TempLength-1] != '\'') &&
           (TempValue[TempLength-1] != '"'))  {
      fgets(SecondaryLineBuffer, LINEBUFFERSIZE - 1, ConfigSH);
      /* Force a trailing null, just in case */
      SecondaryLineBuffer[LINEBUFFERSIZE - 1] = '\0';
      /* Go substitute */
      tilde_sub(SecondaryLineBuffer, TildeSub, TildeSubCount);
      /* Tack a nweline on the end of our primary buffer */
      strcat(TempValue, "\n");
      /* Concat the new line we just read */
      strcat(TempValue, SecondaryLineBuffer);

      /* Refigure the length */
      TempLength = strlen(TempValue);
      
      /* Chop trailing control characters */
      while((TempLength > 0) && (TempValue[TempLength-1] < ' ')) {
        TempValue[TempLength - 1] = '\0';
        TempLength--;
      }
    }
    
    /* And finally one over the trailing ' */
    TempValue[TempLength-1] = '\0';

    /* Is there even anything left? */
    if(*TempValue) {
      /* Copy the tag over */
      strcpy(ConfigSub[ConfigSubCount].Tag, LineBuffer);
      /* Copy the value over */
      strcpy(ConfigSub[ConfigSubCount].Value, TempValue);

      /* Up the count */
      ConfigSubCount++;

    }
  }

  /* Okay, we've read in all the substitutions from our config.sh */
  /* equivalent. Read in the config_h.sh equiv and start the substitution */
  
  /* First, eat all the lines until we get to one with !GROK!THIS! in it */
  while(!strstr(fgets(LineBuffer, LINEBUFFERSIZE, Config_H),
                "!GROK!THIS!")) {

    /* Dummy statement to shut up any compiler that'll whine about an empty */
    /* loop */
    DummyVariable++;
  }

  /* Right, we've read all the lines through the first one with !GROK!THIS! */
  /* in it. That gets us through the beginning stuff. Now start in earnest */
  /* with our translations, which run until we get to another !GROK!THIS! */
  while(!strstr(fgets(LineBuffer, LINEBUFFERSIZE, Config_H),
                "!GROK!THIS!")) {
    /* Force a trailing null, just in case */
    LineBuffer[LINEBUFFERSIZE - 1] = '\0';
    
    /* Tilde Substitute */
    tilde_sub(LineBuffer, TildeSub, TildeSubCount);

    LineBufferLength = strlen(LineBuffer);
    
    /* Chop trailing control characters */
    while((LineBufferLength > 0) && (LineBuffer[LineBufferLength-1] < ' ')) {
      LineBuffer[LineBufferLength - 1] = '\0';
      LineBufferLength--;
    }

    OutBufPos = 0;
    /* Right. Go looking for $s. */
    for(LineBufferLoop = 0; LineBufferLoop < LineBufferLength;
        LineBufferLoop++) {
      /* Did we find one? */
      if ('$' != LineBuffer[LineBufferLoop]) {
        /* Nope, spit out the value */
        OutBuf[OutBufPos++] = LineBuffer[LineBufferLoop];
      } else {
        /* Yes, we did. Is it escaped? */
        if ((LineBufferLoop > 0) && ('\\' == LineBuffer[LineBufferLoop -
                                                       1])) {
          /* Yup. Spit it out */
          OutBuf[OutBufPos++] = LineBuffer[LineBufferLoop];
        } else {
         /* Nope. Go grab us a token */
          TokenBufferLoop = 0;
          /* Advance to the next character in the input stream */
          LineBufferLoop++;
          while((LineBufferLoop < LineBufferLength) &&
                ((isalnum(LineBuffer[LineBufferLoop]) || ('_' ==
                                                          LineBuffer[LineBufferLoop])))) {
            TokenBuffer[TokenBufferLoop] = LineBuffer[LineBufferLoop];
            LineBufferLoop++;
            TokenBufferLoop++;
          }

          /* Trailing null on the token buffer */
          TokenBuffer[TokenBufferLoop] = '\0';

          /* Back the line buffer pointer up one */
          LineBufferLoop--;
          
          /* Right, we're done grabbing a token. Check to make sure we got */
          /* something */
          if (TokenBufferLoop) {
            /* Well, we do. Run through all the tokens we've got in the */
            /* ConfigSub array and see if any match */
            GotIt = 0;
            for(ConfigSubLoop = 0; ConfigSubLoop < ConfigSubCount;
                ConfigSubLoop++) {
              if (!strcmp(TokenBuffer, ConfigSub[ConfigSubLoop].Tag)) {
                char *cp = ConfigSub[ConfigSubLoop].Value;
                GotIt = 1;
                while (*cp) OutBuf[OutBufPos++] = *(cp++);
                break;
              }
            }

            /* Did we find something? If not, spit out what was in our */
            /* buffer */
            if (!GotIt) {
              char *cp = TokenBuffer;
              OutBuf[OutBufPos++] = '$';
              while (*cp) OutBuf[OutBufPos++] = *(cp++);
            }
            
          } else {
            /* Just a bare $. Spit it out */
            OutBuf[OutBufPos++] = '$';
          }       
        }
      }
    }
    
    /* If we've created an #undef line, make sure we don't output anything
     * after the "#undef FOO" besides comments.  We could do this as we
     * go by recognizing the #undef as it goes by, and thus avoid another
     * use of a fixed-length buffer, but this is simpler.
     */
    if (!strncmp(OutBuf,"#undef",6)) {
      char *cp = OutBuf;
      int i, incomment = 0;
      LineBufferLoop = 0;
      OutBuf[OutBufPos] = '\0';
      for (i = 0; i <= 1; i++) {
        while (!isspace(*cp)) LineBuffer[LineBufferLoop++] = *(cp++);
        while ( isspace(*cp)) LineBuffer[LineBufferLoop++] = *(cp++);
      }
      while (*cp) {
        while (isspace(*cp)) LineBuffer[LineBufferLoop++] = *(cp++);
        if (!incomment && *cp == '/' && *(cp+1) == '*') incomment = 1;
        while (*cp && !isspace(*cp)) {
          if (incomment) LineBuffer[LineBufferLoop++] = *cp;
          cp++;
        }
        if (incomment && *cp == '*' && *(cp+1) == '/') incomment = 0;
      }
      LineBuffer[LineBufferLoop] = '\0';
      puts(LineBuffer);
    }	
    else {
      OutBuf[OutBufPos] = '\0';
      puts(OutBuf);
    }
  }
  
  /* Close the files */
  fclose(ConfigSH);
  fclose(Config_H);
  if (ifile) fclose(Extra_Subs);
}

void
tilde_sub(char LineBuffer[], Translate TildeSub[], int TildeSubCount)
{
  char TempBuffer[LINEBUFFERSIZE], TempTilde[TOKENBUFFERSIZE];
  int TildeLoop, InTilde, CopiedBufferLength, TildeBufferLength, k, GotIt;
  int TempLength;
  InTilde = 0;
  CopiedBufferLength = 0;
  TildeBufferLength = 0;
  TempLength = strlen(LineBuffer);

  /* Grovel over our input looking for ~foo~ constructs */
  for(TildeLoop = 0; TildeLoop < TempLength; TildeLoop++) {
    /* Are we in a tilde? */
    if (InTilde) {
      /* Yup. Is the current character a tilde? */
      if (LineBuffer[TildeLoop] == '~') {
        /* Yup. That means we're ready to do a substitution */
        InTilde = 0;
        GotIt = 0;
        /* Trailing null */
        TempTilde[TildeBufferLength] = '\0';
        for( k=0; k < TildeSubCount; k++) {
          if (!strcmp(TildeSub[k].Tag, TempTilde)) {
            GotIt = 1;
            /* Tack on the trailing null to the main buffer */
            TempBuffer[CopiedBufferLength] = '\0';
            /* Copy the tilde substitution over */
            strcat(TempBuffer, TildeSub[k].Value);
            CopiedBufferLength = strlen(TempBuffer);
          }
        }
        
        /* Did we find anything? */
        if (GotIt == 0) {
          /* Guess not. Copy the whole thing out verbatim */
          TempBuffer[CopiedBufferLength] = '\0';
          TempBuffer[CopiedBufferLength++] = '~';
          TempBuffer[CopiedBufferLength] = '\0';
          strcat(TempBuffer, TempTilde);
          strcat(TempBuffer, "~");
          CopiedBufferLength = strlen(TempBuffer);
        }
        
      } else {
        /* 'Kay, not a tilde. Is it a word character? */
        if (isalnum(LineBuffer[TildeLoop]) ||
            (LineBuffer[TildeLoop] == '-')) {
          TempTilde[TildeBufferLength++] = LineBuffer[TildeLoop];
        } else {
          /* No, it's not a tilde character. For shame! We've got a */
          /* bogus token. Copy a ~ into the output buffer, then append */
          /* whatever we've got in our token buffer */
          TempBuffer[CopiedBufferLength++] = '~';
          TempBuffer[CopiedBufferLength] = '\0';
          TempTilde[TildeBufferLength] = '\0';
          strcat(TempBuffer, TempTilde);
          CopiedBufferLength += TildeBufferLength;
          InTilde = 0;
        }
      }
    } else {
      /* We're not in a tilde. Do we want to be? */
      if (LineBuffer[TildeLoop] == '~') {
        /* Guess so */
        InTilde = 1;
        TildeBufferLength = 0;
      } else {
        /* Nope. Copy the character to the output buffer */
        TempBuffer[CopiedBufferLength++] = LineBuffer[TildeLoop];
      }
    }
  }
  
  /* Out of the loop. First, double-check to see if there was anything */
  /* pending. */
  if (InTilde) {
    /* bogus token. Copy a ~ into the output buffer, then append */
    /* whatever we've got in our token buffer */
    TempBuffer[CopiedBufferLength++] = '~';
    TempBuffer[CopiedBufferLength] = '\0';
    TempTilde[TildeBufferLength] = '\0';
    strcat(TempBuffer, TempTilde);
    CopiedBufferLength += TildeBufferLength;
  } else {
    /* Nope, nothing pensing. Tack on a \0 */
    TempBuffer[CopiedBufferLength] = '\0';
  }

  /* Okay, we're done. Copy the temp buffer back into the line buffer */
  strcpy(LineBuffer, TempBuffer);

}

