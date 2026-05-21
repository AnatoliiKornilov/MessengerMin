CREATE EXTENSION IF NOT EXISTS "pgcrypto";

CREATE TABLE users (
  user_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  user_name TEXT NOT NULL CHECK (char_length(user_name) <= 50),
  password_hash TEXT NOT NULL,
  UNIQUE (user_name)
);

CREATE TABLE chats (
  chat_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  is_group BOOLEAN NOT NULL DEFAULT false,
  chat_name TEXT CHECK (chat_name IS NULL OR char_length(chat_name) <= 100)
);

CREATE TABLE chat_members (
  member_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  chat_id UUID NOT NULL REFERENCES chats(chat_id) ON DELETE CASCADE,
  user_id UUID NOT NULL REFERENCES users(user_id) ON DELETE CASCADE,
  UNIQUE (chat_id, user_id)
);

CREATE TABLE messages (
  message_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  chat_id UUID NOT NULL REFERENCES chats(chat_id) ON DELETE CASCADE,
  sender_id UUID NOT NULL REFERENCES users(user_id) ON DELETE CASCADE,
  text_message TEXT NOT NULL CHECK (char_length(text_message) <= 1000),
  sent_time TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE INDEX idx_chat_members_user ON chat_members(user_id);
CREATE INDEX idx_messages_chat_sent ON messages(chat_id, sent_time DESC);
