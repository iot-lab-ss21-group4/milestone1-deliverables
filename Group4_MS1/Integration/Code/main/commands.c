#include "common.h"
#include "commands.h"
#include "publisher.h"

static uint8_t expected_room_count = 0;

#define TEST_ENTER_ROOM                                            \
	do                                                             \
	{                                                              \
		expected_room_count = count;                               \
		enterRoom();                                               \
		if (expected_room_count < MAX_ROOM_COUNT)                  \
			expected_room_count++;                                 \
		assert_room_count(expected_room_count, "Test Enter Room"); \
	} while (0);
#define TEST_LEAVE_ROOM                                            \
	do                                                             \
	{                                                              \
		expected_room_count = count;                               \
		leaveRoom();                                               \
		if (expected_room_count > MIN_ROOM_COUNT)                  \
			expected_room_count--;                                 \
		assert_room_count(expected_room_count, "Test Leave Room"); \
	} while (0);
#define TEST_PEAK_OUT_ROOM                                               \
	do                                                                   \
	{                                                                    \
		expected_room_count = count;                                     \
		peakOutofRoom();                                                 \
		assert_room_count(expected_room_count, "Test Peak Out Of Room"); \
	} while (0);
#define TEST_PEAK_IN_ROOM                                              \
	do                                                                 \
	{                                                                  \
		expected_room_count = count;                                   \
		peakIntoRoom();                                                \
		assert_room_count(expected_room_count, "Test Peak Into Room"); \
	} while (0);
#define TEST_HALFWAY_LEAVE_ROOM                                            \
	do                                                                     \
	{                                                                      \
		expected_room_count = count;                                       \
		halfwayLeave();                                                    \
		assert_room_count(expected_room_count, "Test Halfway Leave Room"); \
	} while (0);
#define TEST_HALFWAY_ENTER_ROOM                                            \
	do                                                                     \
	{                                                                      \
		expected_room_count = count;                                       \
		halfwayEnter();                                                    \
		assert_room_count(expected_room_count, "Test Halfway Enter Room"); \
	} while (0);
#define TEST_MANIPULATION_LEAVE_ROOM                                            \
	do                                                                          \
	{                                                                           \
		expected_room_count = count;                                            \
		manipulationLeave();                                                    \
		assert_room_count(expected_room_count, "Test Manipulation Leave Room"); \
	} while (0);
#define TEST_MANIPULATION_ENTER_ROOM                                            \
	do                                                                          \
	{                                                                           \
		expected_room_count = count;                                            \
		manipulationEnter();                                                    \
		assert_room_count(expected_room_count, "Test Manipulation Enter Room"); \
	} while (0);
#define TEST_BREAKS_OUTER_AND_INNER_BUT_RETURNS_G4                                       \
	do                                                                                   \
	{                                                                                    \
		expected_room_count = count;                                                     \
		breaksOuterAndInnerButReturnsG4();                                               \
		assert_room_count(expected_room_count, "Test BreaksOuterAndInnerButReturns G4"); \
	} while (0);
#define TEST_BREAKS_INNER_AND_OUTER_BUT_RETURNS_G4                                       \
	do                                                                                   \
	{                                                                                    \
		expected_room_count = count;                                                     \
		breaksInnerAndOuterButReturnsG4();                                               \
		assert_room_count(expected_room_count, "Test BreaksInnerAndOuterButReturns G4"); \
	} while (0);
#define TEST_PERSON_TURNED_G9                                            \
	do                                                                   \
	{                                                                    \
		expected_room_count = count;                                     \
		personTurnedG9();                                                \
		assert_room_count(expected_room_count, "Test Person Turned G9"); \
	} while (0);
#define TEST_UNSURE_ENTER_ROOM                                       \
	do                                                               \
	{                                                                \
		expected_room_count = count;                                 \
		unsureEnter();                                               \
		if (expected_room_count < MAX_ROOM_COUNT)                    \
			expected_room_count++;                                   \
		assert_room_count(expected_room_count, "Test Unsure Enter"); \
	} while (0);
#define TEST_PEEK_INTO_AND_LEAVE_G11                                            \
	do                                                                          \
	{                                                                           \
		expected_room_count = count;                                            \
		peekIntoandLeaveG11();                                                  \
		if (expected_room_count > MIN_ROOM_COUNT)                               \
			expected_room_count--;                                              \
		assert_room_count(expected_room_count, "Test Peek Into And Leave G11"); \
	} while (0);
