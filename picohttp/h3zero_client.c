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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "picoquic_internal.h"
#include "h3zero.h"
#include "h3zero_common.h"

#if 0
/* List of supported protocols 
 */

static picoquic_alpn_list_t alpn_list[] = {
    { picoquic_alpn_http_3, "h3", 2 },
    { picoquic_alpn_http_0_9, "hq-interop", 10 },
    { picoquic_alpn_http_3, "h3-34", 5 },
    { picoquic_alpn_http_0_9, "hq-34", 5 },
    { picoquic_alpn_http_3, "h3-33", 5 },
    { picoquic_alpn_http_0_9, "hq-33", 5 },
    { picoquic_alpn_http_3, "h3-32", 5 },
    { picoquic_alpn_http_0_9, "hq-32", 5 },
    { picoquic_alpn_http_3, "h3-31", 5 },
    { picoquic_alpn_http_0_9, "hq-31", 5 },
    { picoquic_alpn_http_3, "h3-29", 5 },
    { picoquic_alpn_http_0_9, "hq-29", 5 },
    { picoquic_alpn_http_3, "h3-30", 5 },
    { picoquic_alpn_http_0_9, "hq-30", 5 },
    { picoquic_alpn_http_3, "h3-28", 5 },
    { picoquic_alpn_http_0_9, "hq-28", 5 },
    { picoquic_alpn_http_3, "h3-27", 5 },
    { picoquic_alpn_http_0_9, "hq-27", 5 },
    { picoquic_alpn_siduck, "siduck", 6 },
    { picoquic_alpn_siduck, "siduck-00", 9 },
    { picoquic_alpn_quicperf, QUICPERF_ALPN, QUICPERF_ALPN_LEN}
};

static size_t nb_alpn_list = sizeof(alpn_list) / sizeof(picoquic_alpn_list_t);

void h3zero_client_set_alpn_list(void* tls_context)
{
    int ret = 0;

    for (size_t i = 0; i < nb_alpn_list; i++) {
        if (alpn_list[i].alpn_code == picoquic_alpn_http_3 ||
            alpn_list[i].alpn_code == picoquic_alpn_http_0_9) {
            ret = picoquic_add_proposed_alpn(tls_context, alpn_list[i].alpn_val);
            if (ret != 0) {
                DBG_PRINTF("Could not propose ALPN=%s, ret=0x%x", alpn_list[i].alpn_val, ret);
                break;
            }
        }
    }
}

picoquic_alpn_enum picoquic_parse_alpn(char const* alpn)
{
    picoquic_alpn_enum code = picoquic_alpn_undef;

    if (alpn != NULL) {
        for (size_t i = 0; i < nb_alpn_list; i++) {
            if (strcmp(alpn_list[i].alpn_val, alpn) == 0) {
                code = alpn_list[i].alpn_code;
                break;
            }
        }
    }

    return code;
}

picoquic_alpn_enum picoquic_parse_alpn_nz(char const* alpn, size_t len)
{
    picoquic_alpn_enum code = picoquic_alpn_undef;

    if (alpn != NULL) {
        for (size_t i = 0; i < nb_alpn_list; i++) {
            if (len == alpn_list[i].len &&
                memcmp(alpn, alpn_list[i].alpn_val, len) == 0) {
                code = alpn_list[i].alpn_code;
                break;
            }
        }
    }

    return code;
}

int h3zero_client_get_alpn_and_version_from_tickets(picoquic_quic_t* quic,
    char const* sni, char const* alpn, uint32_t proposed_version, uint64_t current_time, 
    char const ** ticket_alpn, uint32_t * ticket_version)
{
    int ret = -1;
    *ticket_version = 0;
    *ticket_alpn = NULL;
    if (sni != NULL) {
        uint16_t sni_length = (uint16_t) strlen(sni);
        uint8_t* ticket = NULL;
        uint16_t ticket_length = 0;
        picoquic_tp_t tp;

        if (alpn == NULL) {
            for (size_t i = 0; i < nb_alpn_list; i++) {
                *ticket_version = 0;
                if ((alpn_list[i].alpn_code == picoquic_alpn_http_3 ||
                    alpn_list[i].alpn_code == picoquic_alpn_http_0_9) &&
                    alpn_list[i].alpn_val != NULL) {
                    if (picoquic_get_ticket_and_version(quic->p_first_ticket, current_time,
                        sni, sni_length, alpn_list[i].alpn_val, (uint16_t)strlen(alpn_list[i].alpn_val), proposed_version, ticket_version,
                        &ticket, &ticket_length, &tp, 0) == 0){
                        ret = 0;
                        *ticket_alpn = alpn_list[i].alpn_val;
                        break;
                    }
                }
            }
        }
        else if (proposed_version == 0) {
            if (picoquic_get_ticket_and_version(quic->p_first_ticket, current_time,
                sni, sni_length, alpn, (uint16_t)strlen(alpn), proposed_version, ticket_version,
                &ticket, &ticket_length, &tp, 0) == 0) {
                ret = 0;
            }
        }
    }
    return ret;
}
#endif

