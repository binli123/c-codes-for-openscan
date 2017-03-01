/*********************************************************************
*
* ANSI C Example program:
*    ContGen-IntClk.c
*
* Example Category:
*    AO
*
* Description:
*    This example demonstrates how to output a continuous periodic
*    waveform using an internal sample clock.
*
* Instructions for Running:
*    1. Select the Physical Channel to correspond to where your
*       signal is output on the DAQ device.
*    2. Enter the Minimum and Maximum Voltage Ranges.
*    3. Enter the desired rate for the generation. The onboard sample
*       clock will operate at this rate.
*    4. Select the desired waveform type.
*    5. The rest of the parameters in the Waveform Information
*       section will affect the way the waveform is created, before
*       it's sent to the analog output of the board. Select the
*       number of samples per cycle and the total number of cycles to
*       be used as waveform data.
*
* Steps:
*    1. Create a task.
*    2. Create an Analog Output Voltage channel.
*    3. Define the update Rate for the Voltage generation.
*       Additionally, define the sample mode to be continuous.
*    4. Write the waveform to the output buffer.
*    5. Call the Start function.
*    6. Wait until the user presses the Stop button.
*    7. Call the Clear Task function to clear the Task.
*    8. Display an error if any.
*
* I/O Connections Overview:
*    Make sure your signal output terminal matches the Physical
*    Channel I/O Control. For further connection information, refer
*    to your hardware reference manual.
*
*********************************************************************/

#include <NIDAQmx.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

#define PI	3.1415926535
#define XResolution 512
#define YResolution 512
#define Freq 500000.0 //scan frequency=Freq/1000.0.  can not be too high!!
int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData);
int32 Waveformgeneration(int32 Resolution, float64 tRetrace, float64 Tclock,float64 data[] );

int main(void)
{
	int32       error=0;
	TaskHandle  taskHandle=0;
	char        errBuff[2048]={'\0'};
	float64		*Waveform = NULL;
	float64		t_Retrace=0.188;
	float64		T_clock=0.001;
	int32		scanLines=XResolution+1;
	int32		datasize=scanLines*1000;
	int32		i=0;

	//*******************************************/
	// generate waveform data
	//******************************************/
	Waveform=(float64*)malloc (sizeof (float64) * 2*datasize);
	Waveformgeneration(XResolution,t_Retrace,T_clock,Waveform);

	

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	
	DAQmxErrChk (DAQmxCreateTask("",&taskHandle));
	
	DAQmxErrChk (DAQmxCreateAOVoltageChan(taskHandle,"PXI1Slot2/ao0:1","",-10.0,10.0,DAQmx_Val_Volts,NULL));
	
	DAQmxErrChk (DAQmxCfgSampClkTiming(taskHandle,"",Freq,DAQmx_Val_Rising,DAQmx_Val_ContSamps,datasize));
	

	DAQmxErrChk (DAQmxRegisterDoneEvent(taskHandle,0,DoneCallback,NULL));
	

	/*********************************************/
	// DAQmx Write Code
	/*********************************************/
	DAQmxErrChk (DAQmxWriteAnalogF64(taskHandle,datasize,0,10.0,DAQmx_Val_GroupByChannel,Waveform,NULL,NULL));
	
	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandle));
	

	printf("Generating voltage continuously. Press Enter to interrupt\n");
	getchar();


Error:
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	if( taskHandle!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(taskHandle);
		
		DAQmxClearTask(taskHandle);
		
	}
	if( DAQmxFailed(error) )
		printf("DAQmx Error: %s\n",errBuff);
	printf("End of program, press Enter key to quit\n");
	getchar();
	return 0;
	}

int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData)
{
	int32   error=0;
	char    errBuff[2048]={'\0'};

	// Check to see if an error stopped the task.
	DAQmxErrChk (status);

Error:
	if( DAQmxFailed(error) ) {
		DAQmxGetExtendedErrorInfo(errBuff,2048);
		DAQmxClearTask(taskHandle);
		printf("DAQmx Error: %s\n",errBuff);
	}
	return 0;
}

//******************************************/
//Waveform generation function
//******************************************/
int32 Waveformgeneration(int32 Resolution, float64 tRetrace, float64 Tclock,float64 dataA[] )
{
	float64 	*dataX = NULL; 
	float64 	*dataY = NULL; 
	int32       i=0;
	int32		xScan=Resolution;
	int32		YScan=Resolution;
	int32		xUndershoot=50;
	float64		stepX=1.0/(xScan-1);
	float64		stepY=1.0/(YScan-1);
	int32		xRetrace=(int32)(tRetrace/Tclock)+1;
	int32		xZeros=100;
	float64		fallingStep=0.0;
	int32		j=0;
	int32		scanLines=Resolution+1;
	int32		datasize=scanLines*1000;
	
	dataX=(float64*)malloc (sizeof (float64) * datasize);
	dataY=(float64*)malloc (sizeof (float64) * datasize);
	
	printf("stepX=%lf\n",stepX);
	printf("xRetrace=%d\n",xRetrace);
	dataX[0]=(float64)-0.5-stepX*(xUndershoot+1);
	dataA[0]=dataX[0];
	fallingStep=(float64)-(0.5-dataX[0])/(xRetrace+1);
	printf("dataA[%d]=%lf\n",0,dataA[0]);


	for(j=0;j<scanLines;j++) //repetition times = scan lines
	{
		dataX[j*1000]=(float64)-0.5-stepX*(xUndershoot+1);
		dataA[j*1000]=dataX[j*1000];
		for (i=1;i<xUndershoot+xScan;i++)
		{
		dataX[i+j*1000]=(float64)dataX[(i+j*1000)-1]+stepX;
		dataA[i+j*1000]=dataX[i+j*1000];
		//printf("dataX[%d]=%lf\n",i,dataX[i+j*1000]);
		}
	
		for(i=xUndershoot+xScan;i<xUndershoot+xScan+xRetrace;i++)
		{
		dataX[i+j*1000]=(float64)fallingStep+dataX[(i+j*1000)-1];
		dataA[i+j*1000]=dataX[i+j*1000];
		//printf("dataX[%d]=%lf\n",i,dataX[i+j*1000]);
		}
		for (i=xUndershoot+xScan+xRetrace;i<1000;i++)
		{
		dataX[i+j*1000]=dataX[0];
		dataA[i+j*1000]=dataX[i+j*1000];
		//printf("dataX[%d]=%lf\n",i,dataX[i+j*1000]);
		}
	}
	
	for(i=0;i<1000;i++)
	{
		dataY[i]=-0.5;
		dataA[i+1000*scanLines]=dataY[i];
	}

	for(j=1;j<scanLines;j++)
	{
		for(i=0;i<1000;i++)
		{
			dataY[i+1000*j]=-0.5+stepY*(j-1);
			dataA[i+1000*(j+scanLines)]=dataY[i+1000*j];
			//printf("dataY[%d]=%lf\n",i+1000*j,dataY[i+j*1000]);
			
		}
		//printf("dataA[%d]=%lf\n",i-1+1000*(j+scanLines),dataA[i-1+1000*(j+scanLines)]);
	}
}