#ifndef _MAIN_H
#define _MAIN_H

#define BUFFER_SIZE 1024
#define LOCKFILE "/var/run/judged.pid"
#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define STD_MB 1048576

#define OJ_WT0 0			//Pending
#define OJ_WT1 1			//Pending Rejudge
#define OJ_CI 2				//Compiling
#define OJ_RI 3				//Running & Judging
#define OJ_AC 4				//Accepted
#define OJ_PE 5				//Persentation Error
#define OJ_WA 6				//Wrong Answer
#define OJ_TL 7				//Time Limit Exceeded
#define OJ_ML 8				//Memory Limit Exceeded
#define OJ_OL 9				//Output Limit Exceeded
#define OJ_RE 10			//Runtime Error
#define OJ_CE 11			//Compile Error
#define OJ_CO 12

#endif		// _MAIN_H