#if 0
/*
 * Code common to H3 and H09 clients
 */

static h3zero_client_stream_ctx_t* h3zero_client_find_stream(
    picoquic_demo_callback_ctx_t* ctx, uint64_t stream_id)
{
    h3zero_client_stream_ctx_t * stream_ctx = ctx->first_stream;

    while (stream_ctx != NULL && stream_ctx->stream_id != stream_id) {
        stream_ctx = stream_ctx->next_stream;
    }

    return stream_ctx;
}


int demo_client_prepare_to_send(void * context, size_t space, uint64_t echo_length, uint64_t * echo_sent, FILE * F)
{
    int ret = 0;

    if (*echo_sent < echo_length) {
        uint8_t * buffer;
        uint64_t available = echo_length - *echo_sent;
        int is_fin = 1;

        if (available > space) {
            available = space;
            is_fin = 0;
        }

        buffer = picoquic_provide_stream_data_buffer(context, (size_t)available, is_fin, !is_fin);
        if (buffer != NULL) {
            if (F) {
                size_t nb_read = fread(buffer, 1, (size_t)available, F);

                if (nb_read != available) {
                    ret = -1;
                }
                else {
                    *echo_sent += available;
                    ret = 0;
                }
            }
            else {
                /* TODO: fill buffer with some text */
                memset(buffer, 0x5A, (size_t)available);
                *echo_sent += available;
                ret = 0;
            }
        }
        else {
            ret = -1;
        }
    }

    return ret;
}

#endif

/*
 * H3Zero client. This is a simple client that conforms to HTTP 3.0,
 * but the client implementation is barebone.
 */

int h3zero_client_create_stream_request(
    uint8_t * buffer, size_t max_bytes, uint8_t const * path, size_t path_len, uint64_t post_size, const char * host, size_t * consumed)
{
    int ret = 0;
    uint8_t * o_bytes = buffer;
    uint8_t * o_bytes_max = o_bytes + max_bytes;

    *consumed = 0;

    if (max_bytes < 3) {
        o_bytes = NULL;
    }
    else if (host == NULL) {
        o_bytes = NULL;
    }
    else {
        /* Create the request frame for the specified document */
        *o_bytes++ = h3zero_frame_header;
        o_bytes += 2; /* reserve two bytes for frame length */
        if (post_size == 0) {
            o_bytes = h3zero_create_request_header_frame(o_bytes, o_bytes_max,
                (const uint8_t *)path, path_len, host);
        }
        else {
            o_bytes = h3zero_create_post_header_frame(o_bytes, o_bytes_max,
                (const uint8_t *)path, path_len, host, h3zero_content_type_text_plain);
        }
    }

    if (o_bytes == NULL) {
        ret = -1;
    }
    else {
        size_t header_length = o_bytes - &buffer[3];
        if (header_length < 64) {
            buffer[1] = (uint8_t)(header_length);
            memmove(&buffer[2], &buffer[3], header_length);
            o_bytes--;
        }
        else {
            buffer[1] = (uint8_t)((header_length >> 8) | 0x40);
            buffer[2] = (uint8_t)(header_length & 0xFF);
        }

        if (post_size > 0) {
            /* Add initial DATA frame for POST */
            size_t ll = 0;

            if (o_bytes < o_bytes_max) {
                *o_bytes++ = h3zero_frame_data;
                ll = picoquic_varint_encode(o_bytes, o_bytes_max - o_bytes, post_size);
                o_bytes += ll;
            }
            if (ll == 0) {
                ret = -1;
            }
            else {
                *consumed = o_bytes - buffer;
            }
        }
        else {
            *consumed = o_bytes - buffer;
        }
    }

    return ret;
}

