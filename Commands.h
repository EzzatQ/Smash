#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <unistd.h>
#include <signal.h>
#include <vector>
#include <string.h>
#include <iostream>
#include <list>
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include <stdlib.h>
#include <time.h>
#include <exception>
#include <sys/fcntl.h>


#define COMMAND_ARGS_MAX_LENGTH (80)
#define COMMAND_MAX_ARGS (20)
#define HISTORY_MAX_RECORDS (50)

using namespace std;


bool StringIsIntNumber(char *str);

void _removeBackgroundSign(char *cmd_line);

bool _isBackgroundComamnd(const char *cmd_line);

int _parseCommandLine(const char *cmd_line, char **args);

char *removeExtraSpaces(const char *str);

class Command {
protected:
    char *unProccessedCmd;
    char *cmdLine;// to return with error
    int argNum;
    char *proccessedCmd;
    char **args;// arg[0] is command
    char *cmd;
    bool Bg; //is Background
    bool Ext;
public:


    Command(const char *cmd_line) : Bg(false), Ext(false) {
        cmdLine = new char[strlen(cmd_line) + 1];
        strcpy(cmdLine, cmd_line);
        unProccessedCmd = new char[strlen(cmd_line) + 1];
        strcpy(unProccessedCmd, cmd_line);
        Bg = _isBackgroundComamnd(cmdLine);
        _removeBackgroundSign(cmdLine);
        args = new char *[COMMAND_MAX_ARGS + 1];
        char *tmp = removeExtraSpaces(cmd_line);
        proccessedCmd = new char[strlen(tmp) + 1];
        argNum = _parseCommandLine(tmp, args);
        cmd = args[0];
        free(tmp);
    }// in sons constructor, check arguments
    virtual ~Command() {
        delete[] cmdLine;
        for (int i = 0; i < argNum; i++) {
            free(args[i]);
        }
        delete[] args;
        delete[] unProccessedCmd;
        delete[] proccessedCmd;
    };

    bool isBg() {
        return Bg;
    }

    void setBg(bool bg) {
        Bg = bg;
    }

    char *getCmdLine() {
        return cmdLine;
    }

    char *getUnproccessedCmd() {
        return unProccessedCmd;
    }

    virtual void execute() = 0;

    bool isExt() { return Ext; }
    //virtual void prepare();
    //virtual void cleanup();
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char *cmd_line) : Command(cmd_line) {}

    virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
public:
    ExternalCommand(const char *cmd_line) : Command(cmd_line) {
        Ext = true;
    }

    virtual ~ExternalCommand() {}

    void execute() override {}
};

class PipeCommand : public Command {
    // TODO: Add your data members
public:
    PipeCommand(const char *cmd_line);

    virtual ~PipeCommand() {}

    void execute() override;
};

class ChangeDirCommand : public BuiltInCommand {
    char **lastPwd;
    bool success;
public:
    ChangeDirCommand(const char *cmd_line, char **plastPwd) : BuiltInCommand(cmd_line), lastPwd(plastPwd),
                                                              success(true) {}

    virtual ~ChangeDirCommand() {}

    void execute() override {
        int ret = 0;
        if (argNum > 2) {
            success = false;
            cout << "smash error: cd: too many arguments" << endl;
            return;
        }
        if (argNum == 1) {
            ret = chdir(getenv("HOME"));
        }
        if (argNum == 2) {
            if (!strcmp(args[1], "-")) {
                if (*lastPwd) {
                    ret = chdir(*lastPwd);
                } else {
                    success = false;
                    cout << "smash error: cd: OLDPWD not set" << endl;
                }
            } else ret = chdir(args[1]);
            if (ret == -1) {
                success = false;
                perror("smash error: chdir failed");
            }
        }
    }

    bool getSuccess() { return success; }
};

class GetCurrDirCommand : public BuiltInCommand {
public:
    GetCurrDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

    virtual ~GetCurrDirCommand() {}

