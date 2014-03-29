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


#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include "thdpool.h"

void callbackfunc1(void *pUser){
	char *p = (char*)pUser;
	printf("callbackfunc1: %s\n", p);
	sleep(3);
}

void callbackfunc2(void *pUser){
	char *p = (char*)pUser;
	printf("callbackfunc2: %s\n", p);
	sleep(2);
}

int main(int argc, char *argv[]){
	int thd_count = 5;
	if(argc == 2){
		thd_count = atoi(argv[1]);
	}
	CThdPool thd_pool;	
	thd_pool.PoolInit(thd_count, callbackfunc1); 
	char str1[] = "Test string, hello world";
	char str2[] = "Test string, Linux V5";
	int n = 0;
	while(true){
		if(n++ % 3 == 0){
			thd_pool.AddTask(str1, callbackfunc2);
		}else{
			thd_pool.AddTask(str2);
		}
		if(n%5 == 0){
			sleep(3);
		}else{
			usleep(10000);
		}
	}
}


/*
	nothing  V2
*/