#if 0
/*
 * Unified procedures used for H3 and H09 clients
 */

static int h3zero_client_open_stream(picoquic_cnx_t* cnx,
    picoquic_demo_callback_ctx_t* ctx,
    uint64_t stream_id, char const* doc_name, char const* fname, uint64_t post_size, uint64_t nb_repeat)
{
    int ret = 0;
    uint8_t buffer[1024];
    h3zero_client_stream_ctx_t* stream_ctx = (h3zero_client_stream_ctx_t*)
        malloc(sizeof(h3zero_client_stream_ctx_t));

    if (stream_ctx == NULL) {
		fprintf(stdout, "Memory Error, cannot create stream context %d\n", (int)stream_id);
        ret = -1;
    }
    else {
        ctx->nb_open_streams++;
        ctx->nb_client_streams++;
        memset(stream_ctx, 0, sizeof(h3zero_client_stream_ctx_t));
        stream_ctx->next_stream = ctx->first_stream;
        ctx->first_stream = stream_ctx;
        stream_ctx->stream_id = stream_id + nb_repeat*4u;
        stream_ctx->post_size = post_size;

        if (ctx->no_disk) {
            stream_ctx->F = NULL;
        }
        else {
#ifdef _WINDOWS
            char const* sep = "\\";
#else
            char const* sep = "/";
#endif
            char const * x_name = fname;
            char path_name[1024];

            if (ctx->out_dir != NULL && (x_name[0] == '/' || x_name[0] == '_')) {
                /* If writing in the specified directory, remove the initial "/",
                 * or '_' if it was sanitized to that before. */
                x_name++;
            }

            if (nb_repeat > 0) {
                ret = picoquic_sprintf(path_name, sizeof(path_name), NULL, "%s%sr%dx%s",
                    (ctx->out_dir == NULL)?".": ctx->out_dir, sep, (int)nb_repeat, x_name);
            }
            else {
                ret = picoquic_sprintf(path_name, sizeof(path_name), NULL, "%s%s%s",
                    (ctx->out_dir == NULL) ? "." : ctx->out_dir, sep, x_name);
            }

            if (ret == 0) {
                stream_ctx->f_name = picoquic_string_duplicate(path_name);
                /* In order to reduce the number of open files, we only open the file when we start receiving data.*/
            }
            else {
                stream_ctx->F = NULL;
            }
        }
        if (ret == 0) {
            stream_ctx->is_open = 1;
        }else {
            picoquic_log_app_message(cnx, "Cannot create file name: %s", fname);
            fprintf(stdout, "Cannot create file name: %s\n", fname);
        }
    }

    if (ret == 0) {
        size_t request_length = 0;
        uint8_t name_buffer[514];
        uint8_t * path;
        size_t path_len;

		/* make sure that the doc name is properly formated */
        path = (uint8_t *)doc_name;
        path_len = strlen(doc_name);
        if (doc_name[0] != '/' && path_len + 2 <= sizeof(name_buffer)) {
            name_buffer[0] = '/';
            if (path_len > 0) {
                memcpy(&name_buffer[1], doc_name, path_len);
            }
            path = name_buffer;
            path_len++;
            name_buffer[path_len] = 0;
        }
        
        picoquic_log_app_message(cnx, "Preparing %s on stream %" PRIu64 " for %s",
                (post_size == 0) ? "GET" : "POST", stream_id, path);

        /* Format the protocol specific request */

        switch (ctx->alpn) {
        case picoquic_alpn_http_3:
            ret = h3zero_client_create_stream_request(
                buffer, sizeof(buffer), path, path_len, post_size, cnx->sni, &request_length);
            break;
        case picoquic_alpn_http_0_9:
        default:
            ret = h09_demo_client_prepare_stream_open_command(
                buffer, sizeof(buffer), path, path_len, post_size, cnx->sni, &request_length);
            break;
        }

		/* Send the request */

        if (ret == 0) {
            ret = picoquic_add_to_stream_with_ctx(cnx, stream_ctx->stream_id, buffer, request_length,
                (post_size > 0 || (ctx->delay_fin && stream_id == 0))?0:1, stream_ctx);
            if (post_size > 0) {
                ret = picoquic_mark_active_stream(cnx, stream_id, 1, stream_ctx);
            }
        }

        if (!ctx->no_print) {
            if (ret != 0) {
                fprintf(stdout, "Cannot send %s command for stream(%" PRIu64 "): %s\n",
                    (post_size == 0) ? "GET" : "POST", stream_ctx->stream_id, path);
            }
            else if (nb_repeat == 0) {
                fprintf(stdout, "Opening stream %d to %s %s\n", (int)stream_ctx->stream_id, (post_size == 0) ? "GET" : "POST", path);
            }
        }
    }

    return ret;
}

