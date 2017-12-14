
#ifndef PLATEDATARECVIER_H_ 
#define	PLATEDATARECVIER_H_

#pragma once
#include <stdio.h>
#include <string.h>

#include <semaphore.h>	
#include "cJSON.h"


#define STR_APPNAME	"studentgpsdatarecv" 

#define STR_APPVER	"v2.0"
#define STR_APPDATE "2013-07-03 15:00"



void * ThreadListener(void *pContext);


void * ThreadWorker(void *pContext);
void * ThreadWebListener(void *pContext);


void * ThreadWebWorker(void *pContext);

void handleSignal(int iSignal);




#endif /* PLATEDATARECVIER_H_ */
