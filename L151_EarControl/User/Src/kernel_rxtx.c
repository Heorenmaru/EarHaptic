/*
 * au_logic.c
 *
 *  Created on: Mar 16, 2022
 *      Author: Heorenmaru
 */

#include "kernel.h"

#define DEVICE_CODE 0x00FFU
#define DEVICE_VER 	0x00U

/////////////////////////////////////////////////////////////////////////////////
// HANDLERS and VARS
/////////////////////////////////////////////////////////////////////////////////

uint8_t lora_state = 1; //0 - Standby; 1 - Listen
uint8_t lora_rx_ch = 0;

#define T_TIMEOUT  1
#define T_OK 0



extern DAC_HandleTypeDef hdac;
extern ADC_HandleTypeDef hadc;
/////////////////////////////////////////////////////////////////////////////////
// USB Receive logic
/////////////////////////////////////////////////////////////////////////////////
uint8_t EarLPulsesD = 0;
uint32_t tdelay = 0;
uint8_t EarRPulsesD = 0;


void usb_callback(uint8_t *arr, uint16_t len){



	////////////
	// DEV INFO
	// (STANDART COMMAND)
	if(arr[0] == 0 ){

		uint16_t *idBase0 = (uint16_t*)(UID_BASE);
		uint16_t *idBase1 = (uint16_t*)(UID_BASE + 0x02);
		uint32_t *idBase2 = (uint32_t*)(UID_BASE + 0x04);
		uint32_t *idBase3 = (uint32_t*)(UID_BASE + 0x08);

		usb_rst_cursor();
		usb_add_uint8(0x00);
		usb_add_uint16(idBase0);
		usb_add_uint16(idBase1);
		usb_add_uint32(idBase2);
		usb_add_uint32(idBase3);

		usb_add_uint16((uint8_t)DEVICE_CODE);
		usb_add_uint8((uint8_t)DEVICE_VER);

		usb_send_buff();

	}


	////////////////Ear L Digital
	if(arr[0] == 0x01 ){

		uint8_t state = arr[1];
		if(state>0){
			HAL_GPIO_WritePin(L1_GPIO_Port, L1_Pin, 1);
		}else{
			HAL_GPIO_WritePin(L1_GPIO_Port, L1_Pin, 0);
		}


		usb_rst_cursor();
		usb_add_uint8(0x01);
		usb_add_uint8(0x00);

		usb_send_buff();
	}

	////////////////Ear R Digital
	if(arr[0] == 0x02 ){


		uint8_t state = arr[1];
		if(state>0){
			HAL_GPIO_WritePin(L2_GPIO_Port, L2_Pin, 1);
		}else{
			HAL_GPIO_WritePin(L2_GPIO_Port, L2_Pin, 0);
		}


		usb_rst_cursor();
		usb_add_uint8(0x02);
		usb_add_uint8(0x00);

		usb_send_buff();
	}

	////////////////Ear L Analog
	if(arr[0] == 0x03 ){


		uint32_t pwr = arr[1]<<16 | arr[2]<<8 | arr[3];


		if(pwr>4095){
			pwr = 4095;
		}

		HAL_DAC_SetValue(&hdac, DAC1_CHANNEL_1, DAC_ALIGN_12B_R, pwr);



		usb_rst_cursor();
		usb_add_uint8(0x03);
		usb_add_uint8(0x00);

		usb_send_buff();
	}

	////////////////Ear R Analog
	if(arr[0] == 0x04 ){

		uint32_t pwr = arr[1]<<16 | arr[2]<<8 | arr[3];


		if(pwr>4095){
			pwr = 4095;
		}

		HAL_DAC_SetValue(&hdac, DAC1_CHANNEL_2, DAC_ALIGN_12B_R, pwr);




		usb_rst_cursor();
		usb_add_uint8(0x04);
		usb_add_uint8(0x00);

		usb_send_buff();
	}

	//////////////// Leda
	if(arr[0] == 0x05 ){
		uint8_t leds = arr[1];
		if((leds & 0b00000001) > 0){
			HAL_GPIO_WritePin(L1_GPIO_Port, L1_Pin, 1);
		} else {
			HAL_GPIO_WritePin(L1_GPIO_Port, L1_Pin, 0);
		}
		if((leds & 0b00000010) > 0){
			HAL_GPIO_WritePin(L2_GPIO_Port, L2_Pin, 1);
		} else {
			HAL_GPIO_WritePin(L2_GPIO_Port, L2_Pin, 0);
		}
		if((leds & 0b00000100) > 0){
			HAL_GPIO_WritePin(L3_GPIO_Port, L3_Pin, 1);
		} else {
			HAL_GPIO_WritePin(L3_GPIO_Port, L3_Pin, 0);
		}
		if((leds & 0b00001000) > 0){
			HAL_GPIO_WritePin(L4_GPIO_Port, L4_Pin, 1);
		} else {
			HAL_GPIO_WritePin(L4_GPIO_Port, L4_Pin, 0);
		}



		usb_rst_cursor();
		usb_add_uint8(0x05);
		usb_add_uint8(0x00);

		usb_send_buff();
	}



}


