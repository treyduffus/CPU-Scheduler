// Author : Tremayne Duffus
// Date : 10/4/2023
// COP4610 Assignment 1: CPU Scheduler


#include <iostream>
#include <queue>

using namespace std;


class Process {
public:
	// constructors
	Process();
	Process(string process_id_, queue<int> cpu_bursts_, queue<int> io_bursts_);
	
	// getters and setters
	bool isInCpuBurst();
	bool isFinished(); 
	bool isStarted();
	void setResponseTime(double global_time);
	void setTurnAroundTime(double global_time);
	void setProcessID(string id);
	string getProcessID();
	int get_current_burst_time();
	int get_current_queue();
	double getResponseTime();
	double getTurnAroundTime();
	double getWaitingTime();
	queue<int> getCpuBursts();
	queue<int> getIOBursts();


	// methods
	void startProcess();
	void start_burst(int queue_id);
	void incrementWaitingTime(); 
	void decrementCpuBurst();
	void decrementIOBurst();

	

private:
	// member variables
	string process_id;
	queue<int> cpu_bursts;
	queue<int> io_bursts;
	bool in_cpu_burst;
	bool finished;
	bool started; // assuming everything starts with a cpu burst
	double waiting_time;
	double turnaround_time;
	double response_time;
	int current_burst_time; 
	int current_queue; // 0 = high_priority, 1 = medium_priority, 2 = low_priority, to know what queue this process was last in
};

class JobQueue {
public:
	JobQueue(int TimeQuantum_);
	JobQueue(queue<Process> processes_, int TimeQuantum_);
	void sortProcesses(); // sorts ascending by CPU burst length
	void reQueueProcess(Process process);
	queue<Process> processes;
	int TimeQuantum;
};

// helper functions
Process init_process(string process_name,vector<int> arr);
queue<Process> IncrementWaitTimes(queue<Process> q);
int get_shortest_process_index_from_list(vector<Process> q);

// printing functions
void print_process_queue(JobQueue q);
void print_IO_vector(vector<Process> IO);
void print_context_switch_multi_queue(int time, int NUM_PROCESSES, Process curr_process, vector<Process> IO, vector<JobQueue> queues);
void print_context_switch_single_queue(int time, int NUM_PROCESSES, Process curr_process, vector<Process> IO, JobQueue queue);
void print_scheduler_stats(string secheduler_name, int num_processes, double stats[]);
void print_process_completion(Process process);


// scheduling algorithms
void FCFS(JobQueue ready_queue, double stats[]);
void SJF(JobQueue ready_queue, double stats[]);
void MLFQ(JobQueue high_priority_queue, double stats[]);




