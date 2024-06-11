// Original Norminator
// Revision for @request Chat Holic & Brent Code
// Revision for Multiple Channel Holic & Asheraf
// Revision for Reading Link Item for Latest Rathena Holic & Asheraf
// Added Robust Connection Holic
// Added @reloaddiscord Holic & Brent Code
// Linux g++
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <ctime>
#include <regex>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h> // For close function
#include <pthread.h>
#include <fcntl.h> // For non-blocking mode
#include "discordbot.hpp"

#include "../common/mmo.hpp"
#include "../common/socket.hpp"
#include "../common/strlib.hpp"
#include "../common/nullpo.hpp"
#include "../common/timer.hpp"
#include "../common/malloc.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../map/clif.hpp"
#include "../map/pc.hpp"
#include "../map/channel.hpp"
#include "../common/showmsg.hpp"
#include "../common/socket.hpp"

#define MAX_CONNECT_ATTEMPTS 5 // Maximum number of connection attempts
#define CONNECTION_TIMEOUT 10 // Timeout for connection attempts in seconds

#define DEBUG

struct discord_bot_interface discord_bot_s;
struct discord_bot_interface *discord;

char send_string[DISCORD_MESSAGE_LENGTH];

void do_init_discord() {
    discord_bot_defaults();
    discord->init();
	ShowInfo("Successfully loaded Discord Bot\n");
}

struct Channel *discord_channel_init(char *name, char *alias, char *color) {
	nullpo_retv(name);
	nullpo_retv(color);
	
	struct Channel tmp_chn;

	
	memset(&tmp_chn, 0, sizeof(struct Channel));

	safestrncpy(tmp_chn.name, name + 1, sizeof(tmp_chn.name) - 1);
    safestrncpy(tmp_chn.alias, alias, sizeof(tmp_chn.alias) - 1);
    tmp_chn.pass[0] = '\0';
    tmp_chn.type = CHAN_TYPE_PUBLIC;
    tmp_chn.color = channel_getColor(color);
    tmp_chn.opt = CHAN_OPT_BASE;
    tmp_chn.msg_delay = 10000;

	struct Channel *channel = channel_create(&tmp_chn);
	if (channel == NULL) {
		ShowError("Discord: Channel creation failed for %s!\n", name);
	}
	return channel;
}

void discord_bot_init() {
    if (!(discord->ip = host2ip(discord->ip_name))) {
        ShowError("Unable to resolve %s (discord bridge)!\n", discord->ip_name);
        return;
    }

    discord->channel_discord = discord_channel_init("#main","[main]", "Blue");
	discord->channel_support = discord_channel_init("#support","[support]", "Blue");
	discord->channel_english = discord_channel_init("#english","[english]", "Blue");
	discord->channel_filipino = discord_channel_init("#filipino","[filipino]", "LightGreen");
	discord->channel_malaysian = discord_channel_init("#malaysian","[malaysian]", "LightGreen");
	discord->channel_indonesian = discord_channel_init("#indonesian","[indonesian]", "LightGreen");
	discord->channel_brazil = discord_channel_init("#brazil","[brazil]", "LightGreen");
    discord->fails = 0;
    discord->fd = 0;
    discord->isOn = false;

    add_timer_func_list(discord->connect_timer, "discord_connect_timer");
    discord->connect_timer_id = add_timer(gettick() + 7000, discord->connect_timer, 0, NULL);
}

