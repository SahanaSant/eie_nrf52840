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

/* ---------- timing ---------- */
#define TOGGLE4_MS    125   // 4 cycles/sec -> toggle every 125 ms
#define TOGGLE16_MS    31   // ~16 cycles/sec -> toggle every ~31 ms
#define ONE_SEC_MS   1000   // S2 dwell
#define TWO_SEC_MS   2000   // S3 dwell

/* ---------- prototypes ---------- */
static void led_on_state_entry(void* o);
static enum smf_state_result led_on_state_run(void* o);
static void led_on_state_exit(void *o);

static void led_off_state_entry(void* o);
static enum smf_state_result led_off_state_run(void* o); 
static void led_off_state_exit(void *o);

static void state1_entry(void *o);
static enum smf_state_result state1_run (void *o);
static void state1_exit(void *o);

static void state2_entry(void *o);
static enum smf_state_result state2_run (void *o);
static void state2_exit(void *o);

static void state3_entry(void *o);
static enum smf_state_result state3_run (void *o);
static void state3_exit(void *o);

static void state4_entry(void *o);
static enum smf_state_result state4_run (void *o);
static void state4_exit(void *o);

/* ---------- types ---------- */
enum led_state_machine_states{
    LED_ON_STATE,
    LED_OFF_STATE,  // S0 (all off / idle)
    State_1,        // LED1 blink @ 4 Hz
    State_2,        // 1&3 ON, 2&4 OFF (1s -> State_3)
    State_3,        // 1&3 OFF, 2&4 ON (2s -> State_2)
    State_4         // all blink @ 16 Hz
};

typedef struct {
    struct smf_ctx ctx;  // must be first
    uint16_t count;      // ms-ish counter used per state
    bool phase;          // general toggle phase
} led_state_object_t;

/* ---------- state table ---------- */
static const struct smf_state led_states[] = {
    [LED_ON_STATE]  = SMF_CREATE_STATE(led_on_state_entry,  led_on_state_run,  led_on_state_exit,  NULL, NULL),
    [LED_OFF_STATE] = SMF_CREATE_STATE(led_off_state_entry, led_off_state_run, led_off_state_exit, NULL, NULL),
    [State_1]       = SMF_CREATE_STATE(state1_entry,        state1_run,        state1_exit,        NULL, NULL),
    [State_2]       = SMF_CREATE_STATE(state2_entry,        state2_run,        state2_exit,        NULL, NULL),
    [State_3]       = SMF_CREATE_STATE(state3_entry,        state3_run,        state3_exit,        NULL, NULL),
    [State_4]       = SMF_CREATE_STATE(state4_entry,        state4_run,        state4_exit,        NULL, NULL)
}; 

static led_state_object_t led_state_object;  

/* ---------- API ---------- */
void state_machine_init(){
    led_state_object.count = 0; 
    led_state_object.phase = false;
    /* S0 initial = all off */
    smf_set_initial(SMF_CTX(&led_state_object), &led_states[LED_OFF_STATE]);
}
int state_machine_run(){
    return smf_run_state(SMF_CTX(&led_state_object));
}

/* ================= LED_ON_STATE (not S0; simple “LED0 on” state) ================= */
static void led_on_state_entry(void* o){
    /* keep simple for your exercise: LED0 on, reset counter */
    ((led_state_object_t*)o)->count = 0;
    LED_set(LED0, LED_ON);
}
static enum smf_state_result led_on_state_run(void* o){
    /* no transitions here unless you want some; keep handled */
    return SMF_EVENT_HANDLED;
} 
static void led_on_state_exit(void *o){ (void)o; }

/* ================= LED_OFF_STATE (S0: all LEDs OFF) ================= */
static void led_off_state_entry(void *o) {
    led_state_object_t *s = o;
    s->count = 0; s->phase = false;
    LED_set(LED0, LED_OFF);
    LED_set(LED1, LED_OFF);
    LED_set(LED2, LED_OFF);
    LED_set(LED3, LED_OFF);
}
static enum smf_state_result led_off_state_run(void *o){ // S0 behavior
    led_state_object_t *s = o;

    if (b1()) { smf_set_state(SMF_CTX(s), &led_states[State_1]); return SMF_EVENT_HANDLED; }
    if (b4()) { smf_set_state(SMF_CTX(s), &led_states[State_2]); return SMF_EVENT_HANDLED; }

    return SMF_EVENT_HANDLED;
}
static void led_off_state_exit(void *o){ (void)o; }

