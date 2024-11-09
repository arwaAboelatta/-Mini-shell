
/*
 * CS354: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <ctime>
#include <errno.h>
#include <glob.h>
#include "command.h"

SimpleCommand::SimpleCommand()
{
    // Creat available space for 5 arguments
    _numberOfAvailableArguments = 5;
    _numberOfArguments = 0;
    _arguments = (char **) malloc( _numberOfAvailableArguments * sizeof( char * ) );
}

void
SimpleCommand::insertArgument( char * argument )
{
    if ( _numberOfAvailableArguments == _numberOfArguments  + 1 ) {
        // Double the available space
        _numberOfAvailableArguments *= 2;
        _arguments = (char **) realloc( _arguments,
                                        _numberOfAvailableArguments * sizeof( char * ) );
    }

    _arguments[ _numberOfArguments ] = argument;

    // Add NULL argument at the end
    _arguments[ _numberOfArguments + 1] = NULL;

    _numberOfArguments++;
}

Command::Command()
{
    // Create available space for one simple command
    _numberOfAvailableSimpleCommands = 1;
    _simpleCommands = (SimpleCommand **)
                      malloc( _numberOfSimpleCommands * sizeof( SimpleCommand * ) );

    _numberOfSimpleCommands = 0;
    _outFile = 0;
    _inputFile = 0;
    _errFile = 0;
    _background = 0;

}

void
Command::insertSimpleCommand( SimpleCommand * simpleCommand )
{
    if ( _numberOfAvailableSimpleCommands == _numberOfSimpleCommands ) {
        _numberOfAvailableSimpleCommands *= 2;
        _simpleCommands = (SimpleCommand **) realloc( _simpleCommands,
                          _numberOfAvailableSimpleCommands * sizeof( SimpleCommand * ) );
    }

    _simpleCommands[ _numberOfSimpleCommands ] = simpleCommand;
    _numberOfSimpleCommands++;
}

void
Command:: clear()
{
    for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
        for ( int j = 0; j < _simpleCommands[ i ]->_numberOfArguments; j ++ ) {
            free ( _simpleCommands[ i ]->_arguments[ j ] );
        }

        free ( _simpleCommands[ i ]->_arguments );
        free ( _simpleCommands[ i ] );
    }

    if ( _outFile ) {
        free( _outFile );
    }

    if ( _inputFile ) {
        free( _inputFile );
    }

    if ( _errFile ) {
        free( _errFile );
    }

    _numberOfSimpleCommands = 0;
    _outFile = 0;
    _inputFile = 0;
    _errFile = 0;
    _background = 0;
}

void
Command::print()
{
    printf("\n\n");
    printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple Commands\n");
    printf("  --- ----------------------------------------------------------\n");

    for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
        printf("  %-3d ", i );
        for ( int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++ ) {
            printf("\"%s\" \t", _simpleCommands[i]->_arguments[ j ] );
        }
    }

    printf( "\n\n" );
    printf( "  Output       Input        Error        Background\n" );
    printf( "  ------------ ------------ ------------ ------------\n" );
    printf( "  %-12s %-12s %-12s %-12s\n", _outFile?_outFile:"default",
            _inputFile?_inputFile:"default", _errFile?_errFile:"default",
            _background?"YES":"NO");
    printf( "\n\n" );

}
void
Command::changeDirectory(int i) {

    const char *dir = _simpleCommands[i]->_arguments[1];
    if (dir) {
        if(chdir(dir)==-1)
        {
            perror("cd");
        }
        else
        {
            chdir(dir);
        }
    } else {
        const char *homeDir = getenv("HOME");
        if (chdir(homeDir)==-1) {
            perror("cd");
        } else {
            chdir(homeDir);
        }
    }
}

void sigchld_handler(int sig_num)
{

    FILE *f;
    time_t now = time(0);
    struct tm *localTime = localtime(&now);

    char timeString [20];
    strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", localTime);
    f=fopen("childlog.log","a");
    fprintf(f,"child terminated at [%s] \n",timeString);
    fclose(f);
}



void Command::Wildcarding(int i,int j){
	glob_t glob_result;

	char* pattern = _simpleCommands[i]->_arguments[j];
	int glob_status = glob(pattern, 0, NULL, &glob_result);
	if (glob_status == 0) {
        // Access the matched filenames through the gl_pathv array
        for (size_t i = 0; i < glob_result.gl_pathc; i++) {
            printf("%s\n", glob_result.gl_pathv[i]);
        }

        // Free the memory allocated by glob
        globfree(&glob_result);
    } else if (glob_status == GLOB_NOMATCH) {
        printf("No match found\n");
    } else {
        fprintf(stderr, "Error during glob: %d\n", glob_status);
    }
}
void
Command::execute()
{
    // Don't do anything if there are no simple commands
    if ( _numberOfSimpleCommands == 0 ) {
        prompt();
        return;
    }

    if(strcmp(_simpleCommands[0]->_arguments[0],"exit")==0)
    {

        printf("Good bye \n");
        exit(0);
    }

    // Print contents of Command data structure
    print();
    int defaultin = dup(0);
    int defaultout = dup(1);
    int defaulterr = dup(2);

    int fin;
    int fout;
    int ferr;
    int pid;
    if (_inputFile) {
        fin = open(_inputFile, O_RDONLY);
        if (fin < 0) {
            perror("Cannot open input file");

        }
    }
    else
    {
        fin=dup(defaultin);
    }

    // Add execution here
    // For every simple command fork a new process
    // Setup i/o redirection
    // and call exec

    for (int i = 0; i < _numberOfSimpleCommands; i++) {
        // Handle input redirection
        dup2(fin,0);
        close (fin);
        if(strcmp(_simpleCommands[i]->_arguments[0],"cd")==0)
        {
            changeDirectory(i);
            break;
        }

        if (i== _numberOfSimpleCommands - 1) {
            if (_outFile) {
                // Check if it's an append or overwrite redirection
                if (_append) {
                    fout = open(_outFile, O_WRONLY | O_CREAT | O_APPEND, 0666);
                } else {
                    fout = open(_outFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                    _append = 0 ;
                }
                if (fout < 0) {
                    perror("Cannot open output file");
                    exit(1);
                }
            }
            else {
                fout=dup(defaultout);
            }

            // Handle error redirection
            if (_errFile) {
                ferr = open(_errFile, O_WRONLY | O_CREAT | O_APPEND, 0666);
                if (ferr < 0) {
                    perror("Cannot open error file");
                    exit(1);
                }
            }
            else {
                ferr = dup(defaulterr);
            }
        }

        else
        {
            int fdpipe[2];
            pipe(fdpipe);
            fout=fdpipe[1];
            fin=fdpipe[0];

        }
        dup2(fout,1);
        dup2(ferr,2);
        close(fout);
        close(ferr);

        //create child process
        int wildcardflag = 0;
        if (strcmp(_simpleCommands[i]->_arguments[0], "echo") == 0) {
            for(int j=0; j<_simpleCommands[i]->_numberOfArguments; j++) {
                for(int z=0; z<strlen(_simpleCommands[i]->_arguments[j]); z++) {
                    if(_simpleCommands[i]->_arguments[j][z] == '*' || _simpleCommands[i]->_arguments[j][z] == '?') {
                        Wildcarding(i,j);
                        wildcardflag = 1;
                        break;
                    }
                    if(wildcardflag) {
                        break;
                    }
                }
            }
            if(wildcardflag) {
                break;
            }
        }

        pid = fork();
        if (pid < 0) {
            perror("Error: fork failed");
            exit(1);
        }
        else if(pid == 0) {
            //child process
            execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments);
            perror( "error in child\n");
            exit( 2 );
        }
        signal(SIGCHLD,sigchld_handler);
    }


    // Restore default input, output, and error
    dup2(defaultin, 0);
    dup2(defaultout, 1);
    dup2(defaulterr, 2);

    // Close file descriptors
    close(defaultin);
    close(defaultout);
    close(defaulterr);

    if (!_background) {
        waitpid(pid,0, 0);
    }


    // Clear to prepare for next command
    clear();

    // Print new prompt
    prompt();
}

// Shell implementation

void
Command::prompt()
{
    printf("myshell>");
    fflush(stdout);
}

Command Command::_currentCommand;
SimpleCommand * Command::_currentSimpleCommand;

int yyparse(void);

int
main()
{
    Command::_currentCommand.prompt();
    signal(SIGINT,SIG_IGN);
    yyparse();
    return 0;
}

