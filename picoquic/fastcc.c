/*
* Author: Christian Huitema
* Copyright (c) 2019, Private Octopus, Inc.
* All rights reserved.
*
* Permission to use, copy, modify, and distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL Private Octopus, Inc. BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "picoquic_internal.h"
#include <stdlib.h>
#include <string.h>


#define FASTCC_MIN_ACK_DELAY_FOR_BANDWIDTH 10
#define FASTCC_BANDWIDTH_FRACTION 0.5
#define FASTCC_REPEAT_THRESHOLD 3
#define FASTCC_BETA 0.125
#define FASTCC_EVAL_ALPHA 0.125
#define FASTCC_DELAY_THRESHOLD_MAX 10000
#define FASTCC_NB_PERIOD 6
#define FASTCC_PERIOD 1000000


typedef enum {
    picoquic_fastcc_initial = 0,
    picoquic_fastcc_eval,
    picoquic_fastcc_freeze
} picoquic_fastcc_alg_state_t;

typedef struct st_picoquic_fastcc_state_t {
    picoquic_fastcc_alg_state_t alg_state;
    uint64_t end_of_freeze; /* When to exit the freeze state */
    uint64_t last_ack_time;
    uint64_t ack_interval;
    uint64_t nb_bytes_ack;
    uint64_t nb_bytes_ack_since_rtt; /* accumulate byte count until RTT measured */
    uint64_t end_of_epoch;
    uint64_t rtt_min;
    uint64_t delay_threshold;
    uint64_t rolling_rtt_min; /* Min RTT measured for this epoch */
    uint64_t last_rtt_min[FASTCC_NB_PERIOD];
    int nb_cc_events;
} picoquic_fastcc_state_t;

uint64_t picoquic_fastcc_delay_threshold(uint64_t rtt_min)
{
    uint64_t delay = rtt_min / 8;
    if (delay > FASTCC_DELAY_THRESHOLD_MAX) {
        delay = FASTCC_DELAY_THRESHOLD_MAX;
    }
    return delay;
}

void picoquic_fastcc_init(picoquic_path_t* path_x)
{
    /* Initialize the state of the congestion control algorithm */
    picoquic_fastcc_state_t* fastcc_state = (picoquic_fastcc_state_t*)malloc(sizeof(picoquic_fastcc_state_t));
    path_x->congestion_alg_state = (void*)fastcc_state;

    if (path_x->congestion_alg_state != NULL) {
        memset(fastcc_state, 0, sizeof(picoquic_fastcc_state_t));
        fastcc_state->alg_state = picoquic_fastcc_initial;
        fastcc_state->rtt_min = path_x->smoothed_rtt;
        fastcc_state->rolling_rtt_min = fastcc_state->rtt_min;
        fastcc_state->delay_threshold = picoquic_fastcc_delay_threshold(fastcc_state->rtt_min);
        path_x->cwin = PICOQUIC_CWIN_INITIAL;
    }
}

void fastcc_process_cc_event(picoquic_path_t* path_x, picoquic_fastcc_state_t* fastcc_state, uint64_t current_time)
{
    fastcc_state->nb_cc_events++;
    if (fastcc_state->nb_cc_events >= FASTCC_REPEAT_THRESHOLD) {
        /* Too many events, reduce the window */
        if (fastcc_state->alg_state != picoquic_fastcc_freeze) {
            fastcc_state->alg_state = picoquic_fastcc_freeze;
            fastcc_state->end_of_freeze = current_time + path_x->smoothed_rtt;
            path_x->cwin -= (uint64_t)(FASTCC_BETA * (double)path_x->cwin);
            if (path_x->cwin < PICOQUIC_CWIN_MINIMUM) {
                path_x->cwin = PICOQUIC_CWIN_MINIMUM;
            }
            picoquic_update_pacing_data(path_x);
        }
    }
}


/*
 * Properly implementing fastcc requires managing a number of
 * signals, such as packet losses or acknowledgements. We attempt
 * to condensate all that in a single API, which could be shared
 * by many different congestion control algorithms.
 */