    void execute() override {
        char *buf = getcwd(nullptr, 0);
        if (!buf) perror("smash error: getcwd failed");
        cout << buf << endl;
        free(buf);
    }
};

class ShowPidCommand : public BuiltInCommand {
public:
    ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

    virtual ~ShowPidCommand() {}

    void execute() override {

        cout << "smash pid is " << getpid() << endl;
    }
};

class CommandsHistory {
protected:

    class CommandHistoryEntry {
        string cmd;
        int seqNum;
    public:
        CommandHistoryEntry(string &cmd, int seqNum) : cmd(cmd), seqNum(seqNum) {}

        string &getCmd() {
            return cmd;
        }

        void setSeqNum(int seqNum) {
            this->seqNum = seqNum;
        }

        int getSeqNum() const {
            return seqNum;
        }
    };

    int commandCount;
    list<CommandHistoryEntry> *history;
    int CountNum;
    int RealSize;
    int limit;

public:
    CommandsHistory(int limit = HISTORY_MAX_RECORDS) : history(new list<CommandHistoryEntry>), CountNum(0), RealSize(0),
                                                       limit(limit) {}

    ~CommandsHistory() {
        delete history;
    }

    void addRecord(string cmd_line) {// i've changed it from char* to string

        auto LastInsertedCmd = history->begin();
        if (RealSize != 0 && cmd_line == LastInsertedCmd.operator*().getCmd()) {
            LastInsertedCmd.operator*().setSeqNum(++CountNum);
            return;
        }
        CommandHistoryEntry chEntry(cmd_line, ++CountNum);
        if (RealSize >= limit) {
            history->pop_back();
            history->push_front(chEntry);
        } else {
            history->push_front(chEntry);
            RealSize++;
        }


    }

    void printHistory() {
        for (list<CommandHistoryEntry>::reverse_iterator it = this->history->rbegin();
             it != this->history->rend(); ++it) {
            cout << right << setw(5) << (*it).getSeqNum() << "  " << (*it).getCmd() << endl;
        }

    }
};

class HistoryCommand : public BuiltInCommand {
    CommandsHistory *history;
public:
    HistoryCommand(const char *cmd_line, CommandsHistory *history) : BuiltInCommand(cmd_line), history(history) {}

    virtual ~HistoryCommand() {}

    void execute() override {
        history->printHistory();
    }
};

class JobsList {
public:
    class JobEntry {
        Command *cmd;
        bool isStopped;
        int jobId;
        pid_t PID;
        time_t insertionTime;
        bool bg;
    public:
        JobEntry(Command *cmd, bool isStopped, int jobId, pid_t PID, time_t insertionTime, bool bg = true) : cmd(cmd),
                                                                                                             isStopped(
                                                                                                                     isStopped),
                                                                                                             jobId(jobId),
                                                                                                             PID(PID),
                                                                                                             insertionTime(
                                                                                                                     insertionTime),
                                                                                                             bg(bg) {}

        Command *getCmd() const {
            return cmd;
        }

        void setBg(bool bg) {
            JobEntry::bg = bg;
        }

        bool isBg() const {
            return bg;
        }

        int getJobId() const {
            return jobId;
        }

        void setIsStopped(bool isStopped) {
            JobEntry::isStopped = isStopped;
        }

        void setInsertionTime(time_t insertionTime) {
            JobEntry::insertionTime = insertionTime;
        }

        bool getIsStopped() const {
            return isStopped;
        }

        pid_t getPid() const {
            return PID;
        }

        time_t getInsertionTime() const {
            return insertionTime;
        }

        void resetTimer() {
            insertionTime = time(nullptr);
        }

    };

    list<JobEntry> *entries;
    int jobCount;
public:
    JobsList() : entries(new list<JobEntry>()), jobCount(0) {}

    ~JobsList() {
        delete entries;
    }

    void addJob(Command *cmd, pid_t PID, bool isStopped = false) {
        JobEntry tmp = JobEntry((cmd), isStopped, ++jobCount, PID, time(nullptr));
        entries->push_front(tmp);
    }