#define TEST_SUCCESSIVE_ENTER_ROOM                                       \
	do                                                                   \
	{                                                                    \
		expected_room_count = count;                                     \
		successiveEnter();                                               \
		if (expected_room_count < MAX_ROOM_COUNT)                        \
			expected_room_count++;                                       \
		if (expected_room_count < MAX_ROOM_COUNT)                        \
			expected_room_count++;                                       \
		assert_room_count(expected_room_count, "Test Successive Enter"); \
	} while (0);

static void breaksOuterAndInnerButReturnsG4();
static void breaksInnerAndOuterButReturnsG4();
static void peakIntoRoom();
static void peakOutofRoom();
static void halfwayLeave();
static void halfwayEnter();
static void manipulationEnter();
static void manipulationLeave();
static void obstructionInside();
static void personTurnedG9();
static void unsureEnter();
static void peekIntoandLeaveG11();
static void successiveEnter();
static void assert_room_count(uint8_t expected_count, const char *test_description);

void test_trigger_pins()
{
	in_testing_scenario = true;
	ESP_LOGI(TAG, "Start testing: Scenario Basic");
	ESP_LOGI(TAG, "Inner Trigger PIN: %d", TRIGGER_PIN_IN);
	ESP_LOGI(TAG, "Outer Trigger PIN: %d", TRIGGER_PIN_OUT);
	ESP_LOGI(TAG, "Inner Barrier PIN: %d", INNER_BARRIER_PIN);
	ESP_LOGI(TAG, "Outer Barrier PIN: %d", OUTER_BARRIER_PIN);
	uint8_t old_count = count;

	TEST_ENTER_ROOM
	TEST_PEAK_OUT_ROOM
	TEST_BREAKS_INNER_AND_OUTER_BUT_RETURNS_G4
	TEST_LEAVE_ROOM
	TEST_PEAK_IN_ROOM
	TEST_BREAKS_OUTER_AND_INNER_BUT_RETURNS_G4
	TEST_HALFWAY_LEAVE_ROOM
	TEST_HALFWAY_ENTER_ROOM
	TEST_MANIPULATION_LEAVE_ROOM

	// finish with normal behavior
	TEST_ENTER_ROOM
	TEST_LEAVE_ROOM

	count = old_count;
	in_testing_scenario = false;
	ESP_LOGI(TAG, "End testing");
}

void test_milestone_one()
{
	in_testing_scenario = true;
	expected_room_count = count;
	uint8_t old_count = count;

	TEST_ENTER_ROOM
	TEST_LEAVE_ROOM
	TEST_HALFWAY_ENTER_ROOM
	TEST_BREAKS_OUTER_AND_INNER_BUT_RETURNS_G4
	TEST_PERSON_TURNED_G9
	TEST_UNSURE_ENTER_ROOM
	TEST_MANIPULATION_ENTER_ROOM
	TEST_PEEK_INTO_AND_LEAVE_G11
	TEST_SUCCESSIVE_ENTER_ROOM

	// finish with normal behavior
	TEST_ENTER_ROOM
	TEST_LEAVE_ROOM

	count = old_count;
	in_testing_scenario = false;
	ESP_LOGI(TAG, "End testing");
}

//////COMMANDS//////

void pingOnBoardLED()
{
	gpio_set_direction(ONBOARD_LED_PIN, GPIO_MODE_OUTPUT);
	gpio_set_level(ONBOARD_LED_PIN, 1);
	vTaskDelay(1000 / portTICK_PERIOD_MS);
	gpio_set_level(ONBOARD_LED_PIN, 0);
	vTaskDelay(1000 / portTICK_PERIOD_MS);
}

void enterRoom()
{
	ESP_LOGI(TAG, "Command: Enter");
	gpio_set_level(TRIGGER_PIN_OUT, 1);
	vTaskDelay(100 / portTICK_RATE_MS);
	gpio_set_level(TRIGGER_PIN_IN, 1);
	vTaskDelay(100 / portTICK_RATE_MS);
	gpio_set_level(TRIGGER_PIN_OUT, 0);
	vTaskDelay(100 / portTICK_RATE_MS);
	gpio_set_level(TRIGGER_PIN_IN, 0);
	vTaskDelay(500 / portTICK_RATE_MS);

	if (!in_testing_scenario)
		publish_count();
}

