/*
 * Vitis c code for Master project
 *
 *
 * Ryan Mark
 */
#include "xparameters.h"
#include "xgpio.h"
#include "xaxivdma.h"
#include "xscugic.h"
#include "sleep.h"
#include <stdlib.h>
#include "xil_cache.h"
#include "xil_cache.h"
#include "pixels.h"
#include <stdbool.h>
#define HSize 1920 // display length for monitor
#define VSize 1080 // display width for monitor
#define FrameSize HSize*VSize*3
//my image data parameters
#define imgHSize 640 // width
#define imgVSize 452// length


static XScuGic Intc;
int activeBuffer = 0;// ALlows us to switch buffer when ever GPIO are updated



static int SetupIntrSystem(XAxiVdma *AxiVdmaPtr, u16 ReadIntrId);
int drawImage(u32 displayHSize,u32 displayVSize,u32 imageHSize,u32 imageVSize,u32 hOffset, u32 vOffset,unsigned char *imagePointer, int activeBuffer);
int drawColorImage(u32 displayHSize,u32 displayVSize,u32 imageHSize,u32 imageVSize,u32 hOffset, u32 vOffset,unsigned int *imagePointer, int activeBuffer);
int checkparking(u32 imageHSize,u32 imageVSize,int X ,int Y , unsigned char *image1 ,unsigned char *image2,u32 dip,int activeBuffer);
void drawRedX(int centerX, int centerY,int activeBuffer);
void drawGreenCircle(int centerX, int centerY,int activeBuffer);
int select_image(u32 btn ,int activeBuffer  );

unsigned char Buffer[2][FrameSize]; // 2 buffers
int main(){

	// image coordinate for parking lots
	  int centroid [14][2] = {{150,133},{224,133},{296,135},{370,135},{444,136},{517,138},{591,138},
	    						{150,301},{223,301},{296,300},{370,301},{442,301},{515,300},{590,302}
	    						};


	//GIPO for button and switch
	// GPIO Switch set up
	XGpio dip;
	   u32   dip_check;

	   // Use XGpio_Initialize to initialize the DIP GPIO device

	   XGpio_Initialize(&dip, XPAR_AXI_GPIO_1_DEVICE_ID);
	   // Use XGpio_SetDataDirection to set the input/output direction
	   XGpio_SetDataDirection(&dip, 1, 0xFFFFFFFF);
	//GPIO button setup
	XGpio btn;
	u32   btn_check;

	// initialize push button the push button control which image will be displayed
	 XGpio_Initialize(&btn, XPAR_AXI_GPIO_0_DEVICE_ID);
	 XGpio_SetDataDirection(&btn, 1, 0xFFFFFFFF);

	//VGA and VDMA setup
	int status;


	XAxiVdma myVDMA;
	XAxiVdma_Config *config = XAxiVdma_LookupConfig(XPAR_AXI_VDMA_0_DEVICE_ID);
	XAxiVdma_DmaSetup ReadCfg;
	status = XAxiVdma_CfgInitialize(&myVDMA, config, config->BaseAddress);
    if(status != XST_SUCCESS){
    	xil_printf("DMA Initialization failed");
    }
    ReadCfg.VertSizeInput = VSize;
    ReadCfg.HoriSizeInput = HSize*3;
    ReadCfg.Stride = HSize*3;
    ReadCfg.FrameDelay = 0;
    ReadCfg.EnableCircularBuf = 1;
    ReadCfg.EnableSync = 1;
    ReadCfg.PointNum = 0;
    ReadCfg.EnableFrameCounter = 0;
    ReadCfg.FixedFrameStoreAddr = 0;
    status = XAxiVdma_DmaConfig(&myVDMA, XAXIVDMA_READ, &ReadCfg);
    if (status != XST_SUCCESS) {
    	xil_printf("Write channel config failed %d\r\n", status);
    	return status;
    }


    ReadCfg.FrameStoreStartAddr[0] = (u32)&Buffer[0][0];
    ReadCfg.FrameStoreStartAddr[1] = (u32)&Buffer[1][0];


	status = XAxiVdma_DmaSetBufferAddr(&myVDMA, XAXIVDMA_READ,ReadCfg.FrameStoreStartAddr);
	if (status != XST_SUCCESS) {
		xil_printf("Read channel set buffer address failed %d\r\n", status);
		return XST_FAILURE;
	}

	XAxiVdma_IntrEnable(&myVDMA, XAXIVDMA_IXR_COMPLETION_MASK, XAXIVDMA_READ);

	SetupIntrSystem(&myVDMA, XPAR_FABRIC_AXI_VDMA_0_MM2S_INTROUT_INTR);



	status = XAxiVdma_DmaStart(&myVDMA,XAXIVDMA_READ);
	if (status != XST_SUCCESS) {
		if(status == XST_VDMA_MISMATCH_ERROR)
			xil_printf("DMA Mismatch Error\r\n");
		return XST_FAILURE;
	}
 int prev_btn = 0xfffff;
 int prev_dip = 0;
    while(1){
    	int change = 0;
    	btn_check = XGpio_DiscreteRead(&btn, 1);
    	dip_check = XGpio_DiscreteRead(&dip, 1);

    	if (btn_check!= prev_btn)
    	{
    		change ++;
    		prev_btn = btn_check;
    		activeBuffer = 1 - activeBuffer;
    		select_image( btn_check , activeBuffer  );

    	}



    	if ( dip_check!= prev_dip )
    	    	{
    	    		prev_dip = dip_check;
    	    		change ++;
    	    		if(change == 2)
    	    		{
    	    			activeBuffer = activeBuffer; // buffer does not change if the button and switch change in the same iteration
    	    		}
    	    		else
    	    		{
    	    			activeBuffer = 1 - activeBuffer;
    	    		}
    	    		select_image( btn_check , activeBuffer  ); // rewrite the same image in new buffer so we can add in the over lay of the symbol when a car is taken or not

    	    	}

    		if( dip_check >0 && dip_check <15 )
    		 {
    			 int x =  centroid [dip_check -1] [0]; // define x coordinate of the image
    			 int y =  centroid [dip_check - 1] [1]; // define y coordinate of the image
    			     		checkparking(imgHSize,imgVSize,x,y,carmask,parking_mask,dip_check, activeBuffer);

    		 }
    		else{
    			xil_printf("Select Which parking lot to check for a car \r\n");

    		}
    		XAxiVdma_StartParking(&myVDMA, activeBuffer, XAXIVDMA_READ);
    		 usleep(500000);
    		//sleep(1);
    }
}


