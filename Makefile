TARGET?=powermonkey
#defining chip speed
F_CPU=1000000UL
MCU?=attiny85
MCU_AVRDUDE?=t85
USB_PORT?=/dev/ttyACM0
PROGRAMMER_BAUD?=19200
# For uploading to the PI
PI_ADDRESS?=pi@192.168.0.105:/flash/program_attiny

TWI_LIB_HEADERS=./usitwislave/usitwislave.h

HEADERS=utils.h 
OBJECT_FILES=powermonkey.o

CXX=avr-gcc
CXXFLAGS=-g -Os -Wall -mcall-prologues -mmcu=$(MCU) -ffunction-sections -fdata-sections -Wl,--gc-sections -D F_CPU=$(F_CPU)
AR=avr-ar
OBJ2HEX=avr-objcopy
AVRDUDE=avrdude

export CXX
export CXXFLAGS
export AR

all: $(TARGET).hex

%.o: %.cpp $(HEADERS)
	$(CXX) -c $(CXXFLAGS) $< -o $@

usitwislave.a: usitwislave/usitwislave.c
	$(CXX) -c $(CXXFLAGS) $< -o usitwislave/usitwislave.o
	$(AR) cr $@ usitwislave/usitwislave.o

attiny_utils.a: attiny_utils/attiny_utils.a
	cp attiny_utils/attiny_utils.a .
	
attiny_utils/attiny_utils.a :
	$(MAKE) -C attiny_utils

$(TARGET): $(OBJECT_FILES) usitwislave.a attiny_utils.a
	$(CXX) $(CXXFLAGS) $^ -o $@

$(TARGET).hex: $(TARGET)
	$(OBJ2HEX) -R .eeprom -O ihex $(TARGET) $@

upload: $(TARGET).hex
	$(AVRDUDE) -c avrisp -p $(MCU_AVRDUDE) -P $(USB_PORT) -b $(PROGRAMMER_BAUD) -U flash:w:$(TARGET).hex:i 

copy_to_pi: $(TARGET).hex
	scp $^ $(PI_ADDRESS)

clean:
	$(MAKE) -C attiny_utils clean
	rm $(TARGET) $(TARGET).hex $(OBJECT_FILES) usitwislave.a usitwislave/usitwislave.o

.PHONY: upload copy_to_pi clean
