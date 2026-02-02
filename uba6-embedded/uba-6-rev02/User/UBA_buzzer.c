/*
 * buzzer.c
 *
 *  Created on: Aug 19, 2024
 *      Author: ORA
 */
#include <UBA_buzzer.h>
#include "tim.h"
#include "uart_log.h"
#include "UBA_util.h"

#define UBA_COMP "Buzzer"

#define WHOLE_NOTE_DURATION_MS 1000
#define QUARTER_NOTE_DURATION_MS (WHOLE_NOTE_DURATION_MS / 4)
#define EIGHTH_NOTE_DURATION_MS (WHOLE_NOTE_DURATION_MS / 8)

#define BUZZER_TIMER htim5
#define BUZZER_TIMER_CHANNEL TIM_CHANNEL_2

void UBA_buzzer_update_state(UBA_buzzer *buzzer);
void UBA_buzzer_init_enter(UBA_buzzer*);
void UBA_buzzer_init(UBA_buzzer*);
void UBA_buzzer_init_exit(UBA_buzzer*);
void UBA_buzzer_off_enter(UBA_buzzer*);
void UBA_buzzer_off(UBA_buzzer*);
void UBA_buzzer_off_exit(UBA_buzzer*);
void UBA_buzzer_play_note_enter(UBA_buzzer*);
void UBA_buzzer_play_note(UBA_buzzer*);
void UBA_buzzer_play_note_exit(UBA_buzzer*);

UBA_buzzer buzzer_g = { 0 };

typedef void (*step_cb_t)(UBA_buzzer *buzzer);
/***
 * UBA Buzzer State Machine Assigner Rule
 */
struct UBABSMA_rule {
	step_cb_t enter;
	step_cb_t run;
	step_cb_t exit;
};
/*UBA State Buzzer Machine Assigner */
#define UBABSMA(step, cbe, cbr, cbx)[step] = {.enter = (step_cb_t)cbe, .run = (step_cb_t)cbr, .exit = (step_cb_t)cbx}
//@formatter:off

static const struct UBABSMA_rule rule_g[UBA_BUZZER_STATE_MAX] ={
		//				State
		UBABSMA(UBA_BUZZER_STATE_INIT,		UBA_buzzer_init_enter,		UBA_buzzer_init,		UBA_buzzer_init_exit),
		UBABSMA(UBA_BUZZER_STATE_OFF,		UBA_buzzer_off_enter,		UBA_buzzer_off,			UBA_buzzer_off_exit),
		UBABSMA(UBA_BUZZER_STATE_PLAY_NOTE,	UBA_buzzer_play_note_enter,	UBA_buzzer_play_note,	UBA_buzzer_play_note_exit),
		UBABSMA(UBA_BUZZER_STATE_MUTE,		UBA_buzzer_update_state,	NULL,					NULL),
};

Note power_up[]={{NOTE_E5, 125}, {NOTE_E5, 125}, {REST, 125}, {NOTE_E5, 125}, {REST, 125}, {NOTE_C5, 125}};

Note boot_note[]={{NOTE_G3, 125},{NOTE_D4, 250},{NOTE_G4, 500}};


Note done_note[]={{NOTE_A5, 125},{NOTE_C6, 125},{NOTE_D6, 250},{NOTE_C6, 125},{NOTE_D6, 500}};

Note error_note[]={{NOTE_C7, 500},{NOTE_B7, 500},{NOTE_C7, 500},{NOTE_B7, 500},{NOTE_C7, 500},{NOTE_B7, 500},{NOTE_C7, 500},{NOTE_B7, 500},{NOTE_C7, 500},{NOTE_B7, 500},{NOTE_C7, 500},{NOTE_B7, 500}};

Note ch_1_up[]={{NOTE_E5, 125}};
Note ch_2_up[]={{NOTE_F5, 125}};
Note ch_1_down[]={{NOTE_FS5, 125}};
Note ch_2_down[]={{NOTE_G5, 125}};
Note ch_1_set[]={{NOTE_GS5, 125}};
Note ch_2_set[]={{NOTE_A5, 125}};