static int h3zero_client_close_stream(picoquic_cnx_t * cnx,
    picoquic_demo_callback_ctx_t* ctx, h3zero_client_stream_ctx_t* stream_ctx)
{
    int ret = 0;
    if (stream_ctx != NULL && stream_ctx->is_open) {
        picoquic_unlink_app_stream_ctx(cnx, stream_ctx->stream_id);
        if (stream_ctx->f_name != NULL) {
            free(stream_ctx->f_name);
            stream_ctx->f_name = NULL;
        }
        stream_ctx->F = picoquic_file_close(stream_ctx->F);
        if (stream_ctx->is_file_open) {
            ctx->nb_open_files--;
            stream_ctx->is_file_open = 0;
        }
        stream_ctx->is_open = 0;
        ctx->nb_open_streams--; 
        ret = 1;
    }
    return ret;
}

int h3zero_client_start_streams(picoquic_cnx_t* cnx,
    picoquic_demo_callback_ctx_t* ctx, uint64_t fin_stream_id)
{
    int ret = 0;

    /* First perform ALPN specific initialization.
	 * This will trigger sending the "settings" in H3 mode */
    if (fin_stream_id == PICOQUIC_DEMO_STREAM_ID_INITIAL) {
        switch (ctx->alpn) {
        case picoquic_alpn_http_3:
            ret = h3zero_client_init(cnx);
            break;
        default:
            break;
        }
    }

	/* Open all the streams scheduled after the stream that
	 * just finished */
    for (size_t i = 0; ret == 0 && i < ctx->nb_demo_streams; i++) {
        if (ctx->demo_stream[i].previous_stream_id == fin_stream_id) {
            uint64_t repeat_nb = 0;
            do {
                ret = h3zero_client_open_stream(cnx, ctx, ctx->demo_stream[i].stream_id,
                    ctx->demo_stream[i].doc_name,
                    ctx->demo_stream[i].f_name,
                    (size_t)ctx->demo_stream[i].post_size,
                    repeat_nb);
                repeat_nb++;
            } while (ret == 0 && repeat_nb < ctx->demo_stream[i].repeat_count);

            if (ret == 0 && repeat_nb > 1 && !ctx->no_print) {
                fprintf(stdout, "Repeated stream opening %d times.\n", (int)repeat_nb);
            }
            
            if (repeat_nb < ctx->demo_stream[i].repeat_count) {
                fprintf(stdout, "Could only open %d streams out of %d, ret = %d.\n", (int)repeat_nb, (int)ctx->demo_stream[i].repeat_count, ret);
            }
        }
    }

    return ret;
}

/* TODO: isolate file handling, etc., in demo specific callback */
int h3zero_client_open_stream_file(picoquic_cnx_t* cnx, picoquic_demo_callback_ctx_t* ctx, h3zero_client_stream_ctx_t* stream_ctx)
{
    int ret = 0;

    if (!stream_ctx->is_file_open && ctx->no_disk == 0) {
        int last_err = 0;
        stream_ctx->F = picoquic_file_open_ex(stream_ctx->f_name, "wb", &last_err);
        if (stream_ctx->F == NULL) {
            picoquic_log_app_message(cnx,
                "Could not open file <%s> for stream %" PRIu64 ", error %d (0x%x)\n", stream_ctx->f_name, stream_ctx->stream_id, last_err, last_err);
            DBG_PRINTF("Could not open file <%s> for stream %" PRIu64 ", error %d (0x%x)", stream_ctx->f_name, stream_ctx->stream_id, last_err, last_err);
            ret = -1;
        }
        else {
            stream_ctx->is_file_open = 1;
            ctx->nb_open_files++;
        }
    }

    return ret;
}
#endif


