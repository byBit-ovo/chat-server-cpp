use IM;

DROP TABLE IF EXISTS `notify`;

CREATE TABLE `notify` (
  `id` BIGINT UNSIGNED NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `event_id` varchar(64) NOT NULL,
  `user_id` varchar(64) NOT NULL,
  `notify_type` TINYINT UNSIGNED NOT NULL,
  `payload` LONGTEXT NOT NULL,
  `status` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `create_time` TIMESTAMP NULL
) ENGINE=InnoDB;

CREATE UNIQUE INDEX `notify_event_id_i`
  ON `notify` (`event_id`);

CREATE INDEX `notify_user_status_i`
  ON `notify` (`user_id`, `status`, `create_time`);
