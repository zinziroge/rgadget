#include <SPI.h>

/*
  Digital Pin Assign
  0   RX for connection between Arduino and PC
  1   TX for connection between Arduino and PC
  2   RX for ANT+. Software Serial.
  3   TX for ANT+. Software Serial.
  4   CS SD // Chip Select SD Card
  5   RX for GPS. Software Serial.
  6   TX for GPS. Software Serial.
  7
  8   
  9   STB   // Latch Shift Register for LED Matrix
  10  SS    // SPI. no use
  11  MOSI  // SPI. 'Master Out Slave In' for SD and LED Matrix.
  12  MISO  // SPI. 'Master In Slave Out' for SD and LED Matrix.  
  13  SCK   // SPI. 'Mater Clock' for SD and LED Matrix.
*/
//#define __DEBUG__

//#define RX_PC  0
//#define TX_PC  1
#define RX_ANT  2
#define TX_ANT  3
#define CS_SD  4
#define STB  9

/*
  MSB                    LSB
    000 r4r3..r0 c24c23..c0
*/
#define DELAY_COL  1  // ms
#define MATRIX_WIDTH  24
#define MATRIX_HEIGHT  8
#define N_VRAM  2
#define VRAM_WIDTH  ((MATRIX_WIDTH)*(N_VRAM))
#define NUMBER_WIDTH  3

byte g_vram[VRAM_WIDTH];  // 2画面分

byte g_number[10][NUMBER_WIDTH] = {
  {
    B00011111,
    B00010001,
    B00011111
  },  // 0
  {
    B00000001,
    B00011111,
    B00010001
  },  // 1
  {
    B00011101,
    B00010101,
    B00010111
  },  // 2
  {
    B00011111,
    B00010101,
    B00010101
  },  // 3
  {
    B00011111,
    B00000100,
    B00011100
  },  // 4
  {
    B00010111,
    B00010101,
    B00011101
  },  // 5
  {
    B00010111,
    B00010101,
    B00011111
  },  // 6
  {
    B00011111,
    B00010000,
    B00010000
  },  // 7
  {
    B00011111,
    B00010101,
    B00011111
  },  // 8
  {
    B00011111,
    B00010101,
    B00011101
  }  // 9  
};

////////////////////////////////////////////////////////////////////////////////
void output_col(const unsigned long c_num, const byte r_data)
{
  //unsigned long value;
  //unsigned long col;
  //unsigned long row = r_data & 0x1F;  // 5bit
  int i;
  byte v[4];
  
  //col = 1<<c_num;
  //Serial.print( col, BIN );
  //Serial.print(" ");
  //col = ~col;
  //Serial.print( col, BIN );
  //Serial.print(" ");
  //col &= 0xFFFFFF; // 24bit
  
  if( c_num < 8 ) {
    v[0] = (1<<c_num);
    v[1] = 0;
    v[2] = 0;;
  } else if( c_num < 16 ) {
    v[0] = 0;
    v[1] = (1<<(c_num-8));
    v[2] = 0;
  } else {
    v[0] = 0;
    v[1] = 0;
    v[2] = (1<<(c_num-16));
  }
  v[3] = ~(r_data & 0x1F);
  
  //Serial.print( v[3] );
  //Serial.print( " " );
  //Serial.print( v[2] );
  //Serial.print( " " );
  //Serial.print( v[1] );
  //Serial.print( " " );
  //Serial.print( v[0] );
  //Serial.println( " " );
  //col = (~c[2]<<16) | (~c[1]<<8) | (~c[0]);
  //col = ~col;
  //value = (row<<24) | ~(c[2]<<16) | ~(c[1]<<8) | ~(c[0]);
  
  //Serial.print( value, HEX );
  //Serial.println();
  //Serial.println(c_num);
  //Serial.println();
  //Serial.print( col, HEX );
  //Serial.print(" ");
  //Serial.print( row, HEX );
  //Serial.println();
  for(i=0; i<4; i++) {
    //SPI.transfer( (byte)((value>>(i*8))&0xFF) );
    SPI.transfer( v[4 - i - 1] );
#ifdef __DEBUG__
    Serial.print( v[4 - i - 1], BIN );
    Serial.println("");
#endif
  }
  digitalWrite(STB, LOW);
  //delay(1);
  digitalWrite(STB, HIGH);
  //delay(1);
#ifdef __DEBUG__
  Serial.println("--------");  
#endif
}

// output VRAM to LED Matrix
void output_matrix(const int start_col)
{
  int i;
  
  for(i=0; i<MATRIX_WIDTH; i++) {
#ifdef __DEBUG__
    Serial.println(i);
    Serial.print(g_vram[i+start_col], BIN);
    Serial.println();
#endif
    output_col(i, g_vram[i+start_col]);
    delay(DELAY_COL);
  }
}

void clear_matrix()
{
  unsigned long value = 0x00FFFFFF;
  int i;
  
  for(i=0; i<4; i++) {
    SPI.transfer( (byte)((value>>(i*8))&0xFF) );
  }
}

void write_vram(const int offset_col, const byte* data, const int data_size)
{
  int i;
  
  for(i=0; i<data_size; i++) {
    //g_vram[i+offset_col] = data[i];
    g_vram[i+offset_col] = data[data_size-i-1];
  }
}

void clear_vram() {
  int i;
  
  for(i=0; i<VRAM_WIDTH; i++) {
    g_vram[i] = 0x0;
  }
}

void dbg_output_matrix()
{
  int i, j;
  
    for(i=0; i < MATRIX_WIDTH; i++) {
      Serial.print(i);
      Serial.print(":");
      Serial.print(g_vram[i], BIN);
      Serial.println();
    }
    
}

// output check pattern
void test_matrix()
{
  int i;
  int j;
  int k;
  
    // test patter 1 
    Serial.println("test pattern 1");  
    for(j=0; j < MATRIX_HEIGHT; j++) {
      for(i=0; i < MATRIX_WIDTH; i++) {
        g_vram[i] = 1<<j;
      }
      //g_vram[0] = 1<<j;
#ifdef __DEBUG__
      dbg_output_matrix();
#endif
      output_matrix(0);
    }
}

void test_number()
{
  int i;
  
  for(i=0; i<10; i++) {
    write_vram(i*(NUMBER_WIDTH+1), g_number[i], NUMBER_WIDTH);
  }
  
  for(i=0; i<1000; i++) {
    output_matrix((i/4)%VRAM_WIDTH);
  //  delay(1);
  }
}
////////////////////////////////////////////////////////////////////////////////
void setup()
{
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV2);  // 16MHz/16=1MHz
  SPI.setDataMode(SPI_MODE0);
  
  Serial.begin(9600);

  pinMode(STB, OUTPUT);
  digitalWrite(STB, HIGH);
}

void loop()
{
  test_matrix();
  delay(1000);
  test_number();
  delay(1000);
}