void leaveRoom()
{
	ESP_LOGI(TAG, "Command: Leave");
	gpio_set_level(TRIGGER_PIN_IN, 1);
	vTaskDelay(100 / portTICK_RATE_MS);
	gpio_set_level(TRIGGER_PIN_OUT, 1);
	vTaskDelay(100 / portTICK_RATE_MS);
	gpio_set_level(TRIGGER_PIN_IN, 0);
	vTaskDelay(100 / portTICK_RATE_MS);
	gpio_set_level(TRIGGER_PIN_OUT, 0);
	vTaskDelay(500 / portTICK_RATE_MS);

	if (!in_testing_scenario)
		publish_count();
}

static void breaksOuterAndInnerButReturnsG4()
{
	// original: almost enter
	ESP_LOGI(TAG, "Command: breakOuterAndInnerButReturnsG4");
	gpio_set_level(TRIGGER_PIN_OUT, 1);
	vTaskDelay(200 / portTICK_RATE_MS);
	gpio_set_level(TRIGGER_PIN_IN, 1);
	vTaskDelay(200 / portTICK_RATE_MS);
	gpio_set_level(TRIGGER_PIN_OUT, 0);
	vTaskDelay(200 / portTICK_RATE_MS);

	gpio_set_level(TRIGGER_PIN_OUT, 1);
	vTaskDelay(200 / portTICK_RATE_MS);
	gpio_set_level(TRIGGER_PIN_IN, 0);
	vTaskDelay(200 / portTICK_RATE_MS);
	gpio_set_level(TRIGGER_PIN_OUT, 0);
	vTaskDelay(200 / portTICK_RATE_MS);

	if (!in_testing_scenario)
		publish_count();
}

static void breaksInnerAndOuterButReturnsG4()
{
	// original: almost leave
	ESP_LOGI(TAG, "Command: breakInnerAndOuterButReturnsG4");
	gpio_set_level(TRIGGER_PIN_IN, 1);
	vTaskDelay(200 / portTICK_RATE_MS);
	gpio_set_level(TRIGGER_PIN_OUT, 1);
	vTaskDelay(200 / portTICK_RATE_MS);
	gpio_set_level(TRIGGER_PIN_IN, 0);
	vTaskDelay(200 / portTICK_RATE_MS);

	gpio_set_level(TRIGGER_PIN_IN, 1);
	vTaskDelay(200 / portTICK_RATE_MS);
	gpio_set_level(TRIGGER_PIN_OUT, 0);
	vTaskDelay(200 / portTICK_RATE_MS);
	gpio_set_level(TRIGGER_PIN_IN, 0);
	vTaskDelay(200 / portTICK_RATE_MS);

	if (!in_testing_scenario)
		publish_count();
}

static void peakIntoRoom()
{
	/*reach out one's head towards room, then lean back*/
	ESP_LOGI(TAG, "Command: Peak In");
	gpio_set_level(TRIGGER_PIN_OUT, 1);
	vTaskDelay(100 / portTICK_RATE_MS);
	gpio_set_level(TRIGGER_PIN_OUT, 0);
	vTaskDelay(500 / portTICK_RATE_MS);

	if (!in_testing_scenario)
		publish_count();
}

static void peakOutofRoom()
{
	ESP_LOGI(TAG, "Command: Peak Out");
	gpio_set_level(TRIGGER_PIN_IN, 1);
	vTaskDelay(100 / portTICK_RATE_MS);
	gpio_set_level(TRIGGER_PIN_IN, 0);
	vTaskDelay(500 / portTICK_RATE_MS);

	if (!in_testing_scenario)
		publish_count();
}

static void halfwayLeave()
{
	/*someone go to the middle of the doorway, and then turns around*/
	ESP_LOGI(TAG, "Command: Half Leave");
	gpio_set_level(TRIGGER_PIN_IN, 1);
	vTaskDelay(100 / portTICK_RATE_MS);
	gpio_set_level(TRIGGER_PIN_OUT, 1);
	vTaskDelay(50 / portTICK_RATE_MS);
	gpio_set_level(TRIGGER_PIN_OUT, 0);
	vTaskDelay(100 / portTICK_RATE_MS);
	gpio_set_level(TRIGGER_PIN_IN, 0);
	vTaskDelay(500 / portTICK_RATE_MS);

	if (!in_testing_scenario)
		publish_count();
}