int main() {

	// Converting processes from a array of burst times to a processes class instance
	vector<int> temp_p1{5, 27, 3, 31, 5, 43, 4, 18, 6, 22, 4, 26, 3, 24, 4};
	Process p1 = init_process("p1", temp_p1);

	vector<int> temp_p2{4, 48, 5, 44, 7, 42, 12, 37, 9, 76, 4, 41, 9, 31, 7, 43, 8};
	Process p2 = init_process("p2", temp_p2);

	vector<int> temp_p3{8, 33, 12, 41, 18, 65, 14, 21, 4, 61, 15, 18, 14, 26, 5, 31, 6};
	Process p3 = init_process("p3", temp_p3);

	vector<int> temp_p4{3, 35, 4, 41, 5, 45, 3, 51, 4, 61, 5, 54, 6, 82, 5, 77, 3};
	Process p4 = init_process("p4", temp_p4);

	vector<int> temp_p5{16, 24, 17, 21, 5, 36, 16, 26, 7, 31, 13, 28, 11, 21, 6, 13, 3, 11, 4};
	Process p5 = init_process("p5", temp_p5);

	vector<int> temp_p6{11, 22, 4, 8, 5, 10, 6, 12, 7, 14, 9, 18, 12, 24, 15, 30, 8};
	Process p6 = init_process("p6", temp_p6);

	vector<int> temp_p7{14, 46, 17, 41, 11, 42, 15, 21, 4, 32, 7, 19, 16, 33, 10};
	Process p7 = init_process("p7", temp_p7);

	vector<int> temp_p8{4, 14, 5, 33, 6, 51, 14, 73, 16, 87, 6};
	Process p8 = init_process("p8", temp_p8);

	// building starting queues from given processes
	queue<Process> tempQueue;
	tempQueue.push(p1);
	tempQueue.push(p2);
	tempQueue.push(p3);
	tempQueue.push(p4);
	tempQueue.push(p5);
	tempQueue.push(p6);
	tempQueue.push(p7);
	tempQueue.push(p8);
	JobQueue FCFS_QUEUE(tempQueue, 1000); // A large time quantum makes the queue function as FCFS
	JobQueue high_priority_queue(tempQueue, 5);

	double FCFS_stats[5] = {0, 0, 0, 0, 0};
	double SJF_stats[5] = {0, 0, 0, 0, 0};
	double MLFQ_stats[5] = { 0, 0, 0, 0, 0 };

	FCFS(FCFS_QUEUE, FCFS_stats);
	SJF(FCFS_QUEUE, SJF_stats); // We can use the FCFS queue for SJF because sorting is done before every context switch
	MLFQ(high_priority_queue, MLFQ_stats);

	print_scheduler_stats("First Come First Serve", FCFS_QUEUE.processes.size(), FCFS_stats);
	print_scheduler_stats("Shortest Job First", FCFS_QUEUE.processes.size(), SJF_stats);
	print_scheduler_stats("Multilevel Feedback Queue", high_priority_queue.processes.size(), MLFQ_stats);

	return 0;
}


