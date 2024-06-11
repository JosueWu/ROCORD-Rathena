#!/usr/bin/env node

const Discord = require('discord.js');
const axios = require('axios');
const net = require('net');

const BOT_TOKEN = 'your-bot-token-here'; // Replace with your actual bot token
const PORT = 9999;
const IP = '127.0.0.1'; // Replace with your Server IP


const bot = new Discord.Client();

// Sockets array to manage TCP connections
const sockets = [];
// Input Webhook https://support.discord.com/hc/en-us/articles/228383668-Intro-to-Webhooks
const MainWebhookURL = 'Replace_with_your_Webhook';
const englishWebhookURL = 'Replace_with_your_Webhook';
const filipinoWebhookURL = 'Replace_with_your_Webhook';
const malaysianWebhookURL = 'Replace_with_your_Webhook';
const indonesianWebhookURL = 'Replace_with_your_Webhook';
const brazilWebhookURL = 'Replace_with_your_Webhook';

// https://github.com/ScDarko/ROChargenPHP
const avatarURL = 'https://domain.com/image/index.php/characterhead/{username}';

// Your Server LOGO
const avatarURL2 = 'https://domain.com/themes/th1/images/icon/Ragnarok%20MMORPG-min.png';

async function sendMessage(content, username, avatarURL, webhookURL) {
    try {
      await axios.post(webhookURL, { content, username, avatar_url: avatarURL }, { encoding: 'utf-8' });
      console.log('Message sent!');
    } catch (error) {
      console.error('Failed to send message:', error);
    }
}

// Setup TCP server
const tcpServer = net.createServer(socket => {
    socket.setEncoding("utf8");

    // Error handler for each socket
    socket.on('error', err => console.error(`Error in socket: ${err}`));

    // Close event handler
    socket.on('close', () => {
        console.log(`Connection from ${socket.remoteAddress} closed`);
        const index = sockets.indexOf(socket);
        if (index !== -1) {
            sockets.splice(index, 1);
        }
    });

    // Handle incoming data
    socket.on('data', msg => {
        handleIncomingMessage(socket, msg);
    });

    // Add the socket to the array of sockets
    sockets.push(socket);
});

// Handle incoming messages
function handleIncomingMessage(socket, msg) {
    let messageContent = msg.trim();
    let botName = 'Server Message';
    let avatar;
    let webhookURL = MainWebhookURL;

    const mainPrefix = '>: [[main]]';
    const filipinoPrefix = '>: [[filipino]]';
	const malaysianPrefix = '>: [[malaysian]]';
	const indonesianPrefix = '>: [[indonesian]]';
	const brazilPrefix = '>: [[brazil]]';
	const englishPrefix = '>: [[english]]';
	const supportPrefix = '>: [[support]]';
    const endDelimiter = ':>>:';
	const serverFilter = '[Server]';

    if (messageContent.endsWith(endDelimiter)) {
      messageContent = messageContent.slice(0, -endDelimiter.length).trim();
    }

    if (messageContent.startsWith('<') && messageContent.includes(mainPrefix)) {
      const colonIndex = messageContent.indexOf(mainPrefix);
      botName = messageContent.substring(1, colonIndex).trim();
      const filteredContent = messageContent.substring(colonIndex + mainPrefix.length).trim();
      const username = encodeURIComponent(botName);
      avatar = avatarURL.replace('{username}', username);
      webhookURL = MainWebhookURL;
      sendMessage(filteredContent, botName, avatar, webhookURL);
    } else if (messageContent.startsWith('<') && messageContent.includes(filipinoPrefix)) {
      const colonIndex = messageContent.indexOf(filipinoPrefix);
      botName = messageContent.substring(1, colonIndex).trim();
      const filteredContent = messageContent.substring(colonIndex + filipinoPrefix.length).trim();
      const username = encodeURIComponent(botName);
      avatar = avatarURL.replace('{username}', username);
      webhookURL = filipinoWebhookURL;
      sendMessage(filteredContent, botName, avatar, webhookURL);
    } else if (messageContent.startsWith('<') && messageContent.includes(malaysianPrefix)) {
      const colonIndex = messageContent.indexOf(malaysianPrefix);
      botName = messageContent.substring(1, colonIndex).trim();
      const filteredContent = messageContent.substring(colonIndex + malaysianPrefix.length).trim();
      const username = encodeURIComponent(botName);
      avatar = avatarURL.replace('{username}', username);
      webhookURL = malaysianWebhookURL;
      sendMessage(filteredContent, botName, avatar, webhookURL);
    } else if (messageContent.startsWith('<') && messageContent.includes(indonesianPrefix)) {
      const colonIndex = messageContent.indexOf(indonesianPrefix);
      botName = messageContent.substring(1, colonIndex).trim();
      const filteredContent = messageContent.substring(colonIndex + indonesianPrefix.length).trim();
      const username = encodeURIComponent(botName);
      avatar = avatarURL.replace('{username}', username);
      webhookURL = indonesianWebhookURL;
      sendMessage(filteredContent, botName, avatar, webhookURL);
    } else if (messageContent.startsWith('<') && messageContent.includes(brazilPrefix)) {
      const colonIndex = messageContent.indexOf(brazilPrefix);
      botName = messageContent.substring(1, colonIndex).trim();
      const filteredContent = messageContent.substring(colonIndex + brazilPrefix.length).trim();
      const username = encodeURIComponent(botName);
      avatar = avatarURL.replace('{username}', username);
      webhookURL = brazilWebhookURL;
      sendMessage(filteredContent, botName, avatar, webhookURL);
    } else if (messageContent.startsWith('<') && messageContent.includes(englishPrefix)) {
      const colonIndex = messageContent.indexOf(englishPrefix);
      botName = messageContent.substring(1, colonIndex).trim();
      const filteredContent = messageContent.substring(colonIndex + englishPrefix.length).trim();
      const username = encodeURIComponent(botName);
      avatar = avatarURL.replace('{username}', username);
      webhookURL = englishWebhookURL;
      sendMessage(filteredContent, botName, avatar, webhookURL);
    } else if (messageContent.startsWith('<') && messageContent.includes(supportPrefix)) {
      const colonIndex = messageContent.indexOf(supportPrefix);
      botName = messageContent.substring(1, colonIndex).trim();
      const filteredContent = messageContent.substring(colonIndex + supportPrefix.length).trim();
      const username = encodeURIComponent(botName);
      avatar = avatarURL.replace('{username}', username);
      webhookURL = brazilWebhookURL;
      sendMessage(filteredContent, botName, avatar, webhookURL);
	}
	else if (messageContent.startsWith('@request')) {
		const requestMessage = messageContent.substring('@request'.length).trim();
		// @request Webhook
		const webhookURL = 'Replace_with_your_Webhook';
		sendMessage(requestMessage, botName, avatarURL2, webhookURL);
    } else if (messageContent.startsWith(serverFilter)) {
      // Process messages with [Server] prefix here
        const filteredContent = messageContent.substring(serverFilter.length).trim();
      webhookURL = 'Replace_with_your_Webhook';
      sendMessage(filteredContent, botName, avatarURL2, webhookURL);
    } else {
      sendMessage(messageContent, botName, avatarURL2, webhookURL);
    }

}