static void halfwayEnter()
{
	/*someone go to the middle of the doorway, and then turns around*/
	ESP_LOGI(TAG, "Command: Half Enter");
	gpio_set_level(TRIGGER_PIN_OUT, 1);
	vTaskDelay(100 / portTICK_RATE_MS);
	gpio_set_level(TRIGGER_PIN_IN, 1);
	vTaskDelay(50 / portTICK_RATE_MS);
	gpio_set_level(TRIGGER_PIN_IN, 0);
	vTaskDelay(100 / portTICK_RATE_MS);
	gpio_set_level(TRIGGER_PIN_OUT, 0);
	vTaskDelay(500 / portTICK_RATE_MS);

	if (!in_testing_scenario)
		publish_count();
}

/**
 * Corner case from Group9: Almost Enter (slim person).
 * a person enters the room (breaking the first and then the second light barrier),
 * turns around (while the second light barrier is still broken),
 * and leaves the rooms (breaking the first light barrier again quickly after).
 * expected outcome: no change
 */
void personTurnedG9()
{
	ESP_LOGI(TAG, "Command: Person entered the room and turned around");
	// person entering
	gpio_set_level(TRIGGER_PIN_OUT, 1);
	vTaskDelay(100 / portTICK_PERIOD_MS);
	gpio_set_level(TRIGGER_PIN_OUT, 0);
	vTaskDelay(100 / portTICK_PERIOD_MS);
	// person turning around
	gpio_set_level(TRIGGER_PIN_IN, 1);
	vTaskDelay(300 / portTICK_PERIOD_MS); // turning takes time
	gpio_set_level(TRIGGER_PIN_IN, 0);
	vTaskDelay(100 / portTICK_PERIOD_MS);
	// person left the room again
	gpio_set_level(TRIGGER_PIN_OUT, 1);
	vTaskDelay(100 / portTICK_PERIOD_MS);
	gpio_set_level(TRIGGER_PIN_OUT, 0);
	vTaskDelay(1000 / portTICK_PERIOD_MS);

	if (!in_testing_scenario)
		publish_count();
}

/**
 * someone almost enters the room, but takes one step back (PinIn goes low before PinOut)
 * before finally entering.
 * expected outcome: +1
 */
void unsureEnter()
{
	ESP_LOGI(TAG, "Command: Unsure Enter");
	gpio_set_level(TRIGGER_PIN_OUT, 1);
	vTaskDelay(100 / portTICK_PERIOD_MS);
	gpio_set_level(TRIGGER_PIN_IN, 1);
	vTaskDelay(100 / portTICK_PERIOD_MS);
	gpio_set_level(TRIGGER_PIN_IN, 0);
	vTaskDelay(100 / portTICK_PERIOD_MS);
	gpio_set_level(TRIGGER_PIN_OUT, 0);
	vTaskDelay(100 / portTICK_PERIOD_MS);
	gpio_set_level(TRIGGER_PIN_OUT, 1);
	vTaskDelay(100 / portTICK_PERIOD_MS);
	gpio_set_level(TRIGGER_PIN_IN, 1);
	vTaskDelay(100 / portTICK_PERIOD_MS);
	gpio_set_level(TRIGGER_PIN_OUT, 0);
	vTaskDelay(100 / portTICK_PERIOD_MS);
	gpio_set_level(TRIGGER_PIN_IN, 0);
	vTaskDelay(100 / portTICK_PERIOD_MS);

	if (!in_testing_scenario)
		publish_count();
}

static void manipulationLeave()
{
	/*Someone is trying to manipulate the count by waving their arm through the barrier towards the outside
 Sequence is not possible if a person enters*/
	ESP_LOGI(TAG, "Command: Manipulation Leave");
	gpio_set_level(TRIGGER_PIN_IN, 1);
	vTaskDelay(15 / portTICK_RATE_MS);
	gpio_set_level(TRIGGER_PIN_IN, 0);
	vTaskDelay(15 / portTICK_RATE_MS);
	gpio_set_level(TRIGGER_PIN_OUT, 1);
	vTaskDelay(15 / portTICK_RATE_MS);
	gpio_set_level(TRIGGER_PIN_OUT, 0);
	vTaskDelay(500 / portTICK_RATE_MS);

	if (!in_testing_scenario)
		publish_count();
}

