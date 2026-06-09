import { registerUser, loginUser } from '../services/api.js';
import { setToken, setUser, hasSavedSession } from '../services/authService.js';
import { showScreen } from '../ui/router.js';
import { showNotification } from '../ui/notifications.js';
import { loadChats, startChatPolling } from './chatController.js';

function validateUsername(name) {
  if (!name || name.length < 3 || name.length > 50) {
    return 'Имя должно быть от 3 до 50 символов';
  }

  if (!/^[a-zA-Z0-9_-]+$/.test(name)) {
    return 'Имя может содержать только буквы, цифры, - и _';
  }

  return null;
}

function validatePassword(password) {
  if (!password || password.length < 8 || password.length > 128) {
    return 'Пароль должен быть от 8 до 128 символов';
  }
  return null;
}

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

    const err = validateUsername(username) || validatePassword(password);
    if (err) {
      showNotification(err, 'error');
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

    const err = validateUsername(username) || validatePassword(password);
    if (err) {
      showNotification(err, 'error');
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
