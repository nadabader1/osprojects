#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <algorithm>
#include <queue>

using namespace std;

struct PCB {
    int id;
    int arrivalTime;
    int burstTime;
    int remainingTime;
    int startTime;
    int finishTime;
    int waitingTime;
    int turnaroundTime;
    float contextswitch;
};

vector<PCB> readInputFromFile(const string& filename) {
    vector<PCB> pcbs;
    ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        cerr << "Error: Unable to open file " << filename << endl;
        exit(1);
    }
    string line;
    while (getline(inputFile, line)) {
        stringstream ss(line);
        PCB pcb;



        ss >> pcb.id >> pcb.arrivalTime >> pcb.burstTime >> pcb.contextswitch;
        pcb.remainingTime = pcb.burstTime;
        pcb.startTime = -1;
        pcbs.push_back(pcb);
    }
    inputFile.close();
    return pcbs;
}
int IdealTime;
int numOfContextSwitch;
void FCFS(vector<PCB>& pcbs) {
    int currentTime = 0;
    IdealTime = 0;
    numOfContextSwitch = 0;
    for (size_t i = 0; i < pcbs.size(); ++i) {
        PCB& pcb = pcbs[i];
        if (currentTime < pcb.arrivalTime) {
            IdealTime += pcb.arrivalTime - currentTime;
            currentTime = pcb.arrivalTime;
        }
        if (i > 0) { // Check if it's not the first process
            numOfContextSwitch++;
        }
        if (pcb.startTime == -1) {
            pcb.startTime = currentTime;
        }
        else {
            pcb.startTime = currentTime + pcb.contextswitch;
        }
        currentTime += pcb.burstTime;
        pcb.finishTime = currentTime;
        pcb.waitingTime = pcb.startTime - pcb.arrivalTime;
        pcb.turnaroundTime = pcb.finishTime - pcb.arrivalTime;
    }

}


struct ComparePCB {
    bool operator()(PCB* a, PCB* b) {
        return a->remainingTime > b->remainingTime; // Min-heap based on remainingTime
    }
};

void SRTF(vector<PCB>& pcbs) {
    sort(pcbs.begin(), pcbs.end(), [](const PCB& a, const PCB& b) {
        return a.arrivalTime < b.arrivalTime;
        });

    IdealTime = 0;
    numOfContextSwitch = 0;
    priority_queue<PCB*, vector<PCB*>, ComparePCB> pq;

    int currentTime = 0;
    size_t idx = 0; // Index to keep track of the next process
    int completed = 0;

    while (completed < pcbs.size()) {
        // Check if there are processes arriving at the current time
        while (idx < pcbs.size() && pcbs[idx].arrivalTime <= currentTime) {
            pq.push(&pcbs[idx]);
            idx++;
        }

        // If no process is currently arriving and the queue is empty,
        // move the current time to the next arrival time
        if (pq.empty() && idx < pcbs.size()) {
            IdealTime += pcbs[idx].arrivalTime - currentTime;
            currentTime = pcbs[idx].arrivalTime;
        }

        if (!pq.empty()) {
            PCB* current = pq.top();
            pq.pop();
            if (current->startTime == -1) {
                current->startTime = currentTime;
            }
            else {
                current->startTime = currentTime + current->contextswitch;
                numOfContextSwitch++;

            }
            currentTime++;
            current->remainingTime--;

            if (current->remainingTime == 0) {
                current->finishTime = currentTime;
                current->waitingTime = current->finishTime - current->arrivalTime - current->burstTime;
                current->turnaroundTime = current->finishTime - current->arrivalTime;
                completed++;
            }
            else {
                pq.push(current);
            }
        }
        else {
            currentTime++;
        }
    }
}