/* ================= State_1: LED1 blink @ 4 Hz ================= */
static void state1_entry(void* o)
{
    led_state_object_t *s = o;
    s->count = 0; s->phase = false;

    /* known phase; others off */
    LED_set(LED0, LED_OFF);
    LED_set(LED1, LED_OFF);
    LED_set(LED2, LED_OFF);
    LED_set(LED3, LED_OFF);
}
static enum smf_state_result state1_run(void *o)
{
    led_state_object_t *s = o;

    /* buttons per diagram */
    if (b2()) { smf_set_state(SMF_CTX(s), &led_states[State_2]);       return SMF_EVENT_HANDLED; }
    if (b3()) { smf_set_state(SMF_CTX(s), &led_states[State_4]);       return SMF_EVENT_HANDLED; }
    if (b4()) { smf_set_state(SMF_CTX(s), &led_states[LED_OFF_STATE]); return SMF_EVENT_HANDLED; }

    /* blink LED1 (LED0 index) @ 4 Hz: toggle every 125 ms, stay in this state */
    if (++s->count >= TOGGLE4_MS){
        s->count = 0;
        s->phase = !s->phase;
        LED_set(LED0, s->phase ? LED_ON : LED_OFF);
    }
    return SMF_EVENT_HANDLED;
}
static void state1_exit(void *o)
{
    (void)o;
    /* optional: force LED off on exit */
    // LED_set(LED0, LED_OFF);
}

/* ================= State_2: 1&3 ON, 2&4 OFF (after 1 s -> State_3) ================= */
static void state2_entry(void* o)
{
    led_state_object_t *s = o;
    s->count = 0;

    LED_set(LED0, LED_ON);   // LED1
    LED_set(LED2, LED_ON);   // LED3
    LED_set(LED1, LED_OFF);  // LED2
    LED_set(LED3, LED_OFF);  // LED4
}
static enum smf_state_result state2_run(void *o)
{
    led_state_object_t *s = o;

    if (b4()) { smf_set_state(SMF_CTX(s), &led_states[LED_OFF_STATE]); return SMF_EVENT_HANDLED; }

    if (++s->count >= ONE_SEC_MS){
        s->count = 0;
        smf_set_state(SMF_CTX(s), &led_states[State_3]);
        return SMF_EVENT_HANDLED;
    }
    return SMF_EVENT_HANDLED;
}
static void state2_exit(void *o)
{
    (void)o;
    /* no teardown required */
}

/* ================= State_3: 1&3 OFF, 2&4 ON (after 2 s -> State_2) ================= */
static void state3_entry(void* o)
{
    led_state_object_t *s = o;
    s->count = 0;

    LED_set(LED0, LED_OFF);
    LED_set(LED2, LED_OFF);
    LED_set(LED1, LED_ON);
    LED_set(LED3, LED_ON);
}
static enum smf_state_result state3_run(void *o)
{
    led_state_object_t *s = o;

    if (b4()) { smf_set_state(SMF_CTX(s), &led_states[LED_OFF_STATE]); return SMF_EVENT_HANDLED; }

    if (++s->count >= TWO_SEC_MS){
        s->count = 0;
        smf_set_state(SMF_CTX(s), &led_states[State_2]);
        return SMF_EVENT_HANDLED;
    }
    return SMF_EVENT_HANDLED;
}
static void state3_exit(void *o)
{
    (void)o;
}

/* ================= State_4: all LEDs blink @ ~16 Hz ================= */
static void state4_entry(void* o)
{
    led_state_object_t *s = o;
    s->count = 0; s->phase = false;

    /* start from known phase; all off initially is fine */
    LED_set(LED0, LED_OFF);
    LED_set(LED1, LED_OFF);
    LED_set(LED2, LED_OFF);
    LED_set(LED3, LED_OFF);
}
static enum smf_state_result state4_run(void *o)
{
    led_state_object_t *s = o;

    if (b4()) { smf_set_state(SMF_CTX(s), &led_states[LED_OFF_STATE]); return SMF_EVENT_HANDLED; }

    /* toggle all LEDs every ~31 ms and stay in this state */
    if (++s->count >= TOGGLE16_MS){
        s->count = 0;
        s->phase = !s->phase;
        LED_set(LED0, s->phase ? LED_ON : LED_OFF);
        LED_set(LED1, s->phase ? LED_ON : LED_OFF);
        LED_set(LED2, s->phase ? LED_ON : LED_OFF);
        LED_set(LED3, s->phase ? LED_ON : LED_OFF);
    }
    return SMF_EVENT_HANDLED;
}
static void state4_exit(void *o)
{
    (void)o;
    /* optional: force all off on exit */
    // LED_set(LED0, LED_OFF); LED_set(LED1, LED_OFF); LED_set(LED2, LED_OFF); LED_set(LED3, LED_OFF);
}
