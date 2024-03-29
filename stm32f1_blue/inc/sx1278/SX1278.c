/**
 * Author Wojciech Domski <Wojciech.Domski@gmail.com>
 * www: www.Domski.pl
 *
 * work based on DORJI.COM sample code and
 * https://github.com/realspinner/SX1278_LoRa
 */

#include "SX1278.h"
#include <string.h>

#include "inc/tool/delay.h"
//#include "inc/spi/spi.h"

#include "inc/tool/UARTLib.h"

//////////////////////////////////
// hardware											//
//////////////////////////////////

//=======================================================================
uint8_t SX1278_spi(SX1278_hw_t * hw, u08 byte) //sr0
	{
		uint8_t counter;
		
		for(counter = 8; counter; counter--)
		{
			if (byte & 0x80)	*(hw->mosi) = 1;
			else							*(hw->mosi) = 0;
			
			byte <<= 1;
			Delay_us(0);
			*(hw->sck) = 1; /* a slave latches input data bit */
			Delay_us(0);
			//if (*(hw->miso)==1)		byte |= 0x01;
			byte |= *(hw->miso);
			
			*(hw->sck) = 0; /* a slave shifts out next output data bit */
			Delay_us(0);
		}
		return(byte);
	}
	
__weak void SX1278_hw_SPICommand(SX1278_hw_t * hw, uint8_t cmd) 
{		
	SX1278_spi(hw, cmd);
	
	/*
		uint8_t counter;
		
		for(counter = 8; counter; counter--)
		{
			if (cmd & 0x80)	*(hw->mosi) = 1;
			else						*(hw->mosi) = 0;
			
			cmd <<= 1;
			
			*(hw->sck) = 1; 
			Delay_us(1);
			*(hw->sck) = 0; 
			Delay_us(1);
		}
		*/
	}

__weak void SX1278_hw_SetNSS(SX1278_hw_t * hw, int value) 
{
		//HAL_GPIO_WritePin(hw->nss.port, hw->nss.pin, (value == 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
		*(hw->nss) = value;
	}

__weak uint8_t SX1278_hw_SPIReadByte(SX1278_hw_t * hw) 
	{
		
		/* uint8_t txByte = 0x00;
		uint8_t rxByte = 0x00;
		SX1278_hw_SetNSS(hw, 0);
		rxByte = SX1278_spi(hw,txByte);
		return rxByte; */
		//SX1278_hw_SetNSS(hw, 0);
		return SX1278_spi(hw,0);
	}
	
__weak void SX1278_hw_init(SX1278_hw_t * hw) 
{
		//SX1278_hw_SetNSS(hw, 1);
		*(hw->nss) = 1;
		
		
		*(hw->rst) = 0;
		SX1278_hw_DelayMs(1);
		//LED=0;
		*(hw->rst) = 1;
		SX1278_hw_DelayMs(100);
		
	}

__weak void SX1278_hw_Reset(SX1278_hw_t * hw) 
{
		
		SX1278_hw_SetNSS(hw,1);
		*(hw->rst) = 0;
		SX1278_hw_DelayMs(1);
		*(hw->rst) = 1;
		SX1278_hw_DelayMs(100);
	}


__weak void SX1278_hw_DelayMs(uint32_t msec) 
{
		Delay_ms(msec);
		//Delay3_ms(msec); //required TIM3
	}

__weak int SX1278_hw_GetDIO0(SX1278_hw_t * hw) 
{	
		return *(hw->dio0);
	}

//////////////////////////////////
// logic												//
//////////////////////////////////

uint8_t SX1278_SPIRead(SX1278_t * module, uint8_t addr) 
{
		uint8_t tmp;
		SX1278_hw_SetNSS(module->hw, 0);
		SX1278_hw_SPICommand(module->hw, addr);
		tmp = SX1278_hw_SPIReadByte(module->hw);
		SX1278_hw_SetNSS(module->hw, 1);
		return tmp;
	}

void SX1278_SPIWrite(SX1278_t * module, uint8_t addr, uint8_t cmd) 
{
	SX1278_hw_SetNSS(module->hw, 0);
	SX1278_hw_SPICommand(module->hw, addr | 0x80);
	SX1278_hw_SPICommand(module->hw, cmd);
	SX1278_hw_SetNSS(module->hw, 1);
}

void SX1278_SPIBurstRead(SX1278_t * module, uint8_t addr, uint8_t* rxBuf,		uint8_t length) 
{
		uint8_t i;
		if (length == 0) {
			return;
		} else {
			SX1278_hw_SetNSS(module->hw, 0);
			SX1278_hw_SPICommand(module->hw, addr);
			for (i = 0; i < length; i++) {
				*(rxBuf + i) = SX1278_hw_SPIReadByte(module->hw);
				//*(rxBuf + i) = SX1278_spi(module->hw,0);
			}
			SX1278_hw_SetNSS(module->hw, 1);
		}
	}