/**
* Someone is trying to manipulate the count by waving their arm through the barrier towards the inside
* Sequence is not possible if a person enters
* expected outcome: no change
*/
static void manipulationEnter()
{
	ESP_LOGI(TAG, "Command: Manipulation Enter ");
	gpio_set_level(TRIGGER_PIN_OUT, 1);
	vTaskDelay(15 / portTICK_PERIOD_MS);
	gpio_set_level(TRIGGER_PIN_OUT, 0);
	vTaskDelay(15 / portTICK_PERIOD_MS);
	gpio_set_level(TRIGGER_PIN_IN, 1);
	vTaskDelay(15 / portTICK_PERIOD_MS);
	gpio_set_level(TRIGGER_PIN_IN, 0);
	vTaskDelay(500 / portTICK_PERIOD_MS);

	if (!in_testing_scenario)
		publish_count();
}

static void obstructionInside()
{
	/*Someone is standing in the inside barrier, making counting impossible*/
	//TODO test would need separate task (not part of milestone)
	ESP_LOGI(TAG, "Command: Obstruction Inside");
	gpio_set_level(TRIGGER_PIN_IN, 1);
	vTaskDelay(6000 / portTICK_RATE_MS);
	gpio_set_level(TRIGGER_PIN_OUT, 1);
	vTaskDelay(3000 / portTICK_RATE_MS);
	/*resolve obstruction*/
	gpio_set_level(TRIGGER_PIN_IN, 0);
	gpio_set_level(TRIGGER_PIN_OUT, 0);
	vTaskDelay(500 / portTICK_RATE_MS);

	if (!in_testing_scenario)
		publish_count();
}

/**
 * Corner case from Group11:
 * Alice peeks into the room, shortly afterwards Bob leaves the room
 * expected count result: -1
 */
void peekIntoandLeaveG11()
{
	ESP_LOGI(TAG, "Command: Peek into and leave");
	gpio_set_level(TRIGGER_PIN_OUT, 1);
	vTaskDelay(3000 / portTICK_PERIOD_MS);
	gpio_set_level(TRIGGER_PIN_OUT, 0);
	vTaskDelay(100 / portTICK_PERIOD_MS);

	gpio_set_level(TRIGGER_PIN_IN, 1);
	vTaskDelay(100 / portTICK_PERIOD_MS);
	gpio_set_level(TRIGGER_PIN_OUT, 1);
	vTaskDelay(100 / portTICK_PERIOD_MS);
	gpio_set_level(TRIGGER_PIN_IN, 0);
	vTaskDelay(100 / portTICK_PERIOD_MS);
	gpio_set_level(TRIGGER_PIN_OUT, 0);
	vTaskDelay(500 / portTICK_PERIOD_MS);

	if (!in_testing_scenario)
		publish_count();
}

/**
 * successive entering
 * Alice enters the room while Bob also enters
 * expected outcome: +2
 */
void successiveEnter()
{
	ESP_LOGI(TAG, "Command: Successive Enter");
	// first person entering
	gpio_set_level(TRIGGER_PIN_OUT, 1);
	vTaskDelay(50 / portTICK_PERIOD_MS);
	gpio_set_level(TRIGGER_PIN_IN, 1);
	vTaskDelay(50 / portTICK_PERIOD_MS);
	// first person almost inside
	gpio_set_level(TRIGGER_PIN_OUT, 0);
	vTaskDelay(50 / portTICK_PERIOD_MS);
	// second person entering
	gpio_set_level(TRIGGER_PIN_OUT, 1);
	vTaskDelay(50 / portTICK_PERIOD_MS);
	// first person inside
	gpio_set_level(TRIGGER_PIN_IN, 0);
	vTaskDelay(50 / portTICK_PERIOD_MS);
	// second person entering
	gpio_set_level(TRIGGER_PIN_IN, 1);
	vTaskDelay(50 / portTICK_PERIOD_MS);
	gpio_set_level(TRIGGER_PIN_OUT, 0);
	vTaskDelay(50 / portTICK_PERIOD_MS);
	gpio_set_level(TRIGGER_PIN_IN, 0);
	vTaskDelay(500 / portTICK_PERIOD_MS);

	if (!in_testing_scenario)
		publish_count();
}

///////////////////////////////////////

static void assert_room_count(uint8_t expected_count, const char *test_description)
{
	if (count == expected_count)
	{
		ESP_LOGI(TAG, "%s: Ok", test_description);
	}
	else
	{
		ESP_LOGE(TAG, "%s: Fail", test_description);
	}
}
