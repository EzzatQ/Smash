#include <iostream>
#include <unistd.h>
#include <string>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"

#define MAX_PROCESS_NUM (100)
#define MAX_PROCESS_NAME_LENGTH (50)


int main(int argc, char *argv[]) {
	if (signal(SIGTSTP, ctrlZHandler) == SIG_ERR) {
		perror("smash error: failed to set ctrl-Z handler");
	}
	if (signal(SIGINT, ctrlCHandler) == SIG_ERR) {
		perror("smash error: failed to set ctrl-C handler");
	}
	SmallShell& smash  = SmallShell::getInstance();
	while (true) {
		std::cout << "smash> ";
		std::string cmd_line;
		cin.clear();
		cin.sync();
		std::getline(std::cin, cmd_line);
		try {
			smash.InsertToHistory(cmd_line);
		} catch (std::bad_alloc& e){
			std::cout << "smash error: memory allocation failed" << std::endl;
			break;
		}
		smash.executeCommand(cmd_line.c_str());
		if (smash.isQuitCommand()) break;
	}
	delete smash.getQCmd();
	return 0;
}
