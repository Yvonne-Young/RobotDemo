#include <wiringPi.h>
#include "MySoftPwm.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define PHOTORESISTOR_PIN_L 17
#define PHOTORESISTOR_PIN_R 16
#define AI1                 18
#define AI2                 19
#define BI1                 22
#define BI2                 23
#define PWA                 12
#define PWB                 13
#define AI1_2               5
#define AI2_2               6
#define BI1_2               20
#define BI2_2               21
#define PWA_2               4
#define PWB_2               24
#define FREQ                50
#define STANDBY             25
#define STANDBY_2           27
#define STAIGHT_DC          25
#define TURNING_DC          100


//             TB6612FNGDc control board control logic
// AIN1    AIN2    BIN1    BIN2    PWMA    PWMB    AO1/AO2
// -----------------------------------------------------------
// 1       0       1       0       1       1       positive
// 0       1       0       1       1       1       reverse
// 1       1       1       1       1       1       brake
// 0       0       0       0       1       1       free stop
// x       x       x       x       0       0       brake

struct TB6612FNGDc 
{
    // pin1: GPIO connected to AI1 or BI1;
    // pin2: GPIO connected to AI2 or BI2;
    // pin_pwm: GPIO connected to PWA or PWB;
    int pin1;
    int pin2;
    int pin_pwm;
    int freq;

    // methods: 
    void (*setup)( int *pin1, int *pin2, int *pin_pwm, int *freq );
    void (*forward)( int *pin1, int *pin2, int *pin_pwm, int duty_cycle );
    void (*stop)( int *pin1, int *pin2, int *pin_pwm, int duty_cycle );
};

void setup( int *pin1, int *pin2, int *pin_pwm, int *freq )
{
    int gpio_setup = wiringPiSetupGpio();  // use wiringPiSetupGpio to setup as BCM mode
    if ( gpio_setup == -1) {
        perror( "Error: gpio setup" );
    }
    pinMode( * pin1, OUTPUT );
    pinMode( * pin2, OUTPUT );
    pinMode( * pin_pwm, OUTPUT );
    softPwmCreate( * pin_pwm, 0, * freq);
}

void forward( int *pin1, int *pin2, int *pin_pwm, int duty_cycle )
{
    digitalWrite( * pin1, 1 );
    digitalWrite( * pin2, 0 );
    softPwmWrite( * pin_pwm, duty_cycle );
}

void stop( int *pin1, int *pin2, int *pin_pwm, int duty_cycle ) 
{
    digitalWrite( * pin1, 0 );
    digitalWrite( * pin2, 0 );
    softPwmWrite( * pin_pwm, duty_cycle );
}

void *left_motor_thread( void * thread_id )
{
    // todo: setup GPIO here
    printf ( "thread: left motor control\n" );
    struct TB6612FNGDc motor1;
    struct TB6612FNGDc motor3;
    int freq = FREQ;
    motor1.pin1 = AI1;
    motor1.pin2 = AI2;
    motor1.pin_pwm = PWA;
    motor1.freq = freq;
    motor3.pin1 = AI1_2;
    motor3.pin2 = AI2_2;
    motor3.pin_pwm = PWA_2;
    motor3.freq = freq;
    motor1.setup = setup;
    motor3.setup = setup;
    motor1.forward = forward;
    motor3.forward = forward;
    motor1.stop = stop;
    motor3.stop = stop;
    motor1.setup( &motor1.pin1, &motor1.pin2, &motor1.pin_pwm, &motor1.freq );
    motor3.setup( &motor3.pin1, &motor3.pin2, &motor3.pin_pwm, &motor3.freq );

    // todo: while loop body
    while (1) {
        int l = digitalRead( PHOTORESISTOR_PIN_L );
        int r = digitalRead( PHOTORESISTOR_PIN_R );
        // printf( "left photoresistor read: %d\n", l );
	    // printf( "right photoresistor read: %d\n", r );
        if ( l == 0 ) {
            if ( r == 0 ) {
                motor1.forward( &motor1.pin1, &motor1.pin2, &motor1.pin_pwm, STAIGHT_DC );
                motor3.forward( &motor3.pin1, &motor3.pin2, &motor3.pin_pwm, STAIGHT_DC );
            }
            else {
                motor1.forward( &motor1.pin1, &motor1.pin2, &motor1.pin_pwm, TURNING_DC );
                motor3.forward( &motor3.pin1, &motor3.pin2, &motor3.pin_pwm, TURNING_DC );
            }
        }
	    else if ( l == 1 ) {
            motor1.stop( &motor1.pin1, &motor1.pin2, &motor1.pin_pwm, 0 );
            motor3.stop( &motor3.pin1, &motor3.pin2, &motor3.pin_pwm, 0 );
        }
    }
    
    pthread_exit( NULL );
}

