-- MySQL dump 10.13  Distrib 8.0.20, for Win64 (x86_64)
--
-- Host: localhost    Database: production
-- ------------------------------------------------------
-- Server version	8.0.20

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!50503 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `sysdiagrams`
--

DROP TABLE IF EXISTS `sysdiagrams`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `sysdiagrams` (
  `name` varchar(160) NOT NULL,
  `principal_id` int NOT NULL,
  `diagram_id` int NOT NULL,
  `version` int DEFAULT NULL,
  `definition` longblob,
  PRIMARY KEY (`diagram_id`),
  UNIQUE KEY `UK_principal_name` (`principal_id`,`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sysdiagrams`
--

LOCK TABLES `sysdiagrams` WRITE;
/*!40000 ALTER TABLE `sysdiagrams` DISABLE KEYS */;
/*!40000 ALTER TABLE `sysdiagrams` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `варианты`
--

DROP TABLE IF EXISTS `варианты`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `варианты` (
  `Номер_изделия` int NOT NULL,
  `Номер_варианта` int NOT NULL,
  `Количество_шт` int DEFAULT NULL,
  `Selected` tinyint unsigned DEFAULT NULL,
  PRIMARY KEY (`Номер_изделия`,`Номер_варианта`),
  CONSTRAINT `FK_Варианты_Изделия` FOREIGN KEY (`Номер_изделия`) REFERENCES `изделия` (`Номер_изделия`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `варианты`
--

LOCK TABLES `варианты` WRITE;
/*!40000 ALTER TABLE `варианты` DISABLE KEYS */;
INSERT INTO `варианты` VALUES (1,1,100,NULL),(1,2,150,NULL),(1,3,200,NULL),(1,4,60,NULL),(1,5,130,NULL),(1,7,140,1),(2,1,300,NULL),(2,2,300,NULL),(2,3,300,NULL),(2,4,250,NULL),(2,5,280,NULL),(2,7,300,1),(3,1,200,NULL),(3,2,325,NULL),(3,3,200,NULL),(3,4,490,NULL),(3,5,420,NULL),(3,7,340,1),(4,1,150,NULL),(4,2,75,NULL),(4,3,100,NULL),(4,4,90,NULL),(4,5,140,NULL),(4,7,70,1);
/*!40000 ALTER TABLE `варианты` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `группы рабочих`
--

DROP TABLE IF EXISTS `группы рабочих`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `группы рабочих` (
  `Код группы` int NOT NULL,
  `Наименование` longtext NOT NULL,
  PRIMARY KEY (`Код группы`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `группы рабочих`
--

LOCK TABLES `группы рабочих` WRITE;
/*!40000 ALTER TABLE `группы рабочих` DISABLE KEYS */;
/*!40000 ALTER TABLE `группы рабочих` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `деталь`
--

DROP TABLE IF EXISTS `деталь`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `деталь` (
  `Номер_детали` int NOT NULL AUTO_INCREMENT,
  `Название_детали` varchar(50) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL,
  PRIMARY KEY (`Номер_детали`),
  UNIQUE KEY `XPKДеталь` (`Номер_детали`),
  KEY `XIF1Деталь` (`Номер_детали`),
  CONSTRAINT `R_5` FOREIGN KEY (`Номер_детали`) REFERENCES `наличие_незавершенного_производства` (`Номер_детали`)
) ENGINE=InnoDB AUTO_INCREMENT=9 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `деталь`
--

LOCK TABLES `деталь` WRITE;
/*!40000 ALTER TABLE `деталь` DISABLE KEYS */;
INSERT INTO `деталь` VALUES (1,'Деталь 1'),(2,'Деталь 2'),(3,'Деталь 3'),(4,'Деталь 4'),(5,'Деталь 5'),(6,'Деталь 6'),(7,'Деталь 7'),(8,'Деталь 8');
/*!40000 ALTER TABLE `деталь` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `изделия`
--

DROP TABLE IF EXISTS `изделия`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `изделия` (
  `Номер_изделия` int NOT NULL,
  `Наименование_изделия` varchar(20) NOT NULL,
  `Цена_изделия` int DEFAULT NULL,
  PRIMARY KEY (`Номер_изделия`),
  UNIQUE KEY `XPKИзделия` (`Номер_изделия`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `изделия`
--

LOCK TABLES `изделия` WRITE;
/*!40000 ALTER TABLE `изделия` DISABLE KEYS */;
INSERT INTO `изделия` VALUES (1,'Изделие 1',10),(2,'Изделие 2',30),(3,'Изделие 3',45),(4,'Изделие 4',10);
/*!40000 ALTER TABLE `изделия` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `наличие_незавершенного_производства`
--

DROP TABLE IF EXISTS `наличие_незавершенного_производства`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `наличие_незавершенного_производства` (
  `Номер_детали` int NOT NULL,
  `Детали_неукомплектованные` int DEFAULT NULL,
  `Детали_не_прошедшие_обработку` int DEFAULT NULL,
  `Стоимость единицы незавершенного производства` double DEFAULT NULL,
  PRIMARY KEY (`Номер_детали`),
  UNIQUE KEY `XPKНаличие_незавершенного_производства` (`Номер_детали`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `наличие_незавершенного_производства`
--

LOCK TABLES `наличие_незавершенного_производства` WRITE;
/*!40000 ALTER TABLE `наличие_незавершенного_производства` DISABLE KEYS */;
INSERT INTO `наличие_незавершенного_производства` VALUES (1,59,3,1.67),(2,163,3,2.5),(3,22,3,1.99),(4,34,4,1.87),(5,28,1,1.21),(6,14,3,2.8),(7,27,3,1.48),(8,81,3,4.57);
/*!40000 ALTER TABLE `наличие_незавершенного_производства` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `оборудование_цеха`
--

DROP TABLE IF EXISTS `оборудование_цеха`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `оборудование_цеха` (
  `Номер_группы` int NOT NULL,
  `Наименования_оборудования` varchar(40) NOT NULL,
  `Средний_коэффициент` double NOT NULL,
  `Средний_разряд_работы` int NOT NULL,
  `Время_предварительного_пролеживания` double NOT NULL,
  `Время_обработки_в_другом_цехе` double NOT NULL DEFAULT '0',
  `Длительность_профилактики` double DEFAULT NULL,
  `Интервал_между_профилактиками` double DEFAULT NULL,
  `Число_станков` int DEFAULT NULL,
  PRIMARY KEY (`Номер_группы`),
  UNIQUE KEY `XPKОборудование_цеха` (`Номер_группы`),
  KEY `XIF1Оборудование_цеха` (`Номер_группы`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `оборудование_цеха`
--

LOCK TABLES `оборудование_цеха` WRITE;
/*!40000 ALTER TABLE `оборудование_цеха` DISABLE KEYS */;
INSERT INTO `оборудование_цеха` VALUES (1,'пила',1.15,2,8.2,0,0,0,1),(2,'токарные станки',1.22,3,8.2,0,0,0,NULL),(3,'револьверные станки',1.2,3,8.2,0,1.3,10,NULL),(4,'револьверные станки с вертикальной осью',1.2,3,8.2,0,1.6,5,NULL),(5,'круглошлифовальные наружные станки',1.22,4,8.2,0,0,0,NULL),(6,'вертикально-сверлильные станки',1.25,2,8.2,0,1.1,10,NULL),(7,'радиально-сверлильные станки',1.25,2,8.2,0,0,0,NULL),(8,'горизонтально-фрезерные станки',1.2,3,8.2,0,0,0,NULL),(9,'плоскошлифовальные станки',1.22,4,8.2,0,0,0,NULL),(10,'контрольное',1.2,4,8.2,0,0,0,NULL),(11,'слесарное',1.25,4,8.2,0,0,0,NULL),(12,'цех гальванопокрытий',0,0,0,52.1,0,0,NULL);
/*!40000 ALTER TABLE `оборудование_цеха` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `операции`
--

DROP TABLE IF EXISTS `операции`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `операции` (
  `Номер_группы` int NOT NULL,
  `Штучное_время` double DEFAULT NULL,
  `Подготовительно_заключительное_время` double DEFAULT NULL,
  `Код_операции` int NOT NULL DEFAULT '0',
  `Номер_операции` int NOT NULL,
  `Номер_детали` int DEFAULT NULL,
  `Время_пролеживания` double DEFAULT NULL,
  PRIMARY KEY (`Код_операции`),
  KEY `XIF1Операции` (`Номер_детали`),
  KEY `FK_Операции_Оборудование_цеха` (`Номер_группы`),
  CONSTRAINT `FK_Операции_Деталь` FOREIGN KEY (`Номер_детали`) REFERENCES `деталь` (`Номер_детали`),
  CONSTRAINT `FK_Операции_Оборудование_цеха` FOREIGN KEY (`Номер_группы`) REFERENCES `оборудование_цеха` (`Номер_группы`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `операции`
--

LOCK TABLES `операции` WRITE;
/*!40000 ALTER TABLE `операции` DISABLE KEYS */;
INSERT INTO `операции` VALUES (1,0.05,0.1,1,1,1,8.2),(4,0.19,0.42,2,2,1,8.2),(3,0.09,0.17,3,3,1,8.2),(5,0.07,0.15,4,4,1,8.2),(10,0.03,0.1,5,5,1,8.2),(11,0.02,0,6,6,1,0),(1,0.07,0.1,7,1,2,8.2),(4,0.16,0.35,8,2,2,8.2),(3,0.3,0.5,9,3,2,8.2),(8,0.04,0.25,10,4,2,8.2),(11,0.05,0,11,5,2,8.2),(10,0.03,0.1,12,6,2,8.2),(11,0.02,0,13,7,2,0),(1,0.02,0.1,14,1,3,8.2),(3,0.11,0.25,15,2,3,8.2),(7,0.03,0.25,16,3,3,8.2),(11,0.07,0,17,4,3,8.2),(10,0.03,0.1,18,5,3,0),(12,0,0,19,6,3,0),(1,0.05,0.1,20,1,4,8.2),(4,0.19,0.42,21,2,4,8.2),(3,0.09,0.17,22,3,4,8.2),(5,0.07,0.15,23,4,4,8.2),(10,0.03,0.1,24,5,4,8.2),(11,0.02,0,25,6,4,0),(12,0,0,26,7,4,0),(9,0.03,0.15,27,1,5,8.2),(11,0.04,0,28,2,5,8.2),(10,0.01,0.1,29,3,5,0),(3,1,0.6,30,1,6,8.2),(6,0.05,0.1,31,2,6,8.2),(11,0.01,0,32,3,6,8.2),(10,0.01,0.08,33,4,6,0),(12,0,0,34,5,6,0),(4,0.1,0.1,35,1,7,8.2),(1,0.03,0.1,36,2,7,8.2),(3,0.02,0.17,37,3,7,8.2),(11,0.03,0,38,4,7,8.2),(10,0.01,0.1,39,5,7,0),(12,0,0,40,6,7,0),(7,0.05,0.3,41,1,8,8.2),(2,0.46,0.5,42,2,8,8.2),(7,0.09,0.4,43,3,8,8.2),(6,0.15,0.3,44,4,8,8.2),(7,0.07,0.2,45,5,8,8.2),(11,0.22,0,46,6,8,8.2),(10,0.01,0.09,47,7,8,0),(12,0,0,48,8,8,0),(12,0,0,50,8,2,0);
/*!40000 ALTER TABLE `операции` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `поставщики`
--

DROP TABLE IF EXISTS `поставщики`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `поставщики` (
  `Код поставщика` int NOT NULL,
  `Наименование поставщика` longtext NOT NULL,
  `Адресс постащика` longtext NOT NULL,
  `Код накладной` int NOT NULL,
  `Код счета- фактуры` int NOT NULL,
  `Код акта приема -передачи` int NOT NULL,
  PRIMARY KEY (`Код поставщика`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `поставщики`
--

LOCK TABLES `поставщики` WRITE;
/*!40000 ALTER TABLE `поставщики` DISABLE KEYS */;
/*!40000 ALTER TABLE `поставщики` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `принятый размер партии`
--

DROP TABLE IF EXISTS `принятый размер партии`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `принятый размер партии` (
  `Номер детали` int NOT NULL,
  `Принятый размер партии` int NOT NULL,
  `Интервал запуска` double DEFAULT NULL,
  PRIMARY KEY (`Номер детали`),
  CONSTRAINT `FK_Принятый размер партии_Деталь` FOREIGN KEY (`Номер детали`) REFERENCES `деталь` (`Номер_детали`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `принятый размер партии`
--

LOCK TABLES `принятый размер партии` WRITE;
/*!40000 ALTER TABLE `принятый размер партии` DISABLE KEYS */;
INSERT INTO `принятый размер партии` VALUES (1,104,NULL),(2,97,NULL),(3,134,NULL),(4,104,NULL),(5,156,NULL),(6,36,NULL),(7,123,NULL),(8,85,NULL);
/*!40000 ALTER TABLE `принятый размер партии` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `профессии`
--

DROP TABLE IF EXISTS `профессии`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `профессии` (
  `Код` int NOT NULL,
  `Группа_оборудования` int NOT NULL,
  `Название` varchar(50) NOT NULL,
  PRIMARY KEY (`Код`),
  KEY `FK_Профессии_Оборудование_цеха` (`Группа_оборудования`),
  CONSTRAINT `FK_Профессии_Оборудование_цеха` FOREIGN KEY (`Группа_оборудования`) REFERENCES `оборудование_цеха` (`Номер_группы`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `профессии`
--

LOCK TABLES `профессии` WRITE;
/*!40000 ALTER TABLE `профессии` DISABLE KEYS */;
INSERT INTO `профессии` VALUES (1,1,'Резчик'),(2,2,'Токарь'),(3,3,'Револьверщик'),(4,4,'Револьверщик'),(5,5,'Шлифовщик'),(6,6,'Сверловщик'),(7,7,'Сверловщик'),(8,8,'Фрезеровщик'),(9,9,'Шлифовщик'),(10,10,'Конторолер'),(11,11,'Слесарь');
/*!40000 ALTER TABLE `профессии` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `состав_изделия`
--

DROP TABLE IF EXISTS `состав_изделия`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `состав_изделия` (
  `Номер_изделия` int NOT NULL,
  `Номер_детали` int NOT NULL,
  `Применяемость` int DEFAULT NULL,
  PRIMARY KEY (`Номер_изделия`,`Номер_детали`),
  UNIQUE KEY `XPKСостав_изделия` (`Номер_изделия`,`Номер_детали`),
  KEY `XIF1Состав_изделия` (`Номер_изделия`),
  CONSTRAINT `FK_Состав_изделия_Деталь` FOREIGN KEY (`Номер_изделия`) REFERENCES `деталь` (`Номер_детали`),
  CONSTRAINT `R_7` FOREIGN KEY (`Номер_изделия`) REFERENCES `изделия` (`Номер_изделия`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `состав_изделия`
--

LOCK TABLES `состав_изделия` WRITE;
/*!40000 ALTER TABLE `состав_изделия` DISABLE KEYS */;
INSERT INTO `состав_изделия` VALUES (1,1,6),(1,2,2),(1,3,2),(2,4,6),(2,5,12),(3,6,1),(4,7,2),(4,8,1);
/*!40000 ALTER TABLE `состав_изделия` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `сотрудники`
--

DROP TABLE IF EXISTS `сотрудники`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `сотрудники` (
  `Код сотрудника` int NOT NULL,
  `ФИО сотрудника` longtext NOT NULL,
  `Должность` longtext NOT NULL,
  `Дата выдачи` datetime(6) NOT NULL,
  `Цех` longtext NOT NULL,
  `Код одежды` int NOT NULL,
  `Код группы` int NOT NULL,
  PRIMARY KEY (`Код сотрудника`),
  KEY `FK_Сотрудники_Спецодежда` (`Код одежды`),
  KEY `FK_Сотрудники_Группы рабочих` (`Код группы`),
  CONSTRAINT `FK_Сотрудники_Группы рабочих` FOREIGN KEY (`Код группы`) REFERENCES `группы рабочих` (`Код группы`),
  CONSTRAINT `FK_Сотрудники_Спецодежда` FOREIGN KEY (`Код одежды`) REFERENCES `спецодежда` (`Код одежды`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `сотрудники`
--

LOCK TABLES `сотрудники` WRITE;
/*!40000 ALTER TABLE `сотрудники` DISABLE KEYS */;
/*!40000 ALTER TABLE `сотрудники` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `спецодежда`
--

DROP TABLE IF EXISTS `спецодежда`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `спецодежда` (
  `Код одежды` int NOT NULL,
  `Вид покроя` longtext NOT NULL,
  `Вид комплекта` longtext NOT NULL,
  `Стоимость одежды` decimal(19,4) NOT NULL,
  `Нормативный срок эксплуатации` int NOT NULL,
  `[Код поставщика` int NOT NULL,
  PRIMARY KEY (`Код одежды`),
  KEY `FK_Спецодежда_Поставщики` (`[Код поставщика`),
  CONSTRAINT `FK_Спецодежда_Поставщики` FOREIGN KEY (`[Код поставщика`) REFERENCES `поставщики` (`Код поставщика`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `спецодежда`
--

LOCK TABLES `спецодежда` WRITE;
/*!40000 ALTER TABLE `спецодежда` DISABLE KEYS */;
/*!40000 ALTER TABLE `спецодежда` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `станки`
--

DROP TABLE IF EXISTS `станки`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `станки` (
  `Номер_группы` int NOT NULL,
  `Норма_простоя` double DEFAULT NULL,
  `Идентификатор` int NOT NULL,
  `Название` varchar(30) NOT NULL,
  PRIMARY KEY (`Идентификатор`),
  KEY `FK_Станки_Оборудование_цеха` (`Номер_группы`),
  CONSTRAINT `FK_Станки_Оборудование_цеха` FOREIGN KEY (`Номер_группы`) REFERENCES `оборудование_цеха` (`Номер_группы`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `станки`
--

LOCK TABLES `станки` WRITE;
/*!40000 ALTER TABLE `станки` DISABLE KEYS */;
INSERT INTO `станки` VALUES (3,2.5,1,'Станок 1'),(3,2.75,5,'Станок 3'),(4,7.2,6,'Станок 1'),(6,2.5,7,'Станок 1'),(6,2.5,8,'Станок 3');
/*!40000 ALTER TABLE `станки` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2021-04-29 12:43:04