void MLFQ(JobQueue high_priority_queue, double stats[]) {
	vector<Process> IO; // holds processes that are currently in IO 
	int NUM_PROCESSES = high_priority_queue.processes.size(); // total number of processes
	int num_context_switch = 0;
	double time = 0, idle_time = 0; // amount of time elasped
	Process curr_process;
	JobQueue medium_priority_queue(10), low_priority_queue(1000); // medium_priority_queue has a Tq of 10 while low priority is FCFS

	cout << "\n-------------------------Starting Multilevel Feedback Queue----------------------------\n";

	while (NUM_PROCESSES > 0) {
		vector<JobQueue> queues;
		queues.push_back(high_priority_queue);
		queues.push_back(medium_priority_queue);
		queues.push_back(low_priority_queue);

		// If CPU is IDLE grab next process
		if (curr_process.isFinished() || !(curr_process.isInCpuBurst()) || curr_process.getProcessID() == "temp") {
			if (!(high_priority_queue.processes.empty())) { // grab from highier priority queues first
				curr_process = high_priority_queue.processes.front();
				high_priority_queue.processes.pop();
				if (!(curr_process.isStarted())) {
					curr_process.setResponseTime(time);
					curr_process.startProcess();
				}
				curr_process.start_burst(0);
				
				print_context_switch_multi_queue(time, NUM_PROCESSES, curr_process, IO, queues);
				num_context_switch++;
			}
			else if (!(medium_priority_queue.processes.empty())) {
				curr_process = medium_priority_queue.processes.front();
				medium_priority_queue.processes.pop();
				if (!(curr_process.isStarted())) {
					curr_process.setResponseTime(time);
					curr_process.startProcess();
				}
				curr_process.start_burst(1);

				print_context_switch_multi_queue(time, NUM_PROCESSES, curr_process, IO, queues);
				num_context_switch++;

			}
			else if (!(low_priority_queue.processes.empty())) {
				curr_process = low_priority_queue.processes.front();
				low_priority_queue.processes.pop();
				if (!(curr_process.isStarted())) {
					curr_process.setResponseTime(time);
					curr_process.startProcess();
				}
				curr_process.start_burst(2);

				print_context_switch_multi_queue(time, NUM_PROCESSES, curr_process, IO, queues);
				num_context_switch++;

			}
		}
		else { // if cpu is not idle check for preemption
			if ((curr_process.get_current_queue() == 1 || curr_process.get_current_queue() == 2) && !(high_priority_queue.processes.empty())) {
				// Context switch to high priority
				if (curr_process.get_current_queue() == 1) {
					medium_priority_queue.reQueueProcess(curr_process);
				}
				else if (curr_process.get_current_queue() == 2) {
					low_priority_queue.reQueueProcess(curr_process);
				}
				curr_process = high_priority_queue.processes.front();
				high_priority_queue.processes.pop();
				if (!(curr_process.isStarted())) {
					curr_process.setResponseTime(time);
					curr_process.startProcess();
				}
				curr_process.start_burst(0);

				print_context_switch_multi_queue(time, NUM_PROCESSES, curr_process, IO, queues);
				num_context_switch++;
			}
			else if (curr_process.get_current_queue() == 2 && !(medium_priority_queue.processes.empty())) {
				// context switch to meduim priority
				low_priority_queue.reQueueProcess(curr_process);
				curr_process = medium_priority_queue.processes.front();
				medium_priority_queue.processes.pop();
				if (!(curr_process.isStarted())) {
					curr_process.setResponseTime(time);
					curr_process.startProcess();
				}
				curr_process.start_burst(1);

				print_context_switch_multi_queue(time, NUM_PROCESSES, curr_process, IO, queues);
				num_context_switch++;
			}
		}

		// all processes in the ready list are waiting
		high_priority_queue.processes = IncrementWaitTimes(high_priority_queue.processes); 
		medium_priority_queue.processes = IncrementWaitTimes(medium_priority_queue.processes); 
		low_priority_queue.processes = IncrementWaitTimes(low_priority_queue.processes); 

		vector<Process> new_IO;
		// For every process in I/O decrement one time unit
		for (int i = 0; i < IO.size(); i++) {
			IO.at(i).decrementIOBurst();

			if (IO.at(i).isFinished()) { // Some processess may finish in I/O
				NUM_PROCESSES--;
				curr_process.setTurnAroundTime(time);
				stats[2] += IO.at(i).getWaitingTime();
				stats[3] += IO.at(i).getTurnAroundTime();
				stats[4] += IO.at(i).getResponseTime();
				print_process_completion(IO.at(i));
			}
			else if (IO.at(i).isInCpuBurst()) { // if a process is done with I/O we put it back in the ready queue
				high_priority_queue.processes.push(IO.at(i)); // All processes start in the high priority queue
			}
			else { // else we continue with I/O for this process
				new_IO.push_back(IO.at(i));
			}
		}

		IO = new_IO;

		// If all processes are in I/O CPU stays idle
		if ((!(curr_process.isInCpuBurst()) || curr_process.isFinished())) {
			idle_time++;
		} // else we can decrement 1 from the current cpu burst
		else {
			curr_process.decrementCpuBurst();
			if (curr_process.isFinished()) {
				NUM_PROCESSES--;
				curr_process.setTurnAroundTime(time);
				stats[2] += curr_process.getWaitingTime();
				stats[3] += curr_process.getTurnAroundTime();
				stats[4] += curr_process.getResponseTime();
				print_process_completion(curr_process);
			}
			else if (!(curr_process.isInCpuBurst())) { // If the CPU burst is finished send process to I/O
				IO.push_back(curr_process);
			}
			// demote process priority if it exceeds the time quantum
			else if (curr_process.get_current_queue() == 0 && curr_process.get_current_burst_time() >= high_priority_queue.TimeQuantum) {
				medium_priority_queue.processes.push(curr_process);
				curr_process.setProcessID("temp");
			}
			// demote process priority if it exceeds the time quantum
			else if (curr_process.get_current_queue() == 1 && curr_process.get_current_burst_time() >= medium_priority_queue.TimeQuantum) {
				low_priority_queue.processes.push(curr_process);
				curr_process.setProcessID("temp");
			}
		}
		
		time++;

		if (NUM_PROCESSES == 0) {
			cout << "\nMultilevel Feedback Queue Complete at time: " << time << endl;
			cout << "With " << num_context_switch << " context switches" << endl;
			stats[0] = time;
			stats[1] = idle_time;
		}
	}

}

