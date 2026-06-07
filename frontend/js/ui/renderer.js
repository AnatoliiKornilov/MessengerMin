export function renderChatList(chatList, onSelect) {
  const list = document.getElementById('chat-list');

  list.innerHTML = '';

  chatList.forEach(chat => {
    const item = document.createElement('li');

    item.className = 'chat-item';
    item.dataset.chatId = chat.chat_id;

    const name = chat.name || 'Без названия';
    const lastMsg = chat.last_message || 'Сообщений нет';

    item.innerHTML = `
      <div class="name">${name}</div>
      <div class="last-message">${lastMsg}</div>`;

    item.addEventListener('click', () => onSelect(chat.chat_id));
    list.appendChild(item);
  });
}

export function markActiveChat(chatId) {
  document.querySelectorAll('.chat-item').forEach(el => {
    el.classList.toggle('active', el.dataset.chatId === chatId);
  });
}

export function renderMessages(messages, currentUserId, onEdit, onDelete) {
  const container = document.getElementById('messages');

  container.innerHTML = '';

  messages.forEach(msg => {
    const div = document.createElement('div');
    const isOwn = msg.sender_id === currentUserId;

    div.className = `message ${isOwn ? 'own' : 'other'}`;

    div.innerHTML = `
      <div class="sender">${msg.sender_name}</div>
      <div class="text">${msg.text}</div>
      <div class="time">${new Date(msg.sent_at).toLocaleTimeString()}</div>
      ${isOwn ? `
          <div class="message-actions">
              <button class="edit-msg-btn" data-id="${msg.message_id}" data-text="${msg.text}">Изменить</button>
              <button class="delete-msg-btn" data-id="${msg.message_id}">Удалить</button>
          </div>` : ''}`;

    container.appendChild(div);
  });

  container.scrollTop = container.scrollHeight;

  if (onEdit) {
    document.querySelectorAll('.edit-msg-btn').forEach(btn => {
      btn.addEventListener('click', (e) => {
        const msgId = btn.dataset.id;

        const oldText = btn.dataset.text;

        onEdit(msgId, oldText);
      });
    });
  }

  if (onDelete) {
    document.querySelectorAll('.delete-msg-btn').forEach(btn => {
      btn.addEventListener('click', () => {
        const msgId = btn.dataset.id;
        onDelete(msgId);
      });
    });
  }
}

export function renderChatHeader(chat, onAddMember, onRemoveMember) {
  const headerArea = document.getElementById('chat-header');

  if (headerArea === null) {
    return;
  }

  headerArea.innerHTML = `
    <h2>${chat.name || 'Chat'}</h2>
    ${chat.is_group ? `
        <div class="group-actions">
            <button id="add-member-btn">Добавить участника</button>
            <button id="remove-member-btn">Удалить участника</button>
        </div>` : ''}`;

  if (chat.is_group) {
    document.getElementById('add-member-btn')?.addEventListener('click', onAddMember);
    document.getElementById('remove-member-btn')?.addEventListener('click', onRemoveMember);
  }
}