    void printJob(JobEntry &JE) {
        cout << "[" << JE.getJobId() << "]" << " " << JE.getCmd()->getUnproccessedCmd() << ": " << JE.getPid() << " "
             << difftime(time(nullptr), JE.getInsertionTime()) << " secs";
        if (JE.getIsStopped()) cout << " (stopped)";
        cout << endl;
    }

    void printJobsList() {
        for (list<JobEntry>::reverse_iterator it = this->entries->rbegin(); it != this->entries->rend(); ++it) {
            if ((*it).isBg() || (*it).getIsStopped())
                printJob(*it);
        }
    }

    bool empty() {
        return entries->empty();
    }

    void killAllJobs() {
        for (list<JobEntry>::iterator it = this->entries->begin(); it != this->entries->end(); ++it) {
            int killRet = kill((*it).getPid(), SIGKILL);
            if (killRet == -1) perror("smash error: kill failed");
            else delete it->getCmd();
        }
    }

    void PrintQuitMsg() {
        cout << "smash: sending SIGKILL signal to " << this->entries->size() << " jobs:" << endl;
        for (list<JobEntry>::iterator it = this->entries->begin(); it != this->entries->end(); ++it) {
            cout << (*it).getPid() << ": " << (*it).getCmd()->getUnproccessedCmd() << endl;
        }
    }

    void removeFinishedJobs() {
        for (list<JobEntry>::iterator it = this->entries->begin(); it != this->entries->end(); ++it) {
            pid_t wait_ret = waitpid((*it).getPid(), nullptr, WNOHANG);
            if (wait_ret == -1) perror("smash error: waitpid failed");
            else if (wait_ret != 0) {
                if (!(it->getCmd())) delete it->getCmd();
                it = this->entries->erase(it);
            }
        }
    }

    JobEntry *getJobById(int jobId) {
        removeFinishedJobs();
        for (list<JobEntry>::iterator it = this->entries->begin(); it != this->entries->end(); ++it) {
            if ((*it).getJobId() == jobId) return &(*it);
        }
        return nullptr;
    }

    JobEntry *getJobByPid(int jobPid) {
        for (list<JobEntry>::iterator it = this->entries->begin(); it != this->entries->end(); ++it) {
            if ((*it).getPid() == jobPid) return &(*it);
        }
        return nullptr;
    }

    void removeJobByPid(int jobPid) {
        for (list<JobEntry>::iterator it = this->entries->begin(); it != this->entries->end(); ++it) {
            if ((*it).getPid() == jobPid) {
                if (!it->getCmd()) delete it->getCmd();
                this->entries->erase(it);
                return;
            }
        }
    }

    void removeJobById(int jobId) {
        for (list<JobEntry>::iterator it = this->entries->begin(); it != this->entries->end(); ++it) {
            if ((*it).getJobId() == jobId) {
                delete it->getCmd();
                it = this->entries->erase(it);
            }
        }
    }

    pid_t getJobPidByJobId(int jobId) {
        for (list<JobEntry>::iterator it = this->entries->begin(); it != this->entries->end(); ++it) {
            if ((*it).getJobId() == jobId)
                return (*it).getPid();
        }
        return -1;
    }

    JobEntry *getLastJob(int *lastJobId) {
        removeFinishedJobs();
        list<JobEntry>::iterator it = this->entries->begin();
        *lastJobId = (*it).getJobId();
        return &(*it);
    }

    JobEntry *getLastStoppedJob(int *jobId) {
        removeFinishedJobs();
        if (jobId) {
            for (list<JobEntry>::iterator it = this->entries->begin(); it != this->entries->end(); ++it) {
                if ((*it).getIsStopped()) {
                    *jobId = (*it).getJobId();

                    return &(*it);
                }
            }

        }
        return nullptr;
    }

    bool exists(pid_t pid) {
        for (list<JobEntry>::iterator it = this->entries->begin(); it != this->entries->end(); ++it) {
            if ((*it).getPid() == pid) return true;
        }
        return false;
    }

};

