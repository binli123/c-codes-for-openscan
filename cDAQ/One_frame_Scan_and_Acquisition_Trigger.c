/*********************************************************************
*
* ANSI C Example program:
*    MultVoltUpdates-IntClk-Retrig.c
*
* Example Category:
*    AO
*
* Description:
*    This example demonstrates how to output a retrigger in hardware
*    a finite number of voltage samples to an Analog Output Channel
*    using an internal sample clock and a digital start trigger.
*
* Instructions for Running:
*    1. Select the Physical Channel to correspond to where your
*       signal is output on the DAQ device.
*    2. Enter the Minimum and Maximum Voltage Ranges.
*    3. Specify the Desired Frequency of the output waveform.
*    4. Select the desired waveform type.
*    5. Select the Digital Trigger Source.
*    6. Specify the desired Trigger Edge.
*
* Steps:
*    1. Create a task.
*    2. Create an Analog Output Voltage Channel.
*    3. Setup the Timing for the Measurement. In this example we use
*       the internal DAQ Device's clock to produce a finite number of
*       samples.
*    4. Define the Triggering parameters: Source and Edge.
*    5. Set the operation for retriggerable.
*    6. Use the Write function to Generate Multiple Samples to 1
*       Channel on the Data Acquisition Card. The auto start
*       parameter is set to False, so the Start function must
*       explicitly be called to begin the Voltage Generation.
*    7. Call the Start function.
*    8. Call the Clear Task function to clear the Task.
*    9. Display an error if any.
*
* I/O Connections Overview:
*    Make sure your signal output terminal matches the Physical
*    Channel I/O Control. For further connection information, refer
*    to your hardware reference manual.
*
*********************************************************************/

/*********************************************************************
* Microsoft Windows Vista User Account Control
* Running certain applications on Microsoft Windows Vista requires
* administrator privileges, because the application name contains keywords
* such as setup, update, or install. To avoid this problem, you must add an
* additional manifest to the application that specifies the privileges
* required to run the application. Some ANSI-C NI-DAQmx examples include
* these keywords. Therefore, these examples are shipped with an additional
* manifest file that you must embed in the example executable. The manifest
* file is named [ExampleName].exe.manifest, where [ExampleName] is the
* NI-provided example name. For information on how to embed the manifest
* file, refer to http://msdn2.microsoft.com/en-us/library/bb756929.aspx.
*********************************************************************/

#include <stdio.h>
#include <math.h>
#include <NIDAQmx.h>
#include <string.h>
#include <stdlib.h>

#define PI	3.1415926535
#define XResolution 512
#define YResolution 512
#define Freq 12500.0
#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData);
int32 Waveformgeneration(int32 Resolution, float64 tRetrace, float64 Tclock,float64 data[] );
static int32 GetTerminalNameWithDevPrefix(TaskHandle taskHandle, const char terminalName[], char triggerName[]);

