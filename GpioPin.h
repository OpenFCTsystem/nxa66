/*
 * Andy's Workshop NXA66 controller ATMega328p firmware
 * Copyright (c) 2017 Andy Brown. http://www.andybrown.me.uk
 * Please see website for licensing terms.
 */

#pragma once


namespace nxa66 {

  /*
   * GPIO ports
   */

  struct GPIOB {
    enum {
      Port = 0x05,
      Dir  = 0x04,
      Pin  = 0x03
    };
  };

  struct GPIOC {
    enum {
      Port = 0x08,
      Dir  = 0x07,
      Pin  = 0x06
    };
  };

  struct GPIOD {
    enum {
      Port = 0x0b,
      Dir  = 0x0a,
      Pin  = 0x09
    };
  };


  /*
   * Base template for all GPIO pins. Provide support for set/reset
   */

  template<uint8_t TPort,uint8_t TPin>
  struct GpioPin {
   
    /*
     * Immediately set a pin to HIGH
     */

    static void set() {
      asm volatile(
        "  sbi  %[port],%[pin]  \n\t"
        :: [port] "I" (TPort),
           [pin]  "I" (TPin)
      );
    }


    /*
     * Immediately set a pin to LOW
     */

    static void reset() {
      asm volatile(
        "  cbi  %[port],%[pin]  \n\t"
        :: [port] "I" (TPort),
           [pin]  "I" (TPin)
      );
    }


    /*
     * Change state to the given state immediately
     */

    static void changeTo(bool state) {
      if(state)
        set();
      else
        reset();
    }
  };


  /*
   * GPIO output, provide support for init
   */

  template<class TPort,uint8_t TPin>
  struct GpioOutputPin : GpioPin<TPort::Port,TPin> {

    /*
     * Setup the port pin by setting the direction bit in the port register
     */

    static void setup() {    
      asm volatile(
        "  sbi  %[port],%[pin]  \n\t"
        :: [port] "I" (TPort::Dir),
           [pin]  "I" (TPin)
      );
    }
  };


  /*
   * GPIO input, provide support for init, read
   */

  template<class TPort,uint8_t TPin>
  struct GpioInputPin : GpioPin<TPort::Port,TPin> {

    /*
     * Setup the pin by clearing the direction bit in the port register
     */

    static void setup() {    
      asm volatile(
        "  cbi  %[port],%[pin]  \n\t"
        :: [port] "I" (TPort::Dir),
           [pin]  "I" (TPin)
      );
    }


    /*
     * Read the current value of an input pin
     */

    static bool read() {
      
      uint8_t r;

      asm volatile(
          "  clr  %[result]       \n\t"       // result = 0
          "  sbic %[port],%[pin]  \n\t"       // skip next if port bit is clear
          "  inc  %[result]       \n\t"       // result = 1
        : [result] "=r" (r)
        : [port]   "I"  (TPort::Pin),
          [pin]    "I"  (TPin)
      );

      return r;
    }
  };


  /*
   * All pins used in this project
   */

  typedef GpioOutputPin<GPIOD,1> GpioUartTx;
  typedef GpioInputPin<GPIOD,0> GpioUartRx;

  typedef GpioInputPin<GPIOB,0> GpioEncoderA;
  typedef GpioInputPin<GPIOD,6> GpioEncoderB;
  typedef GpioInputPin<GPIOC,1> GpioActionButton;

  typedef GpioOutputPin<GPIOB,2> GpioSpiCs;
  typedef GpioOutputPin<GPIOB,5> GpioSpiClk;

  typedef GpioOutputPin<GPIOD,5> GpioOutputEnable;
  typedef GpioInputPin<GPIOB,7> GpioOutputEnableSwitch;

  typedef GpioOutputPin<GPIOB,1> GpioVsp;
  typedef GpioInputPin<GPIOB,6> GpioVspSwitch;

  typedef GpioOutputPin<GPIOD,3> GpioPowerOn;

  typedef GpioOutputPin<GPIOD,4> GpioFanSwitch;

  typedef GpioOutputPin<GPIOC,3> GpioPowerGoodLed;

  struct GpioSpiMosi : GpioOutputPin<GPIOB,3> {

    static void writeByte(uint8_t b) {
      SPDR=b;
      while(!(SPSR & (1 << SPIF)));
    }
  };

  struct GpioPowerGood : GpioInputPin<GPIOD,2> {
    
    static void setup() {
      
      // set as input. PD2 = INT0

      GpioInputPin<GPIOD,2>::setup();

      // configure the INT0 interrupt

      EICRA = (1 << ISC00);    // both edges of INT0
      EIMSK = (1 << INT0);      // INT0 enabled
    }
  };
}
