/************************************************************************************************
Program Name  :   videotest.cpp
Description   :   This is an example program which automatically takes the first frame and takes
                  in data from the endpoint then calculate the FPS. Note that this application 
		  only supports bulk transfer. This application only calculates the data amount 
		  and do not decode data, so raw format is also supported.
Author        :   Eddie Zhao 
Version       :   0.1
Date          :   7/12/2021 
************************************************************************************************/


#include<stdio.h>
#include<iostream>
#include<cstring>
#include<time.h>
#include<libusb-1.0/libusb.h>
using namespace std;

#define totaltime               100                   //Test time
#define buffercount             16                    //Buffer amount used to store data
#define buffersize              0x10000               //Size for one data buffer

uint32_t framecount=0;
uint32_t framesize=0;
uint32_t frametotalsize=0;

/*
static void xfercallback(struct libusb_transfer *transfer){
        
}
*/

/*
 *Bulk Transfer Callback
 */

static void bulkcallback(struct libusb_transfer *transfer){
	if(transfer->buffer[1]==0x8c||transfer->buffer[1]==0x8d)
	{
             framesize+=transfer->actual_length-12;    //deduct UVC header length from total length
	}
	else if(transfer->buffer[1]==0x8e||transfer->buffer[1]==0x8f)
	{
	     framesize+=transfer->actual_length-12;
	     if(framesize==frametotalsize)
	     {
		     framesize=0;
		     framecount++;
	     }
	     else
	     {
		     cout<<"frame size incorrect, frame discarded"<<endl;
	     }
	}
	else
	{
	      cout<<"Unrecognized transfer"<<endl;
	}
	libusb_submit_transfer(transfer);
}