int main(void)
{
	int         error=0;
	TaskHandle  Scan_taskHandle=0,DOtaskHandle=0,Start_taskHandle=0,Acq_taskHandle=0;
	char        errBuff[2048]={'\0'};
	int32			i=0;
	int32			j=0;
	int32   	written;
	uInt8       tData[400];
	float64		*Waveform = NULL;
	float64		t_Retrace=0.188;
	float64		T_clock=0.001;
	int32		scanLines=XResolution+1;
	int32		datasize=scanLines*1000;
	float64		Acq_freq=Freq/1000;
	char    trigName[256];
	uInt8		 *Acq_data=NULL;

	Acq_data=(uInt8*)malloc (sizeof (uInt8) *datasize);
	

	for(;j<200;j++)
	{
		tData[j] = 0;
		//printf("tData[%d]=%d\n",j,tData[j]);
	}
	for(j=200;j<400;j++)
	{
		tData[j] = 1;
		//printf("tData[%d]=%d\n",i,tData[i]);
	}
	
	
	for(j=0;j<scanLines;j++)
	{
		for(;i<50;i++)
		{
		Acq_data[i+j*1000] = 0;
		//printf("data[%d]=%lf\n",i,data[i]);
		}
		for(i=50;i<562;i++)
		{
		Acq_data[i+j*1000] =1;
		//printf("data[%d]=%lf\n",i,data[i]);
		}
		for(i=562;i<1000;i++)
		{
		Acq_data[i+j*1000] = 0;
		//printf("data[%d]=%lf\n",i,data[i]);
		}
	}
	
	Waveform=(float64*)malloc (sizeof (float64) * 2*datasize);
	Waveformgeneration(XResolution,t_Retrace,T_clock,Waveform);

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/

	// Configure Main trigger info
	DAQmxErrChk (DAQmxCreateTask("",&Start_taskHandle));
	DAQmxErrChk (DAQmxCreateDOChan(Start_taskHandle,"PXI1Slot2/port0/line7","",DAQmx_Val_ChanForAllLines));
	DAQmxErrChk (GetTerminalNameWithDevPrefix(Start_taskHandle,"do/Sampleclock",trigName));	
	
	//Configure acquisition info (replace with a output signal)
	DAQmxErrChk (DAQmxCreateTask("",&Acq_taskHandle));
	DAQmxErrChk (DAQmxCreateDOChan(Acq_taskHandle,"PXI1Slot2/port0/line0","",DAQmx_Val_ChanPerLine));
	DAQmxErrChk (DAQmxCfgSampClkTiming(Acq_taskHandle,"",Freq,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,datasize));
	DAQmxErrChk (DAQmxCfgDigEdgeStartTrig(Acq_taskHandle,"/PXI1Slot2/PFI9",DAQmx_Val_Rising));
	//DAQmxErrChk (DAQmxSetStartTrigRetriggerable(Acq_taskHandle,1));

	//Configure Scan waveform info
	DAQmxErrChk (DAQmxCreateTask("",&Scan_taskHandle));
	DAQmxErrChk (DAQmxCreateAOVoltageChan(Scan_taskHandle,"PXI1Slot2/ao0:1","",-10.0,10.0,DAQmx_Val_Volts,NULL));
	DAQmxErrChk (DAQmxCfgSampClkTiming(Scan_taskHandle,trigName,Freq,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,datasize));
	DAQmxErrChk (DAQmxCfgDigEdgeStartTrig(Scan_taskHandle,"/PXI1Slot2/PFI9",DAQmx_Val_Rising));

	DAQmxErrChk (DAQmxRegisterDoneEvent(Acq_taskHandle,0,DoneCallback,NULL));
	DAQmxErrChk (DAQmxRegisterDoneEvent(Start_taskHandle,0,DoneCallback,NULL));
	DAQmxErrChk (DAQmxRegisterDoneEvent(Scan_taskHandle,0,DoneCallback,NULL));
	
	/*********************************************/
	// DAQmx Write Code
	/*********************************************/



	DAQmxErrChk (DAQmxWriteDigitalLines(Acq_taskHandle,1000,0,10.0,DAQmx_Val_GroupByChannel,Acq_data,&written,NULL));
	DAQmxErrChk (DAQmxWriteAnalogF64(Scan_taskHandle,datasize,0,10.0,DAQmx_Val_GroupByChannel,Waveform,NULL,NULL));
	//DAQmxErrChk (DAQmxWriteAnalogF64(taskHandle, 4000, FALSE, 10.0, DAQmx_Val_GroupByChannel,data, NULL, NULL));
	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(Start_taskHandle));
	DAQmxErrChk (DAQmxStartTask(Acq_taskHandle));
	DAQmxErrChk (DAQmxStartTask(Scan_taskHandle));
		
	DAQmxErrChk (DAQmxWriteDigitalLines(Start_taskHandle,400,0,1.0,DAQmx_Val_GroupByChannel,tData,NULL,NULL));
	
	/*********************************************/
	// DAQmx Wait Code
	/*********************************************/
	printf("Generating voltage on trigger. Press Enter to interrupt\n");
	getchar();

Error:
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	if( Acq_taskHandle!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(Acq_taskHandle);
		DAQmxClearTask(Acq_taskHandle);
	}
	if( Start_taskHandle!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(Start_taskHandle);
		DAQmxClearTask(Start_taskHandle);
	}
	if( Scan_taskHandle!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(Scan_taskHandle);
		DAQmxClearTask(Scan_taskHandle);
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
		printf("Press Enter\n");
	}
	return 0;
}

static int32 GetTerminalNameWithDevPrefix(TaskHandle taskHandle, const char terminalName[], char triggerName[])
{
	int32	error=0;
	char	device[256];
	int32	productCategory;
	uInt32	numDevices,i=1;

	DAQmxErrChk (DAQmxGetTaskNumDevices(taskHandle,&numDevices));
	while( i<=numDevices ) {
		DAQmxErrChk (DAQmxGetNthTaskDevice(taskHandle,i++,device,256));
		DAQmxErrChk (DAQmxGetDevProductCategory(device,&productCategory));
		if( productCategory!=DAQmx_Val_CSeriesModule && productCategory!=DAQmx_Val_SCXIModule ) {
			*triggerName++ = '/';
			strcat(strcat(strcpy(triggerName,device),"/"),terminalName);
			break;
		}
	}

Error:
	return error;
}

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
