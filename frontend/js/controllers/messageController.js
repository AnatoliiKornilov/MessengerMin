import { sendMessage, getMessages, editMessage, deleteMessage } from '../services/api.js';
import { getActiveChatId } from '../state.js';
import { renderMessages } from '../ui/renderer.js';
import { getUser } from '../services/authService.js';
import { showNotification } from '../ui/notifications.js';

let messagePollingIntervalId = null;
let allMessages = [];
let isLoadingOlder = false;
let onScrollHandler = null;

const LIMIT = 50;

export async function loadMessages(chatId, appendOlder = false) {
  if (chatId !== getActiveChatId()) {
    return;
  }

  if (appendOlder) {
    if (isLoadingOlder) {
      return;
    }

    isLoadingOlder = true;

    const offset = allMessages.length;

    try {
      const older = await getMessages(chatId, LIMIT, offset);

      if (chatId !== getActiveChatId()) {
        return;
      }

      if (older.length > 0) {
        older.reverse();

        allMessages = older.concat(allMessages);

        const user = getUser();

        renderMessages(allMessages, user?.user_id || null, handleEditMessage, handleDeleteMessage, false);

        const container = document.getElementById('messages');

        if (container) {
          container.scrollTop += container.scrollHeight - (container.scrollHeight - 50);
        }
      }
    } catch (e) {
      console.error(e);
    } finally {
      isLoadingOlder = false;
    }

    return;
  }

  try {
    const latest = await getMessages(chatId, LIMIT, 0);

    if (chatId !== getActiveChatId()) {
      return;
    }

    latest.reverse();

    mergeMessages(latest);

    const user = getUser();

    renderMessages(allMessages, user?.user_id || null, handleEditMessage, handleDeleteMessage, false);
  } catch (e) {
    console.error(e);
  }
}

function mergeMessages(latest) {
  const existingMap = new Map(allMessages.map(m => [m.message_id, m]));

  for (const msg of latest) {
    if (!existingMap.has(msg.message_id)) {
      allMessages.push(msg);
    } else {
      const idx = allMessages.findIndex(m => m.message_id === msg.message_id);

      if (idx !== -1) {
        allMessages[idx] = msg;
      }
    }
  }

  allMessages.sort((a, b) => new Date(a.sent_at) - new Date(b.sent_at));
}

function setupScroll(chatId) {
  const container = document.getElementById('messages');

  if (!container) {
    return;
  }

  if (onScrollHandler) {
    container.removeEventListener('scroll', onScrollHandler);
  }

  onScrollHandler = () => {
    if (container.scrollTop === 0) {
      loadMessages(chatId, true);
    }
  };

  container.addEventListener('scroll', onScrollHandler);
}

export function onChatSelected(chatId) {
  allMessages = [];

  loadMessages(chatId, false);

  setupScroll(chatId);

  startMessagePolling(chatId);
}

function startMessagePolling(chatId) {
  stopMessagePolling();

  if (chatId) {
    messagePollingIntervalId = setInterval(() => loadMessages(chatId, false), 1000);
  }
}

function stopMessagePolling() {
  if (messagePollingIntervalId) {
    clearInterval(messagePollingIntervalId);

    messagePollingIntervalId = null;
  }
}

export function handleSendMessage() {
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

  sendMessage(chatId, text)
    .then(() => {
      input.value = '';
      loadMessages(chatId, false);
    })
    .catch(e => console.error(e));
}

async function handleEditMessage(messageId, oldText) {
  const newText = prompt('Изменить сообщение:', oldText);

  if (!newText || newText === oldText) {
    return;
  }

  try {
    await editMessage(messageId, newText);

    const msg = allMessages.find(m => m.message_id === messageId);

    if (msg) {
      msg.text = newText;
    }

    const user = getUser();

    renderMessages(allMessages, user?.user_id || null, handleEditMessage, handleDeleteMessage, false);
  } catch (e) {
    console.error(e);
  }
}

async function handleDeleteMessage(messageId) {
  if (!confirm('Удалить сообщение?')) {
    return;
  }

  try {
    await deleteMessage(messageId);

    allMessages = allMessages.filter(m => m.message_id !== messageId);

    const user = getUser();

    renderMessages(allMessages, user?.user_id || null, handleEditMessage, handleDeleteMessage, false);
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