void SJF(JobQueue ready_queue, double stats[]) {
	vector<Process> IO; // holds processes that are currently in IO 
	int NUM_PROCESSES = ready_queue.processes.size(); // total number of processes 
	int num_context_switch = 0;
	double time = 0, idle_time = 0; // amount of time elasped
	Process curr_process;

	cout << "\n-------------------------Starting Shortest Job First----------------------------\n";

	while (NUM_PROCESSES > 0) {
		// If CPU is IDLE grab next process
		if (curr_process.isFinished() || !(curr_process.isInCpuBurst())) {
			if (!ready_queue.processes.empty()) { // CONTEXT SWITCH
				ready_queue.sortProcesses(); // Sort the processes (by cpu burst) so we assign the shortest one first
				curr_process = ready_queue.processes.front();
				ready_queue.processes.pop();
				if (!(curr_process.isStarted())) {
					curr_process.setResponseTime(time);
					curr_process.startProcess();
				}

				print_context_switch_single_queue(time, NUM_PROCESSES, curr_process, IO, ready_queue);
				num_context_switch++;

			}
		}

		ready_queue.processes = IncrementWaitTimes(ready_queue.processes); // all processes in the ready list are waiting

		vector<Process> new_IO;
		// For every process in I/O decrement one time unit
		for (int i = 0; i < IO.size(); i++) {
			IO.at(i).decrementIOBurst();

			if (IO.at(i).isFinished()) { // Some processess may finish in I/O
				NUM_PROCESSES--;
				curr_process.setTurnAroundTime(time);
				stats[2] += IO.at(i).getWaitingTime();
				stats[3] += IO.at(i).getTurnAroundTime();
				stats[4] += IO.at(i).getResponseTime();
				print_process_completion(IO.at(i));
			}
			else if (IO.at(i).isInCpuBurst()) { // if a process is done with I/O we put it back in the ready queue
				ready_queue.processes.push(IO.at(i));
			}
			else { // else we continue with I/O for this process
				new_IO.push_back(IO.at(i));
			}
		}
		IO = new_IO;

		// If all processes are in I/O CPU stays idle
		if ((!(curr_process.isInCpuBurst()) || curr_process.isFinished())) {
			idle_time++;
		} // else we can decrement 1 from the current cpu burst
		else {
			curr_process.decrementCpuBurst();
			if (curr_process.isFinished()) {
				NUM_PROCESSES--;
				curr_process.setTurnAroundTime(time);
				stats[2] += curr_process.getWaitingTime();
				stats[3] += curr_process.getTurnAroundTime();
				stats[4] += curr_process.getResponseTime();
				print_process_completion(curr_process);
			}
			else if (!(curr_process.isInCpuBurst())) { // If the CPU burst is finished send process to I/O
				IO.push_back(curr_process);
			}
		}

		time++;

		if (NUM_PROCESSES == 0) {
			cout << "\nShortest Job First Complete at time: " << time << endl;
			cout << "With " << num_context_switch << " context switches" << endl;
			stats[0] = time;
			stats[1] = idle_time;
		}

	}

}