// Start the TCP server
tcpServer.listen(PORT, IP, () => {
    console.log(`TCP server listening on port ${PORT} at IP "${IP}"`);
});



bot.on('message', msg => {
	//if (!msg.author.bot){
	console.log(`Received message from ${msg.channel.name} in category: ${msg.channel.parent ? msg.channel.parent.name : 'No category'}`);
	//}
  if (msg.channel.parent && msg.channel.parent.name === 'ROCORD' && !msg.author.bot) {
    let senderName = msg.member ? msg.member.displayName : msg.author.username;
    console.log(`Processing message from ${msg.channel.parent.name} ${senderName}: ${msg.content}`);
    sockets[0].write(`[${msg.channel.parent.name} ${msg.channel.name}] <${senderName}> : ${msg.content}\0`);
  }
});

bot.on('message', msg => {
  // Check if the message starts with the command prefix and is not from a bot
  if (msg.author.bot || !msg.content.startsWith('/')) return;

  // Determine the channel's parent name or use 'No Category' if none exists
  const channelParentName = msg.channel.parent ? msg.channel.parent.name : 'No Category';

  // Check if the message is a 'reply' command and process accordingly
  if (msg.content.startsWith('/reply ')) {
    const messageBody = msg.content.slice('/reply '.length).trim();

    let targetUsername, messageContent;

    // Handle quoted usernames
    if (messageBody.startsWith('"')) {
      const endQuoteIndex = messageBody.indexOf('"', 1);
      if (endQuoteIndex === -1) {
        console.log("Incorrect command format.");
        return;
      }
      targetUsername = messageBody.substring(1, endQuoteIndex);
      messageContent = messageBody.substring(endQuoteIndex + 2);
    } else {
      const firstSpaceIndex = messageBody.indexOf(' ');
      if (firstSpaceIndex === -1) {
        console.log("Incorrect command format.");
        return;
      }
      targetUsername = messageBody.substring(0, firstSpaceIndex);
      messageContent = messageBody.substring(firstSpaceIndex + 1);
    }

    // Include channel parent name and format the message with 'p12'
    const socketMessage = `[${channelParentName}] : /reply ${targetUsername} p12 ${messageContent}`;

    // Log and send the message
    console.log(`Command received: ${msg.content}`);
    console.log(`Formatted socket message: ${socketMessage}`);

    if (sockets.length > 0 && sockets[0].writable) {
      sockets[0].write(`${socketMessage}\0`);
    } else {
      console.log("No active or writable sockets available.");
    }
  } else {
    console.log("Command not recognized or incorrect format.");
  }
});

bot.on('disconnect', event => {
    console.log(`Bot disconnected! Attempting to reconnect...`);
    bot.login(BOT_TOKEN);
});

bot.on('ready', () => {
    console.log(`Bot is ready! Logged in as ${bot.user.username}`);
    setInterval(() => bot.user.setPresence({
        activity: { name: 'domain.com/wiki' }, // Change with your Own Status
        status: 'online'
    }), 300000); // 5 minutes
});


bot.login(BOT_TOKEN);
