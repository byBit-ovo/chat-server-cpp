use IM;

DROP TABLE IF EXISTS `channel`;

CREATE TABLE `channel` (
  `id` BIGINT UNSIGNED NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `channel_id` varchar(64) NOT NULL,
  `creator_id` varchar(64) NOT NULL
) ENGINE=InnoDB;

CREATE UNIQUE INDEX `channel_id_i`
  ON `channel` (`channel_id`);