class QuitCommand : public BuiltInCommand {
    JobsList *jobs;

public:
    QuitCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), jobs(jobs) {}

    virtual ~QuitCommand() {}

    void execute() override {
        if (argNum >= 2 && strcmp(this->args[1], "kill") == 0) {
            jobs->PrintQuitMsg();
        }
        jobs->killAllJobs();

    }

};

class JobsCommand : public BuiltInCommand {
    JobsList *jobs;
public:
    JobsCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), jobs(jobs) {}

    virtual ~JobsCommand() {}

    void execute() override {
        if (jobs->entries->size() != 0) {
            jobs->removeFinishedJobs();
            jobs->printJobsList();
        }
    }
};

class KillCommand : public BuiltInCommand {
    JobsList *jobs;

public:
    KillCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), jobs(jobs) {}

    virtual ~KillCommand() {}

    void execute() override {
        if (argNum != 3) {
            cout << "smash error: kill: invalid arguments" << endl;
            return;
        }
        string arg1 = string(args[1]);
        if (arg1.find("-") == 0) {
            char *tmp = args[1];
            tmp++;
            if (!StringIsIntNumber((tmp))) {
                cout << "smash error: kill: invalid arguments" << endl;
                return;
            }

        } else {
            cout << "smash error: kill: invalid arguments" << endl;
            return;
        }
        if (!StringIsIntNumber(args[2])) {
            cout << "smash error: kill: invalid arguments" << endl;
            return;
        }
        char *sigNum = args[1];
        sigNum++;
        char *jobId = args[2];
        pid_t pid = jobs->getJobPidByJobId(stoi(jobId));
        if (pid == -1) {
            cout << "smash error: kill: job-id " << jobId << " does not exist" << endl;;
            return;
        }
        int sigNumber = stoi(sigNum);
        int killRet = kill(pid, sigNumber);
        if (killRet == 0) {
            cout << "signal number " << sigNum << " was sent to pid " << pid << endl;
            if (sigNumber == SIGTSTP || sigNumber == SIGSTOP) {
                JobsList::JobEntry *tmp = jobs->getJobById(stoi(jobId));
                if (tmp) tmp->setIsStopped(true);
            } else if (sigNumber == SIGINT || sigNumber == SIGKILL) {
                jobs->removeJobById(stoi(jobId));
            }
            if (sigNumber == SIGCONT) {
                JobsList::JobEntry *tmp = jobs->getJobById(stoi(jobId));
                if (tmp && tmp->getIsStopped()) {
                    tmp->setIsStopped(false);
                    tmp->resetTimer();
                }
            }
        } else
            perror("smash error: kill failed");
    }
};

class CopyCommand : public BuiltInCommand {

public:
    CopyCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

    virtual ~CopyCommand() {}

    void execute() override {

        if (argNum == 3) {
            int oldFile = open(args[1], O_RDONLY);
            if (oldFile == -1) {
                perror("smash error: open failed");
                return;
            }
            int newFile = open(args[2], O_TRUNC | O_RDWR | O_CREAT, 0644);
            if (newFile == -1) {
                perror("smash error: open failed");
                int ret1 = close(oldFile);
                if (ret1 == -1) {
                    perror("smash error: close failed");
                    return;
                }
            }
                char *buffer = new char[100];
                while (true) {
                    ssize_t ret = read(oldFile, buffer, 100);
                    if (ret == 0) break;
                    if (ret == -1) {
                        perror("smash error: read failed");
                        int ret1 = close(newFile);
                        if (ret1 == -1) perror("smash error: close failed");
                        int ret2 = close(oldFile);
                        if (ret2 == -1) perror("smash error: close failed");
                        delete[] buffer;
                        return;
                    }
                    ssize_t Wret = write(newFile, buffer, ret);
                    if (Wret == -1) {
                        perror("smash error: write failed");
                        int ret1 = close(newFile);
                        if (ret1 == -1) perror("smash error: close failed");
                        int ret2 = close(oldFile);
                        if (ret2 == -1) perror("smash error: close failed");
                        delete[] buffer;
                        return;
                    }
                }

                if (close(newFile) == -1) perror("smash error: close failed");
                int ret2 = close(oldFile);
                if (ret2 == -1) perror("smash error: close failed");
                delete[] buffer;
                cout << "smash: " << args[1] << " was copied to " << args[2] << endl;
            }
        }

};

