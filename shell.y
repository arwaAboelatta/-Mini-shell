
/*
 * CS-413 Spring 98
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%token	<string_val> WORD

%token GREAT GREATGREAT LESS PIPE AMPERSAND NOTOKEN  NEWLINE ERROR


%union	{
		char   *string_val;
	}

%{
extern "C" 
{
	int yylex();
	void yyerror (char const *s);
}
#define yylex yylex
#include <stdio.h>
#include "command.h"
%}

%%

goal:	
	commands
	;

commands: 
	command
	| commands command 
	;

command: simple_command
        ;

simple_command:	
	  command_and_args io background_opt NEWLINE {
		printf("   Yacc: Execute command\n");
		Command::_currentCommand.execute();
	}
	| NEWLINE 
	| error NEWLINE { yyerrok; }
	;

command_and_args:
  command_and_args PIPE command_word arg_list {
    Command::_currentCommand.insertSimpleCommand( Command::_currentSimpleCommand );
    printf("   Yacc: pipelining\n");
    Command::_currentSimpleCommand = new SimpleCommand();
  }
  | command_word arg_list {
    Command::_currentCommand.insertSimpleCommand( Command::_currentSimpleCommand );
    Command::_currentSimpleCommand = new SimpleCommand();
  }
  ;


arg_list:
	arg_list argument
	| /* can be empty */
	;

argument:
	WORD {
               printf("   Yacc: insert argument \"%s\"\n", $1);
	       Command::_currentSimpleCommand->insertArgument( $1 );\
	}
	;

command_word:
	WORD {
               printf("   Yacc: insert command \"%s\"\n", $1);
	       Command::_currentSimpleCommand = new SimpleCommand();
	       Command::_currentSimpleCommand->insertArgument( $1 );
	}
	;
io:
        io iomodifier_opt
        |
        ;


iomodifier_opt:
	GREAT WORD {
		printf("   Yacc: insert output to \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._append = 0;
	}
	|
	GREATGREAT WORD {
            printf("Yacc: Append output to %s\n", $2);
            Command::_currentCommand._append = 1;
            Command::_currentCommand._outFile = $2;
    }
    | LESS WORD {
            printf("Yacc: Input redirect from %s\n", $2);
            Command::_currentCommand._inputFile = $2;
        }  
    | ERROR WORD {
            printf("Yacc: ERROR redirect from %s\n", $2);
            Command::_currentCommand._errFile = $2;
        } 
    ;

background_opt:
    AMPERSAND {
    printf("   Yacc: insert task to the background");
    		Command::_currentCommand._background = 1;
    }
    |
    ;




%%

void
yyerror(const char * s)
{
	fprintf(stderr,"%s", s);
}

#if 0
main()
{
	yyparse();
}
#endif
