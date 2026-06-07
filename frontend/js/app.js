import { showScreen } from './ui/router.js';
import { initAuthHandlers } from './controllers/authController.js';
import { initChatControls } from './controllers/chatController.js';
import { initMessageControls } from './controllers/messageController.js';
import { hasSavedSession } from './services/authService.js';
import { loadChats, startChatPolling } from './controllers/chatController.js';

function applyTheme(theme) {
  if (theme === 'light') {
    document.body.classList.add('light-theme');
  } else {
    document.body.classList.remove('light-theme');
  }
}

function toggleTheme() {
  const isLight = document.body.classList.contains('light-theme');
  const newTheme = isLight ? 'dark' : 'light';

  applyTheme(newTheme);

  localStorage.setItem('theme', newTheme);

  document.getElementById('theme-toggle').textContent = isLight ? '☀' : '☾';
}

document.addEventListener('DOMContentLoaded', () => {
  const savedTheme = localStorage.getItem('theme') || 'dark';

  applyTheme(savedTheme);

  document.getElementById('theme-toggle').textContent = savedTheme === 'light' ? '☾' : '☀';
  document.getElementById('theme-toggle').addEventListener('click', toggleTheme);

  initAuthHandlers();
  initChatControls();
  initMessageControls();

  if (hasSavedSession()) {
    showScreen('chat-screen');
    loadChats();
    startChatPolling();
  } else {
    showScreen('auth-screen');
  }
});