void *right_motor_thread( void * thread_id )
{
    // todo: setup GPIO here
    printf ( "thread: right motor control\n" );
    struct TB6612FNGDc motor2;
    struct TB6612FNGDc motor4;
    motor2.pin1 = BI1;
    motor2.pin2 = BI2;
    motor2.pin_pwm = PWB;
    motor2.freq = FREQ;
    motor4.pin1 = BI1_2;
    motor4.pin2 = BI2_2;
    motor4.pin_pwm = PWB_2;
    motor4.freq = FREQ;
    motor2.setup = setup;
    motor4.setup = setup;
    motor2.forward = forward;
    motor4.forward = forward;
    motor2.stop = stop;
    motor4.stop = stop;
    motor2.setup( &motor2.pin1, &motor2.pin2, &motor2.pin_pwm, &motor2.freq );
    motor4.setup( &motor4.pin1, &motor4.pin2, &motor4.pin_pwm, &motor4.freq );

    // todo: while loop body
    while (1) {
        int l = digitalRead( PHOTORESISTOR_PIN_L );
        int r = digitalRead( PHOTORESISTOR_PIN_R );
	  // printf( "left photoresistor read: %d\n", l );
	  // printf( "right photoresistor read: %d\n", r );
        if ( l == 0 ) {
            if ( r == 0 ) {
                motor2.forward( &motor2.pin1, &motor2.pin2, &motor2.pin_pwm, STAIGHT_DC );
                motor4.forward( &motor4.pin1, &motor4.pin2, &motor4.pin_pwm, STAIGHT_DC );
            }
            else {
                motor2.stop( &motor2.pin1, &motor2.pin2, &motor2.pin_pwm, 0 );
                motor4.stop( &motor4.pin1, &motor4.pin2, &motor4.pin_pwm, 0 );
            }
        }
        else if ( l == 1 ) {
            if ( r == 0 ) {
                motor2.forward( &motor2.pin1, &motor2.pin2, &motor2.pin_pwm, TURNING_DC );
                motor4.forward( &motor4.pin1, &motor4.pin2, &motor4.pin_pwm, TURNING_DC );
            }
            else {
                motor2.stop( &motor2.pin1, &motor2.pin2, &motor2.pin_pwm, 0 );
                motor4.stop( &motor4.pin1, &motor4.pin2, &motor4.pin_pwm, 0 );
            }
        }
    }

    pthread_exit( NULL );
}