#if 0
/* TODO: merge with server callback, make that part of h3zero common */
int h3zero_client_callback(picoquic_cnx_t* cnx,
    uint64_t stream_id, uint8_t* bytes, size_t length,
    picoquic_call_back_event_t fin_or_event, void* callback_ctx, void* v_stream_ctx)
{
    int ret = 0;
    uint64_t fin_stream_id = PICOQUIC_DEMO_STREAM_ID_INITIAL;
    picoquic_demo_callback_ctx_t* ctx = (picoquic_demo_callback_ctx_t*)callback_ctx;
    h3zero_client_stream_ctx_t* stream_ctx = (h3zero_client_stream_ctx_t *)v_stream_ctx;

    ctx->last_interaction_time = picoquic_get_quic_time(cnx->quic);
    ctx->progress_observed = 1;

    switch (fin_or_event) {
    case picoquic_callback_stream_data:
    case picoquic_callback_stream_fin:
        /* Data arrival on stream #x, maybe with fin mark */
        if (stream_ctx == NULL) {
            stream_ctx = h3zero_client_find_stream(ctx, stream_id);
        }
        if (stream_ctx != NULL && stream_ctx->is_open) {
            if (!stream_ctx->is_file_open && ctx->no_disk == 0) {
                ret = h3zero_client_open_stream_file(cnx, ctx, stream_ctx);
            }
            if (ret == 0 && length > 0) {
                switch (ctx->alpn) {
                case picoquic_alpn_http_3: {
                    uint16_t error_found = 0;
                    size_t available_data = 0;
                    uint8_t * bytes_max = bytes + length;
                    while (bytes < bytes_max) {
                        bytes = h3zero_parse_data_stream(bytes, bytes_max, &stream_ctx->stream_state, &available_data, &error_found);
                        if (bytes == NULL) {
                            ret = picoquic_close(cnx, error_found);
                            if (ret != 0) {
                                picoquic_log_app_message(cnx,
                                    "Could not parse incoming data from stream %" PRIu64 ", error 0x%x", stream_id, error_found);
                            }
                            break;
                        }
                        else if (available_data > 0) {
                            if (!stream_ctx->flow_opened){
                                if (stream_ctx->stream_state.current_frame_length < 0x100000) {
                                    stream_ctx->flow_opened = 1;
                                }
                                else if (cnx->cnx_state == picoquic_state_ready) {
                                    stream_ctx->flow_opened = 1;
                                    ret = picoquic_open_flow_control(cnx, stream_id, stream_ctx->stream_state.current_frame_length);
                                }
                            }
                            if (ret == 0 && ctx->no_disk == 0) {
                                ret = (fwrite(bytes, 1, available_data, stream_ctx->F) > 0) ? 0 : -1;
                                if (ret != 0) {
                                    picoquic_log_app_message(cnx,
                                        "Could not write data from stream %" PRIu64 ", error 0x%x", stream_id, ret);
                                }
                            }
                            stream_ctx->received_length += available_data;
                            bytes += available_data;
                        }
                    }
                    break;
                }
                case picoquic_alpn_http_0_9:
                    if (ctx->no_disk == 0) {
                        ret = (fwrite(bytes, 1, length, stream_ctx->F) > 0) ? 0 : -1;
                        if (ret != 0) {
                            picoquic_log_app_message(cnx,
                                "Could not write data from stream %" PRIu64 ", error 0x%x", stream_id, ret);
                        }
                    }
                    stream_ctx->received_length += length;
                    break;
                default:
                    DBG_PRINTF("%s", "ALPN not selected!");
                    ret = -1;
                    break;
                }
            }

            if (fin_or_event == picoquic_callback_stream_fin) {
                if (h3zero_client_close_stream(cnx, ctx, stream_ctx)) {
                    fin_stream_id = stream_id;
                    if (stream_id <= 64 && !ctx->no_print) {
                        fprintf(stdout, "Stream %" PRIu64 " ended after %" PRIu64 " bytes\n",
                            stream_id, stream_ctx->received_length);
                    }
                    if (stream_ctx->received_length == 0) {
                        picoquic_log_app_message(cnx, "Stream %" PRIu64 " ended after %" PRIu64 " bytes, ret=0x%x",
                            stream_id, stream_ctx->received_length, ret);
                    }
                }
            }
        }
        break;
    case picoquic_callback_stream_reset: /* Server reset stream #x */
    case picoquic_callback_stop_sending: /* Server asks client to reset stream #x */
        /* TODO: special case for uni streams. */
        if (stream_ctx == NULL) {
            stream_ctx = h3zero_client_find_stream(ctx, stream_id);
        }
        if (h3zero_client_close_stream(cnx, ctx, stream_ctx)) {
            fin_stream_id = stream_id;
            if (!ctx->no_print) {
                fprintf(stdout, "Stream %" PRIu64 " reset after %" PRIu64 " bytes\n",
                    stream_id, stream_ctx->received_length);
            }
        }
        picoquic_reset_stream(cnx, stream_id, 0);
        /* TODO: higher level notify? */
        break;
    case picoquic_callback_stateless_reset:
        if (!ctx->no_print) {
            fprintf(stdout, "Received a stateless reset.\n");
        }
        break;
    case picoquic_callback_close: /* Received connection close */
        if (!ctx->no_print) {
            fprintf(stdout, "Received a request to close the connection.\n");
        }
        ctx->connection_closed = 1;
        break;
    case picoquic_callback_application_close: /* Received application close */
        if (!ctx->no_print) {
            fprintf(stdout, "Received a request to close the application.\n");
        }
        ctx->connection_closed = 1;
        break;
    case picoquic_callback_version_negotiation:
        if (!ctx->no_print) {
            fprintf(stdout, "Received a version negotiation request:");
            for (size_t byte_index = 0; byte_index + 4 <= length; byte_index += 4) {
                uint32_t vn = PICOPARSE_32(bytes + byte_index);
                fprintf(stdout, "%s%08x", (byte_index == 0) ? " " : ", ", vn);
            }
            fprintf(stdout, "\n");
        }
        break;
    case picoquic_callback_stream_gap:
        /* Gap indication, when unreliable streams are supported */
        fprintf(stdout, "Received a gap indication.\n");
        if (stream_ctx == NULL) {
            stream_ctx = h3zero_client_find_stream(ctx, stream_id);
        }
        if (h3zero_client_close_stream(cnx, ctx, stream_ctx)) {
            fin_stream_id = stream_id;
            fprintf(stdout, "Stream %d reset after %d bytes\n",
                (int)stream_id, (int)stream_ctx->received_length);
        }
        /* TODO: Define what error. Stop sending? */
        picoquic_reset_stream(cnx, stream_id, H3ZERO_INTERNAL_ERROR);
        break;
    case picoquic_callback_prepare_to_send:
        /* Used on client when posting data */
            /* Used for active streams */
        if (stream_ctx == NULL) {
            /* Unexpected */
            picoquic_reset_stream(cnx, stream_id, 0);
            return 0;
        }
        else {
            return demo_client_prepare_to_send((void*)bytes, length, stream_ctx->post_size, &stream_ctx->post_sent, NULL);
        }
    case picoquic_callback_almost_ready:
    case picoquic_callback_ready:
        ctx->connection_ready = 1;
        break;
    case picoquic_callback_request_alpn_list:
        h3zero_client_set_alpn_list((void*)bytes);
        break;
    case picoquic_callback_set_alpn:
        ctx->alpn = picoquic_parse_alpn((const char*)bytes);
        break;
    default:
        /* unexpected */
        break;
    }

    if (ret == 0 && fin_stream_id != PICOQUIC_DEMO_STREAM_ID_INITIAL) {
         /* start next batch of streams! */
		 ret = h3zero_client_start_streams(cnx, ctx, fin_stream_id);
    }

    /* that's it */
    return ret;
}
#endif

