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


#include <unistd.h>
#include "thdpool.h"

CThdPool::CThdPool(){
}

CThdPool::~CThdPool(){
	pthread_mutex_destroy(&mTaskListMutex);
	pthread_mutex_destroy(&mThdInfoMutex);
}

int CThdPool::PoolInit(int pool_size, TaskConsumHandle handle/* = NULL*/){
	
	ThdInfo tThdInfo;
	mComTaskHandle = handle;
	tThdInfo.fTaskHandle = NULL;
	mThdInfo.assign(pool_size, tThdInfo);
	mPoolSize = pool_size;
	
	pthread_mutex_init(&mTaskListMutex, NULL);
	pthread_mutex_init(&mThdInfoMutex, NULL);

	CreateThreads();
}

void CThdPool::CreateThreads(){
	TaskThreadData ttd; 
	ttd.pData = this;
	pthread_t hThread;

	for(int i = 0; i < mPoolSize; i++){
		ttd.index = i; // maybe index is unsafe.
		pthread_create(&hThread, NULL, &TaskThreadProc, &ttd);
		usleep(200000);
	}

	pthread_create(&hThread, NULL, &TaskAllocateProc, this);
}

void *CThdPool::TaskThreadProc(void *pUser){
	TaskThreadData *pd = (TaskThreadData*)pUser;
	CThdPool *pThdpool = (CThdPool*)pd->pData;
	pThdpool->DoTask(pd->index);
	return (void*)0;
}

void CThdPool::DoTask(int index){
#ifdef TPDEBUG
	printf("Thread %d is ready.\n", index);
#endif
	while(true){
		sem_wait(&mThdInfo[index].sem);
#ifdef TPDEBUG
		printf("Thread %d start work ...\n", index);
#endif
		if(mThdInfo[index].fTaskHandle) 
			mThdInfo[index].fTaskHandle(mThdInfo[index].pValue);
		mThdInfo[index].idle_flag = 1;
		mThdInfo[index].do_task_frequency++;
#ifdef TPDEBUG
		printf("Thread %d finished work, fight time(s): %ld\n", index, mThdInfo[index].do_task_frequency);
#endif
	}
}

int CThdPool::AddTask(void *pUser, TaskConsumHandle handle/* = NULL*/){
	CLock lock(&mTaskListMutex);
	TaskInfo tTaskInfo;
	tTaskInfo.pValue = pUser;
	if(handle) tTaskInfo.fTaskHandle = handle;
	else tTaskInfo.fTaskHandle = mComTaskHandle;
	mTaskList.push_back(tTaskInfo);
	//printf("New task come in, handle: %p, serve handle: %p, task count: %lu.\n", handle, tTaskInfo.fTaskHandle, mTaskList.size());
}

bool CThdPool::GetNewTask(TaskInfo &task_info)
{
	bool ret = false;
	//printf("To get new task\n");
	CLock lock(&mTaskListMutex);
#ifdef TPDEBUG
	printf("## Undo task size: %lu\n", mTaskList.size());
#endif
	if(mTaskList.size() > 0){
		task_info = *mTaskList.begin();
		mTaskList.pop_front();
		ret = true;		
	}
	//printf("Get task: %d\n", ret);
	return ret;
}

void CThdPool::CheckIdleThread(const TaskInfo &task_info){
	static int n = 0;
	int complete_flag = 0;
	while(true){
		for(int i = 0; i < mPoolSize; i++, n++){
			n %= mPoolSize;
			if(mThdInfo[n].idle_flag){
				mThdInfo[n].idle_flag = 0;
				mThdInfo[n].pValue = task_info.pValue;
				mThdInfo[n].fTaskHandle = task_info.fTaskHandle;
				sem_post(&mThdInfo[n].sem);
				n++;
				complete_flag = 1;
				break;
			}
	    }	
		if(complete_flag){
			break;
		}
		usleep(10000);
	}
}

void *CThdPool::TaskAllocateProc(void *pUser){
	CThdPool *pThdpool = (CThdPool*)pUser;
	pThdpool->DoTaskAllocate();
	return (void*)0;
}

void CThdPool::DoTaskAllocate(){
	TaskInfo task_info;
	while(true){
		if(GetNewTask(task_info)){
			//printf("Handle of new task: %p\n", task_info.fTaskHandle);
			CheckIdleThread(task_info);
		}else{
			usleep(200000);
		}
	}
}