void *connect_thread(void *arg) {
    // Set socket to non-blocking mode
    int flags = fcntl(discord->fd, F_GETFL, 0);
    fcntl(discord->fd, F_SETFL, flags | O_NONBLOCK);

    int attempt = 0;
    while (attempt < MAX_CONNECT_ATTEMPTS) {
        // Attempt connection
        int sockfd = make_connection(discord->ip, discord->port, O_NONBLOCK, NULL);
        if (sockfd >= 0) {
            // Connection established
            session[sockfd]->func_parse = discord->recv_api;
            session[sockfd]->flag.server = 1;
            discord->fd = sockfd;
            discord->isOn = true;
            sprintf(send_string, "@request Server : Login to Bridge!");
            discord->send_api(send_string, true);
            discord->fails = 0; // Reset connection attempt count
            return NULL;
        } else {
            // Connection attempt failed
            if (errno == EINPROGRESS || errno == EWOULDBLOCK) {
                // Connection in progress, handle accordingly
            } else {
                // Other error occurred
                ShowError("Connection attempt failed. Error code: %d. Retrying...\n", errno);
            }
        }
        attempt++;
        sleep(CONNECTION_TIMEOUT); // Wait before retrying
    }

    ShowError("Maximum connection attempts reached. Unable to establish connection.\n");
    return NULL;
}

int discord_connect_timer(int tid, unsigned int tick, int id, intptr_t data) {
#ifdef DEBUG
    ShowDebug("Calling discord_connect_timer\n");
#endif

    if (discord->isOn) {
        return 0; // Connection already established
    }

    if (++discord->fails > MAX_CONNECT_ATTEMPTS) {
        ShowError("Unable to restart bot, bridge seems down! Tried to reconnect %d times!\n", discord->fails - 1);
        return 0; // Max connection attempts reached
    }

    // Create a thread for the connection attempt
    pthread_t thread;
    if (pthread_create(&thread, NULL, connect_thread, NULL) != 0) {
        ShowError("Failed to create connection thread.\n");
        return 0;
    }

    // Detach the thread to avoid joining it later
    pthread_detach(thread);

    return 0;
}

void discord_bot_final() {
	if (discord->isOn) {
		do_close(discord->fd);
	}
	discord->fd = 0;
	discord->isOn = false;
	discord->fails = 0;
}

void reload_rocordbot(void) {
    if (discord->isOn) {
        ShowError("Connection from Bridge was closed!\n");
        discord_bot_final(); // reset bot and try to reconnect.
        discord->connect_timer_id = add_timer(gettick() + 1000, discord->connect_timer, 0, NULL);
    }
}

int discord_bot_recv_api(int fd) {
	char *parse_string = NULL;

	if (session[discord->fd]->flag.eof) {
		ShowError("Connection from Bridge was closed!\n");
		discord_bot_final(); // reset bot and try to reconnect.
		discord->connect_timer_id = add_timer(gettick() + 1000, discord->connect_timer, 0, NULL);
		return 0;
	}

	if (!RFIFOREST(fd))
		return 0;

	parse_string = (char *)aMalloc(RFIFOREST(fd));
	safestrncpy(parse_string, (const char*)RFIFOP(fd, 0), RFIFOREST(fd));
	RFIFOSKIP(fd, RFIFOREST(fd));
	RFIFOFLUSH(fd);

	discord->send_chn(parse_string);

	aFree(parse_string);
	return 0;
}

inline void discord_bot_send_api(const char *str, bool force) {
	if (discord->isOn != true) {
		return;
	}

	size_t len;
	nullpo_retv(str);

	len = strlen(str);

	if (len > DISCORD_MESSAGE_LENGTH - 1) {
		len = DISCORD_MESSAGE_LENGTH - 1;
	}

	WFIFOHEAD(discord->fd, len);
	snprintf((char *)WFIFOP(discord->fd, 0), DISCORD_MESSAGE_LENGTH, str);
	WFIFOSET(discord->fd, len);
}	