#if 0
int h3zero_client_initialize_context(
    picoquic_demo_callback_ctx_t* ctx,
    picoquic_demo_stream_desc_t const * demo_stream,
	size_t nb_demo_streams,
	char const * alpn,
    int no_disk, int delay_fin)
{
    memset(ctx, 0, sizeof(picoquic_demo_callback_ctx_t));
    ctx->demo_stream = demo_stream;
    ctx->nb_demo_streams = nb_demo_streams;
    ctx->alpn = picoquic_parse_alpn(alpn);
    ctx->no_disk = no_disk;
    ctx->delay_fin = delay_fin;

    return 0;
}


static void h3zero_client_delete_stream_context(picoquic_demo_callback_ctx_t* ctx,
    h3zero_client_stream_ctx_t * stream_ctx)
{
    int removed_from_context = 0;

    h3zero_delete_data_stream_state(&stream_ctx->stream_state);

    if (stream_ctx->f_name != NULL) {
        free(stream_ctx->f_name);
        stream_ctx->f_name = NULL;
    }

    if (stream_ctx->F != NULL) {
        DBG_PRINTF("Stream %" PRIu64 ", file close after %zu bytes\n", stream_ctx->stream_id, stream_ctx->received_length);
        stream_ctx->F = picoquic_file_close(stream_ctx->F);
    }

    if (stream_ctx == ctx->first_stream) {
        ctx->first_stream = stream_ctx->next_stream;
        removed_from_context = 1;
    }
    else {
        h3zero_client_stream_ctx_t * previous = ctx->first_stream;

        while (previous != NULL) {
            if (previous->next_stream == stream_ctx) {
                previous->next_stream = stream_ctx->next_stream;
                removed_from_context = 1;
                break;
            }
            else {
                previous = previous->next_stream;
            }
        }
    }

    if (removed_from_context) {
        ctx->nb_client_streams--;
    }

    free(stream_ctx);
}