class SmallShell {
private:

    char *currentPwd;
    char *lastPwd;
    CommandsHistory cmdHistory;
    JobsList jobs;
    pid_t lastExtCmdPid;
    Command *lastExtCmd;
    bool quitCommand;
    Command *QCmd;

public:
    SmallShell() try : cmdHistory(), jobs(), lastExtCmdPid(-1), lastExtCmd(nullptr), quitCommand(false) {
        currentPwd = getcwd(nullptr, 0);
        lastPwd = nullptr;
    } catch (std::bad_alloc &e) {
        cout << "smash error: memory allocation error" << endl;
    }

    Command *CreateCommand(const char *cmd_line);

    void InsertToHistory(string &cmd_line) {
        cmdHistory.addRecord(cmd_line);
    }

    SmallShell(SmallShell const &) = delete; // disable copy ctor
    void operator=(SmallShell const &) = delete; // disable = operator

    static SmallShell &getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }


    ~SmallShell() {
        if (lastPwd) free(lastPwd);
        if (currentPwd) free(currentPwd);

    };

    void executeCommand(const char *cmd_line);

    JobsList &getJobs() {
        return jobs;
    }

    int stopFgJob() {
        if (lastExtCmdPid == -1) return -1;
        int ret = kill(lastExtCmdPid, SIGSTOP);
        if (ret == -1) perror("smash error: kill failed");
        else if (!jobs.exists(lastExtCmdPid)) jobs.addJob(lastExtCmd, lastExtCmdPid, true);
        else {
            JobsList::JobEntry *tmp = jobs.getJobByPid(lastExtCmdPid);
            tmp->setIsStopped(true);
            tmp->resetTimer();
        }
        return ret;
    }

    int killFgJob() {
        if (lastExtCmdPid == -1) return -1;
        int ret = kill(lastExtCmdPid, SIGKILL);
        if (ret == -1) perror("smash error: kill failed");
        return ret;
    }

    void setLastExtCmdPid(pid_t lastExtCmdPid) {
        SmallShell::lastExtCmdPid = lastExtCmdPid;
    }

    void setLastExtCmd(Command *lastExtCmd) {
        SmallShell::lastExtCmd = lastExtCmd;
    }

    pid_t getLastExtCmdPid() {
        return lastExtCmdPid;
    }


    bool isQuitCommand() const {
        return quitCommand;
    }

    Command *getQCmd() {
        return QCmd;
    }

    void setQCmd(Command *cmd) {
        QCmd = cmd;
    }

    char *getCurrentPwd() {
        return currentPwd;
    }

    void setCurrentPwd(char *pwd) {
        currentPwd = pwd;
    }

    char *getLastPwd() {
        return lastPwd;
    }

    void setLastPwd(char *pwd) {
        lastPwd = pwd;
    }

};

