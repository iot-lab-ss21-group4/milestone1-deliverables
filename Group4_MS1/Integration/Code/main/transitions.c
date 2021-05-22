#include "common.h"
#include "transitions.h"
#include "publisher.h"

#define FLAG_COUNT 2 // number of barrier bits (flags)
#define INNER_BARRIER_FLAG (1 << 1)
#define OUTER_BARRIER_FLAG (1 << 0)
#define ESP_INTR_FLAG_DEFAULT 0
#define TRANSITION_TIMER_MS 10
#define TRANSITION_TIMER_TICKS MAX(TRANSITION_TIMER_MS / portTICK_PERIOD_MS, 1)
#define IS_RISING_EDGE_GRIO_INTR_VAL(is_rising) ((is_rising) ? GPIO_INTR_POSEDGE : GPIO_INTR_NEGEDGE)
#define WAS_RISING_EDGE_GRIO_INTR_VAL(was_rising) ((was_rising) ? GPIO_INTR_NEGEDGE : GPIO_INTR_POSEDGE)

#define STATE_IDENTIFIER_LENGTH (2 * FLAG_COUNT)
// Transitions
#define T_0000_0001 1
#define T_0000_0010 2
#define T_0001_0100 20
#define T_0001_0111 23
#define T_0010_1011 43
#define T_0010_1000 40
#define T_0100_0001 65
#define T_0100_0010 66
#define T_0111_1110 126
#define T_0111_1101 125
#define T_1011_1110 190
#define T_1011_1101 189
#define T_1000_0001 129
#define T_1000_0010 130
#define T_1110_1011 235
#define T_1110_1000 232
#define T_1101_0100 212
#define T_1101_0111 215

// Impossible states
#define S_IMP_0000 0 //only allowed as init state
#define S_IMP_0011 3
#define S_IMP_0101 5
#define S_IMP_0110 6
#define S_IMP_1001 9
#define S_IMP_1010 10
#define S_IMP_1111 15
#define S_IMP_1100 12

// assuming that FSM state is defined by 4 bits
typedef uint8_t fsm_transition;
STATIC_ASSERT(
    sizeof(fsm_transition) * 8 >= 2 * STATE_IDENTIFIER_LENGTH,
    sizeof_fsm_transition_is_big_enough);

static barrier_evt_q_item FSM_STATE = 0;
static volatile bool is_inner_rising = true;
static volatile bool is_outer_rising = true;
static TimerHandle_t inner_transition_timer = NULL;
static TimerHandle_t outer_transition_timer = NULL;

static void inner_transition_timer_callback(TimerHandle_t);
static void outer_transition_timer_callback(TimerHandle_t);
static void apply_state_change(barrier_evt_q_item);
static void transition_handling_task(void *);
static void initialize_null_handles();

static void IRAM_ATTR inner_barrier_pin_isr(void *_)
{
    static const barrier_evt_q_item state_change = INNER_BARRIER_FLAG;
    xQueueSendFromISR(barrier_evt_q, &state_change, NULL);
    gpio_isr_handler_remove(INNER_BARRIER_PIN);
    // if we fail to reset the transition timer, then we must register
    // the next opposite edge isr ourselves.
    is_inner_rising = LOGICAL_NOT(is_inner_rising);
    if (xTimerResetFromISR(inner_transition_timer, NULL) != pdPASS)
    {
        gpio_set_intr_type(INNER_BARRIER_PIN, IS_RISING_EDGE_GRIO_INTR_VAL(is_inner_rising));
        gpio_isr_handler_add(INNER_BARRIER_PIN, inner_barrier_pin_isr, NULL);
    }
}

static void IRAM_ATTR outer_barrier_pin_isr(void *_)
{
    static const barrier_evt_q_item state_change = OUTER_BARRIER_FLAG;
    xQueueSendFromISR(barrier_evt_q, &state_change, NULL);
    gpio_isr_handler_remove(OUTER_BARRIER_PIN);
    // if we fail to reset the transition timer, then we must register
    // the next opposite edge isr ourselves.
    is_outer_rising = LOGICAL_NOT(is_outer_rising);
    if (xTimerResetFromISR(outer_transition_timer, NULL) != pdPASS)
    {
        gpio_set_intr_type(OUTER_BARRIER_PIN, IS_RISING_EDGE_GRIO_INTR_VAL(is_outer_rising));
        gpio_isr_handler_add(OUTER_BARRIER_PIN, outer_barrier_pin_isr, NULL);
    }
}

