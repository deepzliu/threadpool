/*
 *	Copyright (c) by Donghua Lau<1485084328@qq.com>
 *
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2.1 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef THD_POOL_H_
#define THD_POOL_H_

#include <iostream>
#include <vector>
#include <list>
using namespace std;

#include <stdio.h>
#include<semaphore.h>
#include <pthread.h>

/*
 *	Pre-define
 */
#define TPDEBUG

/*
 *	Lock class
 */
class CLock{
public:
	CLock(pthread_mutex_t *mutex):m_Mutex(mutex){
		if(m_Mutex != NULL) pthread_mutex_lock(m_Mutex);
	}
	~CLock(){
		if(m_Mutex != NULL) pthread_mutex_unlock(m_Mutex);
	}
private:
	pthread_mutex_t *m_Mutex;
};

typedef void (*TaskConsumHandle)(void *pUser);

/*
 *	Thread information struct
 */
struct ThdInfo{
	bool idle_flag;
	sem_t sem;
	void *pValue;
	TaskConsumHandle fTaskHandle;
	int64_t do_task_frequency;
	ThdInfo(){
		idle_flag = 1;
		pValue = NULL;
		do_task_frequency = 0;
		fTaskHandle = NULL;
		if(sem_init(&sem, 0, 0) != 0){
			fprintf(stderr, "Semaphore initial failed.");
		}
	}
};

/*
 *	Task information struct
 */
struct TaskInfo{
	void *pValue;
	TaskConsumHandle fTaskHandle;
};

struct TaskThreadData{
	int index;
	void *pData;
};

/*
 *	Thread pool class
 */
class CThdPool{

private:
	list<TaskInfo> mTaskList;
	pthread_mutex_t mTaskListMutex;
	TaskConsumHandle mComTaskHandle;

	int mPoolSize;
	vector<ThdInfo> mThdInfo;
	pthread_mutex_t mThdInfoMutex;

	static void *TaskAllocateProc(void *pUser);
	static void *TaskThreadProc(void *pUser);
	void CreateThreads();
	bool GetNewTask(TaskInfo &task_info);
	void CheckIdleThread(const TaskInfo &task_info);
	void DoTask(int index);
	void DoTaskAllocate();

public:
	CThdPool();
	~CThdPool();
	int PoolInit(int pool_size, TaskConsumHandle handle = NULL);
	int AddTask(void *pUser, TaskConsumHandle handle = NULL);
	bool IsBusy();

};

#endif