void FCFS(JobQueue ready_queue, double stats[]) {
	vector<Process> IO; // holds processes that are currently in IO 
	int NUM_PROCESSES = ready_queue.processes.size(); // total number of processes 
	int  num_context_switch = 0;
	double time = 0, idle_time = 0; // amount of time elasped
	Process curr_process;

	cout << "\n-------------------------Starting First Come First Serve----------------------------\n";

	while (NUM_PROCESSES > 0) {
		// If CPU is IDLE grab next process
		if (curr_process.isFinished() || !(curr_process.isInCpuBurst())) {
			if (!ready_queue.processes.empty()) { // CONTEXT SWITCH
				curr_process = ready_queue.processes.front();
				ready_queue.processes.pop();
				if (!(curr_process.isStarted())) {
					curr_process.setResponseTime(time);
					curr_process.startProcess();
				}

				print_context_switch_single_queue(time, NUM_PROCESSES, curr_process, IO, ready_queue);
				num_context_switch++;
				
			}
		}

		ready_queue.processes = IncrementWaitTimes(ready_queue.processes); // all processes in the ready list are waiting

		vector<Process> new_IO;
		// For every process in I/O decrement one time unit
		for (int i = 0; i < IO.size(); i++) {
			IO.at(i).decrementIOBurst();

			if (IO.at(i).isFinished()) { // Some processess may finish in I/O
				NUM_PROCESSES--;
				curr_process.setTurnAroundTime(time);
				stats[2] += IO.at(i).getWaitingTime();
				stats[3] += IO.at(i).getTurnAroundTime();
				stats[4] += IO.at(i).getResponseTime();
				print_process_completion(IO.at(i));
			}
			else if (IO.at(i).isInCpuBurst()) { // if a process is done with I/O we put it back in the ready queue
				ready_queue.processes.push(IO.at(i));
			}
			else { // else we continue with I/O for this process
				new_IO.push_back(IO.at(i));
			}
		}
		IO = new_IO;

		// If all processes are in I/O CPU stays idle
		if ((!(curr_process.isInCpuBurst()) || curr_process.isFinished())) {
			idle_time++;
		} // else we can decrement 1 from the current cpu burst
		else {
			curr_process.decrementCpuBurst();
			if (curr_process.isFinished()) {
				NUM_PROCESSES--;
				curr_process.setTurnAroundTime(time);
				stats[2] += curr_process.getWaitingTime();
				stats[3] += curr_process.getTurnAroundTime();
				stats[4] += curr_process.getResponseTime();
				print_process_completion(curr_process);
			}
			else if (!(curr_process.isInCpuBurst())) { // If the CPU burst is finished send process to I/O
				IO.push_back(curr_process);
			}
		}

		time++;

		if (NUM_PROCESSES == 0) {
			cout << "\nFirst Come First Serve Complete at time: " << time << endl;
			cout << "With " << num_context_switch << " context switches" << endl;
			stats[0] = time;
			stats[1] = idle_time;
		}

	}

}

JobQueue::JobQueue(queue<Process> processes_, int TimeQuantum_) {
	processes = processes_;
	TimeQuantum = TimeQuantum_;
}

JobQueue::JobQueue(int TimeQuantum_) {
	queue<Process> p;
	processes = p;
	TimeQuantum = TimeQuantum_;
}

void JobQueue::sortProcesses() { //Sort CPU bursts in order to process the shortest job first
	int SIZE = processes.size();
	vector<Process> process_list;
	queue<Process> new_q;
	if (processes.empty()) {
		cout << "\nERROR: CANT SORT EMPTY QUEUE\n";
		return;
	}

	while (!(processes.empty())) {
		process_list.push_back(processes.front());
		processes.pop();
	}

	while (process_list.size() > 0) {
			int shortest_index = get_shortest_process_index_from_list(process_list);
			new_q.push(process_list.at(shortest_index));
			process_list.erase(process_list.begin() + shortest_index);
	}

	processes = new_q;
}

void JobQueue::reQueueProcess(Process process) {
	queue<Process> new_q;
	new_q.push(process); // preempted processes return to the front of the queue
	while (!(processes.empty())) {
		new_q.push(processes.front());
		processes.pop();
	}
	processes = new_q;
}