void SX1278_SPIBurstWrite(SX1278_t * module, uint8_t addr, uint8_t* txBuf,		uint8_t length) 
{
		unsigned char i;
		if (length == 0) {
		return;
		} else {
			SX1278_hw_SetNSS(module->hw, 0);
			SX1278_hw_SPICommand(module->hw, addr | 0x80);
			for (i = 0; i < length; i++) {
				SX1278_hw_SPICommand(module->hw, *(txBuf + i));
			}
			SX1278_hw_SetNSS(module->hw, 1);
		}
	}

void SX1278_defaultConfig(SX1278_t * module) 
{
	SX1278_config(module, module->frequency, module->power, module->LoRa_Rate,module->LoRa_BW);
}

void SX1278_config(SX1278_t * module, uint8_t frequency, uint8_t power,	uint8_t LoRa_Rate, uint8_t LoRa_BW) 
{
	SX1278_sleep(module); //Change modem mode Must in Sleep mode
	SX1278_hw_DelayMs(15);

	SX1278_entryLoRa(module);
	//SX1278_SPIWrite(module, 0x5904); //?? Change digital regulator form 1.6V to 1.47V: see errata note

	SX1278_SPIBurstWrite(module, LR_RegFrMsb,	(uint8_t*) SX1278_Frequency[frequency], 3); //setting  frequency parameter

	//setting base parameter
	SX1278_SPIWrite(module, LR_RegPaConfig, SX1278_Power[power]); //Setting output power parameter
	
	
	SX1278_SPIWrite(module, LR_RegOcp, 0x0B);		//RegOcp,Close Ocp
	SX1278_SPIWrite(module, LR_RegLna, 0x23);		//RegLNA,High & LNA Enable
	if (SX1278_SpreadFactor[LoRa_Rate] == 6) {	//SFactor=6
		uint8_t tmp;
		SX1278_SPIWrite(module,
		LR_RegModemConfig1,
		((SX1278_LoRaBandwidth[LoRa_BW] << 4) + (SX1278_CR << 1) + 0x01)); //Implicit Enable CRC Enable(0x02) & Error Coding rate 4/5(0x01), 4/6(0x02), 4/7(0x03), 4/8(0x04)
		
		SX1278_SPIWrite(module,
		LR_RegModemConfig2,
		((SX1278_SpreadFactor[LoRa_Rate] << 4) + (SX1278_CRC << 2) + 0x03));
		
		tmp = SX1278_SPIRead(module, 0x31);
		tmp &= 0xF8;
		tmp |= 0x05;
		SX1278_SPIWrite(module, 0x31, tmp);
		SX1278_SPIWrite(module, 0x37, 0x0C);
	} else {
		SX1278_SPIWrite(module,		LR_RegModemConfig1,
		((SX1278_LoRaBandwidth[LoRa_BW] << 4) + (SX1278_CR << 1) + 0x00)); //Explicit Enable CRC Enable(0x02) & Error Coding rate 4/5(0x01), 4/6(0x02), 4/7(0x03), 4/8(0x04)
		
		SX1278_SPIWrite(module,		LR_RegModemConfig2,
		((SX1278_SpreadFactor[LoRa_Rate] << 4) + (SX1278_CRC << 2) + 0x03)); //SFactor &  LNA gain set by the internal AGC loop
	}

	SX1278_SPIWrite(module, LR_RegSymbTimeoutLsb, 0xFF); 	//RegSymbTimeoutLsb Timeout = 0x3FF(Max)
	SX1278_SPIWrite(module, LR_RegPreambleMsb, 0x00); 		//RegPreambleMsb
	SX1278_SPIWrite(module, LR_RegPreambleLsb, 12); 			//RegPreambleLsb 8+4=12byte Preamble
	SX1278_SPIWrite(module, REG_LR_DIOMAPPING2, 0x01); 		//RegDioMapping2 DIO5=00, DIO4=01
	module->readBytes = 0;
	SX1278_standby(module); 															//Entry standby mode
}

void SX1278_standby(SX1278_t * module) 
{
		SX1278_SPIWrite(module, LR_RegOpMode, 0x09);
		module->status = STANDBY;
	}

void SX1278_sleep(SX1278_t * module) 
{
		SX1278_SPIWrite(module, LR_RegOpMode, 0x08);
		module->status = SLEEP;
	}

void SX1278_entryLoRa(SX1278_t * module) 
{
		SX1278_SPIWrite(module, LR_RegOpMode, 0x88);
	}

void SX1278_clearLoRaIrq(SX1278_t * module) 
{
	SX1278_SPIWrite(module, LR_RegIrqFlags, 0xFF);
}

