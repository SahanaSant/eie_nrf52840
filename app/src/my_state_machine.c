#include <zephyr/smf.h>

#include "LED.h"
#include "my_state_machine.h"
#include "BTN.h"

/* ---------- helpers ---------- */
static inline bool b0(void){ return BTN_check_clear_pressed(BTN0); }
static inline bool b1(void){ return BTN_check_clear_pressed(BTN1); }
static inline bool b2(void){ return BTN_check_clear_pressed(BTN2); }
static inline bool b3(void){ return BTN_check_clear_pressed(BTN3); }

/* ---------- ASCII CODE FUNCTIONS ---------- */
/* 
 * Adds a bit to the ASCII accumulator.
 * bit = 0 → BTN0
 * bit = 1 → BTN1
 * Gives LED feedback.
 */
 //Global Variables
static uint8_t ascii_code = 0;   // stores the 8-bit ASCII being built
static uint8_t bit_index = 0;    // how many bits have been entered (0–7)
#define MAX_STRING_LEN 32    // you can change this
static char ascii_string[MAX_STRING_LEN + 1];   // +1 for null terminator
static uint8_t ascii_string_len = 0;

 
static void ascii_add_bit(uint8_t bit)
{
    if (bit_index < 8) {

        // Shift left to make room
        ascii_code <<= 1;

        // Insert the bit
        ascii_code |= (bit & 0x01);

        bit_index++;

        /* ---- LED feedback ----- */
        if (bit == 0) {
            LED_set(LED0, LED_ON);
            LED_set(LED1, LED_OFF);
        } else {
            LED_set(LED1, LED_ON);
            LED_set(LED0, LED_OFF);
        }

    } else {
        // Already have 8 bits → blink both LEDs quickly
        LED_set(LED0, LED_ON);
        LED_set(LED1, LED_ON);
        // You can optionally ignore new bits or wrap, up to you
    }
}

static void ascii_clear(void)
{
    ascii_code = 0;
    bit_index  = 0;

    LED_set(LED0, LED_OFF);
    LED_set(LED1, LED_OFF);
}

static void ascii_save_code(void)
{
    // Only save if we actually have 8 bits
    if (bit_index == 8) {

        // Convert the 8-bit value into an ASCII character
        char c = (char)ascii_code;

        // Store character into buffer if space available
        if (ascii_string_len < MAX_STRING_LEN) {
            ascii_string[ascii_string_len++] = c;
            ascii_string[ascii_string_len]   = '\0';   // maintain null termination
        }

        // Clear out for next ASCII entry
        ascii_clear();
    }
    else {
        // Optional: blink LEDs to indicate "not complete"
        LED_set(LED0, LED_ON);
        LED_set(LED1, LED_ON);
        // You could also choose to ignore this case
    }
}

static void ascii_string_clear(void)
{
    ascii_string_len = 0;     // no characters stored
    ascii_string[0]  = '\0';  // empty string
}

/* ---- PWM breathing parameters ---- */
#define PULSE_STEP       2      // change per update (brightness smoothness)
#define PULSE_MAX        100    // full brightness (0–100%)
#define PULSE_MIN        0      // off brightness
#define PULSE_UPDATE_MS  10     // speed of pulsing (lower = faster)

/* ---- Breathing state ---- */
static uint8_t pulse_brightness = 0;
static bool pulse_rising = true;
static uint16_t pulse_count = 0;

/* ---- Call this frequently (every run cycle) ---- */
static void pulse_all_leds(void)
{
    // Delay between each brightness change
    if (++pulse_count < PULSE_UPDATE_MS) {
        return;
    }
    pulse_count = 0;

    // Increase or decrease brightness
    if (pulse_rising) {
        if (pulse_brightness < PULSE_MAX) {
            pulse_brightness += PULSE_STEP;
        } else {
            pulse_rising = false;   // reached max → start falling
        }
    } 
    else { // falling
        if (pulse_brightness > PULSE_MIN) {
            pulse_brightness -= PULSE_STEP;
        } else {
            pulse_rising = true;    // reached min → start rising
        }
    }

    // Apply brightness to all 4 LEDs
    LED_pwm(LED0, pulse_brightness);
    LED_pwm(LED1, pulse_brightness);
    LED_pwm(LED2, pulse_brightness);
    LED_pwm(LED3, pulse_brightness);
}