Process::Process(string process_id_, queue<int> cpu_bursts_, queue<int> io_bursts_) {
	process_id = process_id_;
	cpu_bursts = cpu_bursts_;
	io_bursts = io_bursts_;
	in_cpu_burst = true;
	finished = false;
	started = false;
}

Process::Process() {
	process_id = "temp";
	finished = true;
}

string Process::getProcessID() {
	return process_id;
}

bool Process::isFinished() {
	return finished;
}

bool Process::isInCpuBurst() {
	return in_cpu_burst;
}

void Process::decrementCpuBurst() {
	if (cpu_bursts.empty()) {
		cout << "\nERROR : NO MORE CPU BURSTS\n";
		return;
	}
	int curr_burst = cpu_bursts.front();
	curr_burst--;
	if (curr_burst > 0) {
		cpu_bursts.front() = curr_burst;
		current_burst_time++;
	}
	else { // if current burst is finished either send to I/O or finish process
		cpu_bursts.pop();
		if (io_bursts.empty()) {
			finished = true;
		}
		else {
			in_cpu_burst = false;
		}
	}
}

void Process::decrementIOBurst() {
	if (io_bursts.empty()) {
		cout << "\nERROR : NO MORE I/O BURSTS\n";
		return;
	}
	int curr_burst = io_bursts.front();
	curr_burst--;
	if (curr_burst > 0) {
		io_bursts.front() = curr_burst;
	}
	else { // if current burst is finished either send to ready list or finish process
		io_bursts.pop();
		if (cpu_bursts.empty()) {
			finished = true;
		}
		else {
			in_cpu_burst = true;
		}
	}
}

void Process::start_burst(int queue_id) {
	current_burst_time = 0;
	current_queue = queue_id;
}

bool Process::isStarted() {
	return started;
}

void Process::startProcess() {
	started = true;
}

void Process::setResponseTime(double global_time) {
	response_time = global_time;
}

void Process::setTurnAroundTime(double global_time) {
	double start_time = 0; // all processes arrive at time zero for this simulation
	turnaround_time = (global_time - start_time) + 1;
}

void Process::setProcessID(string id) {
	process_id = id;
}

void Process::incrementWaitingTime() {
	waiting_time++;
}

double Process::getResponseTime() {
	return response_time;
}

double Process::getTurnAroundTime() {
	return turnaround_time;
}

double Process::getWaitingTime() {
	return waiting_time;
}

queue<int> Process::getCpuBursts() {
	return cpu_bursts;
}

queue<int> Process::getIOBursts() {
	return io_bursts;
}

int Process::get_current_burst_time() {
	return current_burst_time;
}

int Process::get_current_queue() {
	return current_queue;
}

void print_scheduler_stats(string secheduler_name, int num_processes, double stats[]) {
	cout.precision(2);
	cout << fixed;
	
	cout << "\n\n----------------------------------------------------" << endl;
	cout << "Printing stats for : " << secheduler_name << endl;
	cout << "Total time: " << stats[0] << endl;
	cout << "CPU Utilization: " << 100 - ((stats[1] / stats[0]) * 100) << "%" << endl;
	cout << "Avg Waiting time: " << stats[2] / (double) num_processes << endl;
	cout << "Avg TurnAround time: " << stats[3] / (double) num_processes << endl;
	cout << "Avg Response time: " << stats[4] / (double) num_processes << endl;
	cout << "-------------------------------------------------------\n\n";
}

queue<Process> IncrementWaitTimes(queue<Process> q) {
	queue<Process> temp_queue = q;
	queue<Process> new_queue;
	while (!(temp_queue.empty())) {
		Process temp_process = temp_queue.front();
		temp_process.incrementWaitingTime();
		new_queue.push(temp_process);
		temp_queue.pop();
	}
	return new_queue;
}