int  SX1278_LoRaEntryRx(SX1278_t * module, uint8_t length, uint32_t timeout) 
{
	uint8_t addr;

	module->packetLength = length;

	//SX1278_defaultConfig(module);													// Setting base parameter
	SX1278_SPIWrite(module, REG_LR_PADAC, 0x84);					// Normal and RX
	SX1278_SPIWrite(module, LR_RegHopPeriod, 0xFF);				// No FHSS
	SX1278_SPIWrite(module, REG_LR_DIOMAPPING1, 0x01);		// DIO=00,DIO1=00,DIO2=00, DIO3=01  00-RXdone 01-TXDone
	SX1278_SPIWrite(module, LR_RegIrqFlagsMask, 0x3F);		// Open RxDone interrupt & Timeout
	SX1278_clearLoRaIrq(module);
	SX1278_SPIWrite(module, LR_RegPayloadLength, length);	// Payload Length 21byte(this register must difine when the data long of one byte in SF is 6)
	addr = SX1278_SPIRead(module, LR_RegFifoRxBaseAddr ); // Read RxBaseAddr
	SX1278_SPIWrite(module, LR_RegFifoAddrPtr, addr); 	 	// RxBaseAddr->FiFoAddrPtr
	SX1278_SPIWrite(module, LR_RegOpMode, 0x8d);					// Mode//Low Frequency Mode
	//SX1278_SPIWrite(module, LR_RegOpMode,0x05);	//Continuous Rx Mode //High Frequency Mode
	module->readBytes = 0;

	while (1) {
		if ((SX1278_SPIRead(module, LR_RegModemStat) & 0x04) == 0x04) {	//Rx-on going RegModemStat
			module->status = RX;
			return 1;
		}
		if (--timeout == 0) {
			SX1278_hw_Reset(module->hw);
			SX1278_defaultConfig(module);
			return 0;
		}
		SX1278_hw_DelayMs(1);
	}
}

u08  SX1278_LoRaRxPacket(SX1278_t * module) 
{
	uint8_t addr, packet_size;

	//if (SX1278_hw_GetDIO0(module->hw)) {
		memset(module->rxBuffer, 0x00, SX1278_MAX_PACKET);			// zerowanie bufora odbioru
		
		addr = SX1278_SPIRead(module, LR_RegFifoRxCurrentaddr); //last packet addr
		UaPutC('1');
		SX1278_SPIWrite(module, LR_RegFifoAddrPtr, addr); //RxBaseAddr -> FiFoAddrPtr gdy chcemy odczytac ostatnie dane
		UaPutC('2');
		if (module->LoRa_Rate == SX1278_LORA_SF_6) { //When SpreadFactor is six,will used Implicit Header mode(Excluding internal packet length)
			packet_size = module->packetLength;
		} else {
			packet_size = SX1278_SPIRead(module, LR_RegRxNbBytes); //Number for received bytes
			UaPutC('3');
		}
		
		UaPutC('(');
		uint2uart(module->packetLength);
		UaPutC(',');
		uint2uart(addr);
		UaPutC(')');
		SX1278_SPIBurstRead(module, 0x00, module->rxBuffer, packet_size);
		UaPutC('4');
		module->readBytes = packet_size;
		UaPutC('5');
		SX1278_clearLoRaIrq(module);
		UaPutC('6');
		
		
	//} 
	return module->readBytes;
}

uint8_t  SX1278_LoRaRx(SX1278_t * module) 
{
	uint8_t addr, packet_size;

	//	memset(module->rxBuffer, 0x00, SX1278_MAX_PACKET);			// zerowanie bufora odbioru
		
		addr = SX1278_SPIRead(module, LR_RegFifoRxCurrentaddr); //last packet addr
		SX1278_SPIWrite(module, LR_RegFifoAddrPtr, addr); //RxBaseAddr -> FiFoAddrPtr gdy chcemy odczytac ostatnie dane

		if (module->LoRa_Rate == SX1278_LORA_SF_6) { //When SpreadFactor is six,will used Implicit Header mode(Excluding internal packet length)
			packet_size = module->packetLength;
		} else {
			packet_size = SX1278_SPIRead(module, LR_RegRxNbBytes); //Number for received bytes
		}
		
		if(module->packetLength == packet_size) {
			SX1278_SPIBurstRead(module, 0x00, module->rxBuffer, packet_size);
		}
		
		module->readBytes = packet_size;
		SX1278_clearLoRaIrq(module);
		
	return module->readBytes;
}