void RoundRobin(vector<PCB>& pcbs, int quantum) {
    IdealTime = 0;
    numOfContextSwitch = 0;
    int currentTime = 0;
    queue<PCB*> q;
    vector<PCB*> pcbs_ptrs(pcbs.size());
    for (int i = 0; i < pcbs.size(); ++i) {
        pcbs_ptrs[i] = &pcbs[i];
    }

    sort(pcbs_ptrs.begin(), pcbs_ptrs.end(), [](PCB* a, PCB* b) {
        return a->arrivalTime < b->arrivalTime;
        });

    size_t idx = 0;
    while (idx < pcbs_ptrs.size() && pcbs_ptrs[idx]->arrivalTime <= currentTime) {
        q.push(pcbs_ptrs[idx]);
        idx++;
    }
    if (q.empty() && idx < pcbs_ptrs.size()) {
        // Calculate the ideal time when there are no processes in the queue
        int nextArrivalTime = pcbs_ptrs[idx]->arrivalTime;
        IdealTime += nextArrivalTime - currentTime;
        currentTime = nextArrivalTime;
    }
    while (!q.empty()) {
        PCB* current = q.front();
        q.pop();

        if (current->startTime == -1) {
            current->startTime = currentTime;
        }
        else {
            current->startTime = currentTime + current->contextswitch;
            numOfContextSwitch++;
        }

        int timeSlice = min(quantum, current->remainingTime);
        currentTime += timeSlice;
        current->remainingTime -= timeSlice;

        // Check if other processes have arrived by now
        while (idx < pcbs_ptrs.size() && pcbs_ptrs[idx]->arrivalTime <= currentTime) {
            q.push(pcbs_ptrs[idx]);
            idx++;
        }

        if (current->remainingTime > 0) {
            q.push(current); // Re-queue if there's remaining time
        }
        else {
            current->finishTime = currentTime;
            current->waitingTime = current->finishTime - current->arrivalTime - current->burstTime;
            current->turnaroundTime = current->finishTime - current->arrivalTime;
        }
    }
}
void calculateMetrics(const vector<PCB>& pcbs) {
    double totalWaitingTime = 0;
    double totalTurnaroundTime = 0;
    int totalBurstTime = 0;
    int contextswitchDuration = 0;
    for (const PCB& pcb : pcbs) {
        totalWaitingTime += pcb.waitingTime;
        totalTurnaroundTime += pcb.turnaroundTime;
        totalBurstTime += pcb.burstTime;
        contextswitchDuration = pcb.contextswitch;
    }
    double averageWaitingTime = totalWaitingTime / pcbs.size();
    double averageTurnaroundTime = totalTurnaroundTime / pcbs.size();
    double cpuUtilization = 100.0 * totalBurstTime / (totalBurstTime + (numOfContextSwitch * contextswitchDuration) + IdealTime);

    cout << "Total Waiting Time: " << totalWaitingTime << endl;
    cout << "Average Waiting Time: " << averageWaitingTime << endl;
    cout << "Total Turnaround Time: " << totalTurnaroundTime << endl;
    cout << "Average Turnaround Time: " << averageTurnaroundTime << endl;


    cout << "CPU Utilization: " << cpuUtilization << "%" << endl;
}
void printResults(const vector<PCB>& pcbs) {
    cout << "Process ID\tArrival Time\tBurst Time\tFinish Time\tWaiting Time\tTurnaround Time\tcontextswitch Time\n";
    for (const PCB& pcb : pcbs) {
        cout << pcb.id << "\t\t"
            << pcb.arrivalTime << "\t\t"
            << pcb.burstTime << "\t\t"
            << pcb.finishTime << "\t\t" << pcb.waitingTime << "\t\t"
            << pcb.turnaroundTime << "\t\t" << pcb.contextswitch << endl;
    }
}

int main() {
    string filename = "processes.txt";
    vector<PCB> pcbs = readInputFromFile(filename);
    int choice;
    do {
        cout << "=======================================\n"
            << "1) First-Come First-Served (FCFS)\n"
            << "2) Shortest Remaining Time First (SRTF)\n"
            << "3) Round-Robin (RR)\n"
            << "4) Exit Program\n"
            << "Enter your choice: ";
        cin >> choice;
        switch (choice) {
        case 1:
            FCFS(pcbs);
            printResults(pcbs);
            calculateMetrics(pcbs);
            system("pause");
            break;

        case 2:
            SRTF(pcbs);
            printResults(pcbs);
            calculateMetrics(pcbs);
            system("pause");
            break;

        case 3:
            cout << "Enter quantum time: ";
            int quantum;
            cin >> quantum;
            RoundRobin(pcbs, quantum);
            printResults(pcbs);
            calculateMetrics(pcbs);
            system("pause");
            break;

        case 4:
            cout << "Bye Bye" << endl;
            system("pause");
            break;


        default:
            cout << "Invalid Choice" << endl;
            system("pause");
            break;
        }
    } while (choice != 4);

    return 0;
}