/////////////////////////////////////////////////////////////////////////////////
// ADCs
/////////////////////////////////////////////////////////////////////////////////
float adc_1 = 0.0f;
float adc_2 = 0.0f;
float adc_3 = 0.0f;
float vref_adc = 0.0f;


uint16_t adc_data[12 * 8] = { 0 };
uint16_t adc_data_raw[12] = { 0 };
uint8_t adc_conv_flag = 0;

/*
void calc_voltage()
{
	vref_adc 	= 0 ;
	adc_1 	= 0 ;
	adc_2 	= 0 ;
	adc_3 	= 0 ;

	for (uint8_t i = 0; i < 8*4; i = i + 4) {
		adc_1 	+= adc_data[i];
		adc_2 	+= adc_data[i + 1];
		adc_3 	+= adc_data[i + 2];
		vref_adc 	+= adc_data[i + 3];

	}

	adc_1 	= adc_1 	/ 8;
	adc_2 	= adc_2 	/ 8;
	adc_3 	= adc_3 	/ 8;
	vref_adc 	= vref_adc 	/ 8;


	vref_adc 	= ADC_MAX * ADC_REFERENCE_VOLTAGE / vref_adc;
	adc_1 	= vref_adc / ADC_MAX * adc_1;
	adc_2 	= vref_adc / ADC_MAX * adc_2;
	adc_3 	= vref_adc / ADC_MAX * adc_3;
}
*/
void calc_voltage_raw()
{
	for(uint8_t i=0; i<12; i++){
		adc_data_raw[i] = 0;
	}

	for (uint8_t i = 0; i < 12*8; i = i + 12) {
		for(uint8_t j=0; j<12; j++){
				adc_data_raw[i] += adc_data[i+j];
		}

	}

	for(uint8_t i=0; i<12; i++){
		adc_data_raw[i] = (int) adc_data_raw[i] / 8;
	}

}
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{

    if(hadc->Instance == ADC1)
    {
    	adc_conv_flag = 1;
        HAL_ADC_Stop_DMA(hadc); // это необязательно
        calc_voltage_raw();
        HAL_ADC_Start_DMA(hadc, (uint32_t*)&adc_data, 4*8);
    }
}
/////////////////////////////////////////////////////////////////////////////////
// INIT
/////////////////////////////////////////////////////////////////////////////////


void kernel_init() {

	usb_set_callback(&usb_callback);
    //////////////////////////////////

	HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
	HAL_DAC_Start(&hdac, DAC_CHANNEL_2);

	HAL_DAC_SetValue(&hdac, DAC1_CHANNEL_1, DAC_ALIGN_12B_R, 0);
	HAL_DAC_SetValue(&hdac, DAC1_CHANNEL_1, DAC_ALIGN_12B_R, 0);

	HAL_Delay(2000);

	HAL_GPIO_WritePin(L1_GPIO_Port, L1_Pin, 1);
	HAL_Delay(300);
	HAL_GPIO_WritePin(L1_GPIO_Port, L1_Pin, 0);

	HAL_Delay(300);
	HAL_GPIO_WritePin(L2_GPIO_Port, L2_Pin, 1);
	HAL_Delay(300);
	HAL_GPIO_WritePin(L2_GPIO_Port, L2_Pin, 0);

	HAL_Delay(300);
	HAL_GPIO_WritePin(L3_GPIO_Port, L3_Pin, 1);
	HAL_Delay(300);
	HAL_GPIO_WritePin(L3_GPIO_Port, L3_Pin, 0);

	HAL_Delay(300);
	HAL_GPIO_WritePin(L4_GPIO_Port, L4_Pin, 1);
	HAL_Delay(300);
	HAL_GPIO_WritePin(L4_GPIO_Port, L4_Pin, 0);
	HAL_Delay(300);

	if(HAL_ADC_Start_DMA(&hadc, (uint32_t*)&adc_data,  4*8) != HAL_OK){
			while(1){}
	}
	HAL_GPIO_WritePin(L4_GPIO_Port, L4_Pin, 1);


}


/////////////////////////////////////////////////////////////////////////////////
// MAIN
/////////////////////////////////////////////////////////////////////////////////



void kernel_main() {

	kernel_init();





    while(1){
    	while(!adc_conv_flag);
    	adc_conv_flag = 0;

    	//send adc

    	HAL_Delay(1);
    }
}