int  SX1278_LoRaEntryTx(SX1278_t * module, uint8_t length, uint32_t timeout) 
{
	uint8_t addr;
	uint8_t temp;

	module->packetLength = length;

	//SX1278_defaultConfig(module); 											//setting base parameter
	SX1278_SPIWrite(module, REG_LR_PADAC, 0x87);					//Tx for 20dBm
	SX1278_SPIWrite(module, LR_RegHopPeriod, 0x00); 			//RegHopPeriod NO FHSS
	SX1278_SPIWrite(module, REG_LR_DIOMAPPING1, 0x41); 		//DIO0=01, DIO1=00,DIO2=00, DIO3=01
	SX1278_clearLoRaIrq(module);
	SX1278_SPIWrite(module, LR_RegIrqFlagsMask, 0xF7); 		//Open TxDone interrupt
	SX1278_SPIWrite(module, LR_RegPayloadLength, length); //RegPayloadLength 21byte
	addr = SX1278_SPIRead(module, LR_RegFifoTxBaseAddr); 	//RegFiFoTxBaseAddr
	SX1278_SPIWrite(module, LR_RegFifoAddrPtr, addr); 		//RegFifoAddrPtr

	while (1) {
		temp = SX1278_SPIRead(module, LR_RegPayloadLength);
		if (temp == length) {
			module->status = TX;
			return 1;
		}
		
		if (--timeout == 0) {
			SX1278_hw_Reset(module->hw);
			SX1278_defaultConfig(module);
			return 0;
		}
	}
}

int  SX1278_LoRaTxPacket(SX1278_t * module, uint8_t* txBuffer, uint8_t length,		uint32_t timeout) 
{
	SX1278_SPIBurstWrite(module, 0x00, txBuffer, length);
	SX1278_SPIWrite(module, LR_RegOpMode, 0x8b);	//Tx Mode
	while (1) {
		if (SX1278_hw_GetDIO0(module->hw)) { 				//if(Get_NIRQ()) //Packet send over
			SX1278_SPIRead(module, LR_RegIrqFlags);
			SX1278_clearLoRaIrq(module); 							//Clear irq
			SX1278_standby(module); 									//Entry Standby mode
			return 1;
		}
		
		if (--timeout == 0) {
			SX1278_hw_Reset(module->hw);
			SX1278_defaultConfig(module);
			return 0;
		}
		SX1278_hw_DelayMs(1);
	}
}

void  SX1278_LoRaTxPacketStart(SX1278_t * module, uint8_t* txBuffer, uint8_t length) 
{
	SX1278_SPIBurstWrite(module, 0x00, txBuffer, length);
	SX1278_SPIWrite(module, LR_RegOpMode, 0x8b);	//Tx Mode
}

void  SX1278_LoRaTxPacketEnd(SX1278_t * module) 
{
	//SX1278_SPIRead(module, LR_RegIrqFlags);
	SX1278_clearLoRaIrq(module); 							//Clear irq
	SX1278_standby(module); 									//Entry Standby mode
}

void SX1278_begin(SX1278_t * module, uint8_t frequency, uint8_t power,		uint8_t LoRa_Rate, uint8_t LoRa_BW) 
{
	
	SX1278_hw_init(module->hw);
	module->frequency = frequency;
	module->power 		= power;
	module->LoRa_Rate = LoRa_Rate;
	module->LoRa_BW = LoRa_BW;
	SX1278_defaultConfig(module);
}

int  SX1278_transmit(SX1278_t * module, uint8_t* txBuf, uint8_t length,		uint32_t timeout) 
{
	if (SX1278_LoRaEntryTx(module, length, timeout)) {
		return SX1278_LoRaTxPacket(module, txBuf, length, timeout);
	}
	return 0;
}

int  SX1278_receive(SX1278_t * module, uint8_t length, uint32_t timeout) 
{
	return SX1278_LoRaEntryRx(module, length, timeout);
}

uint8_t SX1278_available(SX1278_t * module) 
{
	return SX1278_LoRaRxPacket(module);
}

uint8_t SX1278_read(SX1278_t * module, uint8_t* rxBuf) 
{
	uint8_t length;
	
	length = module->readBytes;
	//memcpy(rxBuf, module->rxBuffer, length);
	rxBuf[length] = '\0';
	module->readBytes = 0;
	return length;
}

uint8_t SX1278_RSSI_LoRa(SX1278_t * module) 
{
	uint32_t temp = 10;
	temp = SX1278_SPIRead(module, LR_RegRssiValue); //Read RegRssiValue, Rssi value
	temp = temp + 127 - 137; //127:Max RSSI, 137:RSSI offset
	return (uint8_t) temp;
}

uint8_t SX1278_RSSI(SX1278_t * module) 
{
	uint8_t temp = 0xff;
	temp = SX1278_SPIRead(module, 0x11);
	temp = 127 - (temp >> 1);	//127:Max RSSI
	return temp;
}