/*****************************************************************************/
 /* Call back function for read channel
******************************************************************************/

static void ReadCallBack(void *CallbackRef, u32 Mask)
{
	/* User can add his code in this call back function */
	//xil_printf("Read Call back function is called\r\n");
	//activeBuffer = 1 - activeBuffer;

}

/*****************************************************************************/
/*
 * The user can put his code that should get executed when this
 * call back happens.
 *
*
******************************************************************************/
static void ReadErrorCallBack(void *CallbackRef, u32 Mask)
{
	/* User can add his code in this call back function */
	//xil_printf("Read Call back Error function is called\r\n");

}


static int SetupIntrSystem(XAxiVdma *AxiVdmaPtr, u16 ReadIntrId)
{
	int Status;
	XScuGic *IntcInstancePtr =&Intc;

	/* Initialize the interrupt controller and connect the ISRs */
	XScuGic_Config *IntcConfig;
	IntcConfig = XScuGic_LookupConfig(XPAR_PS7_SCUGIC_0_DEVICE_ID);
	Status =  XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig, IntcConfig->CpuBaseAddress);
	if(Status != XST_SUCCESS){
		xil_printf("Interrupt controller initialization failed..");
		return -1;
	}

	Status = XScuGic_Connect(IntcInstancePtr,ReadIntrId,(Xil_InterruptHandler)XAxiVdma_ReadIntrHandler,(void *)AxiVdmaPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed read channel connect intc %d\r\n", Status);
		return XST_FAILURE;
	}

	XScuGic_Enable(IntcInstancePtr,ReadIntrId);

	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,(Xil_ExceptionHandler)XScuGic_InterruptHandler,(void *)IntcInstancePtr);
	Xil_ExceptionEnable();

	/* Register call-back functions
	 */
	XAxiVdma_SetCallBack(AxiVdmaPtr, XAXIVDMA_HANDLER_GENERAL, ReadCallBack, (void *)AxiVdmaPtr, XAXIVDMA_READ);

	XAxiVdma_SetCallBack(AxiVdmaPtr, XAXIVDMA_HANDLER_ERROR, ReadErrorCallBack, (void *)AxiVdmaPtr, XAXIVDMA_READ);

	return XST_SUCCESS;
}

