#include <iostream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <sstream>
#include <vector>

using namespace std;

bool cd(char* &currPwd, char* newPwd);

int main(int argc, char* argv[], char* envp[])
{
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
        getline(cin, input);
        vector<string> inputTokens;
        string inputToken;
        stringstream inputStream(input);
        while(getline(inputStream, inputToken, ' '))
        {
            inputTokens.push_back(inputToken);
        }
        if(inputTokens.size() > 0)
        {
            vector<char*> inputTokensC;
            inputTokensC.reserve(inputTokens.size());
            for(int i = 0; i < inputTokens.size(); i++)
            {
                inputTokensC.push_back(const_cast<char*>(inputTokens[i].c_str()));
            }
            inputTokensC.push_back(NULL);

            if(inputTokens[0] == "cd" && inputTokens.size() == 2)
            {
                if(!(cd(pwd, inputTokensC[1])))
                {
                    cout << "Path \"" << inputTokens[1] << "\" not found." << endl;
                }
            }
            else if(execvp(inputTokensC[0], inputTokensC.data()) == -1)
            {
                cout << "Command \"" << inputTokensC[0] << "\" not found." << endl;
            }
        }
    }

    delete pwd;
    return EXIT_SUCCESS;
}

bool cd(char* &currPwd, char* newPwd)
{
    if(chdir(newPwd) == 0)
    {
        delete currPwd;
        if(!(currPwd = get_current_dir_name()))
        {
            cout << "Envp PWD not found." << endl;
            return EXIT_FAILURE;
        }
        return true;
    }
    return false;
}