//@formatter:on

// Define the Nokia tune melody
Note nokiaTune[] = {
		{ NOTE_E5, 1000 / 8 }, // E5
		{ NOTE_D5, 1000 / 8 }, // C5
		{ NOTE_FS4, 1000 / 4 }, // E5
		{ NOTE_GS4, 1000 / 4 }, // G5
		{ NOTE_CS5, 1000 / 8 }, // E5
		{ NOTE_B4, 1000 / 8 }, // C5
		{ NOTE_D4, 1000 / 4 }, // E5
		{ NOTE_E4, 1000 / 4 }, // G5
		{ NOTE_B4, 1000 / 8 }, // C5
		{ NOTE_A4, 1000 / 8 }, // D5
		{ NOTE_CS4, 1000 / 4 }, // B4
		{ NOTE_E4, 1000 / 4 }, // D5
		{ NOTE_A5, 1000 / 2 }, // D5
		};

Note tetris[] = {
		{ NOTE_E5, QUARTER_NOTE_DURATION_MS }, { NOTE_B4, EIGHTH_NOTE_DURATION_MS }, { NOTE_C5, EIGHTH_NOTE_DURATION_MS },
		{ NOTE_D5, QUARTER_NOTE_DURATION_MS }, { NOTE_C5, EIGHTH_NOTE_DURATION_MS }, { NOTE_B4, EIGHTH_NOTE_DURATION_MS },
		{ NOTE_A4, QUARTER_NOTE_DURATION_MS }, { NOTE_A4, EIGHTH_NOTE_DURATION_MS }, { NOTE_C5, EIGHTH_NOTE_DURATION_MS },
		{ NOTE_E5, QUARTER_NOTE_DURATION_MS }, { NOTE_D5, EIGHTH_NOTE_DURATION_MS }, { NOTE_C5, EIGHTH_NOTE_DURATION_MS },
		{ NOTE_B4, QUARTER_NOTE_DURATION_MS }, { NOTE_C5, EIGHTH_NOTE_DURATION_MS }, { NOTE_D5, QUARTER_NOTE_DURATION_MS },
		{ NOTE_E5, QUARTER_NOTE_DURATION_MS }, { NOTE_C5, QUARTER_NOTE_DURATION_MS }, { NOTE_A4, QUARTER_NOTE_DURATION_MS },
		{ NOTE_A4, EIGHTH_NOTE_DURATION_MS }, { NOTE_A4, QUARTER_NOTE_DURATION_MS }, { NOTE_B4, EIGHTH_NOTE_DURATION_MS },
		{ NOTE_C5, EIGHTH_NOTE_DURATION_MS },

		{ NOTE_D5, QUARTER_NOTE_DURATION_MS }, { NOTE_F5, EIGHTH_NOTE_DURATION_MS }, { NOTE_A5, QUARTER_NOTE_DURATION_MS },
		{ NOTE_G5, EIGHTH_NOTE_DURATION_MS }, { NOTE_F5, EIGHTH_NOTE_DURATION_MS }, { NOTE_E5, QUARTER_NOTE_DURATION_MS },
		{ NOTE_C5, EIGHTH_NOTE_DURATION_MS }, { NOTE_E5, QUARTER_NOTE_DURATION_MS }, { NOTE_D5, EIGHTH_NOTE_DURATION_MS },
		{ NOTE_C5, EIGHTH_NOTE_DURATION_MS }, { NOTE_B4, QUARTER_NOTE_DURATION_MS }, { NOTE_B4, EIGHTH_NOTE_DURATION_MS },
		{ NOTE_C5, EIGHTH_NOTE_DURATION_MS }, { NOTE_D5, QUARTER_NOTE_DURATION_MS }, { NOTE_E5, QUARTER_NOTE_DURATION_MS },
		{ NOTE_C5, QUARTER_NOTE_DURATION_MS }, { NOTE_A4, QUARTER_NOTE_DURATION_MS }, { NOTE_A4, QUARTER_NOTE_DURATION_MS },
		{ REST, QUARTER_NOTE_DURATION_MS },

		{ NOTE_E5, QUARTER_NOTE_DURATION_MS }, { NOTE_B4, EIGHTH_NOTE_DURATION_MS }, { NOTE_C5, EIGHTH_NOTE_DURATION_MS },
		{ NOTE_D5, QUARTER_NOTE_DURATION_MS }, { NOTE_C5, EIGHTH_NOTE_DURATION_MS }, { NOTE_B4, EIGHTH_NOTE_DURATION_MS },
		{ NOTE_A4, QUARTER_NOTE_DURATION_MS }, { NOTE_A4, EIGHTH_NOTE_DURATION_MS }, { NOTE_C5, EIGHTH_NOTE_DURATION_MS },
		{ NOTE_E5, QUARTER_NOTE_DURATION_MS }, { NOTE_D5, EIGHTH_NOTE_DURATION_MS }, { NOTE_C5, EIGHTH_NOTE_DURATION_MS },
		{ NOTE_B4, QUARTER_NOTE_DURATION_MS }, { NOTE_C5, EIGHTH_NOTE_DURATION_MS }, { NOTE_D5, QUARTER_NOTE_DURATION_MS },
		{ NOTE_E5, QUARTER_NOTE_DURATION_MS }, { NOTE_C5, QUARTER_NOTE_DURATION_MS }, { NOTE_A4, QUARTER_NOTE_DURATION_MS },
		{ NOTE_A4, EIGHTH_NOTE_DURATION_MS }, { NOTE_A4, QUARTER_NOTE_DURATION_MS }, { NOTE_B4, EIGHTH_NOTE_DURATION_MS },
		{ NOTE_C5, EIGHTH_NOTE_DURATION_MS },

		{ NOTE_D5, QUARTER_NOTE_DURATION_MS }, { NOTE_F5, EIGHTH_NOTE_DURATION_MS }, { NOTE_A5, QUARTER_NOTE_DURATION_MS },
		{ NOTE_G5, EIGHTH_NOTE_DURATION_MS }, { NOTE_F5, EIGHTH_NOTE_DURATION_MS }, { NOTE_E5, QUARTER_NOTE_DURATION_MS },
		{ NOTE_C5, EIGHTH_NOTE_DURATION_MS }, { NOTE_E5, QUARTER_NOTE_DURATION_MS }, { NOTE_D5, EIGHTH_NOTE_DURATION_MS },
		{ NOTE_C5, EIGHTH_NOTE_DURATION_MS }, { NOTE_B4, QUARTER_NOTE_DURATION_MS }, { NOTE_B4, EIGHTH_NOTE_DURATION_MS },
		{ NOTE_C5, EIGHTH_NOTE_DURATION_MS }, { NOTE_D5, QUARTER_NOTE_DURATION_MS }, { NOTE_E5, QUARTER_NOTE_DURATION_MS },
		{ NOTE_C5, QUARTER_NOTE_DURATION_MS }, { NOTE_A4, QUARTER_NOTE_DURATION_MS }, { NOTE_A4, QUARTER_NOTE_DURATION_MS },
		{ REST, QUARTER_NOTE_DURATION_MS },

		{ NOTE_E5, WHOLE_NOTE_DURATION_MS / 2 }, { NOTE_C5, WHOLE_NOTE_DURATION_MS / 2 },
		{ NOTE_D5, WHOLE_NOTE_DURATION_MS / 2 }, { NOTE_B4, WHOLE_NOTE_DURATION_MS / 2 },
		{ NOTE_C5, WHOLE_NOTE_DURATION_MS / 2 }, { NOTE_A4, WHOLE_NOTE_DURATION_MS / 2 },
		{ NOTE_GS4, WHOLE_NOTE_DURATION_MS / 2 }, { NOTE_B4, QUARTER_NOTE_DURATION_MS }, { REST, EIGHTH_NOTE_DURATION_MS },
		{ NOTE_E5, WHOLE_NOTE_DURATION_MS / 2 }, { NOTE_C5, WHOLE_NOTE_DURATION_MS / 2 },
		{ NOTE_D5, WHOLE_NOTE_DURATION_MS / 2 }, { NOTE_B4, WHOLE_NOTE_DURATION_MS / 2 },
		{ NOTE_C5, QUARTER_NOTE_DURATION_MS }, { NOTE_E5, QUARTER_NOTE_DURATION_MS }, { NOTE_A5, WHOLE_NOTE_DURATION_MS / 2 },
		{ NOTE_GS5, WHOLE_NOTE_DURATION_MS / 2 }
};