void picoquic_fastcc_notify(picoquic_path_t* path_x,
    picoquic_congestion_notification_t notification,
    uint64_t rtt_measurement,
    uint64_t nb_bytes_acknowledged,
    uint64_t lost_packet_number,
    uint64_t current_time)
{
#ifdef _WINDOWS
    UNREFERENCED_PARAMETER(lost_packet_number);
#endif
    picoquic_fastcc_state_t* fastcc_state = (picoquic_fastcc_state_t*)path_x->congestion_alg_state;

    if (fastcc_state != NULL) {
        if (fastcc_state->alg_state == picoquic_fastcc_freeze && current_time > fastcc_state->end_of_freeze) {
            fastcc_state->alg_state = picoquic_fastcc_eval;
            fastcc_state->nb_cc_events = 0;
            fastcc_state->nb_bytes_ack_since_rtt = 0;
        }

        switch (notification) {
        case picoquic_congestion_notification_acknowledgement: 
            if (fastcc_state->alg_state != picoquic_fastcc_freeze) {
                /* Count the bytes since last RTT measurement */
                fastcc_state->nb_bytes_ack_since_rtt += nb_bytes_acknowledged;

                if (fastcc_state->alg_state == picoquic_fastcc_init){
                    /* In initial phase, compute the instant bandwidth and the corresponding bandwidth delay product */
                    /* first evaluate the ack delay and the nb bytes to take into account*/
                    uint64_t delta_time_ack = current_time - fastcc_state->last_ack_time;

                    if (delta_time_ack < FASTCC_MIN_ACK_DELAY_FOR_BANDWIDTH) {
                        /* Merge back to back acknowledgement before evaluating bandwidth */
                        fastcc_state->ack_interval += delta_time_ack;
                        fastcc_state->nb_bytes_ack += nb_bytes_acknowledged;
                    }
                    else {
                        /* Evaluate bandwdith based on this acknowledgement alone */
                        fastcc_state->ack_interval = delta_time_ack;
                        fastcc_state->nb_bytes_ack = nb_bytes_acknowledged;
                    }
                    fastcc_state->last_ack_time = current_time;
                    /* If enough bytes resceived, reset the bandwidth */
                    if (fastcc_state->ack_interval > 0) {
                        double instant_megabyte_per_usec = (double)fastcc_state->nb_bytes_ack / (double)fastcc_state->ack_interval;
                        double instant_bdp_in_bytes = instant_megabyte_per_usec * (double)fastcc_state->rtt_min;
                        double delta_cwin = instant_bdp_in_bytes - (double)path_x->cwin;

                        if (delta_cwin > 0) {
                            /* Only take into account a fraction of the measured bandwidth */
                            delta_cwin *= FASTCC_BANDWIDTH_FRACTION;
                            path_x->cwin += (uint64_t)delta_cwin;
                        }
                    }
                }

                /* Compute pacing data. */
                picoquic_update_pacing_data(path_x);
            }
            break;  
        case picoquic_congestion_notification_ecn_ec:
        case picoquic_congestion_notification_repeat:
        case picoquic_congestion_notification_timeout:
            if (fastcc_state->alg_state != picoquic_fastcc_freeze) {
                /* Count packet losses as maybe cc events */
                fastcc_process_cc_event(path_x, fastcc_state, current_time);
            }
            break;
        case picoquic_congestion_notification_spurious_repeat:
            if (fastcc_state->nb_cc_events > 0) {
                fastcc_state->nb_cc_events--;
            }
            break;
        case picoquic_congestion_notification_rtt_measurement:
        {
            uint64_t delta_rtt = 0;

            if (current_time > fastcc_state->end_of_epoch) {
                /* If end of epoch, reset the min RTT to min of remembered periods,
                 * and roll the period. */
                fastcc_state->rtt_min = (uint64_t)((int64_t)-1);
                for (int i = FASTCC_NB_PERIOD - 1; i > 0; i--) {
                    fastcc_state->last_rtt_min[i] = fastcc_state->last_rtt_min[i - 1];
                    if (fastcc_state->last_rtt_min[i] > 0 &&
                        fastcc_state->last_rtt_min[i] < fastcc_state->rtt_min) {
                        fastcc_state->rtt_min = fastcc_state->last_rtt_min[i];
                    }
                }
                fastcc_state->delay_threshold = picoquic_fastcc_delay_threshold(fastcc_state->rtt_min);
                fastcc_state->last_rtt_min[0] = fastcc_state->rolling_rtt_min;
                fastcc_state->rolling_rtt_min = rtt_measurement;
                fastcc_state->end_of_epoch = current_time + path_x->smoothed_rtt;
            }
            else if (rtt_measurement < fastcc_state->rolling_rtt_min || fastcc_state->rolling_rtt_min == 0) {
                /* If not end of epoch, update the rolling minimum */
                fastcc_state->rolling_rtt_min = rtt_measurement;
            }

            if (fastcc_state->alg_state != picoquic_fastcc_freeze) {
                if (rtt_measurement < fastcc_state->rtt_min) {
                    fastcc_state->rtt_min = rtt_measurement;
                    fastcc_state->delay_threshold = picoquic_fastcc_delay_threshold(fastcc_state->rtt_min);
                }
                else {
                    delta_rtt = rtt_measurement - fastcc_state->rtt_min;
                }

                if (delta_rtt < fastcc_state->delay_threshold) {
                    double alpha = 1.0;

                    if (fastcc_state->nb_cc_events > 0) {
                        fastcc_state->nb_cc_events = 0;
                    }

                    if (fastcc_state->alg_state != picoquic_fastcc_initial) {
                        alpha -= ((double)delta_rtt / (double)fastcc_state->delay_threshold);
                        alpha *= FASTCC_EVAL_ALPHA;
                    }

                    /* Increase the window if it is not frozen */
                    path_x->cwin += (uint64_t)(alpha * (double)fastcc_state->nb_bytes_ack_since_rtt);
                    fastcc_state->nb_bytes_ack_since_rtt = 0;
                }
                else {
                    /* May well be congested */
                    fastcc_process_cc_event(path_x, fastcc_state, current_time);
                }
            }
        }
        break;
        default:
            /* ignore */
            break;
        }
    }
}

/* Release the state of the congestion control algorithm */
void picoquic_fastcc_delete(picoquic_path_t* path_x)
{
    if (path_x->congestion_alg_state != NULL) {
        free(path_x->congestion_alg_state);
        path_x->congestion_alg_state = NULL;
    }
}

/* Definition record for the BAYES CC algorithm */

#define picoquic_fastcc_ID 0x42415945 /* BAYE */

picoquic_congestion_algorithm_t picoquic_fastcc_algorithm_struct = {
    picoquic_fastcc_ID,
    picoquic_fastcc_init,
    picoquic_fastcc_notify,
    picoquic_fastcc_delete
};

picoquic_congestion_algorithm_t* picoquic_fastcc_algorithm = &picoquic_fastcc_algorithm_struct;
