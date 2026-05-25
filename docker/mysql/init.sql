-- Lemondory Game Server — DB Schema
-- MySQL 8.0
-- docker-compose 최초 실행 시 자동으로 적용됨

CREATE DATABASE IF NOT EXISTS lemondory_game;
USE lemondory_game;

-- ── 플레이어 ──────────────────────────────────────────────────────
CREATE TABLE IF NOT EXISTS players (
    id         INT UNSIGNED  NOT NULL AUTO_INCREMENT,
    name       VARCHAR(64)   NOT NULL,
    level      INT UNSIGNED  NOT NULL DEFAULT 1,
    exp        INT UNSIGNED  NOT NULL DEFAULT 0,
    hp         INT UNSIGNED  NOT NULL DEFAULT 100,
    max_hp     INT UNSIGNED  NOT NULL DEFAULT 100,
    pos_x      FLOAT         NOT NULL DEFAULT 0.0,
    pos_y      FLOAT         NOT NULL DEFAULT 0.0,
    pos_z      FLOAT         NOT NULL DEFAULT 0.0,
    map_id     INT UNSIGNED  NOT NULL DEFAULT 1,
    created_at TIMESTAMP     NOT NULL DEFAULT CURRENT_TIMESTAMP,
    last_login TIMESTAMP     NOT NULL DEFAULT CURRENT_TIMESTAMP
                                       ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (id),
    UNIQUE KEY uk_name (name)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ── 길드 ──────────────────────────────────────────────────────────
CREATE TABLE IF NOT EXISTS guilds (
    id               INT UNSIGNED  NOT NULL AUTO_INCREMENT,
    name             VARCHAR(64)   NOT NULL,
    description      VARCHAR(256)  NOT NULL DEFAULT '',
    master_player_id INT UNSIGNED  NOT NULL,
    max_members      INT UNSIGNED  NOT NULL DEFAULT 30,
    created_at       TIMESTAMP     NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (id),
    UNIQUE KEY uk_guild_name (name),
    CONSTRAINT fk_guild_master
        FOREIGN KEY (master_player_id) REFERENCES players (id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS guild_members (
    guild_id   INT UNSIGNED     NOT NULL,
    player_id  INT UNSIGNED     NOT NULL,
    rank       TINYINT UNSIGNED NOT NULL DEFAULT 0,  -- 0: member, 1: officer, 2: master
    joined_at  TIMESTAMP        NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (guild_id, player_id),
    CONSTRAINT fk_gm_guild  FOREIGN KEY (guild_id)  REFERENCES guilds  (id) ON DELETE CASCADE,
    CONSTRAINT fk_gm_player FOREIGN KEY (player_id) REFERENCES players (id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ── Phase 3: 계정 / 세션 (Auth Server 분리 시 사용) ───────────────
CREATE TABLE IF NOT EXISTS accounts (
    id            INT UNSIGNED  NOT NULL AUTO_INCREMENT,
    username      VARCHAR(64)   NOT NULL,
    password_hash VARCHAR(256)  NOT NULL,
    player_id     INT UNSIGNED      NULL DEFAULT NULL,
    created_at    TIMESTAMP     NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (id),
    UNIQUE KEY uk_account_username (username),
    CONSTRAINT fk_account_player
        FOREIGN KEY (player_id) REFERENCES players (id) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS sessions (
    id         INT UNSIGNED  NOT NULL AUTO_INCREMENT,
    player_id  INT UNSIGNED  NOT NULL,
    token      VARCHAR(512)  NOT NULL,
    expires_at TIMESTAMP     NOT NULL,
    created_at TIMESTAMP     NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (id),
    UNIQUE KEY uk_token (token),
    KEY idx_expires_at (expires_at),
    CONSTRAINT fk_session_player
        FOREIGN KEY (player_id) REFERENCES players (id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