int rocordbot_parse_send_message_toplayer(const char *msg) {
    char receiver_name[NAME_LENGTH], message[CHAT_SIZE_MAX];

    // Find the position of " p12 " which separates the receiver's name from the message
    char *p12_pos = strstr(msg, " p12 ");
    if (!p12_pos) {
        ShowDebug("Message parsing failed: 'p12' delimiter not found. Received: %s", msg);
        return 0;
    }

    // Adjust the pointer to skip leading spaces in the receiver's name
    const char *name_start = msg;
    while (*name_start == ' ') name_start++;  // Skip leading spaces

    // Calculate the length of the receiver's name by using the position of " p12 "
    int name_length = p12_pos - name_start;
    if (name_length >= NAME_LENGTH || name_length < 0) {  // Check for length issues
        ShowDebug("Message parsing failed: Receiver name too long or invalid. Received: %s", msg);
        return 0;
    }

    // Copy the receiver's name from the start of the message to just before " p12 "
    strncpy(receiver_name, name_start, name_length);
    receiver_name[name_length] = '\0'; // Null-terminate the receiver name

    // Copy the message starting just after " p12 "
    strcpy(message, p12_pos + 5); // +5 to skip over " p12 "

    struct map_session_data *dstsd = map_nick2sd(receiver_name, false); // Look up the receiver's session data
    if (!dstsd) {
        ShowDebug("@request Server : Receiver not found or offline ( %s )", receiver_name);
        char error_message[256];
        snprintf(error_message, sizeof(error_message), "@request Server : Receiver not found or offline ( %s )", receiver_name);
        discord_bot_script_hook(error_message);
        return 0;
    }

    // Send the message using clif_wis_message
    clif_wis_message(dstsd, "@request", message, strlen(message) + 1, 99);

    // Log success
    char debug_string[256];
    snprintf(debug_string, sizeof(debug_string), "Message successfully sent from @request to %s: %s", receiver_name, message);
    ShowDebug(debug_string);

    return 1;
}

inline void discord_bot_send_channel(const char *msg) {
    char send_string[256];
    snprintf(send_string, sizeof(send_string), " %s", msg); // Add space to handle prefix matching
    ShowDebug("send_string contents: %s\n", send_string);

    struct ChannelEntry {
        const char *keyword;
        void (*handler)(struct Channel *, const char *, const char *);
        struct Channel *channel;
    };

    auto send_to_channel = [](struct Channel *channel, const char *alias, const char *message) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), " %s%s", alias, message);
        clif_channel_msg(channel, buffer, channel->color);
    };

    ChannelEntry channels[] = {
        {"[ROCORD filipino]", send_to_channel, discord->channel_filipino},
        {"[ROCORD malaysian]", send_to_channel, discord->channel_malaysian},
        {"[ROCORD indonesian]", send_to_channel, discord->channel_indonesian},
        {"[ROCORD english]", send_to_channel, discord->channel_english},
        {"[ROCORD support]", send_to_channel, discord->channel_support},
        {"[ROCORD brazil]", send_to_channel, discord->channel_brazil},
        {"[ROCORD main]", send_to_channel, discord->channel_discord}
    };

    bool message_sent = false;

    // Loop over channels and send message to the first matched channel
    for (int i = 0; i < sizeof(channels) / sizeof(ChannelEntry); i++) {
        if (strstr(send_string, channels[i].keyword)) {
            ShowDebug("Sending message to #%s channel\n", channels[i].channel->alias);

            char *start = strstr(send_string, channels[i].keyword) + strlen(channels[i].keyword);
            while (*start == ' ') start++; // Skip spaces
            channels[i].handler(channels[i].channel, channels[i].channel->alias, start);
            message_sent = true;
            break; // Stop after first match
        }
    }

    // Special handling for STAFF/requestsupport
if (!message_sent && strstr(send_string, "[STAFF/requestsupport] : /reply")) {
    // Correct the length to skip the entire command prefix
    char *message_body = strstr(send_string, "[STAFF/requestsupport] : /reply") + strlen("[STAFF/requestsupport] : /reply");
    
    // Debugging the message body, corrected ShowDebug usage
    ShowDebug("Message body: %s", message_body);

    // Process the message body after the command
    rocordbot_parse_send_message_toplayer(message_body);
    message_sent = true;  // Mark message as handled
}


    // If no keyword is matched, send to the default channel
    if (!message_sent) {
        ShowDebug("Sending message to default channel\n");
        clif_channel_msg(discord->channel_discord, send_string, discord->channel_discord->color);
    }
}

