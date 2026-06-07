import { sendMessage, getMessages, editMessage, deleteMessage } from '../services/api.js';
import { getActiveChatId } from '../state.js';
import { renderMessages } from '../ui/renderer.js';
import { getUser } from '../services/authService.js';
import { showNotification } from '../ui/notifications.js';

let messagePollingIntervalId = null;

export async function loadMessages(chatId) {
  try {
    const messages = await getMessages(chatId);

    const user = getUser();

    renderMessages(messages, user?.user_id || null, handleEditMessage, handleDeleteMessage);
  } catch (e) { 
    console.error('Ошибка при загрузке сообщений:', e); 
  }
}

function startMessagePolling(chatId) {
  stopMessagePolling();

  if (chatId) {
    messagePollingIntervalId = setInterval(() => loadMessages(chatId), 1000);
  }
}

function stopMessagePolling() {
  if (messagePollingIntervalId) {
    clearInterval(messagePollingIntervalId);

    messagePollingIntervalId = null;
  }
}

export function onChatSelected(chatId) {
  loadMessages(chatId);
  startMessagePolling(chatId);
}

export async function handleSendMessage() {
  const input = document.getElementById('message-input');
  const text = input.value.trim();

  if (!text) {
    return;
  }

  const chatId = getActiveChatId();

  if (!chatId) {
    showNotification('Чат не выбран', 'error');
    return;
  }

  try {
    await sendMessage(chatId, text);

    input.value = '';

    loadMessages(chatId);
  } catch (e) { 
    console.error(e); 
  }
}

async function handleEditMessage(messageId, oldText) {
  const newText = prompt('Редактировать сообщение:', oldText);

  if (!newText || newText === oldText) {
    return;
  }

  try {
    await editMessage(messageId, newText);

    const chatId = getActiveChatId();

    if (chatId) loadMessages(chatId);
  } catch (e) { 
    console.error(e); 
  }
}

async function handleDeleteMessage(messageId) {
  if (!confirm('Точно удалить это сообщение?')) {
    return;
  }

  try {
    await deleteMessage(messageId);

    const chatId = getActiveChatId();

    if (chatId) {
      loadMessages(chatId);
    }

  } catch (e) { 
    console.error(e); 
  }
}

export function initMessageControls() {
  document.getElementById('send-btn').addEventListener('click', handleSendMessage);

  document.getElementById('message-input').addEventListener('keypress', (e) => {
    if (e.key === 'Enter') {
      handleSendMessage();
    }
  });
}