class ForegroundCommand : public BuiltInCommand {
    JobsList *jobs;
public:
    ForegroundCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), jobs(jobs) {}

    virtual ~ForegroundCommand() {}

    void execute() override {
        if (argNum > 2) {
            cout << "smash error: fg: invalid arguments" << endl;
            return;
        }

        int jobId;
        SmallShell &smash = SmallShell::getInstance();
        if (argNum == 1) {
            if (jobs->entries->size() == 0) {
                cout << "smash error: fg: job list is empty" << endl;
                return;
            }
            JobsList::JobEntry *tmp = jobs->getLastJob(&jobId);
            if (!tmp) {
                cout << "smash error: fg: job list is empty" << endl;
                return;
            }
            cout << tmp->getCmd()->getUnproccessedCmd() << " : " << tmp->getPid() << endl;

            tmp->getCmd()->setBg(false);
            tmp->setBg(false);
            if (tmp->getIsStopped()) {
                int killRet = kill(tmp->getPid(), SIGCONT);
                if (killRet != 0) perror("smash error: kill failed");
                tmp->setIsStopped(false);
            }
            smash.setLastExtCmd(tmp->getCmd());
            smash.setLastExtCmdPid(tmp->getPid());
            int ret = waitpid(tmp->getPid(), nullptr, WUNTRACED);
            if (!tmp->getIsStopped() && ret > 0) {
                jobs->removeJobById(tmp->getJobId());
            }
            return;
        }
        int jobID;
        try {
            jobID = stoi(args[1]);

        } catch (std::invalid_argument &e) {
            cout << "smash error: fg: invalid arguments" << endl;
            return;
        }
        if (jobID < 1) {
            cout << "smash error: fg: invalid arguments" << endl;
            return;
        }
        JobsList::JobEntry *tmp = jobs->getJobById(jobID);
        if (!tmp) {
            cout << "smash error: fg: job-id " << jobID << " does not exist" << endl;
            return;
        }

        cout << tmp->getCmd()->getUnproccessedCmd() << " : " << tmp->getPid() << endl;

        tmp->getCmd()->setBg(false);
        tmp->setBg(false);
        if (tmp->getIsStopped()) {
            int killRet = kill(tmp->getPid(), SIGCONT);
            if (killRet != 0) perror("smash error: kill failed");
        }
        smash.setLastExtCmd(tmp->getCmd());
        smash.setLastExtCmdPid(tmp->getPid());

        int ret = waitpid(tmp->getPid(), nullptr, WUNTRACED);
        if (!tmp->getIsStopped() && ret > 0) {
            jobs->removeJobById(tmp->getJobId());
        }
    }// if job id is specified, bring that job to foreground else bring last job to fg
};

class BackgroundCommand : public BuiltInCommand {
    JobsList *jobs;

public:
    BackgroundCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), jobs(jobs) {}

    virtual ~BackgroundCommand() {}

    void execute() override {
        if (argNum > 2) {

            cout << "smash error: bg: invalid arguments" << endl;
            return;
        }
        SmallShell &smash = SmallShell::getInstance();
        if (argNum == 2) {

            int jobID;

            try {
                jobID = stoi(args[1]);

            } catch (std::invalid_argument &e) {
                cout << "smash error: bg: invalid arguments" << endl;
                return;
            }
            if (jobID < 1) {
                cout << "smash error: bg: invalid arguments" << endl;
                return;
            }
            JobsList::JobEntry *tmp = jobs->getJobById(jobID);

            if (!tmp) {
                cout << "smash error: bg: job-id " << jobID << " does not exist" << endl;
                return;
            }

            if (!tmp->getIsStopped()) {
                cout << "smash error: bg: job-id " << jobID << " is already running in the background" << endl;
                return;
            }
            cout << tmp->getCmd()->getUnproccessedCmd() << " : " << tmp->getPid() << endl;
            tmp->getCmd()->setBg(true);
            tmp->setBg(true);
            if (tmp->getIsStopped()) {
                int killRet = kill(tmp->getPid(), SIGCONT);
                if (killRet != 0) perror("smash error: kill failed");
                tmp->setIsStopped(false);
                tmp->resetTimer();
            }
            smash.setLastExtCmd(tmp->getCmd());
            smash.setLastExtCmdPid(tmp->getPid());
        }
        if (jobs->entries->size() == 0) {
            cout << "smash error: bg: there is no stopped jobs to resume" << endl;
            return;
        }
        int jobId;


        if (argNum == 1) {
            JobsList::JobEntry *tmp = jobs->getLastStoppedJob(&jobId);
            if (!tmp) {
                cout << "smash error: bg: there is no stopped jobs to resume" << endl;
                return;
            }
            if (!tmp->getIsStopped()) {
                cout << "smash error: bg: job-id " << jobId << " is already running in the background" << endl;
                return;
            }
            cout << tmp->getCmd()->getUnproccessedCmd() << " : " << tmp->getPid() << endl;

            tmp->getCmd()->setBg(true);
            tmp->setBg(true);

            if (tmp->getIsStopped()) {
                int killRet = kill(tmp->getPid(), SIGCONT);
                if (killRet != 0) perror("smash error: kill failed");
                tmp->setIsStopped(false);
                tmp->resetTimer();
            }
            smash.setLastExtCmd(tmp->getCmd());
            smash.setLastExtCmdPid(tmp->getPid());
            return;
        }


    }
};

