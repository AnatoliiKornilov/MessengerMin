import { getToken, setToken, clearAuth } from './authService.js';
import { showNotification } from '../ui/notifications.js';

// const API_BASE = 'http://localhost:8080/api';
const API_BASE = '/api';

let isRefreshing = false;
let refreshPromise = null;

async function refreshAccessToken() {
  if (isRefreshing && refreshPromise) {
    return refreshPromise;
  }

  isRefreshing = true;

  refreshPromise = fetch(API_BASE + '/auth/refresh', {
    method: 'POST',
    credentials: 'include'
  }).then(async (res) => {
    if (!res.ok) {
      throw new Error('refresh failed');
    }

    const data = await res.json();

    setToken(data.access_token);

    return data.access_token;
  }).finally(() => {
    isRefreshing = false;
    refreshPromise = null;
  });

  return refreshPromise;
}

async function request(method, path, body = null, retry = true) {
  const headers = { 'Content-Type': 'application/json' };
  const token = getToken();

  if (token) {
    headers['Authorization'] = `Bearer ${token}`;
  }

  const options = { method, headers };

  if (body) {
    options.body = JSON.stringify(body);
  }

  let response;

  try {
    response = await fetch(API_BASE + path, options);
  } catch (e) {
    console.error(e);

    showNotification('Ошибка сети: невозможно подключиться', 'error');

    throw e;
  }

  if (response.status === 401 && retry) {
    try {
      await refreshAccessToken();
      return request(method, path, body, false);
    } catch (refreshError) {
      clearAuth();

      window.location.reload();

        throw refreshError;
    }
  }

  const data = await response.json();

  if (!response.ok) {
    showNotification(data.error || 'An error occurred', 'error');
    throw new Error(data.error || 'Request failed');
  }

  return data;
}

export const registerUser = (user_name, password) =>
  request('POST', '/auth/register', { user_name, password });

export const loginUser = (user_name, password) =>
  request('POST', '/auth/login', { user_name, password });

export const refreshToken = () =>
  request('POST', '/auth/refresh');

export const createPersonalChat = (user_name) =>
  request('POST', '/chats', { user_name });

export const createGroup = (name) =>
  request('POST', '/chats/group', { name });

export const getChats = () =>
  request('GET', '/chats');

export const addMember = (chatId, user_name) =>
  request('POST', `/chats/${chatId}/members`, { user_name });

export const removeMember = (chatId, userId) =>
  request('DELETE', `/chats/${chatId}/members/${userId}`);

export const sendMessage = (chatId, text) =>
  request('POST', `/chats/${chatId}/messages`, { text });

export const getMessages = (chatId, limit = 50, offset = 0) =>
  request('GET', `/chats/${chatId}/messages?limit=${limit}&offset=${offset}`);

export const editMessage = (msgId, text) =>
  request('PUT', `/messages/${msgId}`, { text });

export const deleteMessage = (msgId) =>
  request('DELETE', `/messages/${msgId}`);

export const findUserByName = (user_name) =>
  request('GET', `/users/by-name?name=${encodeURIComponent(user_name)}`);