/* ---------- timing ---------- */
#define TOGGLE4_MS    125   // 4 cycles/sec -> toggle every 125 ms
#define TOGGLE16_MS    31   // ~16 cycles/sec -> toggle every ~31 ms
#define TOGGLE1_MS     500 // 1 cycle/sec

/* ---------- prototypes ---------- */

static void state0_entry(void *o);
static enum smf_state_result state0_run (void *o);
static void state0_exit(void *o);

static void state1_entry(void *o);
static enum smf_state_result state1_run (void *o);
static void state1_exit(void *o);

static void state2_entry(void *o);
static enum smf_state_result state2_run (void *o);
static void state2_exit(void *o);

static void state3_entry(void *o);
static enum smf_state_result state3_run (void *o);
static void state3_exit(void *o);

/* ---------- types ---------- */
enum led_state_machine_states{ 
    State_0,        // LED3 blink @ 1 Hz
    State_1,        // LED3 blink @ 4HZ
    State_2,        // LED3 blink @ 16HZ
    State_3         // LEDS Pulse
};

typedef struct {
    struct smf_ctx ctx;  // must be first
    uint16_t count;      // ms-ish counter used per state
    bool phase;          // general toggle phase
} led_state_object_t;


/* ---------- state table ---------- */
static const struct smf_state led_states[] = {
    [State_0]       = SMF_CREATE_STATE(state0_entry,        state0_run,        state0_exit,        NULL, NULL),
    [State_1]       = SMF_CREATE_STATE(state1_entry,        state1_run,        state1_exit,        NULL, NULL),
    [State_2]       = SMF_CREATE_STATE(state2_entry,        state2_run,        state2_exit,        NULL, NULL),
    [State_3]       = SMF_CREATE_STATE(state3_entry,        state3_run,        state3_exit,        NULL, NULL)
}; 

static led_state_object_t led_state_object; 

/* ---------- API ---------- */
void state_machine_init(){
    led_state_object.count = 0; 
    led_state_object.phase = false;
    smf_set_initial(SMF_CTX(&led_state_object), &led_states[State_0]);
}
int state_machine_run(){
    return smf_run_state(SMF_CTX(&led_state_object));
}


/* ================= State_0: ================= */
static void state0_entry(void* o)
{
    led_state_object_t *s = o;
    s->count = 0;
    s->phase = false;

    LED_set(LED0, LED_OFF);   // LED1
    LED_set(LED2, LED_OFF);   // LED3
    LED_set(LED1, LED_OFF);  // LED2
    LED_set(LED3, LED_OFF);  // LED4
    
     ascii_clear();
    
}
static enum smf_state_result state0_run(void *o){ // S0 behavior
     led_state_object_t *s = o;

    // BTN0 → add a 0 bit
    if (b0()) {
		    printk("Input as 0.\n");
        ascii_add_bit(0);
    }

    // BTN1 → add a 1 bit
    if (b1()) {
		    printk("Input as 1.\n");
        ascii_add_bit(1);
    }

    // BTN2 → clear current ASCII, stay in State_0
    if (b2()) {
        ascii_clear();
        printk("Cleared the ASCII code you made.\n");
        return SMF_EVENT_HANDLED;
    }

    // BTN3 → finished ASCII, go to State_1
    if (b3()) {
		    ascii_save_code();
		    printk("Saved char: %c   Full buffer: %s\n", ascii_string[ascii_string_len-1], ascii_string);
        smf_set_state(SMF_CTX(s), &led_states[State_1]);
        return SMF_EVENT_HANDLED;
    }
    
    if (b0()&&b1()){
	    smf_set_state(SMF_CTX(s), &led_states[State_3]);
        printk("Blinking standby mode.");
        return SMF_EVENT_HANDLED;
    }

    // Blink LED3 (or LED2 depending on mapping) at 1 Hz
    if (++s->count >= TOGGLE1_MS) {
        s->count = 0;
        s->phase = !s->phase;
        LED_set(LED2, s->phase ? LED_ON : LED_OFF);
    }

    return SMF_EVENT_HANDLED;
}

