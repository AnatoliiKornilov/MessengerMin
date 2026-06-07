import { registerUser, loginUser } from '../services/api.js';
import { setToken, setUser, hasSavedSession } from '../services/authService.js';
import { showScreen } from '../ui/router.js';
import { showNotification } from '../ui/notifications.js';
import { loadChats, startChatPolling } from './chatController.js';

export function initAuthHandlers() {
  if (hasSavedSession()) {
    showScreen('chat-screen');

    loadChats();

    startChatPolling();

    return;
  }

  document.getElementById('login-btn').addEventListener('click', async () => {
    const username = document.getElementById('login-username').value.trim();
    const password = document.getElementById('login-password').value;

    if (!username || !password) {
      showNotification('Пожалуйста, заполните все поля', 'error');
      return;
    }

    try {
      const data = await loginUser(username, password);

      setToken(data.access_token);
      setUser({ user_id: data.user_id, username });

      showScreen('chat-screen');

      loadChats();

      startChatPolling();
    } catch (err) {
      console.error('Ошибка входа:', err);
      showNotification('Ошибка входа: ' + err.message, 'error');
    }
  });

  document.getElementById('register-btn').addEventListener('click', async () => {
    const username = document.getElementById('reg-username').value.trim();
    const password = document.getElementById('reg-password').value;

    if (!username || !password) {
      showNotification('Пожалуйста, заполните все поля', 'error');
      return;
    }

    try {
      await registerUser(username, password);

      const data = await loginUser(username, password);

      setToken(data.access_token);
      setUser({ user_id: data.user_id, username });

      showScreen('chat-screen');

      loadChats();

      startChatPolling();

      showNotification('Добро пожаловать', 'success');
    } catch (err) {
      console.error('Ошибка регистрации:', err);
      showNotification('Ошибка регистрации: ' + err.message, 'error');
    }
  });

  document.getElementById('toggle-to-register').addEventListener('click', () => {
    document.getElementById('login-block').classList.add('hidden');
    document.getElementById('register-block').classList.remove('hidden');
  });
  
  document.getElementById('toggle-to-login').addEventListener('click', () => {
    document.getElementById('register-block').classList.add('hidden');
    document.getElementById('login-block').classList.remove('hidden');
  });
}