int main()
{
	struct libusb_device *device;                           
        struct libusb_device_handle *device_handle;
	struct timeval begin,end,start,stop;
	unsigned int timeinterval;
	//unsigned int totaltime=10;
	int streamaddress=-1; 

	uint16_t bitsperpixel=0,width=0,height=0;
	
        libusb_init(NULL);
       	device_handle = libusb_open_device_with_vid_pid(NULL,0x04b4,0x00C3);  
	if(device_handle==NULL)
	{
		cout<<"No Cypress device found"<<endl;
		return 0;
	}
	device = libusb_get_device(device_handle);
	struct libusb_device_descriptor  desc;
	libusb_get_device_descriptor(device,&desc);
	printf("idVendor %04x \n",desc.idVendor);
	printf("idProduct %04x \n",desc.idProduct);
	struct libusb_config_descriptor * confgdescr;
	libusb_get_config_descriptor(device,0,&confgdescr);
	printf("Interface Num %d \n",confgdescr->bNumInterfaces);
	for (int i=0;i<confgdescr->bNumInterfaces;i++)
	{
		for (int j=0;j<confgdescr->interface[i].num_altsetting;j++)
		{
                printf("interface %d, altsetting %d, ",i,j);
		printf("NumEndpoints %d ",confgdescr->interface[i].altsetting[j].bNumEndpoints);
		for(int k=0;k<confgdescr->interface[i].altsetting[j].bNumEndpoints;k++)
		{
		
			if(confgdescr->interface[i].altsetting[j].bInterfaceClass==0x0E)
			{
				if(confgdescr->interface[i].altsetting[j].bInterfaceSubClass==0x01)
		       	     printf("video control endpoint %d address 0x%02x \n",k,confgdescr->interface[i].altsetting[j].endpoint[k].bEndpointAddress);
		                else if(confgdescr->interface[i].altsetting[j].bInterfaceSubClass==0x02)
				{
			       	     printf("video streaming  endpoint %d address %02x \n",k,confgdescr->interface[i].altsetting[j].endpoint[k].bEndpointAddress);
			             streamaddress=confgdescr->interface[i].altsetting[j].endpoint[k].bEndpointAddress;
				     printf("GUID: ");
				     for(int a=0;a<16;a++)
				     {
				             printf("%02x ",confgdescr->interface[i].altsetting[j].extra[19+a]);   //Print Format GUID
				     }
				     
					     bitsperpixel=confgdescr->interface[i].altsetting[j].extra[35];
                                             width = (uint16_t )confgdescr->interface[i].altsetting[j].extra[46]+(uint16_t )confgdescr->interface[i].altsetting[j].extra[47]*256;
					     height =(uint16_t )confgdescr->interface[i].altsetting[j].extra[48]+(uint16_t )confgdescr->interface[i].altsetting[j].extra[49]*256;
					     printf("Pixel Width: %d Pixel Height:%d",width,height);
					     frametotalsize=bitsperpixel/8*width*height;
				     printf("frame size:%d \n",frametotalsize);
				}
			}
		}
		}
	}
	int status=0;
	status=libusb_kernel_driver_active(device_handle,0);
	if(status!=0)
	{
	cout<<"kernel driver active "<<status<<endl;
        status=libusb_detach_kernel_driver(device_handle,0);
	if(status!=0)
	{
		printf("detach error code %x\r\n",status);
		return 0;
	}
	else
	{
		cout<<"detach kernel driver success"<<endl;
	}
	}
	status=libusb_claim_interface(device_handle,0);
	if(status!=0)
		printf("claim error code %x\r\n",status);
	status=libusb_set_interface_alt_setting(device_handle,1,0);
	if(status!=0)
		printf("set interface error code %x\r\n",status);
	
	uint8_t *data;
	struct libusb_transfer *ctrltx,*ctrlrx;
	
	ctrltx=libusb_alloc_transfer(0);
	ctrlrx=libusb_alloc_transfer(0);
	data = (unsigned char *)calloc(1024,sizeof(unsigned char*));
        uint8_t GET_CUR[8]={0xA1,0x81,0x00,0x01,0x01,0x00,0x22,0x00};
	memcpy(data,GET_CUR,sizeof(GET_CUR));
	libusb_fill_control_setup(data,0xA1,0x81,0x0100,0x0001,34);
	libusb_fill_control_transfer(ctrlrx,device_handle,(unsigned char *)data,NULL,NULL,5000);
	status=libusb_submit_transfer(ctrlrx);
	if(status!=0){
		printf("transfer error code %x\r\n",status);
	}
	else{
		cout<<"GET_CUR"<<endl;
	}
	libusb_handle_events(NULL);            //wait for transfer complete otherwise there will be no data in the data buffer
	uint8_t SET_CUR[8]={0x21,0x01,0x00,0x02,0x01,0x00,0x22,0x00};
	memcpy(data,SET_CUR,sizeof(SET_CUR));
	libusb_fill_control_setup(data,0x21,0x01,0x0200,0x0001,34);
	libusb_fill_control_transfer(ctrltx,device_handle,(unsigned char *)data,NULL,NULL,5000);
        status=libusb_submit_transfer(ctrltx);
	if(status!=0){
		printf("transfer error code %x\r\n",status);
	}
	else {
		cout<<"SET_CUR" <<endl;
	}
	libusb_handle_events(NULL);
	struct libusb_transfer **bulktx;
	uint8_t **databuffer;
	bulktx = (struct libusb_transfer **)calloc (buffercount,sizeof(struct libusb_transfer *));
	databuffer = (unsigned char **)calloc(buffercount,sizeof(unsigned char *));
	for(int i=0;i<buffercount;i++)
	{
		databuffer[i]= (unsigned char *)malloc (buffersize);
	        bulktx[i]=libusb_alloc_transfer(0);
	}
	for(int i=0;i<buffercount;i++)
	{
		libusb_fill_bulk_transfer(bulktx[i],device_handle,streamaddress,databuffer[i],buffersize,bulkcallback,NULL,5000);
		status=libusb_submit_transfer(bulktx[i]);
		if(status!=0)
			cout<<"transfer error"<<status<<endl;
	}
	gettimeofday(&start,NULL);
	gettimeofday(&begin,NULL);
	do
	{
		if(framecount==30)
		{
			gettimeofday(&end,NULL);
			timeinterval = ((end.tv_sec-begin.tv_sec)*1000000+(end.tv_usec-begin.tv_usec));
			printf("fps: %f \r\n",((double)framecount/((double)timeinterval/1000000)));
			framecount=0;
			begin=end;
		}
		libusb_handle_events(NULL);
		gettimeofday(&stop,NULL);
	}while((stop.tv_sec-start.tv_sec)<=totaltime);
        
	uint8_t setup2[8]={0x02,0x01,0x00,0x00,0x83,0x00,0x00,0x00};     //send clear feature and stop streaming
	memcpy(data,setup2,sizeof(setup2));
	libusb_fill_control_setup(data,0x02,0x01,0,0x0083,0);
	libusb_fill_control_transfer(ctrltx,device_handle,(unsigned char *)data,NULL,NULL,5000);
	status=libusb_submit_transfer(ctrltx);                                   
	if(status!=0){
		printf("transfer error code %x\n",status);
	}
	else {
		cout<<"clear feature" <<endl;
	}
	for(int i=0;i<buffercount;i++)
	{
		if(databuffer[i]!=NULL)
			free(databuffer[i]);
		databuffer[i]=NULL;
	}

	free(databuffer);
	libusb_free_transfer(ctrltx);
	libusb_free_config_descriptor(confgdescr);
        libusb_close(device_handle);
	libusb_exit(0);
	return 0; 
}
