const container = document.createElement('div');

container.className = 'notification-container';

document.body.appendChild(container);

export function showNotification(message, type = 'info', duration = 4000) {
  const notification = document.createElement('div');

  notification.className = `notification ${type}`;
  notification.textContent = message;

  container.appendChild(notification);

  setTimeout(() => {
    notification.remove();
  }, duration);
}
