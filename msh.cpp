#include <iostream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <sstream>
#include <vector>
#include <wait.h>
#include <signal.h>

using namespace std;

static time_t startTime;

bool cd(char* &currPwd, char* newPwd);

// handler for CTR-C
void handler_sigint(int signal)
{
    time_t endTime = time(NULL);
    time_t elapsedTime = endTime - startTime;
    time_t hours = elapsedTime / 3600;
    time_t minutes = (elapsedTime % 3600) / 60;
    time_t seconds = (elapsedTime % 3600) % 60;
    cout << "\nTime elapsed: " << hours << "h " << minutes << "m " << seconds << "s\n";
    exit(EXIT_SUCCESS);
}

// handler for Zombies
void handler_sigchld(int signal)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

int main(int argc, char* argv[], char* envp[])
{
    startTime = time(NULL);
    signal(SIGINT, handler_sigint);
    signal(SIGCHLD, handler_sigchld);

    char* pwd;
    if(!(pwd = get_current_dir_name()))
    {
        cout << "Envp PWD not found." << endl;
        return EXIT_FAILURE;
    }

    cout << "exit with CTR-C" << endl;
    string input;
    while (true)
    {
        cout << pwd << "> ";
        // getline to get all user input including whitespace
        getline(cin, input);

        // string vector to hold the tokens of the whitespace splitted string
        vector<string> inputTokens;
        // string to hold the current token of the whitespace splitted string
        string inputToken;
        // stringstream to read the whitespace splitted tokens from, initialized with user input string
        stringstream inputStream(input);
        // gets the next token of the stringstream and writes it into inputToken until the stringstream is empty
        while(getline(inputStream, inputToken, ' '))
        {
            // pushes the current token (inputToken) into the inputTokens vector
            inputTokens.push_back(inputToken);
        }

        if(inputTokens.size() > 0)
        {
            bool background = false;
            // checks whether the last token in inputTokens is "&"
            if(inputTokens.back() == "&")
            {
                // if "&" was found toggles the background bool and removes the token from inputTokens
                background = true;
                inputTokens.pop_back();
            }

            // char* vector to hold all tokens in inputTokens as a C-string for use with chdir() and execvp()
            vector<char*> inputTokensC;
            // reserving the same amount of memory for the char* vector inputTokensC as there are elements in string vector inputTokens
            inputTokensC.reserve(inputTokens.size());
            for(int i = 0; i < inputTokens.size(); i++)
            {
                // pushing the C-string stored in the string of string vector inputTokens into char* vector inputTokensC
                inputTokensC.push_back(const_cast<char*>(inputTokens[i].c_str()));
            }
            // pushing a NULL into the last position of char* vector inputTokensC because execvp() excepts its arguments array to be NULL terminated
            inputTokensC.push_back(NULL);

            // handling for cd command
            if(inputTokens[0] == "cd")
            {
                // check if user entered a new path
                if(inputTokens.size() == 2)
                {
                    // call cd() with the new path, output error if cd() returns false
                    if(!(cd(pwd, inputTokensC[1])))
                    {
                        cout << "Path \"" << inputTokens[1] << "\" not found." << endl;
                    }
                }
            }
            // handling for other commands
            else
            {
                int childPid;
                if((childPid = fork()) == -1)
                {
                    cout << "Can't fork" << endl;
                    return EXIT_FAILURE;
                }
                else if(childPid == 0)
                {
                    // CHILD
                    // call execvp() with out char* vector inputTokensC, output error and exit if execvp() fails
                    if(execvp(inputTokensC[0], inputTokensC.data()) == -1)
                    {
                        cout << "Command \"" << inputTokensC[0] << "\" not found." << endl;
                        exit(EXIT_FAILURE);
                    }
                    // exit after execvp() ran successfully
                    exit(EXIT_SUCCESS);
                }
                else
                {
                    // PARENT
                    if(background == true)
                    {
                        // BACKGROUND
                        // output child PID and restart loop
                        cout << "[" << childPid << "]" << endl;
                    }
                    else
                    {
                        // FOREGROUND (blocking)
                        // wait until child calls exit(), then restart loop
                        waitpid(childPid, NULL, WUNTRACED | WCONTINUED);
                    }
                }
            }
        }
    }

    // Free allocated memory from get_current_dir_name() call
    delete pwd;
    return EXIT_SUCCESS;
}

// Returns true if directory change was successful, false otherwise
bool cd(char* &currPwd, char* newPwd)
{
    if(chdir(newPwd) == 0)
    {
        // Free allocated memory from get_current_dir_name() call
        delete currPwd;
        currPwd = get_current_dir_name();
        return true;
    }
    return false;
}