const STORAGE_KEY = 'min_messenger_session';

let currentToken = null;
let currentUser = null;

try {
  const saved = JSON.parse(localStorage.getItem(STORAGE_KEY));

  if (saved) {
    currentToken = saved.token;
    currentUser = saved.user;
  }
} catch (e) {}

function saveToStorage() {
  if (currentToken && currentUser) {
    localStorage.setItem(STORAGE_KEY, JSON.stringify({
      token: currentToken,
      user: currentUser
    }));
  } else {
    localStorage.removeItem(STORAGE_KEY);
  }
}

export function setToken(token) {
  currentToken = token;
  saveToStorage();
}

export function getToken() {
  return currentToken;
}

export function setUser(user) {
  currentUser = user;
  saveToStorage();
}

export function getUser() {
  return currentUser;
}

export function clearAuth() {
  currentToken = null;
  currentUser = null;

  localStorage.removeItem(STORAGE_KEY);
}

export function hasSavedSession() {
  return !!(currentToken && currentUser);
}