int main() 
{
    int GPIO_setup = wiringPiSetupGpio();
    if ( GPIO_setup == -1 ) {
        perror( "Error: wiringPi setup" );
        return -1;
    }

    pinMode( PHOTORESISTOR_PIN_L, INPUT );
    pinMode( PHOTORESISTOR_PIN_R, INPUT );
    pinMode( STANDBY, OUTPUT );
    pinMode( STANDBY_2, OUTPUT );
    digitalWrite( STANDBY, 1 );
    digitalWrite( STANDBY_2, 1 );

    struct TB6612FNGDc motor1;
    struct TB6612FNGDc motor3;
    int freq = FREQ;
    motor1.pin1 = AI1;
    motor1.pin2 = AI2;
    motor1.pin_pwm = PWA;
    motor1.freq = freq;
    motor3.pin1 = AI1_2;
    motor3.pin2 = AI2_2;
    motor3.pin_pwm = PWA_2;
    motor3.freq = freq;
    motor1.setup = setup;
    motor3.setup = setup;
    motor1.forward = forward;
    motor3.forward = forward;
    motor1.stop = stop;
    motor3.stop = stop;
    motor1.setup( &motor1.pin1, &motor1.pin2, &motor1.pin_pwm, &motor1.freq );
    motor3.setup( &motor3.pin1, &motor3.pin2, &motor3.pin_pwm, &motor3.freq );

    struct TB6612FNGDc motor2;
    struct TB6612FNGDc motor4;
    motor2.pin1 = BI1;
    motor2.pin2 = BI2;
    motor2.pin_pwm = PWB;
    motor2.freq = FREQ;
    motor4.pin1 = BI1_2;
    motor4.pin2 = BI2_2;
    motor4.pin_pwm = PWB_2;
    motor4.freq = FREQ;
    motor2.setup = setup;
    motor4.setup = setup;
    motor2.forward = forward;
    motor4.forward = forward;
    motor2.stop = stop;
    motor4.stop = stop;
    motor2.setup( &motor2.pin1, &motor2.pin2, &motor2.pin_pwm, &motor2.freq );
    motor4.setup( &motor4.pin1, &motor4.pin2, &motor4.pin_pwm, &motor4.freq );

    while( 1 )  {
        int l = digitalRead( PHOTORESISTOR_PIN_L );
        int r = digitalRead( PHOTORESISTOR_PIN_R );
	  // printf( "left photoresistor read: %d\n", l );
	  // printf( "right photoresistor read: %d\n", r );
        if ( l == 0 ) {
            if ( r == 0 ) {
                motor2.forward( &motor2.pin1, &motor2.pin2, &motor2.pin_pwm, STAIGHT_DC );
                motor4.forward( &motor4.pin1, &motor4.pin2, &motor4.pin_pwm, STAIGHT_DC );
                motor1.forward( &motor1.pin1, &motor1.pin2, &motor1.pin_pwm, STAIGHT_DC );
                motor3.forward( &motor3.pin1, &motor3.pin2, &motor3.pin_pwm, STAIGHT_DC );
            }
            else {
                motor2.stop( &motor2.pin1, &motor2.pin2, &motor2.pin_pwm, 0 );
                motor4.stop( &motor4.pin1, &motor4.pin2, &motor4.pin_pwm, 0 );
                motor1.forward( &motor1.pin1, &motor1.pin2, &motor1.pin_pwm, TURNING_DC );
                motor3.forward( &motor3.pin1, &motor3.pin2, &motor3.pin_pwm, TURNING_DC );
            }
        }
        else if ( l == 1 ) {
            motor1.stop( &motor1.pin1, &motor1.pin2, &motor1.pin_pwm, 0 );
            motor3.stop( &motor3.pin1, &motor3.pin2, &motor3.pin_pwm, 0 );
            if ( r == 0 ) {
                motor2.forward( &motor2.pin1, &motor2.pin2, &motor2.pin_pwm, TURNING_DC );
                motor4.forward( &motor4.pin1, &motor4.pin2, &motor4.pin_pwm, TURNING_DC );
            }
            else {
                motor2.stop( &motor2.pin1, &motor2.pin2, &motor2.pin_pwm, 0 );
                motor4.stop( &motor4.pin1, &motor4.pin2, &motor4.pin_pwm, 0 );
            }
        }
    }

    // open and join threads
    /*
    pthread_t left_tid;
    pthread_t right_tid;

    int err_r = pthread_create( &right_tid, NULL, right_motor_thread, NULL );
    if ( err_r ) {
        perror( "Error: create right motor control thread" );
        return -1;
    }

    int err_l = pthread_create( &left_tid, NULL, left_motor_thread, NULL );
    if ( err_l ) {
        perror( "Error: create left motor control thread" );
        return -1;
    }

    pthread_join( right_tid, NULL );
    pthread_join( left_tid, NULL );
    */
    return 0;
}