Note doom[] = {
		{ NOTE_E2, EIGHTH_NOTE_DURATION_MS }, { NOTE_E2, EIGHTH_NOTE_DURATION_MS }, { NOTE_E3, EIGHTH_NOTE_DURATION_MS }, { NOTE_E2,
		EIGHTH_NOTE_DURATION_MS }, { NOTE_E2, EIGHTH_NOTE_DURATION_MS }, { NOTE_D3, EIGHTH_NOTE_DURATION_MS }, { NOTE_E2,
		EIGHTH_NOTE_DURATION_MS }, { NOTE_E2, EIGHTH_NOTE_DURATION_MS },
		{ NOTE_C3, EIGHTH_NOTE_DURATION_MS }, { NOTE_E2, EIGHTH_NOTE_DURATION_MS }, { NOTE_E2, EIGHTH_NOTE_DURATION_MS }, { NOTE_AS2,
		EIGHTH_NOTE_DURATION_MS }, { NOTE_E2, EIGHTH_NOTE_DURATION_MS }, { NOTE_E2, EIGHTH_NOTE_DURATION_MS }, { NOTE_B2,
		EIGHTH_NOTE_DURATION_MS }, { NOTE_C3, EIGHTH_NOTE_DURATION_MS },
		{ NOTE_E2, EIGHTH_NOTE_DURATION_MS }, { NOTE_E2, EIGHTH_NOTE_DURATION_MS }, { NOTE_E3, EIGHTH_NOTE_DURATION_MS }, { NOTE_E2,
		EIGHTH_NOTE_DURATION_MS }, { NOTE_E2, EIGHTH_NOTE_DURATION_MS }, { NOTE_D3, EIGHTH_NOTE_DURATION_MS }, { NOTE_E2,
		EIGHTH_NOTE_DURATION_MS }, { NOTE_E2, EIGHTH_NOTE_DURATION_MS },
		{ NOTE_C3, EIGHTH_NOTE_DURATION_MS }, { NOTE_E2, EIGHTH_NOTE_DURATION_MS }, { NOTE_E2, EIGHTH_NOTE_DURATION_MS }, { NOTE_AS2,
		WHOLE_NOTE_DURATION_MS / 2 },

		{ NOTE_E2, EIGHTH_NOTE_DURATION_MS }, { NOTE_E2, EIGHTH_NOTE_DURATION_MS }, { NOTE_E3, EIGHTH_NOTE_DURATION_MS }, { NOTE_E2,
		EIGHTH_NOTE_DURATION_MS }, { NOTE_E2, EIGHTH_NOTE_DURATION_MS }, { NOTE_D3, EIGHTH_NOTE_DURATION_MS }, { NOTE_E2,
		EIGHTH_NOTE_DURATION_MS }, { NOTE_E2, EIGHTH_NOTE_DURATION_MS },
		{ NOTE_C3, EIGHTH_NOTE_DURATION_MS }, { NOTE_E2, EIGHTH_NOTE_DURATION_MS }, { NOTE_E2, EIGHTH_NOTE_DURATION_MS }, { NOTE_AS2,
		EIGHTH_NOTE_DURATION_MS }, { NOTE_E2, EIGHTH_NOTE_DURATION_MS }, { NOTE_E2, EIGHTH_NOTE_DURATION_MS }, { NOTE_B2,
		EIGHTH_NOTE_DURATION_MS }, { NOTE_C3, EIGHTH_NOTE_DURATION_MS },
		{ NOTE_E2, EIGHTH_NOTE_DURATION_MS }, { NOTE_E2, EIGHTH_NOTE_DURATION_MS }, { NOTE_E3, EIGHTH_NOTE_DURATION_MS }, { NOTE_E2,
		EIGHTH_NOTE_DURATION_MS }, { NOTE_E2, EIGHTH_NOTE_DURATION_MS }, { NOTE_D3, EIGHTH_NOTE_DURATION_MS }, { NOTE_E2,
		EIGHTH_NOTE_DURATION_MS }, { NOTE_E2, EIGHTH_NOTE_DURATION_MS },
		{ NOTE_C3, EIGHTH_NOTE_DURATION_MS }, { NOTE_E2, EIGHTH_NOTE_DURATION_MS }, { NOTE_E2, EIGHTH_NOTE_DURATION_MS }, { NOTE_AS2,
		WHOLE_NOTE_DURATION_MS / 2 },

// Add the rest of the melody and duration pairs in a similar pattern

		};
