#include "mbed.h"
#include "SpwfInterface.h"
#include "TCPSocket.h"
#include "XNucleoIKS01A2.h"


//------------------------------------
// Hyperterminal configuration
// 9600 bauds, 8-bit data, no parity
//------------------------------------

Serial pc(USBTX, USBRX);
DigitalOut myled(LED1);

/*************************************
//FRDM-K64: D9->UART1_TX, D7->UART1_RX
Pin connections:
    FRDM      IDW01M1
   ------    ---------
    +3v3 <--> +3v3
    GND  <--> GND
    D9   <--> D8
    D7   <--> D2

SpwfSAInterface spwf(D9, D7, false);
*************************************/
/*************************************
//LPCXpresso11U68: D9->UART1_TX, D7->UART1_RX
Pin connections:
    LPC      IDW01M1
   ------    ---------
    +3v3 <--> +3v3
    GND  <--> GND
    A1   <--> D8
    A2   <--> D2

SpwfSAInterface spwf(A1, A2, false);
*************************************/

//NUCLEO: D8->UART1_TX (PA_9), D2->UART1_RX (PA_10)
SpwfSAInterface spwf(D8, D2, false);


/* Instantiate the Environmental expansion board */
static XNucleoIKS01A2 *mems_expansion_board = XNucleoIKS01A2::instance(D14, D15, D4, D5);
/* Retrieve the composing elements of the expansion board */
static LSM303AGRMagSensor *magnetometer = mems_expansion_board->magnetometer;
static HTS221Sensor *hum_temp = mems_expansion_board->ht_sensor;
static LPS22HBSensor *press_temp = mems_expansion_board->pt_sensor;
static LSM6DSLSensor *acc_gyro = mems_expansion_board->acc_gyro;
static LSM303AGRAccSensor *accelerometer = mems_expansion_board->accelerometer;

/* Helper function for printing floats & doubles */
static char *print_double(char* str, double v, int decimalDigits=2)
{
  int i = 1;
  int intPart, fractPart;
  int len;
  char *ptr;

  /* prepare decimal digits multiplicator */
  for (;decimalDigits!=0; i*=10, decimalDigits--);

  /* calculate integer & fractinal parts */
  intPart = (int)v;
  fractPart = (int)((v-(double)(int)v)*i);

  /* fill in integer part */
  sprintf(str, "%i.", intPart);

  /* prepare fill in of fractional part */
  len = strlen(str);
  ptr = &str[len];

  /* fill in leading fractional zeros */
  for (i/=10;i>1; i/=10, ptr++) {
    if (fractPart >= i) {
      break;
    }
    *ptr = '0';
  }

  /* fill in (rest of) fractional part */
  sprintf(ptr, "%i", fractPart);

  return str;
}

int main() {
    int err;    
    char * ssid = "NETGEAR-3G";
    char * seckey = "pallinoo";  
    
    uint8_t id;
  float value1, value2, value3, value4;
  char buffer1[32], buffer2[32];
  int32_t axes[3];
  
  /* Enable all sensors */
  hum_temp->enable();
  press_temp->enable();
  magnetometer->enable();
  accelerometer->enable();
  acc_gyro->enable_x();
  acc_gyro->enable_g();
  
  printf("\r\n--- Starting new run ---\r\n");

  hum_temp->read_id(&id);
  printf("HTS221  humidity & temperature    = 0x%X\r\n", id);
  press_temp->read_id(&id);
  printf("LPS22HB  pressure & temperature   = 0x%X\r\n", id);
  magnetometer->read_id(&id);
  printf("LSM303AGR magnetometer            = 0x%X\r\n", id);
  accelerometer->read_id(&id);
  printf("LSM303AGR accelerometer           = 0x%X\r\n", id);
  acc_gyro->read_id(&id);
  printf("LSM6DSL accelerometer & gyroscope = 0x%X\r\n", id);
    
    
    
    pc.printf("\r\nX-NUCLEO-IDW01M1 mbed Application\r\n");     
    pc.printf("\r\nconnecting to AP\r\n");
            
    if(spwf.connect(ssid, seckey, NSAPI_SECURITY_WPA2)) {      
        pc.printf("\r\nnow connected\r\n");
    } else {
        pc.printf("\r\nerror connecting to AP.\r\n");
        return -1;
    }   

    const char *ip = spwf.get_ip_address();
    const char *mac = spwf.get_mac_address();
    
    pc.printf("\r\nIP Address is: %s\r\n", (ip) ? ip : "No IP");
    pc.printf("\r\nMAC Address is: %s\r\n", (mac) ? mac : "No MAC");    
    
    SocketAddress addr(&spwf, "st.com");   
    pc.printf("\r\nst.com resolved to: %s\r\n", addr.get_ip_address());    

    pc.printf("\r\nconnecting to http://4.ifcfg.me\r\n");
    
    TCPSocket socket(&spwf);
    err = socket.connect("192.168.0.4", 9090);
    if(err!=0) 
    {
      pc.printf("\r\nCould not connect to Socket, err = %d!!\r\n", err); 
      return -1;
    } else pc.printf("\r\nconnected to host server\r\n"); 
    
    char buffer[100];
    int count = 0;
    int period;
    pc.printf("\r\nReceiving Data\r\n"); 
    count = socket.recv(buffer, sizeof buffer);
    
    if(count > 0)
    {
        buffer [count]='\0';
        
        printf("%c\r\n", buffer[0]);  
        printf("%c\r\n", buffer[1]);
        printf("%c\r\n", buffer[2]);
        printf("%c\r\n", buffer[3]);
        
        period = buffer[5]-'0';
        printf("%d\r\n", period);
        
        while(1){
            printf("--------\r\n");
             // Send message to server
            
            hum_temp->get_temperature(&value1);
            hum_temp->get_humidity(&value2);
            //printf("HTS221: [temp] %7s C,   [hum] %s%%\r\n", print_double(buffer1, value1), print_double(buffer2, value2));
            
            press_temp->get_temperature(&value3);
            press_temp->get_pressure(&value4);
            //printf("LPS22HB: [temp] %7s C, [press] %s mbar\r\n", print_double(buffer1, value1), print_double(buffer2, value2));

            magnetometer->get_m_axes(axes);
            //printf("LSM303AGR [mag/mgauss]:  %6ld, %6ld, %6ld\r\n", axes[0], axes[1], axes[2]);
    
            accelerometer->get_x_axes(axes);
            //printf("LSM303AGR [acc/mg]:  %6ld, %6ld, %6ld\r\n", axes[0], axes[1], axes[2]);

            acc_gyro->get_x_axes(axes);
            //printf("LSM6DSL [acc/mg]:      %6ld, %6ld, %6ld\r\n", axes[0], axes[1], axes[2]);

            acc_gyro->get_g_axes(axes);
            //printf("LSM6DSL [gyro/mdps]:   %6ld, %6ld, %6ld\r\n", axes[0], axes[1], axes[2]);

              
            char hello[100];
            strcpy(hello, print_double(buffer1, value1));
            strcat(hello, "--");
            strcat(hello, print_double(buffer1, value2));
            printf("Sending  message to Server : '%s' \n",hello);
            socket.send(hello, 100);
            
            printf("--------\r\n");

            wait(period);
        }
        
        
    }