int select_image(u32 btn_check ,int activeBuffer  )
{
	///Push button control which image gets displayed
	if (btn_check ==1 )
	    		{
	    			drawImage(HSize,VSize,imgHSize,imgVSize,(HSize-imgHSize)/2,(VSize-imgVSize)/2,carmask,activeBuffer);
	    			xil_printf("Displaying extracted cars  \r\n");

	    		}
	    		else if(btn_check ==2)
				{
	    			xil_printf("Displaying parking lot mask \r\n");
	    			drawImage(HSize,VSize,imgHSize,imgVSize,(HSize-imgHSize)/2,(VSize-imgVSize)/2,parking_mask,activeBuffer);

				}
	    		else
	    		{
	    			xil_printf("Displaying colored parking lot \r\n");
	    			drawColorImage(HSize,VSize,imgHSize,imgVSize,(HSize-imgHSize)/2,(VSize-imgVSize)/2,color_car,activeBuffer);

	    		}
	Xil_DCacheFlush();
		return 0;

}


// draw image for grayscale/binary image reconverted to unint8
int drawImage(u32 displayHSize,u32 displayVSize,u32 imageHSize,u32 imageVSize,u32 hOffset, u32 vOffset,unsigned char *imagePointer,int activeBuffer){
	for(int i=0;i<VSize;i++){
		for(int j=0;j<HSize;j++){
			if(i<vOffset || i >= vOffset+imageVSize){
				Buffer[activeBuffer][(i*HSize*3)+(j*3)]   = 0x00;
			    Buffer[activeBuffer][(i*HSize*3)+(j*3)+1] = 0x00;
			    Buffer[activeBuffer][(i*HSize*3)+(j*3)+2] = 0x00;
			}
			else if(j<hOffset || j >= hOffset+imageHSize){
				Buffer[activeBuffer][(i*HSize*3)+(j*3)]   = 0x00;
			    Buffer[activeBuffer][(i*HSize*3)+(j*3)+1] = 0x00;
			    Buffer[activeBuffer][(i*HSize*3)+(j*3)+2] = 0x00;
			}
			else {
				Buffer[activeBuffer][(i*HSize*3)+j*3]     = *imagePointer >> 4;
			    Buffer[activeBuffer][(i*HSize*3)+(j*3)+1] = *imagePointer >> 4;
			    Buffer[activeBuffer][(i*HSize*3)+(j*3)+2] = *imagePointer >> 4;
			    imagePointer++;
			}
		}
	}
	Xil_DCacheFlush();
	return 0;
}
int drawColorImage(u32 displayHSize,u32 displayVSize,u32 imageHSize,u32 imageVSize,u32 hOffset, u32 vOffset,unsigned int *imagePointer,int activeBuffer){

	for(int i=0;i<VSize;i++){
		for(int j=0;j<HSize;j++){
			//pixel out of frame become black
			if(i<vOffset || i >= vOffset+imageVSize){
				Buffer[activeBuffer][(i*HSize*3)+(j*3)]   = 0x00;
			    Buffer[activeBuffer][(i*HSize*3)+(j*3)+1] = 0x00;
			    Buffer[activeBuffer][(i*HSize*3)+(j*3)+2] = 0x00;
			}
			else if(j<hOffset || j >= hOffset+imageHSize){
				Buffer[activeBuffer][(i*HSize*3)+(j*3)]   = 0x00;
			    Buffer[activeBuffer][(i*HSize*3)+(j*3)+1] = 0x00;
			    Buffer[activeBuffer][(i*HSize*3)+(j*3)+2] = 0x00;
			}
			else {
				uint8_t Red = (*imagePointer & 0xFF0000) >> 16; //extracting red pixel
				uint8_t Green = (*imagePointer & 0xFF00) >> 8;  // extracting green pixel
				uint8_t Blue = (*imagePointer & 0xFF); // extracting blue pixel

				//  assign pixel to buffer while normalize pixels to 4 bits

				Buffer[activeBuffer][(i*HSize*3)+j*3]     = Red >> 4;// red channel
			    Buffer[activeBuffer][(i*HSize*3)+(j*3)+1] = Green >> 4; // green channel
			    Buffer[activeBuffer][(i*HSize*3)+(j*3)+2] = Blue >> 4; // blue channel
			    imagePointer++;
			}
		}
	}
	Xil_DCacheFlush();
	return 0;
}
int checkparking(u32 imageHSize,u32 imageVSize,int x ,int y , unsigned char *image1 ,unsigned char *image2,u32 dip,int activeBuffer)
{
	for(int i=0;i<imageVSize;i++)
	{
		for(int j=0;j<imageHSize;j++)
		{
			if(i== y && j == x)
			{


				uint8_t pixel_car = *image1;
				uint8_t pixel_mask = *image2;
				xil_printf("pixel value for parking lot in car mask is  %d\r\n", pixel_car);
				xil_printf("pixel value for parking lot in parking lot mask is  %d\r\n", pixel_mask);
				xil_printf("Coordinates at x = %d and y = %d\r\n", x, y);
				if(pixel_car == pixel_mask)
				{

					xil_printf("\na car is parked at parking lot %d\r\n ", dip);
					// drawing red  x at taken space
					int screenX = x + (HSize - imgHSize) / 2;
					int screenY = y + (VSize - imgVSize) / 2;
						drawRedX(screenX, screenY, activeBuffer);
						//sleep(3);
				}

				else
				{
					xil_printf("\nthis parking lot is free at parking lot  %d\r\n", dip);
					// draw green circle if parking is free
					int screenX = x + (HSize - imgHSize) / 2;
					int screenY = y + (VSize - imgVSize) / 2;
					drawGreenCircle(screenX, screenY, activeBuffer);
					//sleep(3);
				}

			}
			image1++;
			image2++;

		}


	}
	Xil_DCacheFlush();
		return 0;
}
void drawRedX(int centerX, int centerY,int activeBuffer) {
	int size = 10;       // Half-length of each diagonal line
	int thickness = 4;  // Thickness of the lines

	for (int i = -size; i <= size; i++) {
		for (int t = -thickness; t <= thickness; t++) {

			// Line from top-left to bottom-right
			int x1 = centerX + i + t;
			int y1 = centerY + i;

			// Line from bottom-left to top-right
			int x2 = centerX + i + t;
			int y2 = centerY - i;

			// Bounds check and draw pixel
			if (x1 >= 0 && x1 < HSize && y1 >= 0 && y1 < VSize)
			{
				int index = (y1 * HSize * 3) + (x1 * 3);
				Buffer[activeBuffer][index]     = 0x0F; // Red
				Buffer[activeBuffer][index + 1] = 0x00; // Green
				Buffer[activeBuffer][index + 2] = 0x00; // Blue
			}

			if (x2 >= 0 && x2 < HSize && y2 >= 0 && y2 < VSize)
			{
				int index = (y2 * HSize * 3) + (x2 * 3);
				Buffer[activeBuffer][index]     = 0x0F; // Red
				Buffer[activeBuffer][index + 1] = 0x00; // Green
				Buffer[activeBuffer][index + 2] = 0x00; // Blue
			}
		}
	}
}
void drawGreenCircle(int centerX, int centerY,int activeBuffer) {
	int radius = 10; //raidus of the circle
	int thickness = 3; // assgn how thick the circle will be

	for (int iy = -radius; iy <= radius; iy++) // for loop to draw  circle
	{
		for (int jx = -radius; jx <= radius; jx++)
		{
			int distSquared = jx * jx + iy * iy; //distance formula calculation
			if (distSquared >= (radius - thickness) * (radius - thickness) && distSquared <= radius * radius) {
				int px = centerX + jx;// getting x coordinate in point of circle
				int py = centerY + iy; // getting y coordinate in point of circle

				// Make sure we're within the screen bounds
				if (px >= 0 && px < HSize && py >= 0 && py < VSize) {
					int index = (py * HSize * 3) + (px * 3);
					Buffer[activeBuffer][index]     = 0x00; // Red
					Buffer[activeBuffer][index + 1] = 0x0F; // Green
					Buffer[activeBuffer][index + 2] = 0x00; // Blue
				}
			}
		}
	}
}