Note dungeon_sounds[] = {
		// Deep atmospheric opening (1200ms)
		{ 110, 600 },   // A2 - deep bass rumble
		{ 147, 600 },   // D3 - ominous low tone

		// Mysterious chime sequence (1200ms)
		{ 587, 200 },   // D5 - ethereal chime
		{ 440, 200 },   // A4 - echo
		{ 392, 400 },   // G4 - lingering tone
		{ 294, 400 },   // D4 - fading resonance

		// Dark ambient progression (1300ms)
		{ 196, 400 },   // G3 - dark tone
		{ 220, 300 },   // A3 - rising tension
		{ 247, 300 },   // B3 - building
		{ 262, 300 },   // C4 - peak

		// Magical effect sequence (800ms)
		{ 494, 200 },   // B4 - sparkle
		{ 392, 200 },   // G4 - shimmer
		{ 330, 200 },   // E4 - mystical
		{ 294, 200 },   // D4 - fade

		// Final atmospheric resolve (500ms)
		{ 196, 250 },   // G3 - deep
		{ 147, 250 }    // D3 - final echo
};
const uint32_t tempo = 1000; // Base duration (1000 ms for a whole note)
const uint32_t TEMPO = 1000;
Note pop[] =   // Prince of Persia theme approximation
		{ { NOTE_E4, TEMPO / 4 }, { NOTE_E4, TEMPO / 4 }, { NOTE_E4, TEMPO / 4 }, { NOTE_A4, TEMPO / 4 },
				{ NOTE_E4, TEMPO / 4 }, { NOTE_G4, TEMPO / 4 }, { NOTE_A4, TEMPO / 4 }, { NOTE_B4, TEMPO / 4 },
				{ NOTE_C5, TEMPO / 4 }, { NOTE_B4, TEMPO / 4 }, { NOTE_A4, TEMPO / 4 }, { NOTE_G4, TEMPO / 4 },
				{ NOTE_F4, TEMPO / 4 }, { NOTE_E4, TEMPO / 4 }, { NOTE_D4, TEMPO / 4 }, { NOTE_E4, TEMPO / 2 },
				{ NOTE_E4, TEMPO / 4 }, { NOTE_E4, TEMPO / 4 }, { NOTE_E4, TEMPO / 4 }, { NOTE_A4, TEMPO / 4 },
				{ NOTE_E4, TEMPO / 4 }, { NOTE_G4, TEMPO / 4 }, { NOTE_A4, TEMPO / 4 }, { NOTE_B4, TEMPO / 4 },
				{ NOTE_C5, TEMPO / 4 }, { NOTE_B4, TEMPO / 4 }, { NOTE_A4, TEMPO / 4 }, { NOTE_G4, TEMPO / 4 },
				{ NOTE_F4, TEMPO / 4 }, { NOTE_E4, TEMPO / 4 }, { NOTE_D4, TEMPO / 4 }, { NOTE_E4, TEMPO / 2 }
		};

