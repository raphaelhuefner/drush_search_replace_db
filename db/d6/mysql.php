<?php
class Drush_SearchReplaceDb_D6_Mysql implements Drush_SearchReplaceDb {

  /**
   * List all Drupal tables in database
   *
   * @throws Exception
   */
  public function getTables() {
    $table_names = array();
    $tables_result = db_query("SHOW TABLES LIKE '%s%%'", $db_prefix);
    while ($table_name = db_result($tables_result)) {
      $table_names[] = $table_name;
    }
    return $table_names;
  }

  public function getColumns($tableName) {
    $column_names = array();
    $columns_result = db_query("SHOW COLUMNS FROM `%s`", $tableName);
    while ($column = db_fetch_object($columns_result)) {
      $column_names[] = $column->Field;
    }
    return $column_names;
  }

  public function getSearchIterator($tableName, $columnName, $search) {
    $column_values_result = db_query("SELECT `%s` AS column_value FROM `%s` WHERE `%s` LIKE '%%%s%%';", $columnName, $tableName, $columnName, $search);
    return new Drush_SearchReplaceDb_D6_Mysql_Iterator($column_values_result);
  }

  public function update($tableName, $columnName, $original, $new) {
    db_query("UPDATE `%s` SET `%s` = '%s' WHERE `%s` = '%s';", $tableName, $columnName, $new, $columnName, $original);
  }

  public function updateWithTextReplace($tableName, $columnName, $original, $search, $replace) {
    db_query("UPDATE `%s` SET `%s` = REPLACE(`%s`, '%s', '%s') WHERE `%s` = '%s';", $tableName, $columnName, $columnName, $search, $replace, $columnName, $original);
  }
}

class Drush_SearchReplaceDb_D6_Mysql_Iterator implements Drush_SearchReplaceDb_Iterator {

  protected $db_result = null;

  public function __construct($db_result) {
    $this->db_result = $db_result;
  }

  public function next() {
    return db_result($this->db_result);
  }
}