class RedirectionCommand : public Command {
    Command *cmd;
    bool append;

public:
    explicit RedirectionCommand(const char *cmd_line, bool append) : Command(cmd_line), append(append) {}

    virtual ~RedirectionCommand() {}

    void execute() override {
        SmallShell &smash = SmallShell::getInstance();
        long index = string(unProccessedCmd).find(">");
        string tmp = string(unProccessedCmd).substr(0, index);
        try {
            cmd = smash.CreateCommand(tmp.c_str());
        } catch (std::bad_alloc &e) {
            cout << "smash error: memory allocation failed" << endl;
            return;
        }
        if (typeid(*cmd) == typeid(QuitCommand)) smash.setQCmd(cmd);

        if (!cmd) return;
        if (cmd->isExt()) {
            if (cmd->isBg()) {
                pid_t p = fork();
                if (p == -1) perror("smash error: fork failed");
                if (p == 0) {
                    setpgrp();
                    int outFd = dup(1);
                    if (outFd == -1) {
                        perror("smash error: dup failed");
                        if (typeid(*cmd) != typeid(QuitCommand)) delete cmd;
                        return;
                    }
                    if (close(1) == -1) {
                        perror("smash error: close failed");
                        if (typeid(*cmd) != typeid(QuitCommand)) delete cmd;
                        if (close(outFd) == -1) perror("smash error: close failed");
                        return;
                    }
                    if (!append) {
                        if (open(args[argNum - 1], O_RDWR | O_CREAT, 0644) == -1) {
                            perror("smash error: open failed");
                            if (typeid(*cmd) != typeid(QuitCommand)) delete cmd;
                            if (dup2(outFd, 1) == -1) perror("smash error: dup2 failed");
                            if (close(outFd) == -1) perror("smash error: close failed");
                            return;
                        }
                    } else {
                        if (open(args[argNum - 1], O_APPEND | O_RDWR | O_CREAT, 0644) == -1) {
                            perror("smash error: open failed");
                            if (typeid(*cmd) != typeid(QuitCommand)) delete cmd;
                            if (dup2(outFd, 1) == -1) perror("smash error: dup2 failed");
                            if (close(outFd) == -1) perror("smash error: close failed");
                            return;
                        }
                    }
                    char *const argv[4] = {(char *) "bash", (char *) "-c", cmd->getCmdLine(), nullptr};
                    if (execv("/bin/bash", argv) == -1) {
                        perror("smash error: execv failed");
                        return;
                    }
                } else {
                    if (p != -1) smash.getJobs().addJob(cmd, p);
                    smash.getJobs().removeFinishedJobs();
                }
            } else {
                pid_t p = fork();
                if (p == -1) perror("smash error: fork failed");
                if (p == 0) {
                    setpgrp();
                    int outFd = dup(1);
                    if (outFd == -1) {
                        perror("smash error: dup failed");
                        if (typeid(*cmd) != typeid(QuitCommand)) delete cmd;
                        return;
                    }
                    if (close(1) == -1) {
                        perror("smash error: close failed");
                        if (typeid(*cmd) != typeid(QuitCommand)) delete cmd;
                        if (close(outFd) == -1) perror("smash error: close failed");
                        return;
                    }
                    if (!append) {
                        if (open(args[argNum - 1], O_RDWR | O_CREAT, 0644) == -1) {
                            perror("smash error: open failed");
                            if (typeid(*cmd) != typeid(QuitCommand)) delete cmd;
                            if (dup2(outFd, 1) == -1) perror("smash error: dup2 failed");
                            if (close(outFd) == -1) perror("smash error: close failed");
                            return;
                        }
                    } else {
                        if (open(args[argNum - 1], O_APPEND | O_RDWR | O_CREAT, 0644) == -1) {
                            perror("smash error: open failed");
                            if (typeid(*cmd) != typeid(QuitCommand)) delete cmd;
                            if (dup2(outFd, 1) == -1) perror("smash error: dup2 failed");
                            if (close(outFd) == -1) perror("smash error: close failed");
                            return;
                        }
                    }
                    char *const argv[4] = {(char *) "bash", (char *) "-c", cmd->getCmdLine(), nullptr};
                    if (execv("/bin/bash", argv) == -1) {
                        perror("smash error: execv failed");
                        return;
                    }
                } else {
                    smash.setLastExtCmd(cmd);
                    smash.setLastExtCmdPid(p);
                    int wstatus;
                    int ret = waitpid(p, &wstatus, WUNTRACED);
                    if (!WIFSTOPPED(wstatus) && ret > 0) {
                        delete cmd;
                    }
                    smash.setLastExtCmd(nullptr);
                    smash.setLastExtCmdPid(p);
                }
            }
        } else {
            if (typeid(*cmd) == typeid(QuitCommand) || typeid(*cmd) == typeid(KillCommand))
                smash.getJobs().removeFinishedJobs();
            int outFd = dup(1);
            if (outFd == -1) {
                perror("smash error: dup failed");
                if (typeid(*cmd) != typeid(QuitCommand)) delete cmd;
                return;
            }
            if (close(1) == -1) {
                perror("smash error: close failed");
                if (typeid(*cmd) != typeid(QuitCommand)) delete cmd;
                if (close(outFd) == -1) perror("smash error: close failed");
                return;
            }
            if (!append) {
                if (open(args[argNum - 1], O_RDWR | O_CREAT, 0644) == -1) {
                    perror("smash error: open failed");
                    if (typeid(*cmd) != typeid(QuitCommand)) delete cmd;
                    if (dup2(outFd, 1) == -1) perror("smash error: dup2 failed");
                    if (close(outFd) == -1) perror("smash error: close failed");
                    return;
                }
            } else {
                if (open(args[argNum - 1], O_APPEND | O_RDWR | O_CREAT, 0644) == -1) {
                    perror("smash error: open failed");
                    if (typeid(*cmd) != typeid(QuitCommand)) delete cmd;
                    if (dup2(outFd, 1) == -1) perror("smash error: dup2 failed");
                    if (close(outFd) == -1) perror("smash error: close failed");
                    return;
                }
            }
            cmd->execute();
            if (close(1) == -1) {
                perror("smash error: close failed");
                if (typeid(*cmd) != typeid(QuitCommand)) delete cmd;
                smash.getJobs().removeFinishedJobs();
                if (close(outFd) == -1) perror("smash error: close failed");
                return;
            }
            if (dup2(outFd, 1) == -1) {
                perror("smash error: dup2 failed");
                if (typeid(*cmd) != typeid(QuitCommand)) delete cmd;
                if (close(outFd) == -1) perror("smash error: close failed");
                smash.getJobs().removeFinishedJobs();
                return;
            }
            smash.getJobs().removeFinishedJobs();
            if (typeid(*cmd) == typeid(ChangeDirCommand)) {
                if (dynamic_cast<ChangeDirCommand *>(cmd)->getSuccess()) {
                    if (smash.getLastPwd()) free(smash.getLastPwd());
                    smash.setLastPwd(smash.getCurrentPwd());
                    char *cwd = getcwd(nullptr, 0);
                    if (!cwd) perror("smash error: getcwd failed");
                    else smash.setCurrentPwd(cwd);
                }
            }
            if (typeid(*cmd) != typeid(QuitCommand)) delete cmd;
        }
    }
};


#endif //SMASH_COMMAND_H_
