
#include "Commands.h"
#include <typeinfo>

using namespace std;

#if 0
#define FUNC_ENTRY()  \
cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

const std::string WHITESPACE = " \n\r\t\f\v";

bool StringIsIntNumber(char *str) {
	char *tmp = str;
	while (*tmp) {
		if (!isdigit(*tmp))return false;
		tmp++;
	}
	return true;
}

string _ltrim(const std::string &s) {
	size_t start = s.find_first_not_of(WHITESPACE);
	return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string &s) {
	size_t end = s.find_last_not_of(WHITESPACE);
	return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string &s) {
	return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char *cmd_line, char **args) {
	FUNC_ENTRY()
	int i = 0;
	std::istringstream iss(_trim(string(cmd_line)).c_str());
	for (std::string s; iss >> s;) {
		args[i] = (char *) malloc(s.length() + 1);
		memset(args[i], 0, s.length() + 1);
		strcpy(args[i], s.c_str());
		args[++i] = NULL;
	}
	return i;
	
	FUNC_EXIT()
}

char *removeExtraSpaces(const char *str) {
	char *nStr = (char *) malloc(strlen(str) + 1);
	int count = 0;
	bool space = false;
	for (int i = 0; str[i]; i++) {
		if (str[i] == ' ' && !space) {
			space = true;
			nStr[count++] = str[i];
			continue;
		} else if (str[i] == ' ' && space) {
			continue;
		} else if (str[i] != ' ') {
			nStr[count++] = str[i];
			space = false;
		}
	}
	nStr[count] = '\0';
	return nStr;
}

bool _isBackgroundComamnd(const char *cmd_line) {
	const string whitespace = " \t\n";
	const string str(cmd_line);
	return str[str.find_last_not_of(whitespace)] == '&';
}

void _removeBackgroundSign(char *cmd_line) {
	const string whitespace = " \t\n";
	const string str(cmd_line);
	// find last character other than spaces
	size_t idx = str.find_last_not_of(whitespace);
	// if all characters are spaces then return
	if (idx == string::npos) {
		return;
	}
	// if the command line does not end with & then return
	if (cmd_line[idx] != '&') {
		return;
	}
	// replace the & (background sign) with space and then remove all tailing spaces.
	cmd_line[idx] = ' ';
	// truncate the command line string up to the last non-space character
	cmd_line[str.find_last_not_of(whitespace, idx - 1) + 1] = 0;
}


/**
 * Creates and returns a pointer to Command class which matches the given command line (cmd_line)
 */
