#include <wiringPi.h>
#include <softPwm.h>
// #include <gpiod.h>
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
#define PWB_2               21
#define FREQ                50
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
    if ( gpio_set == -1) {
        perror( "Error: gpio setup" );
    }
    pinMode( pin1, OUTPUT );
    pinMode( pin2, OUTPUT );
    pinMode( pin_pwm, OUTPUT );
    softPwmCreate( pin_pwm, 0, 1.0/freq );
}

void forward( int *pin1, int *pin2, int *pin_pwm, int duty_cycle )
{
    digitalWrite( pin1, true );
    digitalWrite( pin2, false );
    softPwmWrite( pin_pwm, duty_cycle );
}

void stop( int *pin1, int *pin2, int *pin_pwm, int duty_cycle ) 
{
    digitalWrite( pin1, false );
    digitalWrite( pin2, false );
    softPwmWrite( pin_pwm, duty_cycle );
}

void *left_motor_thread( void * thread_id )
{
    // todo: setup GPIO here
    printf ( "thread: left motor control\n" );
    struct TB6612FNGDc motor1;
    struct TB6612FNGDc motor3;
    motor1 = { AI1, AI2, PWA, FREQ };
    motor3 = { AI1_2, AI2_2, PWA_2, FREQ };
    motor1.setup = setup;
    motor3.setup = setup;
    motor1.forward = forward;
    motor3.forward = forward;
    motor1.stop = stop;
    motor3.stop = stop;
    motor1.setup( &motor1.pin1, &motor1.pin2, &motor1.pin_pwm, &motor1.freq );
    motor3.setup( &motor3.pin1, &motor3.pin2, &motor3.pin_pwm, &motor3.freq );

    // todo: while loop body
    while (true) {
        int l = digitalRead( PHOTORESISTOR_L );
        int r = digitalRead( PHOTORESISTOR_R );
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
        elif ( l == 1 ) {
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
    motor2 = { BI1, AI2, PWB, FREQ };
    motor4 = { BI1_2, AI2_2, PWB_2, FREQ };
    motor2.setup = setup;
    motor4.setup = setup;
    motor2.forward = forward;
    motor4.forward = forward;
    motor2.stop = stop;
    motor4.stop = stop;
    motor2.setup( &motor2.pin1, &motor2.pin2, &motor2.pin_pwm, &motor2.freq );
    motor4.setup( &motor4.pin1, &motor4.pin2, &motor4.pin_pwm, &motor4.freq );

    // todo: while loop body
    while (true) {
        int l = digitalRead( PHOTORESISTOR_L );
        int r = digitalRead( PHOTORESISTOR_R );
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
        elif ( l == 1 ) {
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

// void *photo_thread( void * thread_id )
// {
//     // todo: setup GPIO here
//     printf ( "thread: photoresistor detection\n" );
//     pinMode( PHOTORESISTOR_L, INPUT );
//     pinMode( PHOTORESISTOR_R, INPUT );

//     // todo: while loop body
//     while( true ) {
//         int l = digitalRead( PHOTORESISTOR_L );
//         int r = digitalRead( PHOTORESISTOR_R );

//         // tracking logic pseudo code:
//         // if ( l == 0 ) {
//         //     if   ( r == 0 )   both left and right go straight;
//         //     elif ( r == 1 )   left_motor_turn, right_motor_stop;
//         // }
//         // elif ( l == 1 ) {
//         //     if   ( r == 0 )   right_motor_turn, left_motor_stop;
//         //     elif ( r == 1 )   left_motor_stop, right_motor_stop;
//         // }
//     }

//     // terminate the thread
//     pthread_exit( NULL );
// }

int main() 
{
    int GPIO_setup = wiringPiSetupGpio();
    if ( GPIO_setup == -1 ) {
        perror( "Error: wiringPi setup" );
        return -1;
    }

    pinMode( PHOTORESISTOR_L, INPUT );
    pinMode( PHOTORESISTOR_R, INPUT );

    // open and join threads
    pthread_t left_tid;
    pthread_t right_tid;
    // pthread_t photoresistor_tid;

    int err_l = pthread_create( &left_tid, NULL, left_motor_thread, NULL );
    if ( err_l ) {
        perror( "Error: create left motor control thread" );
        return -1;
    }

    int err_r = pthread_create( &right_tid, NULL, right_motor_thread, NULL );
    if ( err_r ) {
        perror( "Error: create right motor control thread" );
        return -1;
    }

    // int err_photo = pthread_create( &photoresistor_tid, NULL, photo_thread, NULL );
    // if ( err_photo ) {
    //     perror( "Error: create photoresistor thread" );
    //     return -1;
    // }

    // pthread_join( photoresistor_tid, NULL );
    pthread_join( left_tid, NULL );
    pthread_join( right_tid, NULL );

    return 0;
}