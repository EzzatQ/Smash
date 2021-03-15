//
// Created by USER on 16/11/2019.
//
#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
    cout << "smash: got ctrl-Z" << endl;
    SmallShell &smash = SmallShell::getInstance();
    int ret = smash.stopFgJob();
    if (ret != -1) cout << "smash: process " << smash.getLastExtCmdPid() << " was stopped" << endl;
	smash.setLastExtCmdPid(-1);
}

void ctrlCHandler(int sig_num) {
    cout << "smash: got ctrl-C" << endl;
    SmallShell &smash = SmallShell::getInstance();
    int ret = smash.killFgJob();
	if (ret != -1){
		cout << "smash: process " << smash.getLastExtCmdPid() << " was killed" << endl;
		smash.getJobs().removeJobByPid(smash.getLastExtCmdPid());
		smash.setLastExtCmdPid(-1);
	}

}