static void inner_transition_timer_callback(TimerHandle_t timer)
{
    gpio_set_intr_type(INNER_BARRIER_PIN, IS_RISING_EDGE_GRIO_INTR_VAL(is_inner_rising));
    gpio_isr_handler_add(INNER_BARRIER_PIN, inner_barrier_pin_isr, NULL);
}

static void outer_transition_timer_callback(TimerHandle_t timer)
{
    gpio_set_intr_type(OUTER_BARRIER_PIN, IS_RISING_EDGE_GRIO_INTR_VAL(is_outer_rising));
    gpio_isr_handler_add(OUTER_BARRIER_PIN, outer_barrier_pin_isr, NULL);
}

static void apply_state_change(barrier_evt_q_item state_change)
{
    barrier_evt_q_item prev_new_bits = FSM_STATE & FIRST_N_BITMASK(FLAG_COUNT);
    // Replace old state bits with new state bits.
    FSM_STATE = (FSM_STATE << FLAG_COUNT) & FIRST_N_BITMASK(2 * FLAG_COUNT);
    // XOR with changed state bits (1 for changed 0 for unchanged).
    FSM_STATE ^= prev_new_bits ^ (state_change & FIRST_N_BITMASK(FLAG_COUNT));
}

void detect_impossible_state()
{
    switch (FSM_STATE)
    {
    case S_IMP_0000: //call method only after first signal
    case S_IMP_0011:
    case S_IMP_0101:
    case S_IMP_0110:
    case S_IMP_1001:
    case S_IMP_1010:
    case S_IMP_1111:
    case S_IMP_1100:
        ESP_LOGE(TAG, "Inside impossible state: " BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(FSM_STATE));
        break;
    default:
        break;
    }
}

static void transition_handling_task(void *_)
{
    barrier_evt_q_item state_change;
    while (1)
    {
        while (xQueueReceive(barrier_evt_q, &state_change, portMAX_DELAY) != pdTRUE)
            ;
        fsm_transition dangling_transition = FSM_STATE << STATE_IDENTIFIER_LENGTH;
        apply_state_change(state_change);
        detect_impossible_state();
        fsm_transition transition = dangling_transition | FSM_STATE;
        switch (transition)
        {
        case T_0111_1110:
            count = (count < MAX_ROOM_COUNT) ? count + 1 : count;
            xQueueSend(count_display_q, (const void *)&count, portMAX_DELAY);
            break;
        case T_1011_1101:
            count = (count > MIN_ROOM_COUNT) ? count - 1 : count;
            xQueueSend(count_display_q, (const void *)&count, portMAX_DELAY);
            break;
        default:
            break;
        }
        if (in_testing_scenario)
            print_sensor_and_fsm_state();
    }
}
static void initialize_null_handles()
{
    if (inner_transition_timer == NULL)
    {
        inner_transition_timer = xTimerCreate("inner_transition_timer", TRANSITION_TIMER_TICKS, pdFALSE, NULL, inner_transition_timer_callback);
    }
    if (outer_transition_timer == NULL)
    {
        outer_transition_timer = xTimerCreate("outer_transition_timer", TRANSITION_TIMER_TICKS, pdFALSE, NULL, outer_transition_timer_callback);
    }
}

void setup_transitions()
{
    initialize_null_handles();

    gpio_set_direction(INNER_BARRIER_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(OUTER_BARRIER_PIN, GPIO_MODE_INPUT);

    gpio_pulldown_en(INNER_BARRIER_PIN);
    gpio_pulldown_en(OUTER_BARRIER_PIN);

    gpio_set_intr_type(INNER_BARRIER_PIN, IS_RISING_EDGE_GRIO_INTR_VAL(is_inner_rising));
    gpio_set_intr_type(OUTER_BARRIER_PIN, IS_RISING_EDGE_GRIO_INTR_VAL(is_outer_rising));

    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(INNER_BARRIER_PIN, inner_barrier_pin_isr, NULL);
    gpio_isr_handler_add(OUTER_BARRIER_PIN, outer_barrier_pin_isr, NULL);

    xTaskCreate(transition_handling_task, "transition_handling_task", 4096, NULL, 9, NULL);
}

void loop_transitions()
{
    print_sensor_and_fsm_state();
}

void print_sensor_and_fsm_state()
{
    ESP_LOGI(TAG, "Sensor state: " BYTE_TO_BINARY_PATTERN ", FSM state: " BYTE_TO_BINARY_PATTERN,
             BYTE_TO_BINARY((gpio_get_level(INNER_BARRIER_PIN) * INNER_BARRIER_FLAG) |
                            (gpio_get_level(OUTER_BARRIER_PIN) * OUTER_BARRIER_FLAG)),
             BYTE_TO_BINARY(FSM_STATE));
}
