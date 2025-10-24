/**
 * @file my_state_machine.c
 */
#include <zephyr/smf.h>

#include "LED.h"
#include "my_state_machine.h"
#include "BTN.h"

/* ---------- helpers ---------- */
static inline bool b1(void){ return BTN_check_clear_pressed(BTN0); }
static inline bool b2(void){ return BTN_check_clear_pressed(BTN1); }
static inline bool b3(void){ return BTN_check_clear_pressed(BTN2); }
static inline bool b4(void){ return BTN_check_clear_pressed(BTN3); }

//Function Prototypes

static void led_on_state_entry(void* o);
static enum smf_state_result led_on_state_run(void* o);
//create one to exit on 
static void led_on_state_exit(void *o);

static void led_off_state_entry(void* o);
static enum smf_state_result led_off_state_run(void* o); 
static void led_off_state_exit(void *o);

//State 1 Prototype
static void state1_entry(void *o);
static enum smf_state_result state1_run (void *o);
static void state1_exit(void *o);

//State 2 Prototype
static void state2_entry(void *o);
static enum smf_state_result state2_run (void *o);
static void state2_exit(void *o);

//State 3 Prototype
static void state3_entry(void *o);
static enum smf_state_result state3_run (void *o);
static void state3_exit(void *o);

//State 4 Prototype
static void state4_entry(void *o);
static enum smf_state_result state4_run (void *o);
static void state4_exit(void *o);

//Typedefs
enum led_state_machine_states{
    LED_ON_STATE,
    LED_OFF_STATE,
    State_1,
    State_2,
    State_3,
    State_4
};

typedef struct {
    //Context variable used by zephyr to track state machine state. Must be first
    struct smf_ctx ctx;

    uint16_t count; 
} led_state_object_t; 

int count =0; 

//Local Variables
static const struct smf_state led_states[] = {
    [LED_ON_STATE] = SMF_CREATE_STATE(led_on_state_entry, led_on_state_run, led_on_state_exit, NULL, NULL),
    [LED_OFF_STATE]= SMF_CREATE_STATE(led_off_state_entry, led_off_state_run, led_off_state_exit, NULL, NULL),
    [State_1] = SMF_CREATE_STATE(state1_entry, state1_run, state1_exit, NULL, NULL),
    [State_2] = SMF_CREATE_STATE(state2_entry, state2_run, state2_exit, NULL, NULL),
    [State_3] = SMF_CREATE_STATE(state3_entry, state3_run, state3_exit, NULL, NULL),
    [State_4] = SMF_CREATE_STATE(state4_entry, state4_run, state4_exit, NULL, NULL)
}; 

static led_state_object_t led_state_object;  

void state_machine_init(){
    led_state_object.count =0; 
    smf_set_initial(SMF_CTX(&led_state_object), &led_states[LED_OFF_STATE]);
}
int state_machine_run(){
    return smf_run_state(SMF_CTX(&led_state_object));
}

//Function definitions
static void led_on_state_entry(void* o){
    LED_set(LED0, LED_ON);
}
static enum smf_state_result led_on_state_run(void* o){
    return SMF_EVENT_HANDLED;
} 

static void led_off_state_entry(void *o) {
    LED_set(LED0, LED_OFF);
}

static enum smf_state_result led_off_state_run(void *o){ //I will treat led_off_state as State 0. 
    if (b1())
    {
        smf_set_state(SMF_CTX(&led_state_object), &led_states[State_1]);
        return SMF_EVENT_HANDLED;
    }
    
}

static void led_on_state_exit(void *o){}
static void led_off_state_exit(void *o){}

//Exercise 2 

//State 1
static void state1_entry(void* o)
{
    LED_set(LED0, LED_ON);
}
static enum smf_state_result state1_run(void *o)
{
    if (led_state_object.count > 125){
        led_state_object.count = 0;
        smf_set_state(SMF_CTX(&led_state_object), &led_states[LED_ON_STATE]);
        return SMF_EVENT_HANDLED;
    } 
    else 
    {
        led_state_object.count++;
    }
    
    if (b2()) {smf_set_state(SMF_CTX(&led_state_object), &led_states[State_2]); return SMF_EVENT_HANDLED;}
    if (b3()) {smf_set_state(SMF_CTX(&led_state_object), &led_states[State_4]); return SMF_EVENT_HANDLED;}
    if (b4()) {smf_set_state(SMF_CTX(&led_state_object), &led_states[LED_OFF_STATE]); return SMF_EVENT_HANDLED;}
    
}
static void state1_exit(void *o)
{
    LED_set(LED0, LED_OFF);
}

static void state2_entry(void* o)
{
    LED_set(LED0, LED_ON);
    LED_set(LED2, LED_ON);
    LED_set(LED1, LED_OFF);
    LED_set(LED3, LED_OFF);
}
static enum smf_state_result state2_run(void *o)
{
    if (led_state_object.count<1){
        smf_set_state(SMF_CTX(&led_state_object), &led_states[LED_ON_STATE]);
        led_state_object.count++;
    }
    led_state_object.count= 0;
    if (b4()) {smf_set_state(SMF_CTX(&led_state_object), &led_states[LED_OFF_STATE]); return SMF_EVENT_HANDLED;}
    smf_set_state(SMF_CTX(&led_state_object), &led_states[State_3]); return SMF_EVENT_HANDLED;
    return SMF_EVENT_HANDLED;
}
static void state2_exit(void *o)
{
    LED_set(LED0, LED_OFF);
    LED_set(LED2, LED_OFF);
    LED_set(LED1, LED_OFF);
    LED_set(LED3, LED_OFF);
}


static void state3_entry(void* o)
{
    LED_set(LED0, LED_OFF);
    LED_set(LED2, LED_OFF);
    LED_set(LED1, LED_ON);
    LED_set(LED3, LED_ON);
}

static enum smf_state_result state3_run(void *o)
{
    if (led_state_object.count<2){
       smf_set_state(SMF_CTX(&led_state_object), &led_states[LED_ON_STATE]);
       led_state_object.count++; 
    }
    led_state_object.count=0; 
    if (b4()) {smf_set_state(SMF_CTX(&led_state_object), &led_states[LED_OFF_STATE]); return SMF_EVENT_HANDLED;}
    smf_set_state(SMF_CTX(&led_state_object), &led_states[State_2]); return SMF_EVENT_HANDLED;
    return SMF_EVENT_HANDLED;
}
static void state3_exit(void *o)
{
    LED_set(LED0, LED_OFF);
    LED_set(LED2, LED_OFF);
    LED_set(LED1, LED_OFF);
    LED_set(LED3, LED_OFF);
}

static void state4_entry(void* o)
{
    LED_set(LED0, LED_ON);
    LED_set(LED2, LED_ON);
    LED_set(LED1, LED_ON);
    LED_set(LED3, LED_ON);
}
static enum smf_state_result state4_run(void *o)
{
    if (led_state_object.count > 31){
        led_state_object.count = 0;
        smf_set_state(SMF_CTX(&led_state_object), &led_states[LED_ON_STATE]);
    } else {
        led_state_object.count++;
    }
    
    if (b4()) {smf_set_state(SMF_CTX(&led_state_object), &led_states[LED_OFF_STATE]); return SMF_EVENT_HANDLED;}
    return SMF_EVENT_HANDLED;

}
static void state4_exit(void *o)
{
    LED_set(LED0, LED_OFF);
    LED_set(LED2, LED_OFF);
    LED_set(LED1, LED_OFF);
    LED_set(LED3, LED_OFF);
}