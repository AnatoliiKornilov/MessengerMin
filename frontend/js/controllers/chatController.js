import { getChats, createPersonalChat, createGroup, addMember, removeMember, findUserByName } from '../services/api.js';
import { renderChatList, markActiveChat, renderChatHeader } from '../ui/renderer.js';
import { showNotification } from '../ui/notifications.js';
import { onChatSelected } from './messageController.js';
import { setActiveChatId, getActiveChatId } from '../state.js';
import { clearAuth } from '../services/authService.js';
import { showScreen } from '../ui/router.js';

let pollingIntervalId = null;
let currentChats = [];

export async function loadChats() {
  try {
    currentChats = await getChats();

    renderChatList(currentChats, (chatId) => {
        setActiveChatId(chatId);
        markActiveChat(chatId);

        const chat = currentChats.find(c => c.chat_id === chatId);

        if (chat) {
          renderChatHeader(chat, () => handleAddMember(chatId), () => handleRemoveMember(chatId));
        }
        onChatSelected(chatId);
    });

    const currentId = getActiveChatId();

    if (currentChats.length > 0 && !currentId) {
      const first = currentChats[0];

      setActiveChatId(first.chat_id);
      markActiveChat(first.chat_id);

      renderChatHeader(first, () => handleAddMember(first.chat_id), () => handleRemoveMember(first.chat_id));

      onChatSelected(first.chat_id);
    }

    markActiveChat(getActiveChatId());
  } catch (e) {
    console.error('Ошибка загрузки чатов:', e);
  }
}

export function startChatPolling() {
  stopChatPolling();
  pollingIntervalId = setInterval(loadChats, 2000);
}

export function stopChatPolling() {
  if (pollingIntervalId) {
    clearInterval(pollingIntervalId);
    pollingIntervalId = null;
  }
}

export async function handleNewPersonalChat() {
  const username = prompt('Введите имя:');

  if (username === null) {
    return;
  }

  try {
    await createPersonalChat(username);

    showNotification('Чат создан', 'success');

    loadChats();
  } catch (e) { 
    console.error(e); 
  }
}

export async function handleNewGroup() {
  const name = prompt('Введите название группы:');

  if (name === null) {
    return;
  }

  try {
    await createGroup(name);

    showNotification('Группа создана', 'success');

    loadChats();
  } catch (e) { 
    console.error(e); 
  }
}

async function handleAddMember(chatId) {
  const username = prompt('Введите имя пользователя для добавления:');

  if (username === null) {
    return;
  }

  try {
    await addMember(chatId, username);

    showNotification('Участник добавлен', 'success');

    loadChats();
  } catch (e) { 
    console.error(e); 
  }
}

async function handleRemoveMember(chatId) {
  const username = prompt('Введите имя пользователя для удаления:');

  if (username === null) {
    return;
  }

  try {
    const user = await findUserByName(username);

    await removeMember(chatId, user.user_id);

    showNotification('Участник удалён', 'success');

    loadChats();
  } catch (e) { 
    console.error(e); 
  }
}

function handleLogout() {
  clearAuth();
  stopChatPolling();
  showScreen('auth-screen');
}

export function initChatControls() {
  document.getElementById('new-personal-chat').addEventListener('click', handleNewPersonalChat);
  document.getElementById('new-group').addEventListener('click', handleNewGroup);
  document.getElementById('logout-btn').addEventListener('click', handleLogout);
}
