-- 18/12/19 create database wipedb for fresh install - philn
--          added create and grant user wipe
--          added show_databases

drop database if exists wipedb;
create database wipedb;
use wipedb;

drop user if exists wipe;
create user 'wipe'@'%' identified by 'wipepw';
grant all privileges on wipedb.* to 'wipe'@'%';

-- MySQL dump 10.13  Distrib 5.5.59, for debian-linux-gnu (x86_64)
--
-- Host: localhost    Database: wipedb
-- ------------------------------------------------------
-- Server version	5.5.59-0ubuntu0.14.04.1

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `computer`
--

DROP TABLE IF EXISTS `computer`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `computer` (
  `asset_no` varchar(8) NOT NULL DEFAULT '',
  `service_tag` varchar(32) DEFAULT NULL,
  `is_laptop` tinyint(1) DEFAULT NULL,
  `make` varchar(64) DEFAULT NULL,
  `model` varchar(64) DEFAULT NULL,
  `processor` varchar(64) DEFAULT NULL,
  `synced` tinyint(1) NOT NULL DEFAULT '0',
  `sync_time` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  PRIMARY KEY (`asset_no`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `disk`
--

DROP TABLE IF EXISTS `disk`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `disk` (
  `disk_model` varchar(64) DEFAULT NULL,
  `disk_serial` varchar(64) NOT NULL DEFAULT '',
  `disk_size` varchar(32) DEFAULT NULL,
  `security_erase` tinyint(1) DEFAULT NULL,
  `enhanced_erase` tinyint(1) DEFAULT NULL,
  `health` varchar(32) DEFAULT NULL,
  `rotational` tinyint(1) DEFAULT NULL,
  `source_drive` varchar(64) DEFAULT NULL,
  `parent` varchar(8) DEFAULT NULL,
  `wiped` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `synced` tinyint(1) DEFAULT '0',
  `firmware` varchar(16) DEFAULT NULL,
  `transport` varchar(16) DEFAULT NULL,
  `form_factor` varchar(16) DEFAULT NULL,
  `rpm` int(11) DEFAULT NULL,
  `sync_time` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  PRIMARY KEY (`disk_serial`,`wiped`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;


/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2018-02-24 14:20:34

show databases;
select user from mysql.user;