void h3zero_client_delete_context(picoquic_demo_callback_ctx_t* ctx)
{
    h3zero_client_stream_ctx_t * stream_ctx;

    while ((stream_ctx = ctx->first_stream) != NULL) {
        h3zero_client_delete_stream_context(ctx, stream_ctx);
    }
}
#endif

#if 0
/* TODO: move to "utils" definition */
char const * demo_client_parse_stream_spaces(char const * text) {
    while (*text == ' ' || *text == '\t' || *text == '\n' || *text == '\r') {
        text++;
    }
    return text;
}

char const * demo_client_parse_stream_repeat(char const * text, int * number)
{
    int rep = 0;

    if (*text == '*') {
        text++;
        while (text[0] >= '0' && text[0] <= '9') {
            rep *= 10;
            rep += *text++ - '0';
        }

        text = demo_client_parse_stream_spaces(text);

        if (*text == ':') {
            text++;
        }
        else {
            text = NULL;
        }
    }
    *number = rep;

    return text;
}

char const * demo_client_parse_stream_number(char const * text, uint64_t default_number, uint64_t * number)
{
    if (text[0] < '0' || text[0] > '9') {
        *number = default_number;
    }
    else {
        *number = 0;
        do {
            int delta = *text++ - '0';
            *number *= 10;
            *number += delta;
        } while (text[0] >= '0' && text[0] <= '9');

        text = demo_client_parse_stream_spaces(text);

        if (*text == ':') {
            text++;
        }
        else {
            text = NULL;
        }
    }

    return text;
}

char const* demo_client_parse_stream_previous(char const* text, uint64_t default_number, uint64_t* number)
{

    if (text[0] != '-') {
        text = demo_client_parse_stream_number(text, default_number, number);
    }
    else {
        *number = PICOQUIC_DEMO_STREAM_ID_INITIAL;
        text++;

        text = demo_client_parse_stream_spaces(text);

        if (*text == ':') {
            text++;
        }
        else {
            text = NULL;
        }
    }

    return text;
}

char const * demo_client_parse_stream_path(char const * text, 
    char ** path, char ** f_name)
{
    size_t l_path = 0;
    int is_complete = 0;
    int need_dup = 0;

    while (text != NULL) {
        char c = text[l_path];

        if (c == 0 || c == ';' || c == ':') {
            is_complete = 1;
            break;
        }
        
        if (c == '/') {
            need_dup = 1;
        }

        l_path++;
    }

    if (is_complete) {
        *path = (char *)malloc(l_path + 1);
        if (*path == NULL) {
            is_complete = 0;
        }
        else {
            if (need_dup) {
                *f_name = (char *)malloc(l_path + 1);
                if (*f_name == NULL) {
                    is_complete = 0;
                    free(*path);
                    *path = NULL;
                }
            }
        }
    }

    if (is_complete) {
        memcpy(*path, text, l_path);
        (*path)[l_path] = 0;
        if (need_dup) {
            for (size_t i = 0; i < l_path; i++) {
                (*f_name)[i] = (text[i] == '/') ? '_' : text[i];
            }
            (*f_name)[l_path] = 0;
        }
        else {
            *f_name = *path;
        }

        text += l_path;

        if (*text == ':') {
            text++;
        }
    }
    else {
        text = NULL;
    }
    
    return text;
}

