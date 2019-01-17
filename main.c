#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

typedef enum state {
	STATE_INITIAL, //'r'
	R_OK, //'i'
	I_OK, //'p'
	P_OK, //'e'
	STATE_SUCCESS,/* success */
	STATE_FAIL, /* fail */
	STATE_MAX = STATE_FAIL,
	/* don't use, keep last */
	STATE_COUNT,
} state_t;

struct sm_state {
	state_t state;
	char ch;
	int (*validator)(char target, char compare);
	struct sm_state *next_state;
};

struct ripe_sm {
	struct sm_state sm_states[STATE_COUNT];
	struct sm_state *curr_state;
	void (*step)(struct ripe_sm *sm, char input);
	char target[5];
};

int char_validator(char target, char compare)
{
	return target == compare;
}

void step(struct ripe_sm *sm, char input)
{
	struct sm_state *state = sm->curr_state;

	if (state->validator(state->ch, input)) {
		sm->curr_state = state->next_state;
		printf("validated %c=%c: state=%d, next_state=%d\n", state->ch, input, state->state, state->next_state->state);
	} else {
		sm->curr_state = &(sm->sm_states[STATE_FAIL]);
		printf("fail validate: state=%d, %c!=%c\n", state->state, state->ch, input);
	}
}

int main(int argc, char **argv) {
	struct ripe_sm sm;
	struct sm_state *init_state;
	char * arg;

	if (argc != 2) {
		fprintf(stderr, "Incorrect # of arguments\n");
		return EINVAL;
	}

	memset(&sm, 0, sizeof(struct ripe_sm));
	strncpy(&(sm.target[0]), "ripe", 5);

	arg = calloc(1, strlen(argv[1]) + 1);
	if (!arg)
		return ENOMEM;

	strncpy(arg, argv[1], strlen(argv[1]));

	printf("SM: size=%lu, target=%s\n", sizeof(struct ripe_sm), (const char *)sm.target);

	sm.step = &step;
	for (int i = 0; i < STATE_COUNT; i++) {
		sm.sm_states[i].state = (state_t)i;

		if ((state_t)i == STATE_FAIL) {
			sm.sm_states[i].validator = NULL;
			sm.sm_states[i].next_state = NULL;
			break;
		}

		sm.sm_states[i].validator = &char_validator;
		sm.sm_states[i].next_state = &(sm.sm_states[i + 1]);
		if (i < strlen(sm.target))
			sm.sm_states[i].ch = sm.target[i];
		else
			sm.sm_states[i].ch = '\0';
		printf("Set: state=%d, char=%c, next_state=%d\n", sm.sm_states[i].state, sm.sm_states[i].ch, sm.sm_states[i].next_state->state);
	}

	printf("here\n");
	sm.curr_state = &(sm.sm_states[0]);
	for (int i = 0; i < strlen(arg); i++) {
		printf("arg[%d]=%c\n", i, arg[i]);
		sm.step(&sm, arg[i]);
		if (sm.curr_state->state == STATE_FAIL)
			return 1;
		else if (sm.curr_state->state == STATE_SUCCESS) {
			printf("\n\nSuccess!\n\n\n");
			break;
		}
	}
	
	return 0;
}