Command *SmallShell::CreateCommand(const char *cmd_line) {
	string cmd_s = string(cmd_line);
	char **args = new char *[COMMAND_MAX_ARGS + 1];
	char *tmp = removeExtraSpaces(cmd_line);
	int argNum = _parseCommandLine(tmp, args);
	free(tmp);
	if (argNum == 0){
		delete[] args;
		return nullptr;
	}
	if(argNum >= 3 && strcmp(args[argNum - 2], ">") == 0){
		for (int i = 0; i < argNum; i++) {
			free(args[i]);
		}
		delete[] args;
		return new RedirectionCommand(cmd_line, false);
	} else if(argNum >= 3 && strcmp(args[argNum - 2], ">>") == 0){
		for (int i = 0; i < argNum; i++) {
			free(args[i]);
		}
		delete[] args;
		return new RedirectionCommand(cmd_line, true);
	} else if (strcmp(args[0], "pwd") == 0) {
		for (int i = 0; i < argNum; i++) {
			free(args[i]);
		}
		delete[] args;
		return new GetCurrDirCommand(cmd_line);
	} else if (strcmp(args[0], "cd") == 0) {
		for (int i = 0; i < argNum; i++) {
			free(args[i]);
		}
		delete[] args;
		return new ChangeDirCommand(cmd_line, &this->lastPwd);
	} else if (strcmp(args[0], "history") == 0) {
		for (int i = 0; i < argNum; i++) {
			free(args[i]);
		}
		delete[] args;
		return new HistoryCommand(cmd_line, &this->cmdHistory);
	} else if (strcmp(args[0], "showpid") == 0) {
		for (int i = 0; i < argNum; i++) {
			free(args[i]);
		}
		delete[] args;
		return new ShowPidCommand(cmd_line);
		
	} else if (strcmp(args[0], "jobs") == 0) {
		for (int i = 0; i < argNum; i++) {
			free(args[i]);
		}
		delete[] args;
		return new JobsCommand(cmd_line, &this->jobs);
		
	} else if (strcmp(args[0], "kill") == 0) {
		for (int i = 0; i < argNum; i++) {
			free(args[i]);
		}
		delete[] args;
		return new KillCommand(cmd_line, &this->jobs);
		
	} else if (strcmp(args[0], "fg") == 0) {
		for (int i = 0; i < argNum; i++) {
			free(args[i]);
		}
		delete[] args;
		return new ForegroundCommand(cmd_line, &this->jobs);
		
	} else if (strcmp(args[0], "bg") == 0) {
		for (int i = 0; i < argNum; i++) {
			free(args[i]);
		}
		delete[] args;
		return new BackgroundCommand(cmd_line, &this->jobs);
		
	} else if ((strcmp(args[0], "quit") == 0)) {
		for (int i = 0; i < argNum; i++) {
			free(args[i]);
		}
		delete[] args;
		this->quitCommand = true;
		return new QuitCommand(cmd_line, &this->jobs);
		
	} else if((strcmp(args[0], "cp") == 0)){
		for (int i = 0; i < argNum; i++) {
			free(args[i]);
		}
		delete[] args;
		return new CopyCommand(cmd_line);
		
	} else {
		for (int i = 0; i < argNum; i++) {
			free(args[i]);
		}
		delete[] args;
		return new ExternalCommand(cmd_line);
	}
}

void SmallShell::executeCommand(const char *cmd_line) {
	Command *cmd;
	try{
		cmd = CreateCommand(cmd_line);
	} catch (std::bad_alloc& e){
		cout << "smash error: memory allocation failed" << endl;
		return;
	}
	if (!cmd) return;
	if(typeid(*cmd) == typeid(QuitCommand)) QCmd = cmd;
	if (cmd->isExt()) {
		if (cmd->isBg()) {
			pid_t p = fork();
			if (p == -1) perror("smash error: fork failed");
			if (p == 0) {
				setpgrp();
				char* const argv[4] = {(char*)"bash", (char*)"-c", cmd->getCmdLine(), nullptr};
				if (execv("/bin/bash", argv) == -1) {
					perror("smash error: execv failed");
					return;
				}
			} else {
				if (p != -1) jobs.addJob(cmd, p);
				jobs.removeFinishedJobs();
			}
		} else {
			pid_t p = fork();
			if (p == -1) perror("smash error: fork failed");
			if (p == 0) {
				setpgrp();
				char *const argv[4] = {(char*)"bash", (char*)"-c", cmd->getCmdLine(), nullptr};
				if (execv("/bin/bash", argv) == -1) {
					perror("smash error: execv failed");
					return;
				}
			} else {
				lastExtCmd = cmd;
				lastExtCmdPid = p;
				int wstatus;
				int ret = waitpid(p, &wstatus, WUNTRACED);
				if(!(WIFSTOPPED(wstatus)) && ret > 0){
					delete cmd;
				}
				lastExtCmdPid = -1;
				lastExtCmd = nullptr;
			}
		}
	} else {
		if(typeid(*cmd) == typeid(QuitCommand) || typeid(*cmd) == typeid(KillCommand)) jobs.removeFinishedJobs();
		cmd->execute();
		jobs.removeFinishedJobs();
		if(typeid(*cmd) == typeid(ChangeDirCommand)) {
			if (dynamic_cast<ChangeDirCommand *>(cmd)->getSuccess()) {
				if (lastPwd) free(lastPwd);
				lastPwd = currentPwd;
				currentPwd = getcwd(nullptr, 0);
				if(!currentPwd) perror("smash error: getpwd failed");
			}
		}
		if(typeid(*cmd) != typeid(QuitCommand)) delete cmd;
	}
}

