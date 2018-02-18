#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "turing.h"

// disable debug mode
#define NDEBUG
#include "debug.h"

// used to hand out ids
int state_id = 0;

void die( char *message )
{
    if( message )
    {
        printf( "Error: %s.\n", message );
    }

    // exit unsuccesfully
    exit(1);
}

Transition* Transition_create( char input, char write, Direction move, State *next )
{
    // allocate memory
    Transition *trans = malloc( sizeof( Transition ));
    if( ! trans ) die( "Memory error" );

    trans->input = input;
    trans->write = write;
    trans->move = move;
    trans->next = next;

    return trans;
}

void Transition_destroy( Transition* trans )
{
    free( trans );
}

State* State_create( Bool accept, Bool reject )
{
    // allocate mem
    State *state = malloc( sizeof( State ));
    if( ! state ) die( "Memory error" );

    state->id = state_id++;
    state->accept = accept;
    state->reject = reject;
    state->trans_count = 0;

    return state;
}

void State_add_transition( State *state, Transition *trans )
{
    // check if we can still add another transition
    if( state->trans_count == MAX_TRANSITIONS ) {
        char buffer[ 50 ];
        sprintf( buffer, "State %d already has the maximum amount of transitions.", state->id );

        die( buffer );
    }

    // add the transition
    state->transitions[ state->trans_count ] = trans;
    state->trans_count++;
}

void State_destroy( State *state )
{
    int i = 0;

    // loop over its transitions
    for( i = 0; i < state->trans_count; i++ ) {
        Transition *trans = state->transitions[ i ];
        if( !trans ) die( "Could not fetch transition." );

        Transition_destroy( trans );
    }

    free( state );
}

Turing* Turing_create()
{
    // allocate mem
    Turing *machine = malloc( sizeof( Turing ));

    machine->state_count = 0;
    machine->current = NULL;
    machine->head = 0;

    return machine;
}

void Turing_destroy( Turing *machine )
{
    int i = 0;

    // loop over it's states
    for( i = 0; i < machine->state_count; i++ ) {
        State *state = machine->states[ i ];
        if( !state ) die( "Could not fetch turing state" );

        State_destroy( state );
    }

    free( machine );
}

void Turing_add_state( Turing *machine, State *state )
{
    if( machine->state_count == MAX_STATES ) {
        die( "The turing machine already has the maximum amount of states" );
    }

    // add the state
    machine->states[ machine->state_count++ ] = state;
}

State* Turing_step( Turing *machine, char* tape, int tape_len )
{
    int i = 0;
    char input = tape[ machine->head ];
    State* state = machine->current;

    // look for a transition on the given input
    for( i = 0; i < state->trans_count; i++ ) {
        Transition* trans = state->transitions[ i ];
        if( !trans ) die( "Transition retrieval error" );

        // check if this is a transition in the given char input
        if( trans->input == input ) {
            debug( "Found transition for input %c", input );

            State *next = trans->next;
            if( !next ) die( "Transitions to NULL state" );

            // write if nescesary
            if( trans->write != '\0' ) {
                debug( "Writing %c", trans->write );
                tape[ machine->head ] = trans->write;
                debug( "Writing done" );
            }

            // move the head
            if( trans->move == LEFT ) {
                if( machine->head > 0 ) {
                    machine->head--;
                    debug( "Moved head left" );
                }
            } else {
                if( machine->head + 1 >= tape_len ) {
                    die( "Machine walked of tape on right side" );
                }

                machine->head++;
                debug( "Moved head right" );
            }

            // move the machine to the next state
            debug( "Setting current state" );
            machine->current = next;

            return next;
        }
    }

    char buffer[ 50 ];
    sprintf( buffer, "Turing machine blocked: state %d for input %c", state->id, input );

    die( buffer );
}

void Turing_run( Turing *machine, char *tape, int tapelen )
{
    // check if the start state is configured properly
    if( !machine->current ) die( "Turing machine has now start state" );

    while( TRUE ) {
        State* state = Turing_step( machine, tape, tapelen );

        if( state->accept ) {
            printf( "Input accepted in state: %d\n", state->id );
            break;
        } else if( state->reject ) {
            printf( "Input rejected in state: %d\n", state->id );
            break;
        } else {
            printf( "Moved to state: %d\n", state->id );    
        }
    }
}

int main( int argc, char* argv[] )
{
    Turing* machine = Turing_create();

    State* q1 = State_create( FALSE, FALSE );
    State* q2 = State_create( FALSE, FALSE );
    State* q3 = State_create( FALSE, FALSE );
    State* q4 = State_create( FALSE, FALSE );
    State* q5 = State_create( FALSE, FALSE );
    State* qaccept = State_create( TRUE, FALSE );
    State* qreject = State_create( FALSE, TRUE );

    Transition* q1_r_space = Transition_create( ' ', '\0', RIGHT, qreject );
    Transition* q1_r_x = Transition_create( 'x', '\0', RIGHT, qreject );
    Transition* q1_q2_zero = Transition_create( '0', ' ', RIGHT, q2 );
    Transition* q2_q2_x = Transition_create( 'x', '\0', RIGHT, q2 );
    Transition* q2_a_space = Transition_create( ' ', '\0', RIGHT, qaccept );
    Transition* q2_q3_zero = Transition_create( '0', 'x', RIGHT, q3 );
    Transition* q3_q3_x = Transition_create( 'x', '\0', RIGHT, q3 );
    Transition* q3_q4_zero = Transition_create( '0', '\0', RIGHT, q4 );
    Transition* q3_q5_space = Transition_create( ' ', '\0', LEFT, q5 );
    Transition* q4_q3_zero = Transition_create( '0', 'x', RIGHT, q3 );
    Transition* q4_q4_x = Transition_create( 'x', '\0', RIGHT, q4 );
    Transition* q4_r_space = Transition_create( ' ', '\0', RIGHT, qreject );
    Transition* q5_q5_zero = Transition_create( '0', '\0', LEFT, q5 ); 
    Transition* q5_q5_x = Transition_create( 'x', '\0', LEFT, q5 ); 
    Transition* q5_q2_space = Transition_create( ' ', '\0', RIGHT, q2 ); 

    State_add_transition( q1, q1_r_space );
    State_add_transition( q1, q1_r_x );
    State_add_transition( q1, q1_q2_zero );
    State_add_transition( q2, q2_q2_x );
    State_add_transition( q2, q2_a_space );
    State_add_transition( q2, q2_q3_zero );
    State_add_transition( q3, q3_q3_x );
    State_add_transition( q3, q3_q4_zero );
    State_add_transition( q3, q3_q5_space );
    State_add_transition( q4, q4_q3_zero );
    State_add_transition( q4, q4_q4_x );
    State_add_transition( q4, q4_r_space );
    State_add_transition( q5, q5_q5_zero );
    State_add_transition( q5, q5_q5_x );
    State_add_transition( q5, q5_q2_space );

    Turing_add_state( machine, q1 );
    Turing_add_state( machine, q2 );
    Turing_add_state( machine, q3 );
    Turing_add_state( machine, q4 );
    Turing_add_state( machine, q5 );
    Turing_add_state( machine, qaccept );
    Turing_add_state( machine, qreject );

    machine->current = q1;

    char* input = "0000000000000000  ";
    int len = strlen( input );
    char* tape = malloc( len * sizeof( char ));
    strcpy( tape, input );
    Turing_run( machine, tape, len );

    // clean up
    Turing_destroy( machine );
    free( tape );
}
