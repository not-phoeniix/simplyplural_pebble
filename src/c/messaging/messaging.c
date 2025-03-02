#include "messaging.h"
#include <pebble.h>
#include "../config/config.h"
#include "../member_collections.h"

#define NUM_KEYS 3

//! NOTE: all the MESSAGE_KEY_WhateverKey defines are added
//!   later via the compiler.... these will look like errors
//!   in vscode but there's nothing wrong with em dw <3

static void inbox_recieved_handler(DictionaryIterator* iter, void* context) {
    ClaySettings* settings = settings_get();

    Tuple* accent_color = dict_find(iter, MESSAGE_KEY_AccentColor);
    if (accent_color != NULL) {
        settings->accent_color = GColorFromHEX(accent_color->value->int32);
    }

    Tuple* compact_member_list = dict_find(iter, MESSAGE_KEY_CompactMemberList);
    if (compact_member_list != NULL) {
        settings->compact_member_list = compact_member_list->value->int16;
    }

    Tuple* member_color_highlight = dict_find(iter, MESSAGE_KEY_MemberColorHighlight);
    if (member_color_highlight != NULL) {
        settings->member_color_highlight = member_color_highlight->value->int16;
    }

    Tuple* members = dict_find(iter, MESSAGE_KEY_Members);
    if (members != NULL) {
        members_set_all(members->value->cstring);
    }

    Tuple* fronters = dict_find(iter, MESSAGE_KEY_Fronters);
    if (fronters != NULL) {
        members_set_fronters(fronters->value->cstring);
    }

    settings_save();
}

static void inbox_dropped_callback(AppMessageResult reason, void* context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped. Reason: %d", (int)reason);
}

static void outbox_sent_handler(DictionaryIterator* iter, void* context) {
    printf("outbox sent !!");
}

static void outbox_failed_callback(DictionaryIterator* iter, AppMessageResult reason, void* context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox failed. Reason: %d", (int)reason);
}

void messaging_init() {
    app_message_register_inbox_received(inbox_recieved_handler);
    app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_register_outbox_sent(outbox_sent_handler);
    app_message_register_outbox_failed(outbox_failed_callback);

    // hoping 1024 bytes is enough, adjust if necessary later <3
    app_message_open(1024, 128);
}

void messaging_add_to_front(Member* member) {
    // dictionary to send!
    DictionaryIterator* iter;

    // begin outbox app message
    AppMessageResult result = app_message_outbox_begin(&iter);
    if (result == APP_MSG_OK) {
        // write member name as the request string data value
        dict_write_cstring(iter, MESSAGE_KEY_AddFrontRequest, member->name);

        // send outbox message itself
        result = app_message_outbox_send();

        if (result != APP_MSG_OK) {
            APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending add_to_front data: %d", (int)result);
        }

    } else {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing add_to_front outbox: %d", (int)result);
    }
}