void print_process_queue(JobQueue q) {
	JobQueue tempQ = q;

	cout << "[";
	while (!tempQ.processes.empty()) {
		if (tempQ.processes.size() > 1) {
			cout << tempQ.processes.front().getProcessID() << ": "<< tempQ.processes.front().getCpuBursts().front() << ", ";
		}
		else {
			cout << tempQ.processes.front().getProcessID() << ": " << tempQ.processes.front().getCpuBursts().front();
		}
		tempQ.processes.pop();
	}
	cout << "]";
}

void print_IO_vector(vector<Process> IO) {

	cout << "[";
	for (int i = 0; i < IO.size(); i++) {
		if (!IO.empty()) {
			if (i == IO.size() - 1) {
				cout << IO[i].getProcessID() << ": " << IO[i].getIOBursts().front();
			}
			else {
				cout << IO[i].getProcessID()  << ": " << IO[i].getIOBursts().front() << ", ";
			}
		}
	}
	cout << "]";
}

Process init_process(string process_id, vector<int> arr) {
	queue<int> cpu_bursts;
	queue<int> io_bursts;
	for (int i = 0; i < arr.size(); i++) {
		if (i % 2 == 0) {
			cpu_bursts.push(arr.at(i));
		}
		else {
			io_bursts.push(arr.at(i));
		}
	}
	Process new_process (process_id, cpu_bursts, io_bursts);
	return new_process;
}

int get_shortest_process_index_from_list(vector<Process> list) { // returns the index of the process with the shortest CPU burst
	Process shortest_process = list.at(0);
	int min_burst = shortest_process.getCpuBursts().front();
	int process_index = 0;
	for (int i = 1; i < list.size(); i++) {
		Process temp_process = list.at(i);
		int temp_burst = temp_process.getCpuBursts().front();
		if (temp_burst < min_burst) {
			shortest_process = temp_process;
			min_burst = temp_burst;
			process_index = i;
		}
	}
	return process_index;
}

void print_context_switch_multi_queue(int time, int NUM_PROCESSES, Process curr_process, vector<Process> IO, vector<JobQueue> queues) {
	cout << "\n---------Context Switch-----------";
	cout << "\nCurrent Excution Time: " << time << endl;
	cout << NUM_PROCESSES << " Processes left" << endl;
	cout << "Current Process: " << curr_process.getProcessID() << endl;
	cout << "High Priority Queue: ";
	print_process_queue(queues[0]);
	cout << endl;
	cout << "Medium Priority Queue: ";
	print_process_queue(queues[1]);
	cout << endl;
	cout << "Low priority Queue: ";
	print_process_queue(queues[2]);
	cout << endl;
	cout << "Processes in I/O: ";
	print_IO_vector(IO);
	cout << "\n-------------------------------------" << endl;
	cout << "\n\n";
}

void print_context_switch_single_queue(int time, int NUM_PROCESSES, Process curr_process, vector<Process> IO, JobQueue queue) {
	cout << "\n---------Context Switch-----------";
	cout << "\nCurrent Excution Time: " << time << endl;
	cout << NUM_PROCESSES << " Processes left" << endl;
	cout << "Current Process: " << curr_process.getProcessID() << endl;
	cout << "Ready Queue: ";
	print_process_queue(queue);
	cout << endl;
	cout << "Processes in I/O: ";
	print_IO_vector(IO);
	cout << "\n-------------------------------------" << endl;
	cout << "\n\n";
}

void print_process_completion(Process process) {
	cout << "\n-------------------------------------" << endl;
	cout << "Process " << process.getProcessID() << " completed!" << endl;
	cout << "Waiting time: " << process.getWaitingTime() << endl;
	cout << "Turn around time time: " << process.getTurnAroundTime() << endl;
	cout << "Response time: " << process.getResponseTime() << endl;
	cout << "-------------------------------------" << endl;
}
