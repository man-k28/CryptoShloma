BEGIN TRANSACTION;
CREATE TABLE IF NOT EXISTS `markets_cache` (
	`market_id`      	TEXT NOT NULL UNIQUE,
	`min_trend`         REAL NOT NULL,
	`cur_trend_rate`    REAL NOT NULL,
	`sell_mode`         INTEGER,
	`pause_mode`        INTEGER,
	`percentage_profit` REAL,
        `trade_summ`        REAL,
	`is_volume_trading` INTEGER,
        `meta_config` 	    TEXT,
	`exchange`          INTEGER,
	`timestamp`         TEXT NOT NULL,
    PRIMARY KEY(`market_id`, `exchange`)
);

CREATE TABLE IF NOT EXISTS `trade_journal` (
	`id`              INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,
	`market_id`       TEXT NOT NULL,
	`type`            TEXT NOT NULL,
	`top_rate`        REAL,
	`rate`            REAL,
	`stop_loss`       REAL,
	`balance`         REAL,
	`amount`          REAL,
	`total`           REAL,
	`exchange`        INTEGER,
	`timestamp`       TEXT NOT NULL
);
CREATE TABLE IF NOT EXISTS `errors` (
	`id`        INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,
	`type`      TEXT NOT NULL,
	`text`      TEXT NOT NULL,
	`request`   BLOB,
	`result`    BLOB,
	`exchange`  INTEGER,
        `timestamp` TEXT NOT NULL
);
CREATE TABLE IF NOT EXISTS `variables` (
	`key`       TEXT PRIMARY KEY,
	`value`     TEXT
);
COMMIT;