char const * demo_client_parse_post_size(char const * text, uint64_t * post_size)
{
    if (text[0] < '0' || text[0] > '9') {
        *post_size = 0;
    }
    else {
        *post_size = 0;
        do {
            int delta = *text - '0';
            *post_size *= 10;
            *post_size += delta;
            if (*post_size > (UINT64_MAX >> 4)) {
                break;
            }
            else {
                text++;
            }
        } while (text[0] >= '0' && text[0] <= '9');

        text = demo_client_parse_stream_spaces(text);

        if (*text == ':') {
            text++;
        }
        else if (*text != 0 && *text != ';'){
            text = NULL;
        }
    }

    return text;
}

char const * demo_client_parse_stream_desc(char const * text, uint64_t default_stream, uint64_t default_previous,
    picoquic_demo_stream_desc_t * desc)
{
    text = demo_client_parse_stream_repeat(text, &desc->repeat_count);

    if (text != NULL) {
        text = demo_client_parse_stream_number(text, default_stream, &desc->stream_id);
    }

    if (text != NULL) {
        text = demo_client_parse_stream_previous(
            demo_client_parse_stream_spaces(text), default_previous, &desc->previous_stream_id);
    }
    
    if (text != NULL){
        text = demo_client_parse_stream_path(
            demo_client_parse_stream_spaces(text), (char **)&desc->doc_name, (char **)&desc->f_name);
    }

    if (text != NULL) {
        text = demo_client_parse_post_size(demo_client_parse_stream_spaces(text), &desc->post_size);
    }

    /* Skip the final ';' */
    if (text != NULL && *text == ';') {
        text++;
    }

    return text;
}

void demo_client_delete_scenario_desc(size_t nb_streams, picoquic_demo_stream_desc_t * desc)
{
    for (size_t i = 0; i < nb_streams; i++) {
        if (desc[i].f_name != desc[i].doc_name && desc[i].f_name != NULL) {
            free((char*)desc[i].f_name);
            *(char**)(&desc[i].f_name) = NULL;
        }
        if (desc[i].doc_name != NULL) {
            free((char*)desc[i].doc_name);
            *(char**)(&desc[i].doc_name) = NULL;
        }
    }
    free(desc);
}

size_t demo_client_parse_nb_stream(char const * text) {
    size_t n = 0;
    int after_semi = 1;

    while (*text != 0) {
        if (*text++ == ';') {
            n++;
            after_semi = 0;
        }
        else {
            after_semi = 1;
        }
    }

    n += after_semi;

    return n;
}

int demo_client_parse_scenario_desc(char const * text, size_t * nb_streams, picoquic_demo_stream_desc_t ** desc)
{
    int ret = 0;
    /* first count the number of streams and allocate memory */
    size_t nb_desc = demo_client_parse_nb_stream(text);
    size_t i = 0;
    uint64_t previous = PICOQUIC_DEMO_STREAM_ID_INITIAL;
    uint64_t stream_id = 0;

    *desc = (picoquic_demo_stream_desc_t *)malloc(nb_desc*sizeof(picoquic_demo_stream_desc_t));

    if (*desc == NULL) {
        *nb_streams = 0;
        ret = -1;
    }
    else {
        while (text != NULL ) {
            text = demo_client_parse_stream_spaces(text);
            if (*text == 0) {
                break;
            }
            if (i >= nb_desc) {
                /* count was wrong! */
                break;
            }
            else {
                picoquic_demo_stream_desc_t* stream_desc = &(*desc)[i];
                text = demo_client_parse_stream_desc(text, stream_id, previous, stream_desc);
                if (text != NULL) {
                    stream_id = stream_desc->stream_id + 4;
                    previous = stream_desc->stream_id;
                    i++;
                }
            }
        }

        *nb_streams = i;

        if (text == NULL) {
            ret = -1;
        }
    }

    return ret;
}
#endif