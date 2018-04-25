# ************************************************************
# Sequel Pro SQL dump
# Version 4541
#
# http://www.sequelpro.com/
# https://github.com/sequelpro/sequelpro
#
# Host: 127.0.0.1 (MySQL 5.6.37)
# Database: ultrain_snapshot
# Generation Time: 2017-11-07 16:47:34 +0000
# ************************************************************


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;


# Dump of table buys
# ------------------------------------------------------------

CREATE TABLE `buys` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `tx_hash` varchar(256) NOT NULL DEFAULT '',
  `block_number` int(255) NOT NULL,
  `address` varchar(256) NOT NULL,
  `period` int(11) NOT NULL,
  `eth_amount` decimal(65,0) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `INDEXADDRESS` (`address`),
  KEY `INDEXBN` (`block_number`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;



# Dump of table claims
# ------------------------------------------------------------

CREATE TABLE `claims` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `tx_hash` varchar(256) NOT NULL DEFAULT '',
  `block_number` int(255) NOT NULL,
  `address` varchar(256) NOT NULL DEFAULT '',
  `period` int(11) NOT NULL,
  `ultrain_amount` decimal(65,0) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `INDEXADDRESS` (`address`),
  KEY `INDEXBN` (`block_number`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;



# Dump of table keys
# ------------------------------------------------------------

CREATE TABLE `keys` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `address` varchar(256) NOT NULL DEFAULT '',
  `tx_hash` varchar(265) NOT NULL DEFAULT '',
  `public_key` varchar(256) NOT NULL DEFAULT '',
  `derived_ultrain_key` varchar(256) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  KEY `INDEX_ADDRESS` (`address`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;



# Dump of table reclaimables
# ------------------------------------------------------------

CREATE TABLE `reclaimables` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `tx_hash` varchar(256) NOT NULL DEFAULT '',
  `block_number` varchar(256) NOT NULL,
  `address` varchar(256) NOT NULL DEFAULT '',
  `ultrain_amount` decimal(65,0) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `INDEXADDRESS` (`address`),
  KEY `INDEXBN` (`block_number`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;



# Dump of table registrations
# ------------------------------------------------------------

CREATE TABLE `registrations` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `tx_hash` varchar(256) NOT NULL DEFAULT '',
  `block_number` int(255) NOT NULL,
  `address` varchar(256) NOT NULL,
  `ultrain_key` varchar(256) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `INDEXADDRESS` (`address`),
  KEY `INDEXBN` (`block_number`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;



# Dump of table snapshot
# ------------------------------------------------------------

CREATE TABLE `snapshot` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `user` varchar(256) NOT NULL DEFAULT '',
  `key` varchar(256) NOT NULL DEFAULT '',
  `balance` decimal(15,4) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;



# Dump of table state
# ------------------------------------------------------------

CREATE TABLE `state` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `meta_key` varchar(256) NOT NULL,
  `meta_value` longtext,
  PRIMARY KEY (`id`),
  UNIQUE KEY `UNIQKEY` (`meta_key`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;



# Dump of table transfers
# ------------------------------------------------------------

CREATE TABLE `transfers` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `tx_hash` mediumtext NOT NULL,
  `block_number` int(255) NOT NULL,
  `from` varchar(256) NOT NULL DEFAULT '',
  `to` varchar(256) NOT NULL DEFAULT '',
  `ultrain_amount` decimal(65,0) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `FROMINDEX` (`from`),
  KEY `TOINDEX` (`to`),
  KEY `INDEXBN` (`block_number`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;



# Dump of table wallets
# ------------------------------------------------------------

CREATE TABLE `wallets` (
  `address` varchar(256) NOT NULL,
  `ultrain_key` varchar(256) DEFAULT NULL,
  `balance_wallet` decimal(15,4) NOT NULL DEFAULT '0.0000',
  `balance_unclaimed` decimal(15,4) NOT NULL DEFAULT '0.0000',
  `balance_reclaimed` decimal(15,4) NOT NULL DEFAULT '0.0000',
  `balance_total` decimal(15,4) NOT NULL DEFAULT '0.0000',
  `registered` tinyint(1) NOT NULL DEFAULT '0',
  `fallback` tinyint(1) NOT NULL DEFAULT '0',
  `register_error` varchar(256) DEFAULT NULL,
  `fallback_error` varchar(256) DEFAULT NULL,
  `valid` tinyint(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`address`),
  KEY `ULTRAINKEYINDEX` (`ultrain_key`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;




/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;
/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