Note startup_sound[] = {
		{ 783, 250 },  // G5 - descending
		{ 659, 200 },  // E5 - continuing down
		{ 523, 200 },  // C5 - near base
		{ 494, 200 },  // B4 - almost there
		{ 440, 500 }   // A4 - final resolution
};
Note snake_sounds[] = {
		// Game Start Sound (500ms)
		{ 1318, 100 },  // E6 - high beep
		{ 988, 100 },   // B5 - quick down
		{ 784, 150 },   // G5 - holding
		{ 659, 150 },   // E5 - base tone
		// Food Pickup Sound (300ms)
		{ 1396, 100 },  // F6 - high ping
		{ 1318, 100 },  // E6 - quick response
		{ 1175, 100 },  // D6 - completion
		// Movement Sound (200ms)
		{ 587, 100 },   // D5 - short blip
		{ 523, 100 },   // C5 - direction change
		// Collision Sound (1000ms)
		{ 247, 200 },   // B3 - low warning
		{ 220, 200 },   // A3 - descending
		{ 196, 300 },   // G3 - game over
		{ 175, 300 }    // F3 - final tone
};

static melody boot = { .notes = boot_note, .size = (sizeof(boot_note) / sizeof(Note)) };
static melody doom_m = { .notes = doom, .size = (sizeof(doom) / sizeof(Note)) };
static melody up1 = { .notes = ch_1_up, .size = 1 };
//static melody down1 = { .notes = ch_1_down, .size = 1 };
//static melody select1 = { .notes = ch_1_set, .size = 1 };
static melody up2 = { .notes = ch_2_up, .size = 1 };
static melody down2 = { .notes = ch_2_down, .size = 1 };
static melody select2 = { .notes = ch_2_set, .size = 1 };
static melody test_complete = { .notes = done_note, .size = (sizeof(done_note) / sizeof(Note)) };
static melody error = { .notes = error_note, .size = (sizeof(error_note) / sizeof(Note)) };