const char base62_dictionary[] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
	'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};

uint32 base62_decode(const std::string &msg) {
	uint32 result = 0;
	for (size_t i = 0; i < msg.length(); i++) {
		for (size_t j = 0; j < 62; j++) {
			if (msg[i] == base62_dictionary[j]) {
				result = result * 62 + j;
				break;
			}
		}
	}
	return result;
}

void discord_bot_recv_channel(struct map_session_data *sd, const char *msg) {
    nullpo_retv(sd);

    // Existing message processing logic
    const std::string start_tag = R"(<ITEML>)";
    const std::string closing_tag = R"(</ITEML>)";
    static std::regex item_regex(start_tag + R"(((\w{5})(\d)(\w+)(%[\d\w]+)?(&[\d\w]+)?(\'[\d\w]+)?[^<]*))" + closing_tag);

    std::smatch match;
    std::string retstr = msg;
    while (std::regex_search(retstr, match, item_regex)) {
        auto itemdb = item_db.find(base62_decode(match[4].str()));
        if (!itemdb) {
            ShowError("Tried to parse itemlink for unknown item %s.\n", match[4].str().c_str());
            return;  // Assuming you want to stop processing if an unknown item is encountered
        }
		auto refine = match[5].str().length() > 1 ? base62_decode(match[5].str().substr(1)) : 0;

		std::string replacestr = "<";
        if (refine > 0)
            replacestr += "+" + std::to_string(refine) + " ";
        replacestr += itemdb->name + ">";

        retstr = std::regex_replace(retstr, item_regex, replacestr,
                                    std::regex_constants::format_first_only);
    }

    // Send the processed message to Discord
    char send_string[150];
    snprintf(send_string, sizeof(send_string), "<%s>: %s", sd->status.name, retstr.c_str());
    discord->send_api(send_string, false);
}


void discord_bot_join_hook(struct Channel *channel, struct map_session_data *sd) {
	if (strcmp(channel->name, "main") != 0 || strcmp(channel->name, "support") != 0) {
		return;
	}
	char output[CHAT_SIZE_MAX];
	snprintf(output, CHAT_SIZE_MAX, "Welcome to %s.\nMessages from discord users are marked with <Username>!", discord->channel_discord->alias);
	clif_displaymessage(sd->fd, output);
}

void discord_bot_script_hook(const char *msg) {
	discord->send_api(msg, false);
}

void discord_bot_hook(struct Channel *channel, struct map_session_data *sd, const char *msg) {

#ifdef DEBUG
    ShowDebug("Discord: Received: %s , with len %d \n", &msg[3], strlen(msg));
#endif
#ifdef WIN32
	discord->recv_chn(sd, &msg[10]); // because all messages from channels start with |00 for an unknown reason.
#else
	discord->recv_chn(sd, msg);
#endif
}

void discord_bot_defaults(void) {
    discord = &discord_bot_s;
    discord->ip_name = "127.0.0.1"; //Replace with your Server IP
    discord->port = 9999;
    discord->isOn = false;
    discord->channel_discord = NULL;
    discord->channel_filipino = NULL;
    discord->channel_malaysian = NULL;
    discord->channel_indonesian = NULL;
    discord->channel_brazil = NULL;
	discord->channel_english = NULL;
	discord->channel_support = NULL;
    discord->init = discord_bot_init;
    discord->final = discord_bot_final;
    discord->recv_api = discord_bot_recv_api;
    discord->send_api = discord_bot_send_api;
    discord->send_chn = discord_bot_send_channel;
    discord->recv_chn = discord_bot_recv_channel;
    discord->connect_timer = discord_connect_timer;
	discord->connect_timer_id = INVALID_TIMER;
}