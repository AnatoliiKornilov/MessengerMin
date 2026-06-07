import { showNotification } from './notifications.js';

let currentScreen = null;

export function showScreen(name) {
  const screens = document.querySelectorAll('.screen');

  screens.forEach(s => s.classList.add('hidden'));

  const target = document.getElementById(name);

  if (target !== null) {
    target.classList.remove('hidden');

    currentScreen = name;
  } else {
    showNotification(`Экран "${name}" не найден`, 'error');
  }
}

export function getCurrentScreen() {
  return currentScreen;
}