void Play_Note(Note note)
{
	if (note.frequency == 0) {
		return;
	}

	uint32_t timer_clock = HAL_RCC_GetPCLK1Freq(); // Timer clock frequency
	uint32_t prescaler = 160;                   // Example prescaler
	uint32_t period = (timer_clock / (prescaler + 1)) / note.frequency - 1;

	// Configure TIM4 for the note's frequency
	BUZZER_TIMER.Instance = TIM5;
	BUZZER_TIMER.Init.Prescaler = prescaler;
	BUZZER_TIMER.Init.CounterMode = TIM_COUNTERMODE_UP;
	BUZZER_TIMER.Init.Period = period;
	BUZZER_TIMER.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	BUZZER_TIMER.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	HAL_TIM_Base_Init(&BUZZER_TIMER);

	// Configure PWM
	TIM_OC_InitTypeDef sConfigOC = { 0 };
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = period / 2; // 50% duty cycle
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	HAL_TIM_PWM_ConfigChannel(&BUZZER_TIMER, &sConfigOC, BUZZER_TIMER_CHANNEL); // Use BUZZER_TIMER_CHANNEL

	// Start PWM on BUZZER_TIMER_CHANNEL
	HAL_TIM_PWM_Start(&BUZZER_TIMER, BUZZER_TIMER_CHANNEL);
}

void UBA_buzzer_update_state(UBA_buzzer *buzzer) {
	UART_LOG_INFO(UBA_COMP, "update state %u ---> %u", buzzer->state.current, buzzer->state.next);
	buzzer->state.pre = buzzer->state.current;
	buzzer->state.current = buzzer->state.next;
	buzzer->state.next = UBA_BUZZER_STATE_INVALID;
	buzzer->state.tick = HAL_GetTick();
}

void UBA_buzzer_init_enter(UBA_buzzer *buzzer) {
	UBA_buzzer_update_state(buzzer);
}

void UBA_buzzer_init(UBA_buzzer *buzzer) {

	UBA_buzzer_play_melody(&buzzer_g, UBA_BUZZER_BUZZ_BOOT);
//	buzzer->state.next = UBA_BUZZER_STATE_MUTE;
}

void UBA_buzzer_init_exit(UBA_buzzer *buzzer) {
}

