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

export function renderMessages(messages, currentUserId, onEdit, onDelete, prepend = false) {
  const container = document.getElementById('messages');
  const wasAtBottom = (container.scrollTop + container.clientHeight + 50) >= container.scrollHeight;

  const fragment = document.createDocumentFragment();

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

    fragment.appendChild(div);
  });

  if (prepend) {
    const prevScrollHeight = container.scrollHeight;

    container.prepend(fragment);

    const newScrollHeight = container.scrollHeight;

    container.scrollTop += newScrollHeight - prevScrollHeight;
  } else {
    container.innerHTML = '';

    container.appendChild(fragment);

    if (wasAtBottom) {
      container.scrollTop = container.scrollHeight;
    }
  }

  if (onEdit) {
    container.querySelectorAll('.edit-msg-btn').forEach(btn => {
      btn.onclick = () => {
        const msgId = btn.dataset.id;
        const oldText = btn.dataset.text;
        onEdit(msgId, oldText);
      };
    });
  }
  if (onDelete) {
    container.querySelectorAll('.delete-msg-btn').forEach(btn => {
      btn.onclick = () => {
        onDelete(btn.dataset.id);
      };
    });
  }
}

export function renderChatHeader(chat, onAddMember, onRemoveMember) {
  const headerArea = document.getElementById('chat-header');

  if (headerArea === null) {
    return;
  }

  const nameEl = headerArea.querySelector('h2');
  const actionsEl = headerArea.querySelector('.group-actions');

  if (nameEl !== null) {
    nameEl.textContent = chat.name || 'Без названия';
  }

  if (actionsEl) {
    if (chat.is_group) {
      actionsEl.innerHTML = `
        <button id="add-member-btn">Добавить участника</button>
        <button id="remove-member-btn">Удалить участника</button>`;

      document.getElementById('add-member-btn')?.addEventListener('click', onAddMember);
      document.getElementById('remove-member-btn')?.addEventListener('click', onRemoveMember);
    } else {
      actionsEl.innerHTML = '';
    }
  }
}