static void state0_exit(void *o)
{
    (void)o;
    /* no teardown required */
}

/* ================= State_1: ================= */
static void state1_entry(void* o)
{
    led_state_object_t *s = o;
    s->count = 0;
    s->phase = false;

    LED_set(LED0, LED_OFF);   // LED1
    LED_set(LED2, LED_OFF);   // LED3
    LED_set(LED1, LED_OFF);  // LED2
    LED_set(LED3, LED_OFF);  // LED4
        
}

static enum smf_state_result state1_run(void *o){ // S1 behavior
   led_state_object_t *s = o;
     
   if (b2()) {
        ascii_clear();
        ascii_string_clear();
        printk("You have chosen to clear your entire string.\n");
        smf_set_state(SMF_CTX(s), &led_states[State_0]);
        return SMF_EVENT_HANDLED;
    }
    
    if (b0()&&b1()){
	    smf_set_state(SMF_CTX(s), &led_states[State_3]);
        printk("Blinking standby mode.");
        return SMF_EVENT_HANDLED;
    }
        // BTN0 → add a 0 bit
    if (b0()) {
		    printk("Input as 0.\n");
        ascii_add_bit(0);
    }

    // BTN1 → add a 1 bit
    if (b1()) {
		    printk("Input as 1.\n");
        ascii_add_bit(1);
    }
    
    // BTN3 → finished ASCII, go to State_2
    if (b3()) {
		    ascii_save_code();
		    printk("Saved char: %c   Full buffer: %s\n", ascii_string[ascii_string_len-1], ascii_string);
        smf_set_state(SMF_CTX(s), &led_states[State_2]);
        return SMF_EVENT_HANDLED;
    }
    
     if (++s->count >= TOGGLE4_MS) {
      s->count = 0;
      s->phase = !s->phase;
      LED_set(LED2, s->phase ? LED_ON : LED_OFF);
    }
      return SMF_EVENT_HANDLED;
 }
 
 static void state1_exit(void *o)
{
    (void)o;
    /* no teardown required */
}

/* ================= State_2: ================= */
static void state2_entry(void* o)
{
    led_state_object_t *s = o;
    s->count = 0;
    s->phase = false;

    LED_set(LED0, LED_OFF);   // LED1
    LED_set(LED2, LED_OFF);   // LED3
    LED_set(LED1, LED_OFF);  // LED2
    LED_set(LED3, LED_OFF);  // LED4
        
}

static enum smf_state_result state2_run(void *o){ // S2 behavior

	led_state_object_t *s = o;

	if (b0()&&b1()){
	    smf_set_state(SMF_CTX(s), &led_states[State_3]);
        printk("Blinking standby mode.");
        return SMF_EVENT_HANDLED;
    }
   
    if (b2()) {
        ascii_clear();
        ascii_string_clear();
        printk("You have chosen to clear your entire string.\n");
        smf_set_state(SMF_CTX(s), &led_states[State_0]);
        return SMF_EVENT_HANDLED;
    }

      if (++s->count >= TOGGLE16_MS) {
        s->count = 0;
        s->phase = !s->phase;
        LED_set(LED2, s->phase ? LED_ON : LED_OFF);
    }

    return SMF_EVENT_HANDLED;
  
	
}

 static void state2_exit(void *o)
{
    (void)o;
    /* no teardown required */
}

/* ================= State_3: ================= */
static void state2_entry(void* o)
{
    led_state_object_t *s = o;
    s->count = 0;
    s->phase = false;

    LED_set(LED0, LED_OFF);   // LED1
    LED_set(LED2, LED_OFF);   // LED3
    LED_set(LED1, LED_OFF);  // LED2
    LED_set(LED3, LED_OFF);  // LED4
        
}

static enum smf_state_result state3_run(void *o){ // S3 behavior

	led_state_object_t *s = o;
    // If any button clicked → exit standby and return to previous state
    if (b0() || b1() || b2() || b3()) {
        smf_set_state(SMF_CTX(o), &led_states[previous_state]);
        return SMF_EVENT_HANDLED;
    }

    // Pulse all LEDs
    pulse_all_leds();

    return SMF_EVENT_HANDLED;
}

static void state3_exit(void *o)
{
    (void)o;
    /* no teardown required */
}