void UBA_buzzer_off_enter(UBA_buzzer *buzzer) {
	UBA_buzzer_update_state(buzzer);
}

void UBA_buzzer_off(UBA_buzzer *buuzzer) {

}

void UBA_buzzer_off_exit(UBA_buzzer *buzzer) {
	buzzer->note_index = 0;
}
void UBA_buzzer_play_note_enter(UBA_buzzer *buzzer) {
	UBA_buzzer_update_state(buzzer);
	Play_Note(buzzer->melody_p->notes[buzzer->note_index]);

}

void UBA_buzzer_play_note(UBA_buzzer *buzzer) {
	if ((HAL_GetTick() - buzzer->state.tick) > buzzer->melody_p->notes[buzzer->note_index].duration) {
		buzzer->state.next = UBA_BUZZER_STATE_PLAY_NOTE;
	}
}

void UBA_buzzer_play_note_exit(UBA_buzzer *buzzer) {
	HAL_TIM_PWM_Stop(&BUZZER_TIMER, BUZZER_TIMER_CHANNEL); // Stop the note
	if (++buzzer->note_index >= buzzer->melody_p->size) {
		buzzer->state.next = UBA_BUZZER_STATE_OFF;
	}

}

void UBA_buzzer_play_melody(UBA_buzzer *buzzer, UBA_BUZZER_BUZZ_TYPE m_num) {
	if(buzzer->state.current == UBA_BUZZER_STATE_MUTE){
		return;
	}
	buzzer->state.next = UBA_BUZZER_STATE_PLAY_NOTE;
	buzzer->note_index = 0;
	switch (m_num) {
		case UBA_BUZZER_BUZZ_BOOT:
			buzzer->melody_p = &boot;
			break;
		case UBA_BUZZER_BUZZ_BNT_CH1_UP:
			buzzer->melody_p = &up2;
			break;
		case UBA_BUZZER_BUZZ_BNT_CH1_SELECT:
			buzzer->melody_p = &select2;
			break;
		case UBA_BUZZER_BUZZ_BNT_CH1_DOWN:
			buzzer->melody_p = &down2;
			break;
		case UBA_BUZZER_BUZZ_BNT_CH2_UP:
			buzzer->melody_p = &up2;
			break;
		case UBA_BUZZER_BUZZ_BNT_CH2_SELECT:
			buzzer->melody_p = &select2;
			break;
		case UBA_BUZZER_BUZZ_BNT_CH2_DOWN:
			buzzer->melody_p = &down2;
			break;
		case UBA_BUZZER_BUZZ_WARNNIG:
			buzzer->melody_p = &up1;
			break;
		case UBA_BUZZER_BUZZ_ERROR:
			buzzer->melody_p = &error;
			break;
		case UBA_BUZZER_BUZZ_COMPLEATE:
			buzzer->melody_p = &test_complete;
			break;
		case UBA_BUZZER_BUZZ_DOOM:
			buzzer->melody_p = &doom_m;
			break;
		default:
			buzzer->melody_p = &error;
			break;
	}

}

void UBA_buzzer_run(UBA_buzzer *buzzer) {
	if (buzzer->state.next == UBA_BUZZER_STATE_INVALID) { // if there the next state is not define , then run this state function
		if (rule_g[buzzer->state.current].run) {
			rule_g[buzzer->state.current].run(buzzer); // run the main function of the state
		}
	} else {
		if (buzzer->state.current < UBA_BUZZER_STATE_MAX) {
			if (rule_g[buzzer->state.current].exit) {
				rule_g[buzzer->state.current].exit(buzzer); // run the status exit function
			}
		}
		if (rule_g[buzzer->state.next].enter) {
			rule_g[buzzer->state.next].enter(buzzer); // run the next state enter function
		}
	}
}

void buzz(void) {
	uint8_t numNotes = sizeof(done_note) / sizeof(Note);
	for (uint8_t i = 0; i < numNotes; i++) {
		Play_Note(done_note[i]);
		HAL_Delay(50); // Short delay between notes
	}
}

